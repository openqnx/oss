/* xorgproto (X11 protocol headers): compile a consumer of the core protocol,
 * keysym and extension headers, and check what goes on the wire: request
 * opcodes, the fixed sizes of requests and events (sz_* against sizeof),
 * keysym values (core, Latin-1, Greek, XF86 media keys), and extension
 * constants (XKB, RandR, Render, XInput 2). */
#include "common.h"
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/extensions/XKB.h>
#include <X11/extensions/randr.h>
#include <X11/extensions/render.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/shapeconst.h>

int main(void) {
  QT_BEGIN("xorgproto");
  QT_CHECK(X_PROTOCOL == 11 && X_PROTOCOL_REVISION == 0, "X protocol 11.0");
  QT_CHECK(X_CreateWindow == 1 && X_MapWindow == 8 && X_InternAtom == 16 && X_NoOperation == 127, "request opcodes");
  QT_CHECK(sizeof(xReq) == 4 && sizeof(xCreateWindowReq) == sz_xCreateWindowReq && sz_xCreateWindowReq == 32,
           "xReq 4 bytes, xCreateWindowReq 32 = sz_xCreateWindowReq");
  QT_CHECK(sizeof(xEvent) == 32 && sizeof(xError) == 32 && sizeof(xConnClientPrefix) == sz_xConnClientPrefix,
           "events and errors are 32 bytes, connection prefix");
  QT_CHECK(sizeof(xInternAtomReq) == sz_xInternAtomReq && sizeof(xGetPropertyReply) == 32, "InternAtom request, GetProperty reply");
  QT_CHECK(KeyPress == 2 && Expose == 12 && ClientMessage == 33 && LASTEvent == 36, "event codes");
  QT_CHECK(ButtonPressMask == (1L << 2) && StructureNotifyMask == (1L << 17), "event masks");
  QT_CHECK(XK_a == 0x61 && XK_A == 0x41 && XK_Return == 0xff0d && XK_Escape == 0xff1b && XK_F1 == 0xffbe, "core keysyms");
  QT_CHECK(XK_eacute == 0xe9 && XK_Greek_alpha == 0x7e1 && XK_EuroSign == 0x20ac, "Latin-1, Greek, Euro keysyms");
  QT_CHECK(XF86XK_AudioPlay == 0x1008ff14 && XF86XK_MonBrightnessUp == 0x1008ff02, "XF86 media keysyms");
  QT_CHECK(XkbNumKbdGroups == 4 && XkbMaxLegalKeyCode == 255 && XkbUseCoreKbd == 0x0100, "XKB constants");
  QT_CHECK(RR_Rotate_0 == 1 && RR_Rotate_90 == 2 && RR_Reflect_X == 16, "RandR rotations");
  QT_CHECK(PictOpOver == 3 && PictTypeDirect == 1 && PictOpDisjointClear == 0x10, "Render ops and picture types");
  QT_CHECK(XI_2_Major == 2 && XI_ButtonPress == 4 && XI_LASTEVENT == XI_GestureSwipeEnd, "XInput 2 events");
  QT_CHECK(ShapeBounding == 0 && ShapeInput == 2, "Shape kinds");
  /* a request built the way Xlib does */
  xCreateWindowReq req; memset(&req, 0, sizeof req);
  req.reqType = X_CreateWindow; req.length = (sz_xCreateWindowReq >> 2); req.width = 640; req.height = 480;
  const unsigned char *b = (const unsigned char *)&req;
  QT_CHECK(b[0] == 1 && req.length == 8 && req.width == 640, "CreateWindow request: opcode byte 1, length 8 words");
  QT_END();
}
