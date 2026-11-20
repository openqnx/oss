/* input.h -- mouse and keyboard events into the Wayland session, through the
 * compositor's virtual devices (wlr-virtual-pointer, virtual-keyboard). */
#ifndef SCREENCAST_INPUT_H
#define SCREENCAST_INPUT_H
#ifdef __cplusplus
extern "C" {
#endif

/* Connects to the compositor and starts the thread that feeds it. Returns 0,
 * or -1 when the compositor has no virtual devices (the reason on stderr). */
int input_start(const char *display);
void input_stop(void);

/* The calls below may come from any thread; they only queue the event. */
void input_motion(double x, double y);     /* position in the picture, 0..1 */
void input_button(int button, int down);   /* 0 left, 1 middle, 2 right */
void input_wheel(double dx, double dy);    /* in wheel steps; down and right are positive */
void input_key(int code, int down);        /* a Linux evdev key code (KEY_A = 30) */

#ifdef __cplusplus
}
#endif
#endif
