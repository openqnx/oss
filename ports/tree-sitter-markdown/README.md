# tree-sitter-markdown 0.5.1

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/tree-sitter-grammars/tree-sitter-markdown.git (tag `v0.5.1`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install tree-sitter-markdown
oss-get install tree-sitter-markdown-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild tree-sitter-markdown` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
git clone --depth 1 -b v0.5.1 --recurse-submodules --shallow-submodules \
    https://github.com/tree-sitter-grammars/tree-sitter-markdown.git src/tree-sitter-markdown
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/tree-sitter-markdown; B=build/$QNX_ARCH/tree-sitter-markdown; mkdir -p $B
```

```bash
do_build() {
  local g d
  for g in markdown markdown-inline; do
    d="$S/tree-sitter-$g/src"
    qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$d" -o "$B/${g/-/_}.so" "$d/parser.c" "$d/scanner.c" || return 1
  done
}
```

```bash
do_install() { install -d "$STAGE$PREFIX/lib/tree-sitter" && install -m755 "$B"/markdown.so "$B"/markdown_inline.so "$STAGE$PREFIX/lib/tree-sitter/"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- tree-sitter grammars: markdown and markdown_inline (one repository, two parsers). Installs lib/tree-sitter/{markdown,markdown_inline}.so. Version as in the QNX Everywhere 8.0.4r1 image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH tree-sitter-markdown`.

### 5. What it installs

2 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/tree-sitter-markdown` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH tree-sitter-markdown
ls repo/8.0/*/ | grep '^tree-sitter-markdown-'
```

This writes `tree-sitter-markdown-<ver>-r0.tar.gz` (run time) and `tree-sitter-markdown-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install tree-sitter-markdown
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/tree-sitter-markdown-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/tree-sitter-markdown-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# tree-sitter grammars: markdown and markdown_inline (one repository, two
# parsers). Installs lib/tree-sitter/{markdown,markdown_inline}.so. Version as
# in the QNX Everywhere 8.0.4r1 image.
VERSION=0.5.1; GIT=https://github.com/tree-sitter-grammars/tree-sitter-markdown.git; REF=v0.5.1
BUILDSYS=custom
do_build() {
  local g d
  for g in markdown markdown-inline; do
    d="$S/tree-sitter-$g/src"
    qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$d" -o "$B/${g/-/_}.so" "$d/parser.c" "$d/scanner.c" || return 1
  done
}
do_install() { install -d "$STAGE$PREFIX/lib/tree-sitter" && install -m755 "$B"/markdown.so "$B"/markdown_inline.so "$STAGE$PREFIX/lib/tree-sitter/"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
