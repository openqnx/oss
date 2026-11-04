# Workspace layout

Everything the work makes lives inside the workspace (the repository root),
each kind of file in one fixed place. Nothing goes in `/tmp`, `$TMPDIR`,
`~/work` or another directory outside it.

| What | Where | Tracked |
|---|---|---|
| Recipes and patches (the master copy) | `ports/<name>/recipe.sh`, `ports/<name>/patches/` | yes |
| Upstream source with the patches applied (working tree) | `src/<name>/` | no |
| Original source + QNX patches, the deliverable | `ported/` (`scripts/gen-ported`) | no |
| Build trees | `build/<arch>/<name>/` | no |
| Staged installs | `stage/<arch>/` | no |
| Packaging intermediates | `pkgroot/`, `dist/` | no |
| Packages, the deliverable | `dists/` | no |
| Host-side tools | `hosttools/` | no |
| Test sources, run-level groups | `tests/src/`, `tests/run-level/` | yes |
| Test build trees | `tests/build/<arch>-<group>/` | no |
| Test packages, the deliverable | `test_suites/` | no |
| Recorded test results and logs | `tests/results/<arch>.tsv`, `tests/logs/<arch>/` | yes |
| Build and tool logs | `logs/` | no |
| Trial runs of run-level groups | `logs/run-level/<group>-<arch>.out` | no |
| Screen-test results and screenshots | `logs/gui/<arch>/` | no |
| VM consoles | `logs/vm/` | no |
| QEMU VMs (all of them) | `lab/vm/oss-<kind>-<arch>/`, kind `vanilla`, `graphics` or `desktop` | no |
| VM scripts | `vanilla-vm/` (`make-vm`, `run-all`) | yes |
| Lab tools (trial runner, screen tests) | `lab/bin/` | no |
| Locks shared by builds, packaging and VMs | `lab/locks/` | no |
| Instructions for parallel agents | `lab/agents/` | no |
| CI runner | `lab/actions-runner/` | no |
| Reports and guides for the user | `reports/` | no |
| Scratch | `tmp/<batch-or-task>/` | no |
| Old scratch, kept until removed by hand | `tmp/archive/<date>/` | no |

`<arch>` is `x86_64` or `aarch64le`.

A VM runs only while a test needs it: stop it when the tests are done. The
standing images, `lab/vm/oss-vanilla-<arch>` (run-level tests) and
`lab/vm/oss-graphics-<arch>` (screen tests), are kept; an image made for one
experiment is deleted afterwards (`mkqnximage --wipe`, then its directory). `scripts/workspace-pack` carries the
untracked deliverables to another PC; everything else is rebuilt there.
