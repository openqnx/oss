import json, os, html
os.chdir(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
d=json.load(open('/tmp/pkgs.json')); CAT=json.load(open('docs/categories.json'))['CAT']
P={p['name']:p for p in d['pkgs'] if p['built']}; TESTS=d['tests']

manual = {
 'boost':['test_boost','test_boost_extra'],
 'capicxx-core-runtime':['test_capicxx'], 'capicxx-someip-runtime':['test_capicxx'],
 'dlt-daemon':['test_dlt'], 'ceres-solver':['test_ceres'],
 'icu':['test_textstack'], 'pango':['test_textstack'], 'harfbuzz':['test_textstack'],
 'cairo':['test_textstack'], 'fribidi':['test_textstack'],
 'msgpack-c':['test_msgpack'], 'libjpeg-turbo':['test_libjpeg'],
 'libvorbis':['test_vorbis'], 'libogg':['test_vorbis'], 'libtheora':['test_theora'],
 'libtool':['test_ltdl'], 'p11-kit':['test_p11kit'],
 'libx11':['test_x11'], 'libxcb':['test_x11'], 'libxau':['test_x11'],
 'xorgproto':['test_x11'], 'libxdmcp':['test_x11'], 'xcb-proto':['test_x11'],
 'xtrans':['test_x11'], 'pthread-stubs':['test_x11'],
 'python3':['test_python'], 'paho.mqtt.c':['test_paho_mqtt_c'],
 'paho.mqtt.cpp':['test_paho_mqtt_cpp'], 'nlohmann_json':['test_nlohmann_json'],
 'yaml-cpp':['test_yaml_cpp'], 'foonathan_memory':['test_foonathan_memory'],
 'svt-av1':['test_svt_av1'], 'fdk-aac':['test_fdk_aac'], 'tree-sitter':['test_tree_sitter'],
 'c-ares':['test_c_ares'], 'gst-plugins-base':['test_gstreamer'],
 'gst-plugins-good':['test_gstreamer'], 'gst-plugins-bad':['test_gstreamer'],
 'gst-plugins-ugly':['test_gstreamer'], 'gst-libav':['test_gstreamer'],
 'gdk-pixbuf':['test_gdk_pixbuf'], 'json-glib':['test_json_glib'],
 'libgpg-error':['test_libgcrypt'], 'wayland-protocols':['test_wayland'],
 'xkeyboard-config':['test_wayland'], 'libxkbcommon':['test_wayland'],
 'libinih':['test_libinih'], 'sqlite':['test_sqlite'], 'libpcap':['test_libpcap'],
}
def tests_for(n):
    if n in manual: return [t for t in manual[n] if t in TESTS]
    for c in ('test_'+n, 'test_'+n.replace('-','_'), 'test_'+n.replace('.','_')):
        if c in TESTS: return [c]
    return []

def esc(s): return html.escape(s or '')

def fetch_cmd(p):
    n=p['name']
    if p['git']:
        ref=f" --branch {p['ref']}" if p['ref'] else ''
        return f"git clone --depth 1 --recurse-submodules{ref} \\\n  {p['git']} src/{n}"
    if p['tarball']:
        base=os.path.basename(p['tarball'])
        return (f"curl -fsSL -o dist/{base} {p['tarball']}\n"
                f"mkdir -p src/{n} && tar xf dist/{base} -C src/{n} --strip-components=1")
    return f"# custom do_fetch — see ports/{n}/recipe.sh"

def wrap_opts(s, width=72):
    toks=s.split(); lines=[]; cur=''
    for t in toks:
        if cur and len(cur)+len(t)+1>width: lines.append(cur); cur=t
        else: cur=(cur+' '+t).strip()
    if cur: lines.append(cur)
    return ' \\\n  '.join(lines)

def build_cmd(p):
    n=p['name']; bs=p['buildsys']
    src=f"src/{n}" + (f"/{p['subdir']}" if p['subdir'] else '')
    if bs=='cmake':
        o=wrap_opts(p['cmake']) if p['cmake'] else ''
        return (f"cmake -S {src} -B build/$ARCH/{n} -G Ninja \\\n"
                f"  -DCMAKE_TOOLCHAIN_FILE=toolchain/qnx-$ARCH.cmake \\\n"
                f"  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \\\n"
                + (f"  {o} \\\n" if o else "")
                + f"  -DBUILD_TESTING=OFF\n"
                f"cmake --build build/$ARCH/{n} -j$(nproc)\n"
                f"DESTDIR=$STAGE cmake --install build/$ARCH/{n}")
    if bs=='meson':
        o=wrap_opts(p['meson']) if p['meson'] else ''
        return (f"meson setup build/$ARCH/{n} {src} \\\n"
                f"  --cross-file toolchain/meson-$ARCH.ini \\\n"
                f"  --native-file toolchain/meson-native.ini \\\n"
                f"  --prefix=/usr/local --buildtype=release \\\n"
                + (f"  {o}\n" if o else "\n")
                + f"ninja -C build/$ARCH/{n}\n"
                f"DESTDIR=$STAGE ninja -C build/$ARCH/{n} install")
    if bs=='autotools':
        env=(p['confenv']+' ') if p['confenv'] else ''
        o=wrap_opts(p['conf']) if p['conf'] else ''
        return (f"cd build/$ARCH/{n}\n"
                f"CC=\"qcc -V$QCC_TARGET -D_QNX_SOURCE\" CXX=\"q++ -V${{QCC_TARGET}}_cxx -D_QNX_SOURCE\" \\\n"
                f"CPP=\"$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E\" \\\n"
                f"CPPFLAGS=\"-D_QNX_SOURCE -I$STAGE/usr/local/include\" \\\n"
                f"LDFLAGS=\"-L$STAGE/usr/local/lib -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify\" \\\n"
                + (f"env {env}\\\n" if env else "")
                + f"  $WS/{src}/configure --host=$QNX_TRIPLE --prefix=/usr/local \\\n"
                + (f"  {o}\n" if o else "\n")
                + f"make -j$(nproc) && make install DESTDIR=$STAGE")
    return (f"# custom build — hooks: {', '.join(p['hooks']) or 'see recipe'}\n"
            f"# read ports/{n}/recipe.sh; it defines the exact commands")

# ---- build HTML
out=[]
A=out.append
def code(path, body, cls=''):
    A(f'<div class="code {cls}"><div class="bar"><span class="path">{esc(path)}</span>'
      f'<button type="button" data-copy>Copy</button></div><pre>{esc(body)}</pre></div>')

total=len(P)
patched=sum(1 for p in P.values() if p['patches'] or p['files'])
tested=sum(1 for n in P if tests_for(n))

A(f'''<title>QNX Port Recipes</title>
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Fraunces:opsz,wght@9..144,400;9..144,600&family=Source+Sans+3:wght@300;400;500;600&family=JetBrains+Mono:wght@400;500;700&display=swap">''')
A(open('docs/style.css').read())
A('<div class="wrap">')
A(f'''<header class="mast">
<p class="eyebrow">Per-package reference · QNX SDP 8.0 · aarch64le + x86_64</p>
<h1>How every one of these {total} packages was built</h1>
<p class="standfirst">For each package: where the source comes from, the exact configure
options, any patch applied and why, and the test that proves it works on target.
Generated from the recipes themselves, so it cannot drift from what actually built.</p>
<div class="figures">
<div class="fig"><span class="n">{total}</span><span class="l">packages</span></div>
<div class="fig"><span class="n">{patched}</span><span class="l">needed a patch</span></div>
<div class="fig"><span class="n">{tested}</span><span class="l">covered by a test</span></div>
</div>
<div class="note">
<strong>Two ways to use this.</strong> To reproduce a package, run
<code>./scripts/qnxbuild -a aarch64le &lt;name&gt;</code> — the driver does everything below.
The expanded commands are here so you can port something new by copying the closest
match, or build one package outside the driver. All of them assume
<code>source env.sh &lt;arch&gt;</code> has been run, which sets <code>$STAGE</code>,
<code>$QCC_TARGET</code>, <code>$QNX_TRIPLE</code> and <code>$WS</code>.
</div>
</header>''')

# nav
A('<nav class="toc"><p class="navtitle">Categories</p><ul>')
for cat, names in CAT:
    slug=cat.lower().replace(' ','-').replace('&','and')
    A(f'<li><a href="#{slug}">{esc(cat)}</a><span>{len(names)}</span></li>')
A('</ul></nav>')

for cat, names in CAT:
    slug=cat.lower().replace(' ','-').replace('&','and')
    A(f'<section id="{slug}"><h2>{esc(cat)} <span class="cnt">{len(names)}</span></h2>')
    for n in names:
        p=P[n]; ts=tests_for(n)
        A(f'<article class="pkg" id="pkg-{esc(n)}">')
        A('<div class="phead">')
        A(f'<h3>{esc(n)}</h3>')
        if p['version']: A(f'<span class="ver">{esc(p["version"])}</span>')
        A(f'<span class="bs bs-{esc(p["buildsys"])}">{esc(p["buildsys"])}</span>')
        if p['patches'] or p['files']:
            A(f'<span class="badge patch">{len(p["patches"])+len(p["files"])} patch</span>')
        if p['deps']: A(f'<span class="deps">needs {esc(p["deps"])}</span>')
        A('</div>')

        code(f'1 · fetch', fetch_cmd(p))

        if p['patches'] or p['files']:
            body='\n'.join(
                [f"patch -p1 -d src/{n} < ports/{n}/patches/{f}" for f in p['patches']] +
                [f"# ports/{n}/files/{f}  (copied in by do_patch)" for f in p['files']])
            code('2 · patch', body, 'amber')
            if p['notes']:
                A('<ul class="why">')
                for note in p['notes'][:3]: A(f'<li>{esc(note)}</li>')
                A('</ul>')

        step = '3' if (p['patches'] or p['files']) else '2'
        code(f'{step} · configure, build, install', build_cmd(p))

        if ts:
            tl=[]
            for t in ts:
                info=TESTS[t]
                tl.append(f"# tests/src/{info['src']}  → links: {info['libs']}")
            tl.append('')
            tl.append('./tests/build-tests.sh && ./tests/make-bundle.sh')
            tl.append('# on the target, inside the unpacked bundle:')
            tl.append('LD_LIBRARY_PATH=$PWD/lib ./bin/' + ts[0])
            code('test', '\n'.join(tl), 'teal')
        else:
            A('<p class="notest">No dedicated test — exercised indirectly through the '
              'packages that depend on it.</p>')
        A('</article>')
    A('</section>')

A('<footer>Generated from <code>ports/*/recipe.sh</code> · '
  'driver <code>scripts/qnxbuild</code> · workspace <code>~/oss</code></footer>')
A('</div>')
A('''<script>
document.querySelectorAll('[data-copy]').forEach(function(b){
  b.addEventListener('click',function(){
    var pre=b.closest('.code').querySelector('pre');
    navigator.clipboard.writeText(pre.innerText).then(function(){
      var t=b.textContent; b.textContent='Copied'; b.classList.add('done');
      setTimeout(function(){b.textContent=t;b.classList.remove('done');},1300);
    });
  });
});
</script>''')
open('/tmp/recipes.html','w').write('\n'.join(out))
print('wrote', len('\n'.join(out)), 'bytes;', total, 'packages')
