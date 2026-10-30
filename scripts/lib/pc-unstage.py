#!/usr/bin/env python3
"""pc-unstage.py <stage-prefix> <prefix> <file.pc>... -- remove the staging
tree from pkg-config files.

pkg-config puts the sysroot (the staging tree) in front of every -I/-L by
itself, so a .pc that repeats "-I<stage>/usr/local/include" (autotools
packages copy the driver's CPPFLAGS/LDFLAGS into theirs) names the tree twice,
and a variable such as xkbcomp's xkbconfigdir names a build-machine directory.
The redundant -I/-L/-rpath-link flags are dropped; any other staging path
becomes the target path (<prefix>, /usr/local)."""
import re, sys

sp, prefix = sys.argv[1].rstrip("/"), sys.argv[2].rstrip("/")
for f in sys.argv[3:]:
    t = open(f).read()
    if sp not in t:
        continue
    e = re.escape(sp)
    n = re.sub(r"(?<!\S)-I%s/include(?=\s|$)" % e, "", t)              # the staging include dir itself
    n = re.sub(r"(?<!\S)-L%s/lib(?=\s|$)" % e, "", n)
    n = re.sub(r"(?<!\S)-Wl,-rpath-link,%s/lib(?=\s|$)" % e, "", n)
    n = n.replace(sp, prefix)                                         # e.g. -I.../include/eigen3, xkbconfigdir
    n = re.sub(r"[ \t]+\n", "\n", re.sub(r"(\S)[ \t]{2,}", r"\1 ", n))
    if n != t:
        open(f, "w").write(n)
        print("pc-unstage: %s" % f)
