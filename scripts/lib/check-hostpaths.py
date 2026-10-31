#!/usr/bin/env python3
"""check-hostpaths.py [-v] <arch> [<component>...]
   check-hostpaths.py [-v] --packages <dir>       (*.tar.gz written by qnxpkg)

Build-machine paths in what is packaged (scripts/lib/hostpaths.py has the
classes). pkgroot/<arch>/<component> is read the way scripts/qnxpkg packages
it -- text files sanitized, build-machine-only files left out -- and every
remaining path is reported:
  A  an ELF file with a compiled-in data path into the staging tree
  B  a text file naming the staging tree, the link sysroot or the SDP
  C  a text file naming hosttools/ or a build tree, or a build-machine #!
Class D (RUNPATH, debug information, __FILE__, recorded configure lines,
*.pyc, static archives) and the entries of hostpath-allow marked "allow" pass;
those marked "open" are known defects, listed without failing.
Exit status 1 when A, B or C remain."""
import collections, os, re, subprocess, sys, tarfile
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hostpaths as h


def recipe_version(comp):
    try:
        m = re.search(r"^VERSION=([^;\s]+)", open(os.path.join(h.WS, "ports", comp, "recipe.sh")).read(), re.M)
        return m.group(1).strip("\"'") if m else "0"
    except OSError:
        return "0"


def candidates(top):
    r = subprocess.run(["grep", "-rlaE", h.grep_ere(), top], capture_output=True, text=True)
    return [f for f in r.stdout.split("\n") if f and os.path.isfile(f) and not os.path.islink(f)]


def report(results, verbose):
    rc = 0
    by = collections.defaultdict(list)
    for comp, rel, cls, det in results:
        by[cls].append((comp, rel, det))
    for cls in ("A", "B", "C"):
        files = sorted({(c, r) for c, r, d in by[cls]})
        if not files:
            continue
        rc = 1
        print("FAIL class %s: %d file(s) in %d component(s)" % (cls, len(files), len({c for c, r in files})))
        seen = set()
        for c, r, d in sorted(by[cls]):
            if (c, r) in seen and not verbose:
                continue
            seen.add((c, r))
            print("    %s: %s  | %s" % (c, r, d[:140]))
    opened = sorted({(c, r, d.split(":")[0]) for c, r, d in by["open"]})
    if opened:
        print("KNOWN (hostpath-allow 'open', not failed): %d file(s)" % len(opened))
        for c, r, d in opened:
            print("    %s: %s  | %s" % (c, r, d[:140]))
    return rc


def main():
    args = sys.argv[1:]
    verbose = "-v" in args
    args = [a for a in args if a != "-v"]
    allow = h.load_allow()
    results = []
    if args and args[0] == "--packages":
        d = args[1]
        for fn in sorted(os.listdir(d)):
            if not fn.endswith(".tar.gz"):
                continue
            comp = re.sub(r"(-dev)?-[^-]+-r\d+\.tar\.gz$", "", fn)
            arch = "aarch64le" if "aarch64" in d else "x86_64"
            with tarfile.open(os.path.join(d, fn)) as tf:
                for m in tf.getmembers():
                    if not m.isreg():
                        continue
                    data = tf.extractfile(m).read()
                    if not h.re_any.search(data):
                        continue
                    for cls, det in h.audit_file(m.name, data, comp, arch, "0", allow, packaged=True):
                        results.append((comp, m.name, cls, det))
        return report(results, verbose)
    arch = args[0]
    comps = args[1:]
    top = os.path.join(h.WS, "pkgroot", arch)
    tops = [os.path.join(top, c) for c in comps] if comps else [top]
    for t in tops:
        for f in candidates(t):
            rel_all = os.path.relpath(f, top)
            comp, rel = rel_all.split("/", 1)
            data = open(f, "rb").read()
            for cls, det in h.audit_file(rel, data, comp, arch, recipe_version(comp), allow):
                results.append((comp, rel, cls, det))
    return report(results, verbose)


if __name__ == "__main__":
    sys.exit(main())
