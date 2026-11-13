# More than one SDP release

The ports are built against one QNX SDP per workspace. Packages built against
a newer SDP release may need what an older target does not have (a newer
kernel, libc, the libraries the SDP ships), so each SDP release the packages
are offered for has its own build, its own package repository and its own
test results.

| SDP | workspace | packages | releases on GitHub |
|---|---|---|---|
| 8.0 (the SDP the ports were first made on) | the main checkout | `dists/8.0/<arch>/` | `apk-8.0-<arch>`, `packages-8.0-<arch>`, `tests-8.0-<arch>` |
| 8.0.6 | a second checkout next to it | `dists/8.0.6/<arch>/` | `apk-8.0.6-<arch>`, `packages-8.0.6-<arch>`, `tests-8.0.6-<arch>` |

## A workspace for another SDP

```bash
git worktree add -b sdp-8.0.6 ../oss-806 main     # same repository, second checkout
cd ../oss-806
cat > .sdp <<EOT
root=$HOME/qnx806
version=8.0.6
EOT
```

`.sdp` (untracked) names the SDP of a workspace: `root` is the SDP it builds
against, `version` is what its packages are published as, `config` (optional)
is the SDP's configuration directory with the licence, when that is not where
its `qnxsdp-env.sh` says. `env.sh` reads it; with a `.sdp`, building that
workspace against any other SDP is refused -- a staging tree must never be a
mix of two SDPs. Without `.sdp` a workspace is an 8.0 workspace.

What the two checkouts share (links in the second one): the download cache
`dist/`, the host tools `hosttools/` and `build/host/`, the signing key
`keys/private/`, the lab SSH key `tests/ssh/`. Everything built for the target
is separate: `src/`, `build/<arch>/`, `stage/`, `pkgroot/`, `dists/`,
`test_suites/`, `logs/`, the VMs in `lab/vm/`.

Recipes, patches and tests are the same files for every SDP: port and fix in
the main checkout, commit, `git merge --ff-only main` in the other, build
there. A difference between SDP releases goes into the recipe as a test on
what the SDP has, not as a second recipe.
