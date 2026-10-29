# Working in this repository

Open source components ported to QNX SDP 8.0 (aarch64le, x86_64): one recipe
per component in `ports/<name>/`, built by `scripts/qnxbuild`, packaged by
`scripts/qnxpkg`, tested on QNX targets. Start with `README.md`,
`docs/porting-guide.md` and `docs/target-guide.md`.

## Environment

```bash
export QNX_SDP_ROOT=<SDP 8.0>        # e.g. ~/qnx800
source ./env.sh x86_64               # or aarch64le; qnxbuild without -a builds both
```

Generated, not tracked: `src/ build/ stage/ pkgroot/ logs/ dist/ hosttools/`
and the three deliverables `dists/` (packages), `ported/` (original source +
QNX patches), `test_suites/` (tests). `keys/private/` holds the package
signing key: never commit it, never serve it.

## Requests and how to handle them

**"Port <project>" (not in `ports/` yet)**
1. `scripts/new-port <name> <git-url|tarball-url> [<tag|commit>]` -- recipe skeleton, test skeleton, first build.
2. The porting loop (`docs/porting-guide.md`): fix in `src/<name>`, `scripts/mkpatch <name> <patch> "<why>"`
   (every patch starts with its reason), `scripts/qnxbuild -f <name>`, both architectures. Declare `DEPS`.
3. A real test in `tests/src/test_<name>.c` (behaviour, known answers), registered in `tests/CMakeLists.txt`.
4. `scripts/qnxpkg --key keys/private/*.rsa --tests <name>`; `scripts/gen-ported <name>`;
   `scripts/gen-dependency-index && scripts/gen-port-docs && scripts/gen-root-readme && scripts/gen-target-guide`.
5. `scripts/serve-dists` + `scripts/verify-on-target -t "<target>" <target> <name>` on x86_64 and aarch64.
6. Commit locally. Publishing is the user's call.

**"Check for updates" / "update <name>"**
`scripts/check-updates [<name>...]` lists newer upstream releases (also the
GitHub Actions "Check upstream updates" button). `scripts/port-update <name>
[<version>]` ports one: the old version's recipe and patches go to
`ports/<name>/versions/<old>/`; its packages, sources and results stay.
Ask before porting updates the user did not name.

**"Verify on the target"**
`scripts/target-bootstrap <target> -r http://<host>:8000/dists/8.0` once per
target (vanilla images are fine: it copies the QNX run-time libraries they
lack), then `scripts/verify-on-target` (components, `--standalone`,
`--dependent`, `--changed`). Results go to `tests/results/<arch>.tsv` (tracked).

**"Publish" / "push"**
Only when the user says so, and only what they name.
`scripts/publish-github-release [--verified | --only <name>...] [--changed] --upload`.
Before the first push to github.com/openqnx/oss: the notice that this work
was ported and contributed by Claude (`NOTICE.md`, README) -- confirm the
wording with the user.

## Commits and publishing identity

Commit as `openqnx-oss <noreply@openqnx.invalid>`
(`git -c user.name=openqnx-oss -c user.email=noreply@openqnx.invalid commit ...`):
no personal names or addresses in commits, files or release notes. Push
`main` only. The attribution is NOTICE.md (Claude ported and contributed the
work); the license is Apache-2.0 (LICENSE).

## Rules that bit before

- A published `<name>-<version>-r<N>` is never rewritten: qnxpkg bumps `-r` when contents change, and keeps every version.
- Never run two builds of the same component at once (shared `src/`, `build/`). Check for running queues before restarting one.
- Edit scripts that may be running by writing a new file and renaming it over the old one (bash reads scripts while it runs them).
- QNX's loader takes `LD_LIBRARY_PATH` before RPATH: never ship two copies of one soname (`scripts/check-dupes`, ROS 2 builds against the standalone components).
- `scripts/check-all` must pass before committing component changes.
