/* VTE's own child spawn on a pseudo-terminal, outside any window -- the part
 * of xfce4-terminal that runs the shell. Opens a VtePty, spawns
 * /bin/sh -c '...' on it with vte_pty_spawn_async(), and reports VTE's
 * GError if the spawn fails, else the child's exit status and what it wrote.
 * Also prints the descriptor limits VTE uses to mark descriptors
 * close-on-exec before it spawns (its fdwalk() walks up to that limit).
 * Called by test_xfce4_terminal; runs standalone as a test, too. */
#include <vte/vte.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

static int fails;
static GMainLoop *loop;
static GPid child_pid = -1;
static int child_status = -1;
static gboolean timed_out;

#define OK(...)   do { printf("  ok   "); printf(__VA_ARGS__); printf("\n"); } while (0)
#define BAD(...)  do { printf("  FAIL "); printf(__VA_ARGS__); printf("\n"); fails++; } while (0)
#define INFO(...) do { printf("  info "); printf(__VA_ARGS__); printf("\n"); } while (0)

static void child_exited(GPid pid, gint status, gpointer data)
{
    (void)pid; (void)data;
    child_status = status;
    g_main_loop_quit(loop);
}

static void spawned(GObject *src, GAsyncResult *res, gpointer data)
{
    GError *err = NULL;
    GPid pid = -1;
    (void)data;
    if (!vte_pty_spawn_finish(VTE_PTY(src), res, &pid, &err)) {
        BAD("vte_pty_spawn_async: %s (domain %s, code %d)", err ? err->message : "(no GError)",
            err ? g_quark_to_string(err->domain) : "-", err ? err->code : 0);
        g_clear_error(&err);
        g_main_loop_quit(loop);
        return;
    }
    child_pid = pid;
    OK("vte_pty_spawn_async: child pid %d", (int)pid);
    g_child_watch_add(pid, child_exited, NULL);
}

static gboolean on_timeout(gpointer data)
{
    (void)data;
    timed_out = TRUE;
    g_main_loop_quit(loop);
    return G_SOURCE_REMOVE;
}

int main(int argc, char **argv)
{
    GError *err = NULL;
    struct rlimit rl;
    char out[512], cmd[1024];
    const char *dir = argc > 1 ? argv[1] : g_get_tmp_dir();

    printf("== vte-spawn ==\n");
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0)
        INFO("RLIMIT_NOFILE cur=%lld max=%lld (RLIM_INFINITY=%lld), sysconf(_SC_OPEN_MAX)=%ld",
             (long long)rl.rlim_cur, (long long)rl.rlim_max, (long long)RLIM_INFINITY,
             sysconf(_SC_OPEN_MAX));

    VtePty *pty = vte_pty_new_sync(VTE_PTY_DEFAULT, NULL, &err);
    if (!pty) {
        BAD("vte_pty_new_sync: %s", err ? err->message : "(no GError)");
        printf("FAIL vte-spawn (%d check(s) failed)\n", fails);
        return 1;
    }
    OK("vte_pty_new_sync: pty fd %d", vte_pty_get_fd(pty));

    snprintf(out, sizeof out, "%s/vte-child.%d.out", dir, (int)getpid());
    unlink(out);
    snprintf(cmd, sizeof cmd, "echo \"tty=$(tty) sum=$((6*7))\" > %s", out);
    char *child_argv[] = { "/bin/sh", "-c", cmd, NULL };

    loop = g_main_loop_new(NULL, FALSE);
    vte_pty_spawn_async(pty, NULL, child_argv, NULL, G_SPAWN_DEFAULT, NULL, NULL, NULL,
                        -1, NULL, spawned, NULL);
    g_timeout_add_seconds(30, on_timeout, NULL);
    g_main_loop_run(loop);

    if (timed_out)
        BAD("no spawn result or child exit within 30 s (child pid %d)", (int)child_pid);
    else if (child_pid > 0) {
        if (WIFEXITED(child_status) && WEXITSTATUS(child_status) == 0)
            OK("child exited with status 0");
        else
            BAD("child exit status 0x%x", child_status);
        gchar *text = NULL;
        if (g_file_get_contents(out, &text, NULL, NULL)) {
            g_strchomp(text);
            if (strstr(text, "sum=42") && strstr(text, "tty=/dev/"))
                OK("child ran on the pty: %s", text);
            else
                BAD("child output: '%s'", text);
            g_free(text);
        } else
            BAD("child wrote nothing to %s", out);
    }
    unlink(out);
    g_object_unref(pty);
    if (fails) { printf("FAIL vte-spawn (%d check(s) failed)\n", fails); return 1; }
    printf("PASS vte-spawn\n");
    return 0;
}
