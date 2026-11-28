/* oss-network-plugin ("Network"): the network indicator of the Xfce panel on
 * QNX -- an icon for the state of the connection (connected, no default
 * route, disconnected, no network), a tooltip with the interface, its
 * addresses, the gateway and the DNS servers, and on a click a window with
 * every interface and "Renew DHCP Lease" (root only: dhcpcd's control
 * socket is root's). It reads the state through netinfo.c and refreshes on
 * every routing-socket message, and every 10 s besides. Connection
 * management (static addresses, choosing a Wi-Fi network) is not its job. */
#include <libintl.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <glib-unix.h>
#include <libxfce4panel/libxfce4panel.h>
#include "netinfo.h"

#define _(s) g_dgettext("oss-network-plugin", s)

typedef struct {
  XfcePanelPlugin *plugin;
  GtkWidget *button, *icon;
  GtkWidget *win, *box, *msg;          /* the details window */
  NetState *state;
  gchar *lease;                        /* dhcpcd -U of the primary interface */
  gchar *lease_if;
  gchar *wifi;                         /* wpa_cli status, Wi-Fi only [unverified] */
  gchar *renew_msg;
  int evfd;
  guint ev_src, timer, pending;
} Net;

static void refresh(Net *n);
static void fill_window(Net *n);

static const char *icon_for(NetStatus st, gboolean wifi) {
  switch (st) {
  case NET_OK: return wifi ? "network-wireless-signal-excellent-symbolic" : "network-wired-symbolic";
  case NET_NOROUTE: return wifi ? "network-wireless-no-route-symbolic" : "network-wired-no-route-symbolic";
  case NET_DOWN: return wifi ? "network-wireless-offline-symbolic" : "network-wired-disconnected-symbolic";
  default: return "network-offline-symbolic";
  }
}
static const char *status_label(NetStatus st) {
  switch (st) {
  case NET_OK: return _("Connected");
  case NET_NOROUTE: return _("Connected, no default route");
  case NET_DOWN: return _("Disconnected");
  default: return _("No network");
  }
}

/* ---- child processes, asynchronously ---- */
typedef void (*Done)(Net *n, gboolean ok, const char *out, gpointer data);
typedef struct { Net *n; Done done; gpointer data; } Job;
static void job_finished(GObject *src, GAsyncResult *res, gpointer user) {
  Job *j = user;
  gchar *out = NULL; GError *e = NULL;
  gboolean ok = g_subprocess_communicate_utf8_finish(G_SUBPROCESS(src), res, &out, NULL, &e);
  if (ok) ok = g_subprocess_get_successful(G_SUBPROCESS(src));
  j->done(j->n, ok, out ? out : (e ? e->message : ""), j->data);
  g_clear_error(&e); g_free(out); g_object_unref(src); g_free(j);
}
static void run(Net *n, const char *const *argv, Done done, gpointer data) {
  GError *e = NULL;
  GSubprocess *p = g_subprocess_newv(argv, G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_MERGE, &e);
  if (!p) { done(n, FALSE, e ? e->message : "", data); g_clear_error(&e); return; }
  Job *j = g_new0(Job, 1); j->n = n; j->done = done; j->data = data;
  g_subprocess_communicate_utf8_async(p, NULL, NULL, job_finished, j);
}

/* ---- tooltip ---- */
static void update_tooltip(Net *n) {
  const NetIf *p = NULL;
  NetStatus st = net_status(n->state, &p);
  GString *t = g_string_new(status_label(st));
  if (p) {
    g_string_append_printf(t, "\n%s%s", p->name, p->wifi ? " (Wi-Fi)" : "");
    if (n->wifi && p->wifi) {
      gchar *ssid = net_kv(n->wifi, "ssid");
      if (ssid) { g_string_append_printf(t, "  %s", ssid); g_free(ssid); }
    }
    for (guint i = 0; i < p->v4->len; i++) g_string_append_printf(t, "\n%s", (char *)g_ptr_array_index(p->v4, i));
    for (guint i = 0; i < p->v6->len; i++) {
      const char *a = g_ptr_array_index(p->v6, i);
      if (g_ascii_strncasecmp(a, "fe80", 4)) g_string_append_printf(t, "\n%s", a);  /* global ones only */
    }
    if (n->state->gw[0]) g_string_append_printf(t, "\n%s %s", _("Gateway"), n->state->gw);
    gchar *dns = net_kv(n->lease, "domain_name_servers");
    if (dns && n->lease_if && !strcmp(n->lease_if, p->name)) g_string_append_printf(t, "\n%s %s", _("DNS"), dns);
    g_free(dns);
  }
  gtk_widget_set_tooltip_text(n->button, t->str);
  g_string_free(t, TRUE);
}
static void lease_done(Net *n, gboolean ok, const char *out, gpointer data) {
  g_free(n->lease); n->lease = ok ? g_strdup(out) : NULL;
  g_free(n->lease_if); n->lease_if = data;
  update_tooltip(n);
  if (n->win && gtk_widget_get_visible(n->win)) fill_window(n);
}
static void wifi_done(Net *n, gboolean ok, const char *out, gpointer data) {
  g_free(n->wifi); n->wifi = ok ? g_strdup(out) : NULL;
  update_tooltip(n);
}

static void set_icon(Net *n) {
  const NetIf *p = NULL;
  NetStatus st = net_status(n->state, &p);
  gtk_image_set_from_icon_name(GTK_IMAGE(n->icon), icon_for(st, p && p->wifi), GTK_ICON_SIZE_BUTTON);
  gtk_image_set_pixel_size(GTK_IMAGE(n->icon), xfce_panel_plugin_get_icon_size(n->plugin));
}

static void refresh(Net *n) {
  net_free(n->state);
  n->state = net_read();
  set_icon(n);
  update_tooltip(n);
  const NetIf *p = NULL;
  net_status(n->state, &p);
  if (p) {
    run(n, net_cmd_lease(p->name), lease_done, g_strdup(p->name));
    if (p->wifi) run(n, net_cmd_wifi(p->name), wifi_done, NULL);   /* [unverified: no Wi-Fi in QEMU] */
  }
  if (n->win && gtk_widget_get_visible(n->win)) fill_window(n);
}
static gboolean refresh_later(gpointer d) { Net *n = d; n->pending = 0; refresh(n); return G_SOURCE_REMOVE; }
static gboolean on_route_event(gint fd, GIOCondition c, gpointer d) {
  Net *n = d;
  net_events_drain(fd);
  /* a change comes as several messages: refresh once, a moment later */
  if (!n->pending) n->pending = g_timeout_add(300, refresh_later, n);
  return G_SOURCE_CONTINUE;
}
static gboolean on_timer(gpointer d) { refresh(d); return G_SOURCE_CONTINUE; }

/* ---- the details window ---- */
static void renew_done(Net *n, gboolean ok, const char *out, gpointer data) {
  gchar *ifn = data;
  g_free(n->renew_msg);
  if (ok) n->renew_msg = g_strdup_printf(_("%s: DHCP lease renewal requested"), ifn);
  else {
    gchar *first = g_strdup(out ? out : ""); char *nl = strchr(first, '\n'); if (nl) *nl = 0;
    n->renew_msg = g_strdup_printf(_("%s: renewal failed: %s"), ifn, first[0] ? first : _("dhcpcd did not succeed"));
    g_free(first);
  }
  g_free(ifn);
  refresh(n);
}
static void on_renew(GtkWidget *b, gpointer d) {
  Net *n = d;
  const char *ifn = g_object_get_data(G_OBJECT(b), "if");
  gtk_widget_set_sensitive(b, FALSE);
  g_free(n->renew_msg); n->renew_msg = g_strdup_printf(_("%s: renewing the DHCP lease..."), ifn);
  if (n->msg) gtk_label_set_text(GTK_LABEL(n->msg), n->renew_msg);
  run(n, net_cmd_renew(ifn), renew_done, g_strdup(ifn));
}
static void on_copy(GtkWidget *b, gpointer d) {
  const char *a = g_object_get_data(G_OBJECT(b), "addr");
  gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), a, -1);
}
static GtkWidget *row(GtkWidget *grid, int r, const char *k, const char *v) {
  GtkWidget *kl = gtk_label_new(k), *vl = gtk_label_new(v);
  gtk_widget_set_halign(kl, GTK_ALIGN_END); gtk_widget_set_halign(vl, GTK_ALIGN_START);
  gtk_style_context_add_class(gtk_widget_get_style_context(kl), "dim-label");
  gtk_label_set_selectable(GTK_LABEL(vl), TRUE);
  gtk_grid_attach(GTK_GRID(grid), kl, 0, r, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), vl, 1, r, 1, 1);
  return vl;
}
static void fill_window(Net *n) {
  GList *ch = gtk_container_get_children(GTK_CONTAINER(n->box));
  for (GList *l = ch; l; l = l->next) gtk_widget_destroy(l->data);
  g_list_free(ch);
  const NetIf *p = NULL;
  NetStatus st = net_status(n->state, &p);
  gchar *head = g_markup_printf_escaped("<b>%s</b>", status_label(st));
  GtkWidget *h = gtk_label_new(NULL); gtk_label_set_markup(GTK_LABEL(h), head); g_free(head);
  gtk_widget_set_halign(h, GTK_ALIGN_START);
  gtk_box_pack_start(GTK_BOX(n->box), h, FALSE, FALSE, 0);
  if (!n->state->sock_ok) {
    GtkWidget *l = gtk_label_new(_("The network stack (io-sock) does not answer."));
    gtk_box_pack_start(GTK_BOX(n->box), l, FALSE, FALSE, 0);
  } else if (!n->state->ifs->len) {
    GtkWidget *l = gtk_label_new(_("No network interface."));
    gtk_box_pack_start(GTK_BOX(n->box), l, FALSE, FALSE, 0);
  }
  gboolean root = geteuid() == 0;
  for (guint i = 0; i < n->state->ifs->len; i++) {
    const NetIf *f = g_ptr_array_index(n->state->ifs, i);
    GtkWidget *fr = gtk_frame_new(f->name);
    GtkWidget *g = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(g), 4); gtk_grid_set_column_spacing(GTK_GRID(g), 12);
    g_object_set(g, "margin", 8, NULL);
    gtk_container_add(GTK_CONTAINER(fr), g);
    int r = 0;
    row(g, r++, _("State"), !f->up ? _("down (disabled)") : f->link == 0 ? _("no link (cable or carrier)")
                         : f->running ? _("up") : _("up, not running"));
    if (f->wifi) row(g, r++, _("Type"), "Wi-Fi");
    if (f->mac[0]) row(g, r++, _("Hardware address"), f->mac);
    for (guint k = 0; k < f->v4->len; k++) {
      const char *a = g_ptr_array_index(f->v4, k);
      GtkWidget *hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
      GtkWidget *al = gtk_label_new(a); gtk_label_set_selectable(GTK_LABEL(al), TRUE);
      GtkWidget *cb = gtk_button_new_from_icon_name("edit-copy-symbolic", GTK_ICON_SIZE_BUTTON);
      gtk_widget_set_tooltip_text(cb, _("Copy address"));
      gchar *bare = g_strdup(a); char *sl = strchr(bare, '/'); if (sl) *sl = 0;
      g_object_set_data_full(G_OBJECT(cb), "addr", bare, g_free);
      g_signal_connect(cb, "clicked", G_CALLBACK(on_copy), n);
      gtk_box_pack_start(GTK_BOX(hb), al, FALSE, FALSE, 0); gtk_box_pack_start(GTK_BOX(hb), cb, FALSE, FALSE, 0);
      GtkWidget *kl = gtk_label_new(_("IPv4")); gtk_widget_set_halign(kl, GTK_ALIGN_END);
      gtk_style_context_add_class(gtk_widget_get_style_context(kl), "dim-label");
      gtk_grid_attach(GTK_GRID(g), kl, 0, r, 1, 1); gtk_grid_attach(GTK_GRID(g), hb, 1, r++, 1, 1);
    }
    for (guint k = 0; k < f->v6->len; k++) row(g, r++, _("IPv6"), g_ptr_array_index(f->v6, k));
    if (n->state->gw[0] && !strcmp(n->state->gwif, f->name)) row(g, r++, _("Gateway"), n->state->gw);
    if (n->lease && n->lease_if && !strcmp(n->lease_if, f->name)) {
      gchar *dns = net_kv(n->lease, "domain_name_servers"), *lt = net_kv(n->lease, "dhcp_lease_time");
      if (dns) row(g, r++, _("DNS"), dns);
      if (lt) { gchar *s = g_strdup_printf(_("%s s (DHCP)"), lt); row(g, r++, _("Lease"), s); g_free(s); }
      g_free(dns); g_free(lt);
    }
    if (f->wifi && n->wifi && p == f) {       /* [unverified: no Wi-Fi in QEMU] */
      gchar *ssid = net_kv(n->wifi, "ssid"), *ws = net_kv(n->wifi, "wpa_state");
      if (ssid) row(g, r++, _("Network"), ssid);
      if (ws) row(g, r++, _("Wi-Fi state"), ws);
      g_free(ssid); g_free(ws);
    }
    GtkWidget *rb = gtk_button_new_with_label(_("Renew DHCP Lease"));
    g_object_set_data_full(G_OBJECT(rb), "if", g_strdup(f->name), g_free);
    gtk_widget_set_halign(rb, GTK_ALIGN_START);
    /* only where the running dhcpcd holds a lease: renewing elsewhere would
     * start a dhcpcd of its own when none runs */
    gboolean leased = n->lease && n->lease_if && !strcmp(n->lease_if, f->name);
    if (root && leased) g_signal_connect(rb, "clicked", G_CALLBACK(on_renew), n);
    else {
      gtk_widget_set_sensitive(rb, FALSE);
      gtk_widget_set_tooltip_text(rb, !root ? _("Needs a root session (dhcpcd's control socket)")
                                            : _("No DHCP lease from dhcpcd on this interface"));
    }
    gtk_grid_attach(GTK_GRID(g), rb, 1, r++, 1, 1);
    gtk_box_pack_start(GTK_BOX(n->box), fr, FALSE, FALSE, 0);
  }
  n->msg = gtk_label_new(n->renew_msg ? n->renew_msg : "");
  gtk_widget_set_halign(n->msg, GTK_ALIGN_START);
  gtk_label_set_line_wrap(GTK_LABEL(n->msg), TRUE);
  gtk_box_pack_start(GTK_BOX(n->box), n->msg, FALSE, FALSE, 0);
  gtk_widget_show_all(n->box);
}
static void on_clicked(GtkWidget *b, gpointer d) {
  Net *n = d;
  if (n->win && gtk_widget_get_visible(n->win)) { gtk_widget_hide(n->win); return; }
  if (!n->win) {
    n->win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(n->win), _("Network"));
    gtk_window_set_icon_name(GTK_WINDOW(n->win), "network-wired-symbolic");
    gtk_window_set_default_size(GTK_WINDOW(n->win), 380, -1);
    g_signal_connect(n->win, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(sw), TRUE);
    gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(sw), 600);
    n->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    g_object_set(n->box, "margin", 12, NULL);
    gtk_container_add(GTK_CONTAINER(sw), n->box);
    gtk_container_add(GTK_CONTAINER(n->win), sw);
    gtk_widget_show_all(sw);   /* gtk_window_present shows the window alone */
  }
  refresh(n);
  fill_window(n);
  gtk_window_present(GTK_WINDOW(n->win));
}

static gboolean on_size(XfcePanelPlugin *pl, gint size, Net *n) {
  size /= xfce_panel_plugin_get_nrows(pl);
  gtk_widget_set_size_request(n->button, size, size);
  set_icon(n);
  return TRUE;
}
static void on_free(XfcePanelPlugin *pl, Net *n) {
  if (n->timer) g_source_remove(n->timer);
  if (n->ev_src) g_source_remove(n->ev_src);
  if (n->pending) g_source_remove(n->pending);
  if (n->evfd >= 0) close(n->evfd);
  if (n->win) gtk_widget_destroy(n->win);
  net_free(n->state);
  g_free(n->lease); g_free(n->lease_if); g_free(n->wifi); g_free(n->renew_msg);
  g_free(n);
}

static void construct(XfcePanelPlugin *pl) {
  bindtextdomain("oss-network-plugin", PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset("oss-network-plugin", "UTF-8");
  Net *n = g_new0(Net, 1);
  n->plugin = pl;
  n->button = xfce_panel_create_button();
  n->icon = gtk_image_new();
  gtk_container_add(GTK_CONTAINER(n->button), n->icon);
  gtk_container_add(GTK_CONTAINER(pl), n->button);
  xfce_panel_plugin_add_action_widget(pl, n->button);
  g_signal_connect(n->button, "clicked", G_CALLBACK(on_clicked), n);
  g_signal_connect(pl, "size-changed", G_CALLBACK(on_size), n);
  g_signal_connect(pl, "free-data", G_CALLBACK(on_free), n);
  n->evfd = net_events_open();
  if (n->evfd >= 0) n->ev_src = g_unix_fd_add(n->evfd, G_IO_IN, on_route_event, n);
  n->timer = g_timeout_add_seconds(10, on_timer, n);
  refresh(n);
  gtk_widget_show_all(GTK_WIDGET(pl));
}
XFCE_PANEL_PLUGIN_REGISTER(construct);
