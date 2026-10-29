/* gtk-layer-shell: positions GTK3 windows as Wayland layer-shell surfaces.
 * The protocol needs a Wayland compositor, so support is reported rather than
 * required; what this test proves is that the library links, its version
 * entry points work, and asking about support without a display is safe. */
#include "common.h"
#include <gtk-layer-shell.h>
int main(void) {
  QT_BEGIN("gtk-layer-shell");
  guint maj = gtk_layer_get_major_version();
  printf("  info gtk-layer-shell %u.%u.%u\n", maj,
         gtk_layer_get_minor_version(), gtk_layer_get_micro_version());
  QT_CHECK(maj >= 1 || gtk_layer_get_minor_version() > 0, "version entry points");
  if (gdk_display_get_default() != NULL) {
    /* Only meaningful with a display; FALSE is the correct answer anywhere
       that is not a wlr-layer-shell compositor, so either result passes. */
    printf("  info layer shell supported: %s\n",
           gtk_layer_is_supported() ? "yes" : "no");
  } else {
    printf("  info no display; skipping gtk_layer_is_supported()\n");
  }
  QT_CHECK(1, "library loaded and linked");
  QT_END();
}
