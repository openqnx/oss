/*
 * oss-software -- add, remove and update the desktop's software (apk).
 *
 * A GTK 3 front end to apk-tools 3: three pages (Browse, Installed,
 * Updates) over one package list, a search entry, and Install / Remove /
 * Update All / Check for Updates. Everything apk does runs as a child
 * process (GSubprocess), never in the window's thread: the lists come from
 * `apk query --format json` (machine-readable, read with json-glib), an
 * operation's progress from `--progress-fd` ("done/total phase" lines), and
 * a failed operation shows apk's own error lines in a dialog. While another
 * apk holds the database lock the operation waits and says so, trying again
 * every few seconds.
 *
 * Copyright (c) 2026 openqnx/oss contributors. MIT licence (LICENSE.id).
 */
#include <gtk/gtk.h>
#include <gio/gunixinputstream.h>
#include <json-glib/json-glib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

enum { C_NAME, C_VERSION, C_INSTALLED, C_NEWVER, C_DESC, C_SIZE, C_IS_INSTALLED, C_HAS_UPDATE, C_IS_DEV, N_COLS };
enum { PAGE_BROWSE, PAGE_INSTALLED, PAGE_UPDATES, N_PAGES };
static const char *page_names[N_PAGES] = { "browse", "installed", "updates" };

typedef struct {
	GtkWidget *win, *stack, *search, *views[N_PAGES], *progress, *status, *spinner;
	GtkWidget *b_install, *b_remove, *b_upgrade, *b_refresh, *dev;
	GtkListStore *store;
	GtkTreeModel *filters[N_PAGES];
	GHashTable *installed, *upgradable;  /* name -> version */
	GPtrArray *available;                /* JsonObject* (name, version, description, installed-size) */
	gboolean busy;
	int pending;                         /* queries still running */
	/* the running operation */
	char **op_argv; char *op_title; guint retry_id; int retries;
	GCancellable *cancel;
	char *keep_sel;                      /* package to select again after a reload */
} App;

static App app;

/* ---------- helpers ---------- */

static const char *apk_path(void)
{
	static char *p = NULL;
	if (!p) p = g_find_program_in_path("apk");
	if (!p) p = g_strdup("/data/oss/bin/apk");
	return p;
}

/* -dev, -doc, -dbg, -static, the port tests: shown only on request */
static gboolean is_dev_name(const char *n)
{
	return g_str_has_suffix(n, "-dev") || g_str_has_suffix(n, "-doc") || g_str_has_suffix(n, "-dbg") ||
	       g_str_has_suffix(n, "-static") || g_str_has_prefix(n, "qnx-test-") || strcmp(n, "qnx-port-tests") == 0;
}

static void set_status(const char *msg)
{
	gtk_label_set_text(GTK_LABEL(app.status), msg ? msg : "");
}

static void set_busy(gboolean busy)
{
	app.busy = busy;
	gtk_widget_set_sensitive(app.b_install, !busy);
	gtk_widget_set_sensitive(app.b_remove, !busy);
	gtk_widget_set_sensitive(app.b_upgrade, !busy);
	gtk_widget_set_sensitive(app.b_refresh, !busy);
	if (busy) gtk_spinner_start(GTK_SPINNER(app.spinner)); else gtk_spinner_stop(GTK_SPINNER(app.spinner));
	gtk_widget_set_visible(app.progress, busy);
}

static void error_dialog(const char *title, const char *detail)
{
	GtkWidget *d = gtk_message_dialog_new(GTK_WINDOW(app.win), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                      GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", title);
	if (detail && *detail)
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(d), "%s", detail);
	gtk_window_set_title(GTK_WINDOW(d), "Software");
	g_signal_connect(d, "response", G_CALLBACK(gtk_widget_destroy), NULL);
	gtk_widget_show(d);
}

/* apk's ERROR lines (and what follows them), or the last lines of its output */
static char *error_lines(const char *out)
{
	GString *s = g_string_new(NULL);
	char **lines = g_strsplit(out ? out : "", "\n", -1);
	gboolean in_err = FALSE;
	for (int i = 0; lines[i]; i++) {
		if (g_str_has_prefix(lines[i], "ERROR")) in_err = TRUE;
		if (in_err && *lines[i]) g_string_append_printf(s, "%s\n", lines[i]);
	}
	if (s->len == 0) {
		int n = g_strv_length(lines), from = n > 8 ? n - 8 : 0;
		for (int i = from; i < n; i++) if (*lines[i]) g_string_append_printf(s, "%s\n", lines[i]);
	}
	g_strfreev(lines);
	return g_string_free(s, FALSE);
}

static gboolean is_lock_error(const char *out)
{
	return out && (strstr(out, "lock") || strstr(out, "Lock")) && strstr(out, "database");
}

/* ---------- the lists: apk query --format json ---------- */

static void load_lists(void);
static char *selected_name(void);

/* select @name again in every page's list (after the store was refilled) */
static void reselect(const char *name)
{
	for (int i = 0; i < N_PAGES; i++) {
		GtkTreeIter it;
		gboolean ok = gtk_tree_model_get_iter_first(app.filters[i], &it);
		while (ok) {
			char *n = NULL;
			gtk_tree_model_get(app.filters[i], &it, C_NAME, &n, -1);
			gboolean hit = g_strcmp0(n, name) == 0;
			g_free(n);
			if (hit) {
				GtkTreePath *p = gtk_tree_model_get_path(app.filters[i], &it);
				gtk_tree_view_set_cursor(GTK_TREE_VIEW(app.views[i]), p, NULL, FALSE);
				gtk_tree_path_free(p);
				break;
			}
			ok = gtk_tree_model_iter_next(app.filters[i], &it);
		}
	}
}

static GPtrArray *parse_query(const char *json, GError **err)
{
	JsonParser *p = json_parser_new();
	GPtrArray *a = g_ptr_array_new_with_free_func((GDestroyNotify)json_object_unref);
	if (!json_parser_load_from_data(p, json, -1, err)) { g_object_unref(p); g_ptr_array_unref(a); return NULL; }
	JsonNode *root = json_parser_get_root(p);
	if (root && JSON_NODE_HOLDS_ARRAY(root)) {
		JsonArray *arr = json_node_get_array(root);
		for (guint i = 0; i < json_array_get_length(arr); i++) {
			JsonObject *o = json_array_get_object_element(arr, i);
			if (o && json_object_has_member(o, "name")) g_ptr_array_add(a, json_object_ref(o));
		}
	}
	g_object_unref(p);
	return a;
}

static const char *obj_str(JsonObject *o, const char *m)
{
	return json_object_has_member(o, m) ? json_object_get_string_member(o, m) : "";
}

static void fill_store(void)
{
	gtk_list_store_clear(app.store);
	if (!app.available) return;
	/* installed packages that no repository has any more are listed too */
	GHashTable *seen = g_hash_table_new(g_str_hash, g_str_equal);
	for (guint i = 0; i < app.available->len; i++) {
		JsonObject *o = g_ptr_array_index(app.available, i);
		const char *name = obj_str(o, "name");
		if (g_hash_table_contains(seen, name)) continue;
		g_hash_table_add(seen, (gpointer)name);
		const char *inst = g_hash_table_lookup(app.installed, name);
		const char *newv = g_hash_table_lookup(app.upgradable, name);
		gint64 size = json_object_has_member(o, "installed-size") ? json_object_get_int_member(o, "installed-size") : 0;
		char *sz = g_format_size((guint64)size);
		gtk_list_store_insert_with_values(app.store, NULL, -1,
			C_NAME, name, C_VERSION, obj_str(o, "version"), C_INSTALLED, inst ? inst : "",
			C_NEWVER, newv ? newv : "", C_DESC, obj_str(o, "description"), C_SIZE, sz,
			C_IS_INSTALLED, inst != NULL, C_HAS_UPDATE, newv != NULL, C_IS_DEV, is_dev_name(name), -1);
		g_free(sz);
	}
	g_hash_table_unref(seen);
	for (int i = 0; i < N_PAGES; i++) gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(app.filters[i]));
	if (app.keep_sel) { reselect(app.keep_sel); g_clear_pointer(&app.keep_sel, g_free); }
	char *m = g_strdup_printf("%u packages available, %u installed, %u updates",
	                          app.available->len, g_hash_table_size(app.installed), g_hash_table_size(app.upgradable));
	set_status(m);
	g_free(m);
}

typedef struct { int which; } QueryCtx;  /* 0 available, 1 installed, 2 upgradable */

static void query_done(GObject *src, GAsyncResult *res, gpointer data)
{
	QueryCtx *q = data;
	char *out = NULL, *err = NULL;
	GError *e = NULL;
	if (!g_subprocess_communicate_utf8_finish(G_SUBPROCESS(src), res, &out, &err, &e)) {
		set_status(e->message);
		g_clear_error(&e);
	} else if (!g_subprocess_get_successful(G_SUBPROCESS(src))) {
		char *d = error_lines(err && *err ? err : out);
		set_status("apk query failed");
		if (q->which == 0) error_dialog("The package list could not be read", d);
		g_free(d);
	} else {
		GPtrArray *a = parse_query(out, &e);
		if (!a) { set_status(e->message); g_clear_error(&e); }
		else if (q->which == 0) { if (app.available) g_ptr_array_unref(app.available); app.available = a; a = NULL; }
		else {
			GHashTable *h = q->which == 1 ? app.installed : app.upgradable;
			g_hash_table_remove_all(h);
			for (guint i = 0; i < a->len; i++) {
				JsonObject *o = g_ptr_array_index(a, i);
				g_hash_table_insert(h, g_strdup(obj_str(o, "name")), g_strdup(obj_str(o, "version")));
			}
		}
		if (a) g_ptr_array_unref(a);
	}
	g_free(out); g_free(err); g_free(q);
	if (--app.pending == 0) { fill_store(); set_busy(FALSE); }
}

static void run_query(int which)
{
	const char *sel = which == 0 ? "--available" : which == 1 ? "--installed" : "--upgradable";
	GError *e = NULL;
	GSubprocess *p = g_subprocess_new(G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_PIPE, &e,
		apk_path(), "query", sel, "--format", "json", "--fields", "name,version,description,installed-size", "*", NULL);
	if (!p) { set_status(e->message); g_clear_error(&e); if (--app.pending == 0) set_busy(FALSE); return; }
	QueryCtx *q = g_new0(QueryCtx, 1);
	q->which = which;
	g_subprocess_communicate_utf8_async(p, NULL, NULL, query_done, q);
	g_object_unref(p);
}

static void load_lists(void)
{
	g_free(app.keep_sel);
	app.keep_sel = app.win ? selected_name() : NULL;
	set_busy(TRUE);
	gtk_widget_set_visible(app.progress, FALSE);
	set_status("Reading the package lists...");
	app.pending = 3;
	run_query(0); run_query(1); run_query(2);
}

/* ---------- operations: apk add / del / upgrade / update ---------- */

static void start_operation(void);

typedef struct { GDataInputStream *in; } Prog;

static void progress_line(GObject *src, GAsyncResult *res, gpointer data)
{
	Prog *pr = data;
	gsize len;
	char *line = g_data_input_stream_read_line_finish_utf8(pr->in, res, &len, NULL);
	if (!line) { g_object_unref(pr->in); g_free(pr); return; }
	/* "done/total phase" */
	guint64 done = 0, total = 0;
	char phase[32] = "";
	if (sscanf(line, "%" G_GUINT64_FORMAT "/%" G_GUINT64_FORMAT " %31s", &done, &total, phase) >= 2 && total > 0) {
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app.progress), (double)done / (double)total);
		char *t = g_strdup_printf("%s: %s %d %%", app.op_title, phase, (int)(100.0 * done / total));
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(app.progress), t);
		g_free(t);
	}
	g_free(line);
	g_data_input_stream_read_line_async(pr->in, G_PRIORITY_DEFAULT, NULL, progress_line, pr);
}

static gboolean retry_cb(gpointer d)
{
	app.retry_id = 0;
	start_operation();
	return G_SOURCE_REMOVE;
}

static void op_done(GObject *src, GAsyncResult *res, gpointer data)
{
	char *out = NULL;
	GError *e = NULL;
	gboolean ok = g_subprocess_communicate_utf8_finish(G_SUBPROCESS(src), res, &out, NULL, &e);
	if (!ok) {
		set_busy(FALSE);
		error_dialog(app.op_title, e->message);
		g_clear_error(&e);
	} else if (!g_subprocess_get_successful(G_SUBPROCESS(src))) {
		if (is_lock_error(out) && app.retries < 200) {
			/* another apk is running: wait for it */
			app.retries++;
			set_status("Another package operation is running - waiting for it to finish...");
			app.retry_id = g_timeout_add_seconds(3, retry_cb, NULL);
			g_free(out);
			return;
		}
		set_busy(FALSE);
		char *d = error_lines(out);
		char *t = g_strdup_printf("%s failed", app.op_title);
		error_dialog(t, d);
		g_free(t); g_free(d);
		set_status("");
	} else {
		char *m = g_strdup_printf("%s: done", app.op_title);
		set_status(m);
		g_free(m);
		g_free(out);
		load_lists();          /* clears busy when the lists are read */
		return;
	}
	g_free(out);
}

static void start_operation(void)
{
	int fds[2];
	GError *e = NULL;
	if (pipe(fds) != 0) { error_dialog(app.op_title, g_strerror(errno)); set_busy(FALSE); return; }
	GSubprocessLauncher *l = g_subprocess_launcher_new(G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_MERGE);
	g_subprocess_launcher_take_fd(l, fds[1], 3);
	GSubprocess *p = g_subprocess_launcher_spawnv(l, (const char * const *)app.op_argv, &e);
	g_object_unref(l);       /* closes our copy of the write end: EOF when apk ends */
	if (!p) {
		close(fds[0]);
		set_busy(FALSE);
		error_dialog(app.op_title, e->message);
		g_clear_error(&e);
		return;
	}
	Prog *pr = g_new0(Prog, 1);
	pr->in = g_data_input_stream_new(g_unix_input_stream_new(fds[0], TRUE));
	g_data_input_stream_read_line_async(pr->in, G_PRIORITY_DEFAULT, NULL, progress_line, pr);
	if (app.retries == 0) {
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app.progress), 0.0);
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(app.progress), app.op_title);
		set_status(app.op_title);
	}
	g_subprocess_communicate_utf8_async(p, NULL, NULL, op_done, NULL);
	g_object_unref(p);
}

/* apk <args...> --progress-fd 3 */
static void run_apk(const char *title, const char *const *args)
{
	if (app.busy) return;
	GPtrArray *v = g_ptr_array_new();
	g_ptr_array_add(v, g_strdup(apk_path()));
	for (int i = 0; args[i]; i++) g_ptr_array_add(v, g_strdup(args[i]));
	g_ptr_array_add(v, g_strdup("--progress-fd"));
	g_ptr_array_add(v, g_strdup("3"));
	g_ptr_array_add(v, NULL);
	g_strfreev(app.op_argv);
	app.op_argv = (char **)g_ptr_array_free(v, FALSE);
	g_free(app.op_title);
	app.op_title = g_strdup(title);
	app.retries = 0;
	set_busy(TRUE);
	start_operation();
}

static char *selected_name(void)
{
	const char *page = gtk_stack_get_visible_child_name(GTK_STACK(app.stack));
	int i;
	for (i = 0; i < N_PAGES; i++) if (g_strcmp0(page, page_names[i]) == 0) break;
	if (i == N_PAGES) return NULL;
	GtkTreeModel *m; GtkTreeIter it; char *name = NULL;
	GtkTreeSelection *sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(app.views[i]));
	if (gtk_tree_selection_get_selected(sel, &m, &it)) gtk_tree_model_get(m, &it, C_NAME, &name, -1);
	return name;
}

static void on_install(GtkButton *b, gpointer d)
{
	char *n = selected_name();
	if (!n) { set_status("Select a package first"); return; }
	char *t = g_strdup_printf("Installing %s", n);
	const char *args[] = { "add", n, NULL };
	run_apk(t, args);
	g_free(t); g_free(n);
}

static void on_remove(GtkButton *b, gpointer d)
{
	char *n = selected_name();
	if (!n) { set_status("Select a package first"); return; }
	char *t = g_strdup_printf("Removing %s", n);
	const char *args[] = { "del", n, NULL };
	run_apk(t, args);
	g_free(t); g_free(n);
}

static void on_upgrade(GtkButton *b, gpointer d)
{
	const char *args[] = { "upgrade", NULL };
	run_apk("Updating all packages", args);
}

static void on_refresh(GtkButton *b, gpointer d)
{
	const char *args[] = { "update", NULL };
	run_apk("Checking for updates", args);
}

/* ---------- the window ---------- */

static gboolean visible_row(GtkTreeModel *m, GtkTreeIter *it, gpointer data)
{
	int page = GPOINTER_TO_INT(data);
	char *name = NULL, *desc = NULL;
	gboolean inst, upd, dev, show = TRUE;
	gtk_tree_model_get(m, it, C_NAME, &name, C_DESC, &desc, C_IS_INSTALLED, &inst, C_HAS_UPDATE, &upd, C_IS_DEV, &dev, -1);
	if (!name) return FALSE;
	if (page == PAGE_INSTALLED && !inst) show = FALSE;
	if (page == PAGE_UPDATES && !upd) show = FALSE;
	if (dev && !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(app.dev)) && page != PAGE_UPDATES) show = FALSE;
	const char *q = gtk_entry_get_text(GTK_ENTRY(app.search));
	if (show && q && *q) {
		char *lq = g_utf8_strdown(q, -1), *ln = g_utf8_strdown(name, -1), *ld = g_utf8_strdown(desc ? desc : "", -1);
		show = strstr(ln, lq) || strstr(ld, lq);
		g_free(lq); g_free(ln); g_free(ld);
	}
	g_free(name); g_free(desc);
	return show;
}

static void refilter(void)
{
	for (int i = 0; i < N_PAGES; i++) gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(app.filters[i]));
}

static void on_search(GtkSearchEntry *e, gpointer d) { refilter(); }
static void on_dev(GtkToggleButton *b, gpointer d) { refilter(); }

/* Enter in the search entry: to the list of the visible page, first row selected */
static void on_search_activate(GtkEntry *e, gpointer d)
{
	const char *page = gtk_stack_get_visible_child_name(GTK_STACK(app.stack));
	for (int i = 0; i < N_PAGES; i++) {
		if (g_strcmp0(page, page_names[i]) != 0) continue;
		GtkTreePath *p = gtk_tree_path_new_first();
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(app.views[i]), p, NULL, FALSE);
		gtk_tree_path_free(p);
		gtk_widget_grab_focus(app.views[i]);
	}
}

static GtkWidget *make_view(int page)
{
	app.filters[page] = gtk_tree_model_filter_new(GTK_TREE_MODEL(app.store), NULL);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(app.filters[page]), visible_row, GINT_TO_POINTER(page), NULL);
	GtkWidget *v = gtk_tree_view_new_with_model(app.filters[page]);
	app.views[page] = v;
	GtkCellRenderer *r = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *c;
	c = gtk_tree_view_column_new_with_attributes("Package", r, "text", C_NAME, NULL);
	gtk_tree_view_column_set_sort_column_id(c, C_NAME); gtk_tree_view_column_set_resizable(c, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(v), c);
	if (page == PAGE_UPDATES) {
		gtk_tree_view_append_column(GTK_TREE_VIEW(v), gtk_tree_view_column_new_with_attributes("Installed", r, "text", C_INSTALLED, NULL));
		gtk_tree_view_append_column(GTK_TREE_VIEW(v), gtk_tree_view_column_new_with_attributes("New version", r, "text", C_NEWVER, NULL));
	} else {
		gtk_tree_view_append_column(GTK_TREE_VIEW(v), gtk_tree_view_column_new_with_attributes("Version", r, "text", C_VERSION, NULL));
		gtk_tree_view_append_column(GTK_TREE_VIEW(v), gtk_tree_view_column_new_with_attributes("Size", r, "text", C_SIZE, NULL));
	}
	if (page == PAGE_BROWSE) {
		GtkCellRenderer *t = gtk_cell_renderer_toggle_new();
		gtk_tree_view_append_column(GTK_TREE_VIEW(v), gtk_tree_view_column_new_with_attributes("Installed", t, "active", C_IS_INSTALLED, NULL));
	}
	GtkCellRenderer *rd = gtk_cell_renderer_text_new();
	g_object_set(rd, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	c = gtk_tree_view_column_new_with_attributes("Description", rd, "text", C_DESC, NULL);
	gtk_tree_view_column_set_expand(c, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(v), c);
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(v), C_NAME);
	GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(sw), v);
	return sw;
}

static void activate(GtkApplication *ga, gpointer d)
{
	if (app.win) { gtk_window_present(GTK_WINDOW(app.win)); return; }
	app.installed = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	app.upgradable = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	app.store = gtk_list_store_new(N_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
	                               G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(app.store), C_NAME, GTK_SORT_ASCENDING);

	app.win = gtk_application_window_new(ga);
	gtk_window_set_title(GTK_WINDOW(app.win), "Software");
	gtk_window_set_icon_name(GTK_WINDOW(app.win), "system-software-install");
	gtk_window_set_default_size(GTK_WINDOW(app.win), 860, 560);

	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
	gtk_container_set_border_width(GTK_CONTAINER(box), 8);
	gtk_container_add(GTK_CONTAINER(app.win), box);

	GtkWidget *top = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	app.stack = gtk_stack_new();
	GtkWidget *sw = gtk_stack_switcher_new();
	gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(sw), GTK_STACK(app.stack));
	gtk_box_pack_start(GTK_BOX(top), sw, FALSE, FALSE, 0);
	app.search = gtk_search_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(app.search), "Search packages");
	g_signal_connect(app.search, "search-changed", G_CALLBACK(on_search), NULL);
	g_signal_connect(app.search, "activate", G_CALLBACK(on_search_activate), NULL);
	gtk_box_pack_start(GTK_BOX(top), app.search, TRUE, TRUE, 0);
	app.dev = gtk_check_button_new_with_label("Development packages");
	g_signal_connect(app.dev, "toggled", G_CALLBACK(on_dev), NULL);
	gtk_box_pack_start(GTK_BOX(top), app.dev, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), top, FALSE, FALSE, 0);

	gtk_stack_add_titled(GTK_STACK(app.stack), make_view(PAGE_BROWSE), page_names[PAGE_BROWSE], "Browse");
	gtk_stack_add_titled(GTK_STACK(app.stack), make_view(PAGE_INSTALLED), page_names[PAGE_INSTALLED], "Installed");
	gtk_stack_add_titled(GTK_STACK(app.stack), make_view(PAGE_UPDATES), page_names[PAGE_UPDATES], "Updates");
	gtk_box_pack_start(GTK_BOX(box), app.stack, TRUE, TRUE, 0);

	app.progress = gtk_progress_bar_new();
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(app.progress), TRUE);
	gtk_box_pack_start(GTK_BOX(box), app.progress, FALSE, FALSE, 0);

	GtkWidget *bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	app.spinner = gtk_spinner_new();
	gtk_box_pack_start(GTK_BOX(bottom), app.spinner, FALSE, FALSE, 0);
	app.status = gtk_label_new("");
	gtk_label_set_xalign(GTK_LABEL(app.status), 0.0);
	gtk_label_set_ellipsize(GTK_LABEL(app.status), PANGO_ELLIPSIZE_END);
	gtk_box_pack_start(GTK_BOX(bottom), app.status, TRUE, TRUE, 0);
	app.b_refresh = gtk_button_new_with_mnemonic("_Check for Updates");
	app.b_upgrade = gtk_button_new_with_mnemonic("Update _All");
	app.b_remove = gtk_button_new_with_mnemonic("_Remove");
	app.b_install = gtk_button_new_with_mnemonic("_Install");
	g_signal_connect(app.b_refresh, "clicked", G_CALLBACK(on_refresh), NULL);
	g_signal_connect(app.b_upgrade, "clicked", G_CALLBACK(on_upgrade), NULL);
	g_signal_connect(app.b_remove, "clicked", G_CALLBACK(on_remove), NULL);
	g_signal_connect(app.b_install, "clicked", G_CALLBACK(on_install), NULL);
	gtk_box_pack_end(GTK_BOX(bottom), app.b_install, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(bottom), app.b_remove, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(bottom), app.b_upgrade, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(bottom), app.b_refresh, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), bottom, FALSE, FALSE, 0);

	gtk_widget_show_all(app.win);
	gtk_widget_set_visible(app.progress, FALSE);
	gtk_widget_grab_focus(app.search);   /* typing searches at once */
	load_lists();
}

int main(int argc, char **argv)
{
	/* GTK 3 gives Wayland the program name as app_id: make it the desktop
	 * entry's name, so that the panel shows this window with its name and icon */
	g_set_prgname("org.openqnx.Software");
	GtkApplication *ga = gtk_application_new("org.openqnx.Software", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(ga, "activate", G_CALLBACK(activate), NULL);
	int r = g_application_run(G_APPLICATION(ga), argc, argv);
	g_object_unref(ga);
	return r;
}
