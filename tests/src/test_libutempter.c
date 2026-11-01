/* libutempter: a pseudo-terminal is opened (posix_openpt), then
 * utempter_add_record() has the helper (libexec/utempter/utempter) write a
 * USER_PROCESS utmp record for its slave line, with this process's PID and
 * our user name; utempter_remove_record() marks the line DEAD_PROCESS.
 * Both read back from utmp. QNX's pututline() does not create the
 * file: a target without /var/log/utmp (a stock image has no /var/log) gets
 * one for the test, removed again afterwards. */
#include "common.h"
#include <utempter.h>
#include <utmp.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static int created = 0, created_dir = 0;

/* the last record for this line, whatever its type (getutline() only
   matches LOGIN/USER_PROCESS records) */
static int find_line(const char *line, struct utmp *out) {
  struct utmp *u; int found = 0;
  utmpname(_PATH_UTMP);
  setutent();
  while ((u = getutent()) != NULL)
    if (!strncmp(u->ut_line, line, sizeof u->ut_line)) { *out = *u; found = 1; }
  endutent();
  return found;
}

int main(void) {
  QT_BEGIN("libutempter");
  struct stat st;
  if (stat("/var/log", &st) < 0) created_dir = mkdir("/var/log", 0755) == 0;
  if (stat(_PATH_UTMP, &st) < 0) {
    int fd = open(_PATH_UTMP, O_CREAT | O_WRONLY, 0664);
    created = fd >= 0; if (fd >= 0) close(fd);
    printf("  info %s did not exist: %s\n", _PATH_UTMP, created ? "created for the test" : strerror(errno));
  }
  int m = posix_openpt(O_RDWR | O_NOCTTY);
  QT_CHECK(m >= 0 && grantpt(m) == 0 && unlockpt(m) == 0, "posix_openpt/grantpt/unlockpt");
  const char *slave = m >= 0 ? ptsname(m) : NULL;
  printf("  info slave %s\n", slave ? slave : "(none)");
  QT_CHECK(slave && !strncmp(slave, "/dev/", 5), "ptsname under /dev/");
  const char *line = slave ? slave + 5 : "";
  struct passwd *pw = getpwuid(getuid());

  QT_CHECK(utempter_add_record(m, "qnx.example") == 1, "utempter_add_record");
  struct utmp u;
  int found = find_line(line, &u);
  QT_CHECK(found, "utmp has a record for the slave line");
  if (found) {
    printf("  info record: type %d pid %d user %.*s line %.*s\n", u.ut_type, (int)u.ut_pid,
           (int)sizeof u.ut_name, u.ut_name, (int)sizeof u.ut_line, u.ut_line);
    QT_CHECK(u.ut_type == USER_PROCESS, "record type USER_PROCESS");
    QT_CHECK(u.ut_pid == getpid(), "record carries our PID");
    QT_CHECK(pw && !strncmp(u.ut_name, pw->pw_name, sizeof u.ut_name), "record carries our user name");
  }
  QT_CHECK(utempter_remove_record(m) == 1, "utempter_remove_record");
  found = find_line(line, &u);
  QT_CHECK(found && u.ut_type == DEAD_PROCESS, "record for the line is now DEAD_PROCESS");
  if (m >= 0) close(m);
  if (created) unlink(_PATH_UTMP);
  if (created_dir) { unlink(_PATH_WTMP); rmdir("/var/log"); }
  QT_END();
}
