"""Build-machine paths in packaged files: find them (scripts/check-artifacts)
and rewrite them to the target's (scripts/qnxpkg).

A component installs into pkgroot/<arch>/<name>/ and is merged into
stage/<arch>/, the sysroot later cross builds compile against. Files there
may name the build machine: the staging tree (<ws>/stage/<arch>/usr/local),
the SDP's tree as mirrored by the link sysroot (<ws>/stage/<arch>/linkroot),
the SDP itself (<sdp>/host/linux/x86_64/usr/bin/qcc, <sdp>/target/qnx),
hosttools/ and build/ trees. stage/ keeps them -- later builds use those files
there -- and the packages get the target's paths instead (sanitize()).

Classes (porting-hazards 128-130):
  A  an ELF file with a compiled-in data path into the staging tree
  B  a text file naming the staging tree / link sysroot / SDP
  C  a text file naming hosttools/ or a build tree, a #! line naming a
     build-machine interpreter
  D  allowed: RUNPATH, debug information, __FILE__ strings, recorded
     configure command lines, stale *.pyc caches, static archives

Prefixes: this workspace, plus the earlier workspaces this machine built
from (build-prefixes next to this file, one per line; QNX_OSS_BUILD_PREFIXES,
colon-separated, adds more). A <prefix>/stage/<arch> is recognised under any
prefix.
"""
import os, re

HERE = os.path.dirname(os.path.abspath(__file__))
WS = os.path.dirname(os.path.dirname(HERE))
ARCHES = ("aarch64le", "x86_64")
TARGET_PREFIX = "/usr/local"


def build_prefixes(ws=WS):
    out = [ws, os.path.realpath(ws)]
    f = os.path.join(HERE, "build-prefixes")
    if os.path.exists(f):
        for line in open(f):
            line = line.split("#", 1)[0].strip()
            if line:
                out.append(line.rstrip("/"))
    out += [p.rstrip("/") for p in os.environ.get("QNX_OSS_BUILD_PREFIXES", "").split(":") if p]
    seen, res = set(), []
    for p in sorted(out, key=len, reverse=True):      # longest first in the alternation
        if p and p not in seen:
            seen.add(p); res.append(p)
    return res


_P = "|".join(re.escape(p) for p in build_prefixes())
_ARCH = "(?:aarch64le|x86_64)"
_SEG = r"[A-Za-z0-9_.+@~-]+"
# <any prefix>/stage/<arch>  (also a staging tree nested in another: python3's
# MODULE__CURSES_LDFLAGS once got <ws>/stage/x86_64/<old ws>/stage/x86_64/...)
STAGE = r"(?:/%s)+/stage/%s(?:(?:/%s)*?/stage/%s)?" % (_SEG, _ARCH, _SEG, _ARCH)
LINKROOT = STAGE + r"/linkroot"
# The SDP: <root>/host/linux/x86_64/usr/bin/<tool>, <root>/target/qnx[/<arch>]
SDP_HOST_BIN = r"(?:/%s)+/host/linux/x86_64/usr/bin/" % _SEG
SDP_TARGET = r"(?:/%s)+/target/qnx" % _SEG
WSP = "(?:%s)" % _P                                     # a known workspace prefix

re_any = re.compile(("(?:%s)/(?:stage|build|src|hosttools)/|%s/|%s|%s/"
                     % (WSP, STAGE, SDP_HOST_BIN, SDP_TARGET)).encode())
re_stage = re.compile(STAGE.encode())
re_hosttools = re.compile(("(?:%s)/hosttools/" % WSP).encode())
re_buildtree = re.compile(("(?:%s)/(?:build|src)/" % WSP).encode())

# ---------------------------------------------------------------- rewriting

_END = r"(?=[\s'\";)\]},]|\\n|$)"       # end of a path token
_TOK = r"[^\s'\";)\]},]*"               # rest of a path token
# Flags that point into the SDP's tree (the link sysroot or the SDP itself):
# a target has neither. -I/-L/-isystem/-Wl,-rpath-link into the staging tree
# go too: they become -I/usr/local/include, which the compiler searches anyway,
# and -rpath-link means nothing outside a cross link.
_FLAG_DROP = re.compile((
    r"[ \t]?(?<![^\s'\"(=])(?:-[IL][ \t]?|-isystem[ \t]+)(?:%s|%s)%s%s"
    % (LINKROOT, SDP_TARGET, _TOK, _END)).encode())
_RPATH_LINK = re.compile((r"[ \t]?(?<![^\s'\"(=])-Wl,-rpath-link,(?:%s|%s)%s%s"
                          % (STAGE, SDP_TARGET, _TOK, _END)).encode())
# CMake files: an absolute SDP library becomes its link name
# (".../linkroot/x86_64/lib/libm.so" -> "m"), what CMake links on any system.
_CMAKE_SDPLIB = re.compile((r"(?:%s|%s)/%s(?:/usr)?/lib/lib(%s)\.so(?=[\";)\s])"
                            % (LINKROOT, SDP_TARGET, _ARCH, r"[A-Za-z0-9_+-]+")).encode())
# Anything else in the link sysroot / SDP target tree names the target's own
# directory: linkroot/<arch>/lib -> /lib, linkroot/usr/include -> /usr/include.
_LINKROOT_ARCH = re.compile(("(?:%s|%s)/%s(?=/)" % (LINKROOT, SDP_TARGET, _ARCH)).encode())
_LINKROOT = re.compile(("(?:%s|%s)(?=/)" % (LINKROOT, SDP_TARGET)).encode())
_STAGE_PREFIX = re.compile((STAGE + r"(?=/usr/local)").encode())
# The SDP's cross compiler, by name: <sdp>/host/linux/x86_64/usr/bin/qcc -> qcc
_SDP_TOOL = re.compile(SDP_HOST_BIN.encode())

# #! lines naming an interpreter on the build machine
_SHEBANG = re.compile((r"\A#![ \t]*(?:/usr/bin/env[ \t]+)?((?:%s|%s)/%s(?:/%s)*)" % (WSP, STAGE, _SEG, _SEG)).encode())
INTERP = [
    (re.compile(r"python[0-9.]*$"), "/usr/local/bin/python3"),
    (re.compile(r"perl[0-9.]*$"), "/usr/local/bin/perl"),
    (re.compile(r"sh$"), "/bin/sh"),
    (re.compile(r"bash$"), "/usr/local/bin/bash"),
    (re.compile(r"g?awk$"), "/usr/local/bin/awk"),
    (re.compile(r"env$"), "/usr/bin/env"),
]

# Build trees whose contents a component installs as they are: there a
# build-tree path means the installed location.
def build_tree_map(comp, arch, version):
    if comp == "ros2":
        ros = "%s/opt/ros/%s" % (TARGET_PREFIX, version)
        b = r"(?:%s)/build/%s/ros2" % (WSP, arch)
        return [(re.compile((b + r"/install(?=[/\"';:\s]|$)").encode()), ros.encode()),
                # colcon's setup scripts run the python that built them
                (re.compile((r"(?:%s)/build/host/ros2-venv[^/\s]*/bin/python[0-9.]*(?![\w.])" % WSP).encode()),
                 b"/usr/local/bin/python3"),
                # vendored projects installed to a private prefix first, then
                # copied into install/ (bullet3_vendor/bullet3_install)
                (re.compile((b + r"/build/[^/\s]+/[^/\s\"']+_install(?=[/\"';:\s]|$)").encode()), ros.encode())]
    if comp == "nodejs":
        # node-sysroot/ mirrors the staging tree for node's build (gyp)
        return [(re.compile((r"(?:%s)/build/%s/nodejs/node-sysroot(?=/usr/local)" % (WSP, arch)).encode()), b"")]
    return []


def is_python_sysconfig(rel):
    b = os.path.basename(rel)
    return (b.startswith("_sysconfigdata_") and b.endswith(".py")) or \
           (b.startswith("_sysconfig_vars_") and b.endswith(".json")) or \
           re.search(r"/python3\.[0-9]+/config-[^/]+/Makefile$", rel) is not None or \
           re.search(r"/bin/python3[0-9.]*-config$", rel) is not None


# Python's build configuration: the build machine's python (hosttools) is the
# target's python; -I/-L into the source and build trees are dropped.
_PY_HOSTPY = re.compile((r"(?:%s)/hosttools/bin/(python3(?:\.[0-9]+)?)(?![0-9])" % WSP).encode())
_PY_TREEFLAG = re.compile((r"[ \t]?(?<![^\s'\"(=])-[IL](?:%s)/(?:src|build)/%s%s"
                           % (WSP, _TOK, _END)).encode())


def skip_reason(rel, data):
    """Why a text file must not be rewritten, or None."""
    if rel.endswith(".m4f"):
        # frozen m4 state: every string is length-prefixed (T<len>,<len>)
        return "frozen m4 file (length-prefixed strings)"
    return None


def omit_reason(rel, data):
    """Why a file must not be packaged at all, or None: it exists for cross
    builds on the build machine only."""
    if rel.endswith(".pc"):
        m = re.search(rb"^prefix=(.*)$", data, re.M)
        if m and re_hosttools.match(m.group(1).strip() + b"/"):
            # ports/wayland's wayland-scanner.pc: names the native scanner
            return "pkg-config file of a build-machine tool (prefix in hosttools/)"
    return None


def sanitize(rel, data, comp, arch, version):
    """(new_bytes, [notes]) for one packaged text file; data unchanged when
    nothing applies. rel is the path inside the package (usr/local/...)."""
    if data[:4] == b"\x7fELF" or b"\0" in data:
        return data, []
    if skip_reason(rel, data):
        return data, []
    if not re_any.search(data):
        return data, []
    n = data
    notes = []
    m = _SHEBANG.match(n)
    if m:
        interp = os.path.basename(m.group(1).decode())
        for pat, tgt in INTERP:
            if pat.match(interp):
                n = n[:m.start(1)] + tgt.encode() + n[m.end(1):]
                notes.append("shebang")
                break
        else:
            notes.append("shebang-unknown:" + interp)
    for pat, tgt in build_tree_map(comp, arch, version):
        n2 = pat.sub(tgt, n)
        if n2 != n:
            notes.append("build-tree"); n = n2
    if is_python_sysconfig(rel):
        n2 = _PY_TREEFLAG.sub(b"", _PY_HOSTPY.sub(rb"/usr/local/bin/\1", n))
        if n2 != n:
            notes.append("sysconfig"); n = n2
    n2 = _RPATH_LINK.sub(b"", _FLAG_DROP.sub(b"", n))
    if n2 != n:
        notes.append("flags"); n = n2
    n2 = n
    if rel.endswith(".cmake"):
        n2 = _CMAKE_SDPLIB.sub(rb"\1", n2)
    n2 = _LINKROOT.sub(b"", _LINKROOT_ARCH.sub(b"", n2))
    n2 = _STAGE_PREFIX.sub(b"", n2)
    n2 = _SDP_TOOL.sub(b"", n2)
    if n2 != n:
        notes.append("paths"); n = n2
    return n, notes


# ---------------------------------------------------------------- checking

SRC_EXT = re.compile(r"\.(c|cc|cpp|cxx|c\+\+|h|hh|hpp|hxx|inl|ipp|tcc|ixx|y|l|S|s|asm|rs|go|m|mm|f|f90|F|pyx|cu)(:[0-9]+)?\]?$")
DATA_DIR = re.compile(r"/usr/local/(share|etc|var|libexec)(/|$)|/usr/local/lib/[^/\s]+/")


def elf_alloc_strings(path, minlen=6):
    """(path or the file's bytes) Printable strings in the sections an ELF file loads (no debug info,
    no .dynstr -- RUNPATH and symbol names), with their section names."""
    import struct
    if isinstance(path, bytes):
        d = path
    else:
        with open(path, "rb") as f:
            d = f.read()
    if d[:4] != b"\x7fELF":
        return []
    is64 = d[4] == 2
    end = "<" if d[5] == 1 else ">"
    if is64:
        shoff, = struct.unpack_from(end + "Q", d, 0x28)
        shentsize, shnum, shstrndx = struct.unpack_from(end + "HHH", d, 0x3A)
    else:
        shoff, = struct.unpack_from(end + "I", d, 0x20)
        shentsize, shnum, shstrndx = struct.unpack_from(end + "HHH", d, 0x2E)
    secs = []
    for i in range(shnum):
        o = shoff + i * shentsize
        if is64:
            name, typ, flags, addr, off, size = struct.unpack_from(end + "IIQQQQ", d, o)
        else:
            name, typ, flags, addr, off, size = struct.unpack_from(end + "IIIIII", d, o)
        secs.append((name, typ, flags, off, size))
    if not secs or shstrndx >= len(secs):
        return []
    so = secs[shstrndx][3]
    out = []
    for name, typ, flags, off, size in secs:
        nm = d[so + name:d.index(b"\0", so + name)].decode(errors="replace")
        if not (flags & 2) or typ == 8 or nm in (".dynstr", ".dynsym", ".symtab", ".strtab") or nm.startswith(".debug"):
            continue                                    # not SHF_ALLOC / NOBITS
        for m in re.finditer(rb"[\x20-\x7e\t]{%d,}" % minlen, d[off:off + size]):
            out.append((nm, m.group(0).decode()))
    return out


_BUILD_DATA = re.compile((r"(?:%s)/build/%s(?:/%s)*?/(?:share|etc|var|libexec)(?:/|$)"
                          % (WSP, _SEG, _SEG)).encode())


def classify_elf_string(s):
    """'A' when s is a runtime data path into the staging tree (or into a
    build tree: VLC's contribs), else None (D)."""
    b = s.encode()
    if not re_stage.search(b):
        if len(s.split()) == 1 and not SRC_EXT.search(s) and "/include" not in s \
                and any(_BUILD_DATA.search(p.encode()) for p in re.split(r"[:,;]", s)):
            return "A"
        return None
    toks = s.split()
    if len(toks) > 1:
        return None                     # a recorded command line / flags
    if SRC_EXT.search(s) or "/include/" in s or s.rstrip("/").endswith("/include"):
        return None                     # __FILE__, header paths
    for p in re.split(r"[:,;]", s):
        q = re_stage.sub(b"", p.encode()).decode(errors="replace")
        if DATA_DIR.search(q) or re.search(r"/usr/local/share", q):
            return "A"
    return None


def classify_text(rel, data, comp):
    """[(class, detail)] for a packaged (sanitized) text file."""
    out = []
    first = data.split(b"\n", 1)[0]
    if first.startswith(b"#!") and re_any.search(first):
        out.append(("C", "#! " + first[2:].strip().decode(errors="replace")[:120]))
    for m in re_any.finditer(data):
        ls = data.rfind(b"\n", 0, m.start()) + 1
        le = data.find(b"\n", m.end())
        line = data[ls:le if le >= 0 else len(data)]
        if ls == 0 and line.startswith(b"#!"):
            continue
        hit = m.group(0)
        if re.search(rb"/stage/|/host/linux/|/target/qnx", hit):
            cls = "B"
        else:
            cls = "C"
        out.append((cls, line.strip().decode(errors="replace")[:160]))
    return out


# ---------------------------------------------------------------- the audit

COMMENT = re.compile(rb"^\s*(/\*|\*|//)")
CODE_EXT = (".h", ".hh", ".hpp", ".hxx", ".c", ".cc", ".cpp", ".cxx", ".inl")


def load_allow(path=os.path.join(HERE, "hostpath-allow")):
    """[(kind, comp, glob, reason)] from hostpath-allow: 'allow' for harmless
    paths (class D), 'open' for known defects still to fix (reported, not
    failed)."""
    out = []
    if os.path.exists(path):
        for line in open(path):
            line = line.rstrip("\n")
            if not line.strip() or line.lstrip().startswith("#"):
                continue
            kind, comp, glob_, reason = (line.split(None, 3) + ["", "", "", ""])[:4]
            out.append((kind, comp, glob_, reason))
    return out


def allowed(allow, comp, rel):
    import fnmatch
    for kind, c, g, reason in allow:
        if (c == "*" or c == comp) and fnmatch.fnmatch(rel, g):
            return kind, reason
    return None


def grep_ere():
    """An ERE for grep -E that finds every candidate file quickly."""
    esc = lambda s: re.sub(r"([.^$*+?()\[\]{}|\\])", r"\\\1", s)
    alts = ["%s/(stage|build|src|hosttools)/" % esc(p) for p in build_prefixes()]
    alts += ["/stage/(aarch64le|x86_64)/", "/host/linux/x86_64/usr/bin/", "/target/qnx/"]
    return "|".join(alts)


def audit_file(rel, data, comp, arch, version, allow, packaged=False):
    """Findings for one file as it is (packaged=True) or as qnxpkg would
    package it: [(class, detail)]; class A/B/C fail, D/open are reported."""
    if data[:4] == b"\x7fELF":
        strs = elf_alloc_strings(data)
        found = [("A", s[:160]) for sec, s in strs if classify_elf_string(s)]
        a = found and allowed(allow, comp, rel)
        if a:
            return [] if a[0] == "allow" else [("open", "%s: %s" % (a[1], found[0][1]))]
        return found
    if b"\0" in data:
        return []                       # .pyc caches, static archives: D
    if not packaged:
        why = omit_reason(rel, data)
        if why:
            return []                   # not packaged
        data, _ = sanitize(rel, data, comp, arch, version)
    if skip_reason(rel, data):
        a = allowed(allow, comp, rel)
        return [] if a and a[0] == "allow" else [("open", "kept as is: " + skip_reason(rel, data))]
    found = classify_text(rel, data, comp)
    if not found:
        return []
    if rel.endswith(CODE_EXT):
        found = [(c, l) for c, l in found if not COMMENT.match(l.encode())]
    if is_python_sysconfig(rel):
        # what Python recorded about its build: srcdir, VPATH, *_DEPS
        found = [(c, l) for c, l in found
                 if c != "C" or re.search(r"/hosttools/", l)]
    if not found:
        return []
    a = allowed(allow, comp, rel)
    if a:
        return [] if a[0] == "allow" else [("open", "%s: %s" % (a[1], found[0][1]))]
    return found
