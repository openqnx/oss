// webrtc-screencast -- shows the desktop of this machine in a web browser.
//
// The picture comes from the Wayland compositor (wlr-screencopy, capture.c),
// is encoded and sent by WebRTC (libwebrtc), and a browser gets it from the
// small HTTP server in here: "GET /" is a page with a <video>, the page posts
// its SDP offer to "/offer" and plays what the answer describes. Every browser
// gets its own peer connection; all of them share the one captured picture.
// View only, unless --input is given: then the page sends the mouse and the
// keyboard over a data channel, and they go to the compositor's virtual
// pointer and keyboard (input.c).

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "api/audio/audio_device.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "api/make_ref_counted.h"
#include "api/peer_connection_interface.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include "api/video_codecs/video_decoder_factory_template.h"
#include "api/video_codecs/video_decoder_factory_template_libvpx_vp8_adapter.h"
#include "api/video_codecs/video_encoder_factory_template.h"
#include "api/video_codecs/video_encoder_factory_template_libvpx_vp8_adapter.h"
#include "libyuv/convert.h"
#include "media/base/adapted_video_track_source.h"
#include "rtc_base/event.h"
#include "rtc_base/logging.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/thread.h"
#include "rtc_base/time_utils.h"

#include "capture.h"
#include "input.h"
#include "page.h"

namespace {

const char kVersion[] = "1.1";
volatile int g_stop = 0;
std::atomic<int> g_viewers{0};

struct Options {
  int port = 8080;
  std::string bind = "0.0.0.0";
  int fps = 15;
  int output = 0;
  bool cursor = true;
  int max_viewers = 4;
  std::string token;
  std::string stun;
  std::string display;
  bool verbose = false;
  bool input = false;
};
Options g_opt;

// ---- the captured picture as a WebRTC video source -------------------------

class ScreenSource : public rtc::AdaptedVideoTrackSource {
 public:
  void Captured(const capture_frame& f) {
    rtc::scoped_refptr<webrtc::I420Buffer> buf = webrtc::I420Buffer::Create(f.width, f.height);
    // a negative height makes libyuv read the picture bottom-up
    int h = f.y_invert ? -f.height : f.height;
    int rc = (f.fourcc == static_cast<uint32_t>(cricket::FOURCC_ABGR))
        ? libyuv::ABGRToI420(f.data, f.stride, buf->MutableDataY(), buf->StrideY(), buf->MutableDataU(),
                             buf->StrideU(), buf->MutableDataV(), buf->StrideV(), f.width, h)
        : libyuv::ARGBToI420(f.data, f.stride, buf->MutableDataY(), buf->StrideY(), buf->MutableDataU(),
                             buf->StrideU(), buf->MutableDataV(), buf->StrideV(), f.width, h);
    if (rc != 0) return;
    OnFrame(webrtc::VideoFrame::Builder()
                .set_video_frame_buffer(buf)
                .set_rotation(webrtc::kVideoRotation_0)
                .set_timestamp_us(rtc::TimeMicros())
                .build());
  }
  SourceState state() const override { return kLive; }
  bool remote() const override { return false; }
  bool is_screencast() const override { return true; }
  std::optional<bool> needs_denoising() const override { return false; }
};

// ---- one browser ------------------------------------------------------------

class Waiter {
 public:
  void Done(bool ok, std::string msg = "") { ok_ = ok; msg_ = std::move(msg); ev_.Set(); }
  bool Wait(int ms) { return ev_.Wait(webrtc::TimeDelta::Millis(ms)) && ok_; }
  const std::string& msg() const { return msg_; }
 private:
  rtc::Event ev_;
  bool ok_ = false;
  std::string msg_;
};

class SetObserver : public webrtc::SetLocalDescriptionObserverInterface,
                    public webrtc::SetRemoteDescriptionObserverInterface {
 public:
  explicit SetObserver(Waiter* w) : w_(w) {}
  void OnSetLocalDescriptionComplete(webrtc::RTCError e) override { w_->Done(e.ok(), e.message()); }
  void OnSetRemoteDescriptionComplete(webrtc::RTCError e) override { w_->Done(e.ok(), e.message()); }
 private:
  Waiter* w_;
};

class AnswerObserver : public webrtc::CreateSessionDescriptionObserver {
 public:
  explicit AnswerObserver(Waiter* w) : w_(w) {}
  void OnSuccess(webrtc::SessionDescriptionInterface* d) override { desc.reset(d); w_->Done(true); }
  void OnFailure(webrtc::RTCError e) override { w_->Done(false, e.message()); }
  std::unique_ptr<webrtc::SessionDescriptionInterface> desc;
 private:
  Waiter* w_;
};

// The page's messages, one event a line: "m <x> <y>" (pointer, 0..1 of the
// picture), "b <button> <0|1>", "w <dx> <dy>" (wheel steps), "k <evdev code> <0|1>".
void OnInputMessage(const char* p, size_t n) {
  char line[64];
  if (n == 0 || n >= sizeof line) return;
  memcpy(line, p, n);
  line[n] = 0;
  double x = 0, y = 0;
  int a = 0, b = 0;
  if (line[0] == 'm' && sscanf(line + 1, "%lf %lf", &x, &y) == 2) input_motion(x, y);
  else if (line[0] == 'b' && sscanf(line + 1, "%d %d", &a, &b) == 2) input_button(a, b);
  else if (line[0] == 'w' && sscanf(line + 1, "%lf %lf", &x, &y) == 2) input_wheel(x, y);
  else if (line[0] == 'k' && sscanf(line + 1, "%d %d", &a, &b) == 2 && a > 0) input_key(a, b);
}

class Viewer : public webrtc::PeerConnectionObserver, public webrtc::DataChannelObserver {
 public:
  explicit Viewer(std::string who) : who_(std::move(who)) {}
  ~Viewer() override { if (channel) channel->UnregisterObserver(); }
  void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState) override {}
  void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> c) override {
    if (!g_opt.input || channel) return;      // without --input the channel stays unread
    channel = c;
    channel->RegisterObserver(this);
  }
  void OnStateChange() override {
    if (channel && channel->state() == webrtc::DataChannelInterface::kClosed) input_key(-1, 0);
  }
  void OnMessage(const webrtc::DataBuffer& m) override {
    if (!m.binary) OnInputMessage(m.data.data<char>(), m.data.size());
  }
  void OnIceCandidate(const webrtc::IceCandidateInterface*) override {}
  void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState s) override {
    if (s == webrtc::PeerConnectionInterface::kIceGatheringComplete) gathered.Set();
  }
  void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState s) override {
    using S = webrtc::PeerConnectionInterface::PeerConnectionState;
    if (s == S::kConnected) {
      fprintf(stderr, "screencast: %s is watching\n", who_.c_str());
    } else if (s == S::kFailed || s == S::kClosed || s == S::kDisconnected) {
      if (!gone.exchange(true)) {
        fprintf(stderr, "screencast: %s left\n", who_.c_str());
        if (channel) input_key(-1, 0);   // let go of what this browser held
      }
    }
  }
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc;
  rtc::scoped_refptr<webrtc::DataChannelInterface> channel;
  rtc::Event gathered;
  std::atomic<bool> gone{false};
  int64_t made_ms = rtc::TimeMillis();
 private:
  std::string who_;
};

rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> g_factory;
rtc::scoped_refptr<ScreenSource> g_source;
std::mutex g_mu;
std::vector<std::unique_ptr<Viewer>> g_list;

// Drops the browsers that left, and those that never connected.
void Sweep() {
  std::vector<std::unique_ptr<Viewer>> dead;
  {
    std::lock_guard<std::mutex> l(g_mu);
    for (auto it = g_list.begin(); it != g_list.end();) {
      Viewer* v = it->get();
      bool never = v->pc->peer_connection_state() == webrtc::PeerConnectionInterface::PeerConnectionState::kNew &&
                   rtc::TimeMillis() - v->made_ms > 30000;
      if (v->gone || never) { dead.push_back(std::move(*it)); it = g_list.erase(it); } else { ++it; }
    }
    g_viewers = static_cast<int>(g_list.size());
  }
  for (auto& v : dead) { v->pc->Close(); v->pc = nullptr; }
}

// The browser's offer -> our answer (with all ICE candidates in it).
bool Answer(const std::string& offer, const std::string& who, std::string* out, std::string* err) {
  Sweep();
  {
    std::lock_guard<std::mutex> l(g_mu);
    if (static_cast<int>(g_list.size()) >= g_opt.max_viewers) { *err = "too many viewers"; return false; }
  }
  webrtc::PeerConnectionInterface::RTCConfiguration cfg;
  cfg.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
  if (!g_opt.stun.empty()) {
    webrtc::PeerConnectionInterface::IceServer s;
    s.urls.push_back("stun:" + g_opt.stun);
    cfg.servers.push_back(s);
  }
  auto v = std::make_unique<Viewer>(who);
  auto made = g_factory->CreatePeerConnectionOrError(cfg, webrtc::PeerConnectionDependencies(v.get()));
  if (!made.ok()) { *err = made.error().message(); return false; }
  v->pc = made.MoveValue();
  auto track = g_factory->CreateVideoTrack(g_source, "screen");
  track->set_content_hint(webrtc::VideoTrackInterface::ContentHint::kDetailed);
  auto added = v->pc->AddTrack(track, {"screen"});
  bool ok = added.ok();
  if (!ok) *err = added.error().message();

  webrtc::SdpParseError perr;
  std::unique_ptr<webrtc::SessionDescriptionInterface> remote;
  if (ok) {
    remote = webrtc::CreateSessionDescription(webrtc::SdpType::kOffer, offer, &perr);
    if (!remote) { *err = "bad offer: " + perr.description; ok = false; }
  }
  if (ok) {
    Waiter w;
    v->pc->SetRemoteDescription(std::move(remote), rtc::make_ref_counted<SetObserver>(&w));
    if (!w.Wait(10000)) { *err = "offer refused: " + w.msg(); ok = false; }
  }
  if (ok) {
    Waiter w;
    auto obs = rtc::make_ref_counted<AnswerObserver>(&w);
    v->pc->CreateAnswer(obs.get(), webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    if (!w.Wait(10000) || !obs->desc) { *err = "no answer: " + w.msg(); ok = false; }
    if (ok) {
      Waiter w2;
      v->pc->SetLocalDescription(std::move(obs->desc), rtc::make_ref_counted<SetObserver>(&w2));
      if (!w2.Wait(10000)) { *err = "answer refused: " + w2.msg(); ok = false; }
    }
  }
  if (ok) {
    v->gathered.Wait(webrtc::TimeDelta::Millis(5000));   // without STUN this takes a moment
    const webrtc::SessionDescriptionInterface* local = v->pc->local_description();
    if (!local || !local->ToString(out)) { *err = "no local description"; ok = false; }
  }
  if (!ok) { v->pc->Close(); v->pc = nullptr; return false; }
  std::lock_guard<std::mutex> l(g_mu);
  g_list.push_back(std::move(v));
  g_viewers = static_cast<int>(g_list.size());
  return true;
}

// ---- the HTTP server --------------------------------------------------------

void Send(int fd, const char* status, const char* type, const std::string& body) {
  char head[256];
  int n = snprintf(head, sizeof head,
                   "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n",
                   status, type, body.size());
  std::string all(head, n);
  all += body;
  size_t off = 0;
  while (off < all.size()) {
    ssize_t w = send(fd, all.data() + off, all.size() - off, 0);
    if (w <= 0) break;
    off += static_cast<size_t>(w);
  }
}

std::string Replace(std::string s, const std::string& a, const std::string& b) {
  size_t p = s.find(a);
  if (p != std::string::npos) s.replace(p, a.size(), b);
  return s;
}

void Serve(int fd, std::string who) {
  struct timeval tv = {15, 0};
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
  std::string req;
  char buf[4096];
  size_t head_end = std::string::npos;
  while (req.size() < 256 * 1024) {
    ssize_t r = recv(fd, buf, sizeof buf, 0);
    if (r <= 0) break;
    req.append(buf, static_cast<size_t>(r));
    if (head_end == std::string::npos) head_end = req.find("\r\n\r\n");
    if (head_end == std::string::npos) continue;
    size_t len = 0;
    for (size_t p = 0; p < head_end;) {            // Content-Length, any case
      size_t e = req.find("\r\n", p);
      if (strncasecmp(req.c_str() + p, "content-length:", 15) == 0) len = strtoul(req.c_str() + p + 15, nullptr, 10);
      p = e + 2;
    }
    if (req.size() >= head_end + 4 + len) break;
  }
  if (head_end == std::string::npos) { close(fd); return; }
  size_t sp1 = req.find(' '), sp2 = req.find(' ', sp1 + 1);
  std::string method = req.substr(0, sp1);
  std::string target = sp2 == std::string::npos ? "/" : req.substr(sp1 + 1, sp2 - sp1 - 1);
  std::string path = target.substr(0, target.find('?'));
  std::string query = target.find('?') == std::string::npos ? "" : target.substr(target.find('?') + 1);
  std::string body = req.substr(head_end + 4);

  bool allowed = g_opt.token.empty();
  for (size_t p = 0; !allowed && p <= query.size();) {
    size_t e = query.find('&', p);
    if (e == std::string::npos) e = query.size();
    if (query.compare(p, e - p, "token=" + g_opt.token) == 0) allowed = true;
    p = e + 1;
  }
  if (!allowed) {
    Send(fd, "403 Forbidden", "text/plain", "a token is needed: add ?token=... to the address\n");
  } else if (method == "GET" && (path == "/" || path == "/index.html")) {
    std::string ice = g_opt.stun.empty() ? "[]" : "[{urls:'stun:" + g_opt.stun + "'}]";
    Send(fd, "200 OK", "text/html; charset=utf-8",
         Replace(Replace(kPage, "__ICE__", ice), "__INPUT__", g_opt.input ? "true" : "false"));
  } else if (method == "POST" && path == "/offer") {
    std::string answer, err;
    if (Answer(body, who, &answer, &err)) {
      Send(fd, "200 OK", "application/sdp", answer);
    } else {
      fprintf(stderr, "screencast: %s: %s\n", who.c_str(), err.c_str());
      Send(fd, "503 Service Unavailable", "text/plain", err + "\n");
    }
  } else if (method == "GET" && path == "/status") {
    Sweep();
    Send(fd, "200 OK", "application/json", "{\"viewers\":" + std::to_string(g_viewers.load()) + "}\n");
  } else {
    Send(fd, "404 Not Found", "text/plain", "not found\n");
  }
  shutdown(fd, SHUT_RDWR);
  close(fd);
}

int Listen() {
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) { perror("screencast: socket"); return -1; }
  int one = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
  struct sockaddr_in a;
  memset(&a, 0, sizeof a);
  a.sin_family = AF_INET;
  a.sin_port = htons(static_cast<uint16_t>(g_opt.port));
  if (inet_pton(AF_INET, g_opt.bind.c_str(), &a.sin_addr) != 1) {
    fprintf(stderr, "screencast: --bind %s is not an IPv4 address\n", g_opt.bind.c_str());
    close(s);
    return -1;
  }
  if (bind(s, reinterpret_cast<struct sockaddr*>(&a), sizeof a) != 0 || listen(s, 16) != 0) {
    perror("screencast: cannot listen");
    close(s);
    return -1;
  }
  return s;
}

void HttpLoop(int s) {
  while (!g_stop) {
    struct sockaddr_in peer;
    socklen_t pl = sizeof peer;
    int fd = accept(s, reinterpret_cast<struct sockaddr*>(&peer), &pl);
    if (fd < 0) { if (g_stop) break; usleep(100000); continue; }
    char ip[64] = "?";
    inet_ntop(AF_INET, &peer.sin_addr, ip, sizeof ip);
    std::thread(Serve, fd, std::string(ip)).detach();
  }
}

int OnCapture(void* user, const capture_frame* f) {
  static int64_t last_sweep = 0;
  int64_t now = rtc::TimeMillis();
  if (now - last_sweep > 2000) { last_sweep = now; Sweep(); }
  if (g_viewers.load() == 0) return 500;   // nobody is watching
  static_cast<ScreenSource*>(user)->Captured(*f);
  return 0;
}

void OnSignal(int) { g_stop = 1; }

void Usage() {
  printf(
      "webrtc-screencast %s -- watch this machine's desktop in a web browser (WebRTC)\n"
      "\n"
      "usage: webrtc-screencast [options]\n"
      "  -p, --port <n>        HTTP port of the page (default 8080)\n"
      "  -b, --bind <address>  IPv4 address to listen on (default 0.0.0.0: every interface)\n"
      "  -f, --fps <n>         pictures per second, 1-60 (default 15)\n"
      "  -o, --output <n>      which output of the compositor, 0 = the first (default 0)\n"
      "  -n, --no-cursor       leave the mouse pointer out of the picture\n"
      "  -m, --max <n>         browsers at the same time (default 4)\n"
      "  -i, --input           the browser's mouse and keyboard work the desktop\n"
      "  -t, --token <text>    only http://<address>:<port>/?token=<text> is let in\n"
      "  -s, --stun <host:port> STUN server, for a browser on another network\n"
      "  -d, --display <name>  Wayland display (default: $WAYLAND_DISPLAY, wayland-0)\n"
      "  -v, --verbose         WebRTC's own log on stderr\n"
      "  -V, --version         print the version\n"
      "  -h, --help            this text\n"
      "\n"
      "Start it in the desktop session (or with XDG_RUNTIME_DIR of that session),\n"
      "then open http://<address of this machine>:<port>/ in a browser. View only\n"
      "unless --input is given. Without --token anyone who reaches the port can\n"
      "watch -- and with --input, use the desktop.\n",
      kVersion);
}

}  // namespace

int main(int argc, char** argv) {
  for (int i = 1; i < argc; i++) {
    std::string a = argv[i];
    auto val = [&]() -> const char* {
      if (i + 1 >= argc) { fprintf(stderr, "screencast: %s needs a value\n", a.c_str()); exit(2); }
      return argv[++i];
    };
    if (a == "-h" || a == "--help") { Usage(); return 0; }
    else if (a == "-V" || a == "--version") { printf("webrtc-screencast %s\n", kVersion); return 0; }
    else if (a == "-p" || a == "--port") g_opt.port = atoi(val());
    else if (a == "-b" || a == "--bind") g_opt.bind = val();
    else if (a == "-f" || a == "--fps") g_opt.fps = atoi(val());
    else if (a == "-o" || a == "--output") g_opt.output = atoi(val());
    else if (a == "-n" || a == "--no-cursor") g_opt.cursor = false;
    else if (a == "-m" || a == "--max") g_opt.max_viewers = atoi(val());
    else if (a == "-t" || a == "--token") g_opt.token = val();
    else if (a == "-s" || a == "--stun") g_opt.stun = val();
    else if (a == "-d" || a == "--display") g_opt.display = val();
    else if (a == "-i" || a == "--input") g_opt.input = true;
    else if (a == "-v" || a == "--verbose") g_opt.verbose = true;
    else { fprintf(stderr, "screencast: unknown option %s (see --help)\n", a.c_str()); return 2; }
  }
  if (g_opt.port < 1 || g_opt.port > 65535 || g_opt.fps < 1 || g_opt.fps > 60 || g_opt.max_viewers < 1 || g_opt.output < 0) {
    fprintf(stderr, "screencast: a value is out of range (see --help)\n");
    return 2;
  }
  signal(SIGINT, OnSignal);
  signal(SIGTERM, OnSignal);
  signal(SIGPIPE, SIG_IGN);
  rtc::LogMessage::LogToDebug(g_opt.verbose ? rtc::LS_INFO : rtc::LS_NONE);

  int ls = Listen();
  if (ls < 0) return 1;

  rtc::InitializeSSL();
  std::unique_ptr<rtc::Thread> network = rtc::Thread::CreateWithSocketServer();
  std::unique_ptr<rtc::Thread> worker = rtc::Thread::Create();
  std::unique_ptr<rtc::Thread> signaling = rtc::Thread::Create();
  network->SetName("net", nullptr);
  worker->SetName("work", nullptr);
  signaling->SetName("sig", nullptr);
  network->Start();
  worker->Start();
  signaling->Start();
  // No sound is sent: the dummy audio device, so that no sound driver is needed.
  std::unique_ptr<webrtc::TaskQueueFactory> tqf = webrtc::CreateDefaultTaskQueueFactory();
  rtc::scoped_refptr<webrtc::AudioDeviceModule> adm = worker->BlockingCall(
      [&] { return webrtc::AudioDeviceModule::Create(webrtc::AudioDeviceModule::kDummyAudio, tqf.get()); });
  g_factory = webrtc::CreatePeerConnectionFactory(
      network.get(), worker.get(), signaling.get(), adm,
      webrtc::CreateBuiltinAudioEncoderFactory(), webrtc::CreateBuiltinAudioDecoderFactory(),
      // VP8 in software (libvpx): every browser has it. libwebrtc.a has no
      // "builtin" video factories (they are not part of its webrtc target).
      std::make_unique<webrtc::VideoEncoderFactoryTemplate<webrtc::LibvpxVp8EncoderTemplateAdapter>>(),
      std::make_unique<webrtc::VideoDecoderFactoryTemplate<webrtc::LibvpxVp8DecoderTemplateAdapter>>(),
      nullptr, nullptr);
  if (!g_factory) { fprintf(stderr, "screencast: WebRTC could not be started\n"); return 1; }
  g_source = rtc::make_ref_counted<ScreenSource>();

  const char* disp = g_opt.display.empty() ? nullptr : g_opt.display.c_str();
  if (g_opt.input && input_start(disp) != 0) g_opt.input = false;

  std::thread http(HttpLoop, ls);
  fprintf(stderr, "screencast: open http://<this machine>:%d/%s%s in a browser (%s)\n", g_opt.port,
          g_opt.token.empty() ? "" : "?token=", g_opt.token.c_str(), g_opt.input ? "mouse and keyboard on" : "view only");

  int rc = capture_run(disp, g_opt.output, g_opt.fps,
                       g_opt.cursor ? 1 : 0, OnCapture, g_source.get(), &g_stop);
  g_stop = 1;
  shutdown(ls, SHUT_RDWR);
  close(ls);
  http.join();
  {
    std::lock_guard<std::mutex> l(g_mu);
    for (auto& v : g_list) v->pc->Close();
    g_list.clear();
  }
  input_stop();
  g_source = nullptr;
  g_factory = nullptr;
  worker->BlockingCall([&] { adm = nullptr; });
  signaling->Stop();
  worker->Stop();
  network->Stop();
  rtc::CleanupSSL();
  return rc ? 1 : 0;
}
