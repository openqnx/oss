// page.h -- the page the built-in HTTP server gives a browser.
#ifndef SCREENCAST_PAGE_H
#define SCREENCAST_PAGE_H
static const char kPage[] = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>QNX screen</title>
<style>
 html,body{margin:0;height:100%;background:#111;color:#ccc;font:14px system-ui,sans-serif}
 video{width:100%;height:100%;object-fit:contain;background:#000;display:block}
 #s{position:fixed;left:8px;bottom:8px;padding:4px 8px;background:rgba(0,0,0,.6);border-radius:4px}
</style></head><body>
<video id="v" autoplay muted playsinline></video><div id="s">connecting...</div>
<script>
const v=document.getElementById('v'), s=document.getElementById('s');
let pc=null, timer=null, dc=null;
const INPUT=__INPUT__;
function say(t){ s.textContent=t; s.style.display=t?'block':'none'; }
function again(t){ say(t+' - reconnecting'); clearTimeout(timer); timer=setTimeout(start,2000); }
async function start(){
  if(pc){ pc.onconnectionstatechange=null; pc.close(); }
  pc=new RTCPeerConnection({iceServers:__ICE__});
  const me=pc;
  pc.addTransceiver('video',{direction:'recvonly'});
  if(INPUT){ dc=pc.createDataChannel('input',{ordered:true}); }
  pc.ontrack=e=>{ v.srcObject=e.streams[0]||new MediaStream([e.track]); };
  pc.onconnectionstatechange=()=>{
    if(me!==pc) return;
    if(pc.connectionState==='connected') say('');
    else if(pc.connectionState==='failed'||pc.connectionState==='disconnected'||pc.connectionState==='closed') again(pc.connectionState);
    else say(pc.connectionState);
  };
  try{
    await pc.setLocalDescription(await pc.createOffer());
    await new Promise(r=>{
      if(pc.iceGatheringState==='complete') return r();
      const t=setTimeout(r,3000);
      pc.addEventListener('icegatheringstatechange',()=>{ if(pc.iceGatheringState==='complete'){ clearTimeout(t); r(); } });
    });
    const res=await fetch('offer'+location.search,{method:'POST',headers:{'Content-Type':'application/sdp'},body:pc.localDescription.sdp});
    if(!res.ok) throw new Error(await res.text());
    await pc.setRemoteDescription({type:'answer',sdp:await res.text()});
  }catch(e){ if(me===pc) again(String(e.message||e)); }
}
// --- the mouse and the keyboard, when the target was started with --input ---
const K={Escape:1,Digit1:2,Digit2:3,Digit3:4,Digit4:5,Digit5:6,Digit6:7,Digit7:8,Digit8:9,Digit9:10,Digit0:11,Minus:12,Equal:13,
Backspace:14,Tab:15,KeyQ:16,KeyW:17,KeyE:18,KeyR:19,KeyT:20,KeyY:21,KeyU:22,KeyI:23,KeyO:24,KeyP:25,BracketLeft:26,BracketRight:27,
Enter:28,ControlLeft:29,KeyA:30,KeyS:31,KeyD:32,KeyF:33,KeyG:34,KeyH:35,KeyJ:36,KeyK:37,KeyL:38,Semicolon:39,Quote:40,Backquote:41,
ShiftLeft:42,Backslash:43,KeyZ:44,KeyX:45,KeyC:46,KeyV:47,KeyB:48,KeyN:49,KeyM:50,Comma:51,Period:52,Slash:53,ShiftRight:54,
NumpadMultiply:55,AltLeft:56,Space:57,CapsLock:58,F1:59,F2:60,F3:61,F4:62,F5:63,F6:64,F7:65,F8:66,F9:67,F10:68,NumLock:69,
ScrollLock:70,Numpad7:71,Numpad8:72,Numpad9:73,NumpadSubtract:74,Numpad4:75,Numpad5:76,Numpad6:77,NumpadAdd:78,Numpad1:79,
Numpad2:80,Numpad3:81,Numpad0:82,NumpadDecimal:83,IntlBackslash:86,F11:87,F12:88,NumpadEnter:96,ControlRight:97,NumpadDivide:98,
AltRight:100,Home:102,ArrowUp:103,PageUp:104,ArrowLeft:105,ArrowRight:106,End:107,ArrowDown:108,PageDown:109,Insert:110,
Delete:111,MetaLeft:125,MetaRight:126,ContextMenu:127};
function tx(t){ if(dc&&dc.readyState==='open') dc.send(t); }
function pos(e){            // where in the picture (the video is letterboxed in its box)
  const r=v.getBoundingClientRect(), vw=v.videoWidth||1, vh=v.videoHeight||1;
  const k=Math.min(r.width/vw, r.height/vh), w=vw*k, h=vh*k;
  return [(e.clientX-r.left-(r.width-w)/2)/w, (e.clientY-r.top-(r.height-h)/2)/h];
}
if(INPUT){
  let last=0;
  v.addEventListener('mousemove',e=>{ const n=performance.now(); if(n-last<15) return; last=n; const p=pos(e); tx('m '+p[0].toFixed(5)+' '+p[1].toFixed(5)); });
  const B=[0,1,2];
  v.addEventListener('mousedown',e=>{ e.preventDefault(); const p=pos(e); tx('m '+p[0].toFixed(5)+' '+p[1].toFixed(5)); if(e.button<3) tx('b '+B[e.button]+' 1'); });
  window.addEventListener('mouseup',e=>{ if(e.button<3) tx('b '+B[e.button]+' 0'); });
  v.addEventListener('contextmenu',e=>e.preventDefault());
  v.addEventListener('wheel',e=>{ e.preventDefault(); tx('w '+Math.sign(e.deltaX)+' '+Math.sign(e.deltaY)); },{passive:false});
  window.addEventListener('keydown',e=>{ const c=K[e.code]; if(!c) return; e.preventDefault(); if(!e.repeat) tx('k '+c+' 1'); });
  window.addEventListener('keyup',e=>{ const c=K[e.code]; if(!c) return; e.preventDefault(); tx('k '+c+' 0'); });
  v.style.cursor='crosshair';
}
start();
</script></body></html>
)HTML";
#endif
