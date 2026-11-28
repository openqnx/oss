/* oss-greeter -- the log-in screen of the QNX desktop (oss-desktop).
 *
 * A full-screen GTK 3 window on the layer-shell overlay of a labwc that runs
 * nothing else (qnx-desktop-start starts it as root, with DESKTOP_GREETER=yes
 * in qnx-desktop.conf). It lists the users (root and the ordinary accounts of
 * /etc/passwd), asks for a password and checks it through PAM (policy
 * "oss-greeter", OpenPAM: /usr/local/etc/pam.d/oss-greeter). It starts no
 * session itself: it writes its answer to a file only root can read and
 * exits, and qnx-desktop-start acts on it --
 *     login <user>      start that user's session
 *     reboot | poweroff  restart or shut down the system
 * A wrong password says so and asks again; nothing else gets past it.
 *
 * Options (for tests): --service NAME (the PAM policy), --result FILE,
 * --last-user FILE.
 */
#include <errno.h>
#include <fcntl.h>
#include <libintl.h>
#include <locale.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gtk-layer-shell/gtk-layer-shell.h>
#include <security/pam_appl.h>

#ifndef PREFIX
#define PREFIX "/usr/local"
#endif
#define _(s) gettext(s)

static const char *service = "oss-greeter";
static const char *result_path = "/data/oss/var/run/oss-greeter/result";
static const char *last_user_path = "/data/oss/var/lib/oss-greeter/last-user";

static GtkWidget *win, *clock_lbl, *date_lbl, *users, *pass, *err_lbl, *login_btn;

/* ---- files only root reads ---- */
static int mkdirs(const char *dir, mode_t mode) {
  char buf[512];
  if (strlen(dir) >= sizeof buf) return -1;
  strcpy(buf, dir);
  for (char *p = buf + 1; *p; p++)
    if (*p == '/') { *p = 0; mkdir(buf, 0755); *p = '/'; }
  if (mkdir(buf, mode) != 0 && errno != EEXIST) return -1;
  return chmod(buf, mode);
}
/* write atomically: a temporary file in the same directory, then rename */
static int write_private(const char *path, const char *text, mode_t dirmode) {
  gchar *dir = g_path_get_dirname(path);
  int ok = mkdirs(dir, dirmode) == 0;
  g_free(dir);
  if (!ok) return -1;
  gchar *tmp = g_strdup_printf("%s.tmp", path);
  int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (fd < 0) { g_free(tmp); return -1; }
  size_t n = strlen(text);
  ok = write(fd, text, n) == (ssize_t)n;
  ok = (fsync(fd) == 0) && ok;
  close(fd);
  if (ok) ok = rename(tmp, path) == 0;
  if (!ok) unlink(tmp);
  g_free(tmp);
  return ok ? 0 : -1;
}
static void finish(const char *answer) {
  if (write_private(result_path, answer, 0700) != 0) {
    gtk_label_set_text(GTK_LABEL(err_lbl), _("Cannot save the result; see the log"));
    g_printerr("oss-greeter: cannot write %s: %s\n", result_path, strerror(errno));
    return;
  }
  gtk_main_quit();
}

/* ---- users ---- */
static int listed(const struct passwd *pw) {
  if (pw->pw_uid == 0) return 1;
  if (pw->pw_uid < 1000 || pw->pw_uid >= 60000 || !strcmp(pw->pw_name, "nobody")) return 0;
  const char *sh = pw->pw_shell ? pw->pw_shell : "";
  return !(strstr(sh, "false") || strstr(sh, "nologin"));
}
static gchar *read_last_user(void) {
  gchar *s = NULL;
  if (g_file_get_contents(last_user_path, &s, NULL, NULL)) g_strstrip(s);
  return s;
}
static void fill_users(void) {
  gchar *last = read_last_user();
  GtkListBoxRow *sel = NULL;
  struct passwd *pw;
  setpwent();
  while ((pw = getpwent()) != NULL) {
    if (!listed(pw)) continue;
    GtkWidget *l = gtk_label_new(pw->pw_name);
    gtk_widget_set_halign(l, GTK_ALIGN_START);
    gtk_widget_set_margin_start(l, 12); gtk_widget_set_margin_top(l, 6); gtk_widget_set_margin_bottom(l, 6);
    GtkWidget *row = gtk_list_box_row_new();
    gtk_container_add(GTK_CONTAINER(row), l);
    g_object_set_data_full(G_OBJECT(row), "user", g_strdup(pw->pw_name), g_free);
    gtk_list_box_insert(GTK_LIST_BOX(users), row, -1);
    if (!sel || (last && !strcmp(last, pw->pw_name))) sel = GTK_LIST_BOX_ROW(row);
  }
  endpwent();
  if (sel) gtk_list_box_select_row(GTK_LIST_BOX(users), sel);
  g_free(last);
}
static const char *chosen_user(void) {
  GtkListBoxRow *r = gtk_list_box_get_selected_row(GTK_LIST_BOX(users));
  return r ? g_object_get_data(G_OBJECT(r), "user") : NULL;
}

/* ---- PAM, in a worker thread ---- */
typedef struct { gchar *user, *password; int rc; int stage; } Auth;
static int conv(int n, const struct pam_message **msg, struct pam_response **resp, void *data) {
  Auth *a = data;
  struct pam_response *r = calloc(n, sizeof *r);
  if (!r) return PAM_BUF_ERR;
  for (int i = 0; i < n; i++) {
    switch (msg[i]->msg_style) {
    case PAM_PROMPT_ECHO_OFF: r[i].resp = strdup(a->password); break;
    case PAM_PROMPT_ECHO_ON:  r[i].resp = strdup(a->user); break;
    default: break;  /* PAM_ERROR_MSG, PAM_TEXT_INFO: nothing to answer */
    }
  }
  *resp = r;
  return PAM_SUCCESS;
}
static void auth_thread(GTask *t, gpointer src, gpointer data, GCancellable *c) {
  Auth *a = data;
  struct pam_conv pc = { conv, a };
  pam_handle_t *h = NULL;
  a->stage = 0;
  a->rc = pam_start(service, a->user, &pc, &h);
  if (a->rc == PAM_SUCCESS) { a->stage = 1; a->rc = pam_authenticate(h, 0); }
  if (a->rc == PAM_SUCCESS) { a->stage = 2; a->rc = pam_acct_mgmt(h, 0); }
  if (h) pam_end(h, a->rc);
  if (a->rc != PAM_SUCCESS) g_usleep(G_USEC_PER_SEC);  /* slow down guessing */
  g_task_return_boolean(t, a->rc == PAM_SUCCESS);
}
static void set_busy(gboolean busy) {
  gtk_widget_set_sensitive(pass, !busy);
  gtk_widget_set_sensitive(users, !busy);
  gtk_widget_set_sensitive(login_btn, !busy);
}
static void auth_done(GObject *src, GAsyncResult *res, gpointer data) {
  Auth *a = data;
  gboolean ok = g_task_propagate_boolean(G_TASK(res), NULL);
  if (ok) {
    gchar *ans = g_strdup_printf("login %s\n", a->user);
    gchar *lu = g_strdup_printf("%s\n", a->user);
    if (write_private(last_user_path, lu, 0700) != 0)
      g_printerr("oss-greeter: cannot write %s\n", last_user_path);
    g_print("oss-greeter: %s authenticated\n", a->user);
    finish(ans);
    g_free(ans); g_free(lu);
  } else {
    const char *m = a->stage == 0 ? _("Authentication is not available")
                  : a->stage == 1 ? _("Incorrect password")
                  : _("This account cannot log in");
    g_print("oss-greeter: %s: %s (PAM %d at stage %d)\n", a->user, m, a->rc, a->stage);
    gtk_label_set_text(GTK_LABEL(err_lbl), m);
    set_busy(FALSE);
    gtk_entry_set_text(GTK_ENTRY(pass), "");
    gtk_widget_grab_focus(pass);
  }
  memset(a->password, 0, strlen(a->password));
  g_free(a->password); g_free(a->user); g_free(a);
}
static void on_login(GtkWidget *w, gpointer d) {
  const char *u = chosen_user();
  if (!u) { gtk_label_set_text(GTK_LABEL(err_lbl), _("Choose a user")); return; }
  Auth *a = g_new0(Auth, 1);
  a->user = g_strdup(u);
  a->password = g_strdup(gtk_entry_get_text(GTK_ENTRY(pass)));
  gtk_label_set_text(GTK_LABEL(err_lbl), "");
  set_busy(TRUE);
  GTask *t = g_task_new(NULL, NULL, auth_done, a);
  g_task_set_task_data(t, a, NULL);
  g_task_run_in_thread(t, auth_thread);
  g_object_unref(t);
}
static void on_row(GtkListBox *b, GtkListBoxRow *r, gpointer d) {
  gtk_label_set_text(GTK_LABEL(err_lbl), "");
  gtk_entry_set_text(GTK_ENTRY(pass), "");
  gtk_widget_grab_focus(pass);
}
static void on_restart(GtkWidget *w, gpointer d) { finish("reboot\n"); }
static void on_poweroff(GtkWidget *w, gpointer d) { finish("poweroff\n"); }

static gboolean tick(gpointer d) {
  GDateTime *now = g_date_time_new_now_local();
  gchar *t = g_date_time_format(now, "%H:%M");
  gchar *dt = g_date_time_format(now, "%A, %e %B %Y");
  gchar *m = g_markup_printf_escaped("<span size='xx-large' weight='bold'>%s</span>", t);
  gtk_label_set_markup(GTK_LABEL(clock_lbl), m);
  gtk_label_set_text(GTK_LABEL(date_lbl), dt);
  g_free(m); g_free(t); g_free(dt); g_date_time_unref(now);
  return G_SOURCE_CONTINUE;
}

static const char *css =
  "window.greeter { background-color: #1d4e6e; background-size: cover;"
  "  background-image: url('file://" PREFIX "/share/backgrounds/xfce/xfce-x.svg'); }"
  ".card { background-color: rgba(255,255,255,0.92); border-radius: 12px; padding: 24px; }"
  ".clock { color: white; text-shadow: 0 1px 3px rgba(0,0,0,0.6); }"
  ".error { color: #c01c28; }";

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");
  bindtextdomain("oss-greeter", PREFIX "/share/locale");
  bind_textdomain_codeset("oss-greeter", "UTF-8");
  textdomain("oss-greeter");
  for (int i = 1; i + 1 < argc; i += 2) {
    if (!strcmp(argv[i], "--service")) service = argv[i + 1];
    else if (!strcmp(argv[i], "--result")) result_path = argv[i + 1];
    else if (!strcmp(argv[i], "--last-user")) last_user_path = argv[i + 1];
    else { g_printerr("usage: oss-greeter [--service NAME] [--result FILE] [--last-user FILE]\n"); return 2; }
  }
  g_set_prgname("org.openqnx.Greeter");
  gtk_init(&argc, &argv);
  unlink(result_path);

  GtkCssProvider *cp = gtk_css_provider_new();
  gtk_css_provider_load_from_data(cp, css, -1, NULL);
  gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cp),
                                            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_style_context_add_class(gtk_widget_get_style_context(win), "greeter");
  if (gtk_layer_is_supported()) {
    gtk_layer_init_for_window(GTK_WINDOW(win));
    gtk_layer_set_layer(GTK_WINDOW(win), GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_namespace(GTK_WINDOW(win), "oss-greeter");
    for (int e = 0; e < GTK_LAYER_SHELL_EDGE_ENTRY_NUMBER; e++) gtk_layer_set_anchor(GTK_WINDOW(win), e, TRUE);
    gtk_layer_set_exclusive_zone(GTK_WINDOW(win), -1);
    gtk_layer_set_keyboard_mode(GTK_WINDOW(win), GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);
  } else {
    g_printerr("oss-greeter: no layer-shell; a plain full-screen window\n");
    gtk_window_fullscreen(GTK_WINDOW(win));
  }

  GtkWidget *overlay = gtk_overlay_new();
  gtk_container_add(GTK_CONTAINER(win), overlay);
  GtkWidget *center = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_set_halign(center, GTK_ALIGN_CENTER); gtk_widget_set_valign(center, GTK_ALIGN_CENTER);
  gtk_container_add(GTK_CONTAINER(overlay), center);

  clock_lbl = gtk_label_new(""); date_lbl = gtk_label_new("");
  gtk_style_context_add_class(gtk_widget_get_style_context(clock_lbl), "clock");
  gtk_style_context_add_class(gtk_widget_get_style_context(date_lbl), "clock");
  gtk_box_pack_start(GTK_BOX(center), clock_lbl, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(center), date_lbl, FALSE, FALSE, 0);

  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_style_context_add_class(gtk_widget_get_style_context(card), "card");
  gtk_widget_set_size_request(card, 360, -1);
  gtk_widget_set_margin_top(card, 16);
  gtk_box_pack_start(GTK_BOX(center), card, FALSE, FALSE, 0);

  GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(sw), 140);
  users = gtk_list_box_new();
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(users), GTK_SELECTION_BROWSE);
  gtk_container_add(GTK_CONTAINER(sw), users);
  gtk_box_pack_start(GTK_BOX(card), sw, TRUE, TRUE, 0);

  pass = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(pass), FALSE);
  gtk_entry_set_input_purpose(GTK_ENTRY(pass), GTK_INPUT_PURPOSE_PASSWORD);
  gtk_entry_set_placeholder_text(GTK_ENTRY(pass), _("Password"));
  gtk_box_pack_start(GTK_BOX(card), pass, FALSE, FALSE, 0);
  err_lbl = gtk_label_new("");
  gtk_style_context_add_class(gtk_widget_get_style_context(err_lbl), "error");
  gtk_box_pack_start(GTK_BOX(card), err_lbl, FALSE, FALSE, 0);
  login_btn = gtk_button_new_with_label(_("Log In"));
  gtk_style_context_add_class(gtk_widget_get_style_context(login_btn), "suggested-action");
  gtk_box_pack_start(GTK_BOX(card), login_btn, FALSE, FALSE, 0);

  GtkWidget *bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_widget_set_halign(bottom, GTK_ALIGN_END); gtk_widget_set_valign(bottom, GTK_ALIGN_END);
  gtk_widget_set_margin_end(bottom, 16); gtk_widget_set_margin_bottom(bottom, 16);
  GtkWidget *rb = gtk_button_new_with_label(_("Restart"));
  GtkWidget *pb = gtk_button_new_with_label(_("Shut Down"));
  gtk_box_pack_start(GTK_BOX(bottom), rb, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(bottom), pb, FALSE, FALSE, 0);
  gtk_overlay_add_overlay(GTK_OVERLAY(overlay), bottom);

  fill_users();
  g_signal_connect(users, "row-activated", G_CALLBACK(on_row), NULL);
  g_signal_connect(users, "row-selected", G_CALLBACK(on_row), NULL);
  g_signal_connect(pass, "activate", G_CALLBACK(on_login), NULL);
  g_signal_connect(login_btn, "clicked", G_CALLBACK(on_login), NULL);
  g_signal_connect(rb, "clicked", G_CALLBACK(on_restart), NULL);
  g_signal_connect(pb, "clicked", G_CALLBACK(on_poweroff), NULL);
  /* closing the window (if a compositor ever could) is not a way past it */
  g_signal_connect(win, "delete-event", G_CALLBACK(gtk_true), NULL);

  tick(NULL);
  g_timeout_add_seconds(1, tick, NULL);
  gtk_widget_show_all(win);
  gtk_widget_grab_focus(pass);
  gtk_main();
  return 0;
}
