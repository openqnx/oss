#!/usr/bin/env python3
"""shards -- which GitHub release a package goes to.

GitHub allows at most 1000 assets per release, so the apk and packages
releases of an architecture are split by the first character of the package
name. The ONE place that says how: SHARDS below. Every script that names a
release (publish-github-release, check-release-sync, download-stats,
gen-pkg-site) asks this file; a third shard is one more line here (and a
repository line in target/apk-setup and target/oss-get's defaults).

  (tag suffix, first characters)   -- a regular-expression character class
  the first shard keeps the tag names of before the split: apk-8.0-<arch>,
  packages-8.0-<arch>; the others are apk-8.0-<arch><suffix>.

The tests release (tests-8.0-<arch>) is not sharded.

From the shell:
  scripts/lib/shards.py tags [<sdp> [<arch>...]]  every release tag, one per line
  scripts/lib/shards.py suffixes                  "<suffix> <first characters>" per shard,
                                                  the first shard's empty suffix printed as "-"
  scripts/lib/shards.py split                     names on stdin -> "<suffix> <name>"
  scripts/lib/shards.py of <name>                 the suffix of one name ("" for shard 1)
"""
import re, sys

SHARDS = [
    ("", "0-9a-l"),
    ("-m-z", "m-z"),
]
KINDS = ("apk", "packages")       # the sharded releases
UNSHARDED = ("tests",)
SDP = "8.0"
ARCHES = ("x86_64", "aarch64")

_RX = [(s, re.compile("[%s]" % c)) for s, c in SHARDS]


def shard_of(name):
    """Tag suffix of the shard a package (or its file name) belongs to."""
    c = name[:1].lower()
    for s, rx in _RX:
        if rx.fullmatch(c):
            return s
    return SHARDS[-1][0]          # anything unexpected: the last shard


def tag(kind, arch, name=None, sdp=SDP):
    """Release tag of `kind` for `arch`; with a package name, its shard's."""
    base = "%s-%s-%s" % (kind, sdp, arch)
    return base + (shard_of(name) if name is not None and kind in KINDS else "")


def tags(sdp=SDP, arches=ARCHES):
    """Every release tag: per architecture the shards of apk and packages, then tests."""
    out = []
    for a in arches:
        for k in KINDS:
            out += ["%s-%s-%s%s" % (k, sdp, a, s) for s, _ in SHARDS]
        out += ["%s-%s-%s" % (k, sdp, a) for k in UNSHARDED]
    return out


if __name__ == "__main__":
    cmd = sys.argv[1:2] or ["tags"]
    if cmd[0] == "tags":
        sdp = sys.argv[2] if len(sys.argv) > 2 else SDP
        print("\n".join(tags(sdp, sys.argv[3:] or ARCHES)))
    elif cmd[0] == "suffixes":
        print("\n".join("%s %s" % (s or "-", c) for s, c in SHARDS))
    elif cmd[0] == "split":
        for l in sys.stdin:
            n = l.strip()
            if n:
                print(shard_of(n) or "-", n)
    elif cmd[0] == "of":
        print(shard_of(sys.argv[2]))
    else:
        sys.exit(__doc__)
