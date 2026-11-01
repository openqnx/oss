/* e2fsprogs on a 16 MiB file image (never a device):
 *  - mke2fs makes an ext4 file system with a given label and UUID, 1 KiB
 *    blocks; dumpe2fs -h and libext2fs (ext2fs_open on the image) report
 *    exactly those; blkid (libblkid) probes TYPE="ext4", the label and UUID;
 *  - debugfs -w writes a file and a directory into it; debugfs cat and
 *    libext2fs (ext2fs_namei + ext2fs_file_read) read the file back;
 *  - e2fsck -fn finds it clean; after debugfs unlinks the file without
 *    freeing its inode, e2fsck -fn reports the damage (exit 4), -fy repairs
 *    it (exit 1) and a third run is clean again (exit 0);
 *  - tune2fs -L / e2label relabel; resize2fs grows it to 32 MiB;
 *  - libcom_err: the ext2 error table's messages; libss: a request table of
 *    our own, executed by ss_execute_line. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <et/com_err.h>
#include <ext2fs/ext2_fs.h>
#include <ext2fs/ext2fs.h>
#include <ss/ss.h>

#define UUID "0f8d3c1e-1d2a-4b5e-9c7f-123456789abc"
static char dir[] = "/tmp/qnxport-e2fs.XXXXXX", img[128], sbin[64] = "/usr/local/sbin/";

static int run(const char *cmd, char *out, size_t n) {
  char full[1024];
  snprintf(full, sizeof full, "cd %s && %s 2>&1", dir, cmd);
  out[0] = 0;
  FILE *p = popen(full, "r");
  if (!p) return -1;
  size_t len = fread(out, 1, n - 1, p);
  out[len] = 0;
  int st = pclose(p);
  return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}
/* A program from sbin, by its full path (sbin is often not on PATH). */
static int prog(const char *name, const char *args, char *out, size_t n) {
  char cmd[900];
  snprintf(cmd, sizeof cmd, "%s%s %s", sbin, name, args);
  return run(cmd, out, n);
}
static int has_line(const char *out, const char *key, const char *value) {
  const char *p = strstr(out, key);
  if (!p) return 0;
  p += strlen(key);
  while (*p == ' ') p++;
  return !strncmp(p, value, strlen(value)) && (p[strlen(value)] == '\n' || p[strlen(value)] == 0);
}

static int ss_calls; static char ss_args[128];
static void cmd_greet(int argc, ss_argv_t argv, int sci, void *info) {
  (void)sci; (void)info;
  ss_calls++;
  ss_args[0] = 0;
  for (int i = 0; i < argc; i++) {
    strncat(ss_args, argv[i], sizeof ss_args - strlen(ss_args) - 2);
    if (i + 1 < argc) strcat(ss_args, "|");
  }
}
static const char *const greet_names[] = { "greet", "g", NULL };
static ss_request_entry entries[] = {
  { greet_names, cmd_greet, "say hello", 0 },
  { NULL, NULL, NULL, 0 },
};
static ss_request_table table = { SS_RQT_TBL_V2, entries };

int main(void) {
  QT_BEGIN("e2fsprogs");
  char out[8192];
  if (access("/usr/local/sbin/mke2fs", X_OK) != 0) sbin[0] = 0;   /* then from PATH */

  /* libcom_err */
  initialize_ext2_error_table();
  QT_CHECK(!strcmp(error_message(EXT2_ET_BAD_MAGIC), "Bad magic number in super-block"),
           "com_err: EXT2_ET_BAD_MAGIC = \"Bad magic number in super-block\"");
  QT_CHECK(!strcmp(error_message(EXT2_ET_FILE_NOT_FOUND), "File not found by ext2_lookup"),
           "com_err: EXT2_ET_FILE_NOT_FOUND = \"File not found by ext2_lookup\"");

  /* libss */
  int code = 0;
  int sci = ss_create_invocation("qnxss", "1.0", NULL, &table, &code);
  QT_CHECK(sci >= 0 && code == 0, "ss_create_invocation");
  if (sci >= 0 && code == 0) {
    char l1[] = "greet hello world", l2[] = "g  \"quoted arg\"", l3[] = "nosuch";
    ss_execute_line(sci, l1);
    QT_CHECK(ss_calls == 1 && !strcmp(ss_args, "greet|hello|world"), "ss_execute_line: greet hello world");
    ss_execute_line(sci, l2);
    QT_CHECK(ss_calls == 2 && !strcmp(ss_args, "g|quoted arg"), "ss_execute_line: alias with a quoted argument");
    int r = ss_execute_line(sci, l3);
    QT_CHECK(ss_calls == 2 && r == SS_ET_COMMAND_NOT_FOUND, "unknown command: SS_ET_COMMAND_NOT_FOUND");
    ss_delete_invocation(sci);
  }

  if (!mkdtemp(dir)) { QT_CHECK(0, "scratch directory"); QT_END(); }
  snprintf(img, sizeof img, "%s/fs.img", dir);
  int fd = open(img, O_CREAT | O_TRUNC | O_RDWR, 0644);
  QT_CHECK(fd >= 0 && ftruncate(fd, 16 << 20) == 0, "16 MiB image file");
  if (fd >= 0) close(fd);

  int rc = prog("mke2fs", "-q -F -t ext4 -b 1024 -L qnxtest -U " UUID " -E nodiscard fs.img", out, sizeof out);
  if (rc) printf("  info mke2fs: %.400s\n", out);
  QT_CHECK(rc == 0, "mke2fs -t ext4 -b 1024 -L qnxtest -U <uuid> on the image");

  rc = prog("dumpe2fs", "-h fs.img", out, sizeof out);
  QT_CHECK(rc == 0, "dumpe2fs -h");
  QT_CHECK(has_line(out, "Filesystem volume name:", "qnxtest"), "dumpe2fs: volume name qnxtest");
  QT_CHECK(has_line(out, "Filesystem UUID:", UUID), "dumpe2fs: the UUID given");
  QT_CHECK(has_line(out, "Block count:", "16384") && has_line(out, "Block size:", "1024"),
           "dumpe2fs: 16384 blocks of 1024 bytes");
  { const char *f = strstr(out, "Filesystem features:");
    QT_CHECK(f && strstr(f, "extent") && strstr(f, "has_journal"), "dumpe2fs: ext4 features (extent, has_journal)"); }

  ext2_filsys fs = NULL;
  errcode_t err = ext2fs_open(img, 0, 0, 0, unix_io_manager, &fs);
  QT_CHECK(err == 0, "ext2fs_open on the image");
  if (!err) {
    char label[EXT2_LABEL_LEN + 1] = "";
    memcpy(label, fs->super->s_volume_name, EXT2_LABEL_LEN);
    QT_CHECK(fs->super->s_magic == EXT2_SUPER_MAGIC && ext2fs_blocks_count(fs->super) == 16384 &&
             fs->blocksize == 1024 && !strcmp(label, "qnxtest"),
             "libext2fs: magic 0xEF53, 16384 blocks, 1024-byte blocks, label qnxtest");
    ext2_ino_t ino = 0;
    QT_CHECK(ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, "lost+found", &ino) == 0 && ino == 11,
             "libext2fs: lost+found is inode 11");
    ext2fs_close_free(&fs);
  }
  err = ext2fs_open("/dev/null", 0, 0, 0, unix_io_manager, &fs);
  QT_CHECK(err != 0, "ext2fs_open on something that is not a file system fails");
  if (err) printf("  info /dev/null: %s\n", error_message(err));

  /* e2fsprogs' own blkid (libblkid 1.0): no -o export (util-linux's); no
     cache file (-c /dev/null) so nothing is written to /etc/blkid.tab. */
  rc = prog("blkid", "-c /dev/null fs.img", out, sizeof out);
  if (rc) printf("  info blkid rc=%d: %.200s\n", rc, out);
  QT_CHECK(rc == 0 && strstr(out, "TYPE=\"ext4\"") && strstr(out, "LABEL=\"qnxtest\"") && strstr(out, "UUID=\"" UUID "\""),
           "blkid: TYPE=ext4, LABEL=qnxtest, the UUID");

  /* debugfs writes */
  char src[160]; snprintf(src, sizeof src, "%s/hello.txt", dir);
  FILE *f = fopen(src, "w");
  for (int i = 0; f && i < 300; i++) fprintf(f, "line %03d of the QNX e2fsprogs test\n", i);
  if (f) fclose(f);
  rc = prog("debugfs", "-w -R 'write hello.txt hello.txt' fs.img", out, sizeof out);
  int rc2 = prog("debugfs", "-w -R 'mkdir docs' fs.img", out, sizeof out);
  QT_CHECK(rc == 0 && rc2 == 0, "debugfs -w: write hello.txt, mkdir docs");
  rc = prog("debugfs", "-R 'cat hello.txt' fs.img 2>/dev/null > back.txt && cmp hello.txt back.txt", out, sizeof out);
  QT_CHECK(rc == 0, "debugfs cat: the file reads back identical");
  rc = prog("debugfs", "-R 'ls -l /' fs.img", out, sizeof out);
  QT_CHECK(rc == 0 && strstr(out, "hello.txt") && strstr(out, "docs") && strstr(out, "10500"),
           "debugfs ls: hello.txt (10500 bytes) and docs");

  err = ext2fs_open(img, 0, 0, 0, unix_io_manager, &fs);
  if (!err) {
    ext2_ino_t ino = 0; ext2_file_t file; char buf[12000]; unsigned int got = 0;
    int ok = ext2fs_namei(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, "hello.txt", &ino) == 0 &&
             ext2fs_file_open(fs, ino, 0, &file) == 0;
    if (ok) { ok = ext2fs_file_read(file, buf, sizeof buf, &got) == 0; ext2fs_file_close(file); }
    QT_CHECK(ok && got == 10500 && !memcmp(buf, "line 000 of", 11) && !memcmp(buf + 10500 - 35, "line 299 of the QNX e2fsprogs test\n", 35),
             "libext2fs: ext2fs_file_read of hello.txt, 10500 bytes as written");
    ext2fs_close_free(&fs);
  } else QT_CHECK(0, "ext2fs_open after debugfs");

  rc = prog("e2fsck", "-fn fs.img", out, sizeof out);
  if (rc) printf("  info e2fsck: %.400s\n", out);
  QT_CHECK(rc == 0 && strstr(out, "qnxtest: "), "e2fsck -fn: clean (exit 0)");
  /* unlink without freeing the inode: an unattached inode e2fsck must find */
  prog("debugfs", "-w -R 'unlink hello.txt' fs.img", out, sizeof out);
  rc = prog("e2fsck", "-fn fs.img", out, sizeof out);
  QT_CHECK(rc == 4 && strstr(out, "Unattached inode"), "e2fsck -fn after unlink: unattached inode, exit 4");
  rc = prog("e2fsck", "-fy fs.img", out, sizeof out);
  QT_CHECK(rc == 1 && strstr(out, "FILE SYSTEM WAS MODIFIED"), "e2fsck -fy: repaired (exit 1)");
  rc = prog("e2fsck", "-fn fs.img", out, sizeof out);
  QT_CHECK(rc == 0, "e2fsck -fn after repair: clean (exit 0)");
  rc = prog("debugfs", "-R 'ls /lost+found' fs.img", out, sizeof out);
  QT_CHECK(rc == 0 && strstr(out, "#"), "the file was reconnected in lost+found");

  rc = prog("tune2fs", "-L relabel fs.img", out, sizeof out);
  rc2 = prog("e2label", "fs.img", out, sizeof out);
  QT_CHECK(rc == 0 && rc2 == 0 && !strcmp(out, "relabel\n"), "tune2fs -L relabel; e2label reads relabel");

  rc = truncate(img, 32 << 20);
  rc2 = prog("resize2fs", "fs.img", out, sizeof out);
  if (rc2) printf("  info resize2fs: %.300s\n", out);
  prog("dumpe2fs", "-h fs.img", out, sizeof out);
  QT_CHECK(rc == 0 && rc2 == 0 && has_line(out, "Block count:", "32768"), "resize2fs grows it to 32768 blocks");
  rc = prog("e2fsck", "-fn fs.img", out, sizeof out);
  QT_CHECK(rc == 0, "e2fsck -fn after resize: clean");

  char cmd[200]; snprintf(cmd, sizeof cmd, "rm -rf %s", dir);
  if (system(cmd) != 0) printf("  info could not remove %s\n", dir);
  QT_END();
}
