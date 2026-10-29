/* libgit2: init a repository, commit a file, read the commit back. */
#include "common.h"
#include <git2.h>
#include <stdio.h>
#include <stdlib.h>
int main(void) {
  QT_BEGIN("libgit2");
  QT_CHECK(git_libgit2_init() > 0, "init");
  system("rm -rf /tmp/qnx-git-test");
  git_repository *r; QT_CHECK(git_repository_init(&r, "/tmp/qnx-git-test", 0) == 0, "repository init");
  FILE *f = fopen("/tmp/qnx-git-test/a.txt", "w"); fputs("qnx\n", f); fclose(f);
  git_index *ix; git_oid tree_id, cid; git_tree *tree; git_signature *sig;
  git_repository_index(&ix, r); git_index_add_bypath(ix, "a.txt"); git_index_write(ix);
  git_index_write_tree(&tree_id, ix); git_tree_lookup(&tree, r, &tree_id);
  git_signature_new(&sig, "qnx", "qnx@example.com", 1700000000, 0);
  QT_CHECK(git_commit_create_v(&cid, r, "HEAD", sig, sig, NULL, "first", tree, 0) == 0, "commit");
  git_commit *c; QT_CHECK(git_commit_lookup(&c, r, &cid) == 0 && !strcmp(git_commit_message(c), "first"), "read commit");
  int feat = git_libgit2_features();
  QT_CHECK(feat & GIT_FEATURE_HTTPS, "HTTPS transport (OpenSSL)");
  git_commit_free(c); git_signature_free(sig); git_tree_free(tree); git_index_free(ix); git_repository_free(r);
  git_libgit2_shutdown(); system("rm -rf /tmp/qnx-git-test");
  QT_END();
}
