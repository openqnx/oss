import os, re, json, glob
os.chdir(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

def strip_comments(text):
    out=[]
    for line in text.split('\n'):
        st=line.strip()
        if st.startswith('#'): continue
        out.append(line)
    return '\n'.join(out)

def comments_of(text):
    """Collect comment blocks so we can explain non-obvious options."""
    blocks=[]; cur=[]
    for line in text.split('\n'):
        st=line.strip()
        if st.startswith('#'):
            cur.append(st.lstrip('#').strip())
        else:
            if cur: blocks.append(' '.join(cur)); cur=[]
    if cur: blocks.append(' '.join(cur))
    return [b for b in blocks if len(b) > 25]

def scalar(text, name):
    m=re.search(rf'(?:^|\n|;)\s*{name}=(?:"([^"]*)"|\'([^\']*)\'|([^\s;\n]+))', text)
    if not m: return ''
    return (m.group(1) or m.group(2) or m.group(3) or '').strip()

def array(text, name):
    i=text.find(name+'=(')
    if i<0: return ''
    j=i+len(name)+2; depth=1; buf=[]
    while j < len(text) and depth>0:
        c=text[j]
        if c=='(': depth+=1
        elif c==')':
            depth-=1
            if depth==0: break
        buf.append(c); j+=1
    raw=''.join(buf)
    toks=[t for t in raw.split() if t]
    return ' '.join(toks)

def hooks(text):
    return sorted(set(re.findall(r'\b(do_fetch|do_patch|do_configure|do_build|do_install)\s*\(\)', text)))

# test binary per package, from tests/CMakeLists.txt
tests={}
tc=open('tests/CMakeLists.txt').read()
for m in re.finditer(r'port_test\(\s*(\S+)\s+(\S+)\s+([^)]*)\)', tc):
    tests[m.group(1)] = {'src': m.group(2), 'libs': ' '.join(m.group(3).split())}

pkgs=[]
for rec in sorted(glob.glob('ports/*/recipe.sh')):
    name=os.path.basename(os.path.dirname(rec))
    raw=open(rec).read()
    body=strip_comments(raw)
    pats=sorted(os.path.basename(p) for p in glob.glob(f'ports/{name}/patches/*.patch'))
    files=sorted(os.path.basename(p) for p in glob.glob(f'ports/{name}/files/*'))
    built = os.path.exists(f'stage/aarch64le/.stamp/{name}')
    pkgs.append({
        'name': name,
        'version': scalar(body,'VERSION'),
        'git': scalar(body,'GIT'), 'ref': scalar(body,'REF'),
        'tarball': scalar(body,'TARBALL'),
        'srcroot': scalar(body,'SRCROOT'), 'subdir': scalar(body,'SUBDIR'),
        'buildsys': scalar(body,'BUILDSYS') or 'cmake',
        'deps': scalar(body,'DEPS'),
        'cmake': array(body,'CMAKE_ARGS_EXTRA'),
        'meson': array(body,'MESON_ARGS_EXTRA'),
        'conf':  array(body,'CONF_ARGS_EXTRA'),
        'confenv': array(body,'CONF_ENV'),
        'cflags': scalar(body,'CFLAGS_APPEND'),
        'hooks': hooks(body),
        'patches': pats,
        'files': files,
        'notes': comments_of(raw),
        'built': built,
    })
json.dump({'pkgs':pkgs,'tests':tests}, open('/tmp/pkgs.json','w'), indent=1)
print('packages:', len(pkgs), ' built:', sum(1 for p in pkgs if p['built']), ' tests:', len(tests))
