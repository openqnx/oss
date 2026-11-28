// A vehicle HMI mock-up for QNX: instrument cluster, navigation, media,
// climate, vehicle status, phone and settings on one 1280x720 screen.
// Everything is simulated (a drive cycle drives speed, rpm, gear, fuel and the
// route, one step per frame); all drawing is CustomPainter + Material widgets, no packages, so it
// runs on the engine's software renderer. Input is the pointer only.
import 'dart:math' as math;

import 'package:flutter/material.dart';
import 'package:flutter/scheduler.dart';

void main() => runApp(const HmiApp());

// ---------------------------------------------------------------- theme ----
enum DriveMode { eco, comfort, sport }

const _bg = Color(0xFF0B0F14);
const _panel = Color(0xFF141A22);
const _panel2 = Color(0xFF1B232D);
const _text = Color(0xFFE6EDF3);
const _dim = Color(0xFF8B98A5);
const _warn = Color(0xFFFFB020);
const _alert = Color(0xFFFF4D4F);
const _ok = Color(0xFF3DDC97);

Color accentOf(DriveMode m) => switch (m) {
      DriveMode.eco => const Color(0xFF3DDC97),
      DriveMode.comfort => const Color(0xFF3FA9F5),
      DriveMode.sport => const Color(0xFFFF6A3D),
    };

String modeName(DriveMode m) => switch (m) {
      DriveMode.eco => 'ECO',
      DriveMode.comfort => 'COMFORT',
      DriveMode.sport => 'SPORT',
    };

// ------------------------------------------------------------ simulation ---
class VehicleState extends ChangeNotifier {
  double t = 0; // seconds since start
  double speed = 0; // km/h
  double rpm = 800;
  String gear = 'P';
  int gearNo = 0;
  double fuel = 0.72; // 0..1
  double battery = 0.86; // 12 V / HV state of charge
  double coolant = 0.35; // 0..1 of the gauge
  double odometer = 18452.3;
  double trip = 0;
  double route = 0; // 0..1 along the route
  bool leftSignal = false, rightSignal = false, blink = false;
  bool headlights = true, seatbelt = true, tyreWarning = false, doorOpen = false;
  DriveMode mode = DriveMode.comfort;
  bool demo = true;
  // media
  int track = 0;
  double trackPos = 0;
  bool playing = true;
  double volume = 0.45;
  // climate
  double driverTemp = 21.5, passengerTemp = 22.0;
  int fan = 3;
  bool ac = true, recirc = false, sync = true, seatHeatL = false, seatHeatR = false;
  // tyres (bar)
  final List<double> tyres = [2.4, 2.4, 2.3, 2.4];

  // Advanced once per frame by the app's Ticker (see _HmiAppState): the
  // gauges and the map move at the display's frame rate, not a timer's.
  void advance(double dt) => _tick(dt.clamp(0.0, 0.1));

  // speed target of a repeating 60 s drive cycle
  double _target(double s) {
    final c = s % 60;
    if (c < 4) return 0; // waiting
    if (c < 14) return (c - 4) * 9; // pulling away to 90
    if (c < 30) return 90 + 8 * math.sin((c - 14) / 16 * math.pi * 2); // cruise
    if (c < 36) return 90 - (c - 30) * 6; // easing off
    if (c < 44) return 54 + 4 * math.sin(c); // town
    if (c < 52) return 54 - (c - 44) * 6.75; // braking
    return 0;
  }

  void _tick(double dt) {
    t += dt;
    final m = mode == DriveMode.sport ? 1.15 : (mode == DriveMode.eco ? 0.85 : 1.0);
    if (demo) {
      final target = _target(t) * m;
      speed += (target - speed) * math.min(1, dt * 2.2);
      final c = t % 60;
      leftSignal = c > 40 && c < 44;
      rightSignal = c > 12 && c < 15;
      tyreWarning = (t % 180) > 150;
    }
    speed = speed.clamp(0, 240);
    if (speed < 0.5) {
      gear = (t % 60) < 4 || (t % 60) > 56 ? 'P' : 'D';
      gearNo = 0;
    } else {
      gear = 'D';
      gearNo = math.min(8, 1 + (speed / 22).floor());
    }
    final ratio = speed < 0.5 ? 0.0 : (speed - (gearNo - 1) * 22) / 22;
    final shiftUp = mode == DriveMode.sport ? 2600.0 : (mode == DriveMode.eco ? 1500.0 : 2000.0);
    rpm = speed < 0.5 ? 750 + 30 * math.sin(t * 3) : 1100 + ratio * shiftUp;
    final km = speed * dt / 3600;
    odometer += km;
    trip += km;
    fuel = math.max(0.05, fuel - km * 0.0009);
    coolant += ((speed > 1 ? 0.5 : 0.42) - coolant) * dt * 0.05;
    route = (route + km / 14.0) % 1.0;
    blink = (t * 2.5).floor().isEven;
    if (playing) {
      trackPos += dt / tracks[track].seconds;
      if (trackPos >= 1) {
        trackPos = 0;
        track = (track + 1) % tracks.length;
      }
    }
    notifyListeners();
  }

  void set(void Function() f) {
    f();
    notifyListeners();
  }
}

class Track {
  final String title, artist;
  final int seconds;
  final Color c1, c2;
  const Track(this.title, this.artist, this.seconds, this.c1, this.c2);
}

const tracks = [
  Track('Night Drive', 'The Coastline', 214, Color(0xFF6A3DFF), Color(0xFFFF3D7F)),
  Track('Harbour Lights', 'Mira Vale', 187, Color(0xFF0FB5AE), Color(0xFF3D7BFF)),
  Track('Open Road', 'Northbound', 241, Color(0xFFFFB020), Color(0xFFFF5A3D)),
  Track('Quiet Hours', 'Lumen Trio', 198, Color(0xFF3DDC97), Color(0xFF1E6091)),
];

String clock(double seconds) {
  final base = DateTime(2026, 1, 1, 8, 41).add(Duration(seconds: seconds.floor()));
  return '${base.hour.toString().padLeft(2, '0')}:${base.minute.toString().padLeft(2, '0')}';
}

String mmss(double s) => '${(s ~/ 60)}:${(s.floor() % 60).toString().padLeft(2, '0')}';

// ------------------------------------------------------------------ app ----
class HmiApp extends StatefulWidget {
  const HmiApp({super.key});
  @override
  State<HmiApp> createState() => _HmiAppState();
}

class _HmiAppState extends State<HmiApp> with SingleTickerProviderStateMixin {
  final v = VehicleState();
  late final Ticker _ticker;
  Duration _last = Duration.zero;
  @override
  void initState() {
    super.initState();
    _ticker = createTicker((elapsed) {
      v.advance((elapsed - _last).inMicroseconds / 1e6);
      _last = elapsed;
    })..start();
  }

  @override
  void dispose() {
    _ticker.dispose();
    v.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'Vehicle HMI demo',
      theme: ThemeData(
        brightness: Brightness.dark,
        scaffoldBackgroundColor: _bg,
        useMaterial3: true,
        colorScheme: const ColorScheme.dark(primary: Color(0xFF3FA9F5), surface: _panel),
      ),
      home: ListenableBuilder(listenable: v, builder: (context, _) => HmiScreen(v: v)),
    );
  }
}

enum HmiPage { nav, vehicle, phone, settings }

class HmiScreen extends StatefulWidget {
  final VehicleState v;
  const HmiScreen({super.key, required this.v});
  @override
  State<HmiScreen> createState() => _HmiScreenState();
}

class _HmiScreenState extends State<HmiScreen> {
  HmiPage page = HmiPage.nav;
  bool climateOpen = true;

  @override
  Widget build(BuildContext context) {
    final v = widget.v;
    final accent = accentOf(v.mode);
    return Scaffold(
      body: LayoutBuilder(builder: (context, c) {
        // design size 1280x720, scaled to the window
        return FittedBox(
          fit: BoxFit.contain,
          child: SizedBox(
            width: 1280,
            height: 720,
            child: Column(children: [
              StatusBar(v: v, accent: accent),
              Expanded(
                child: Padding(
                  padding: const EdgeInsets.fromLTRB(12, 4, 12, 8),
                  child: Row(crossAxisAlignment: CrossAxisAlignment.stretch, children: [
                    SizedBox(width: 470, child: Cluster(v: v, accent: accent)),
                    const SizedBox(width: 10),
                    Expanded(child: _centre(v, accent)),
                    const SizedBox(width: 10),
                    SizedBox(
                      width: 300,
                      child: Column(children: [
                        Expanded(flex: 5, child: MediaCard(v: v, accent: accent)),
                        const SizedBox(height: 10),
                        Expanded(flex: 6, child: ClimateCard(v: v, accent: accent)),
                      ]),
                    ),
                  ]),
                ),
              ),
              Dock(
                page: page,
                accent: accent,
                onPage: (p) => setState(() => page = p),
                v: v,
              ),
            ]),
          ),
        );
      }),
    );
  }

  Widget _centre(VehicleState v, Color accent) {
    final child = switch (page) {
      HmiPage.nav => NavCard(v: v, accent: accent),
      HmiPage.vehicle => VehicleCard(v: v, accent: accent),
      HmiPage.phone => PhoneCard(accent: accent),
      HmiPage.settings => SettingsCard(v: v, accent: accent),
    };
    return AnimatedSwitcher(duration: const Duration(milliseconds: 250), child: KeyedSubtree(key: ValueKey(page), child: child));
  }
}

// ----------------------------------------------------------- status bar ----
class StatusBar extends StatelessWidget {
  final VehicleState v;
  final Color accent;
  const StatusBar({super.key, required this.v, required this.accent});
  @override
  Widget build(BuildContext context) {
    return Container(
      height: 44,
      padding: const EdgeInsets.symmetric(horizontal: 18),
      child: Row(children: [
        Icon(Icons.directions_car_filled, color: accent, size: 22),
        const SizedBox(width: 8),
        const Text('QNX Vehicle Demo', style: TextStyle(color: _text, fontWeight: FontWeight.w600, fontSize: 15)),
        const SizedBox(width: 18),
        _chip(modeName(v.mode), accent),
        const Spacer(),
        Text(clock(v.t), style: const TextStyle(color: _text, fontSize: 20, fontWeight: FontWeight.w500)),
        const Spacer(),
        const Icon(Icons.thermostat, color: _dim, size: 18),
        const Text(' 14°C', style: TextStyle(color: _dim, fontSize: 14)),
        const SizedBox(width: 16),
        const Icon(Icons.signal_cellular_alt, color: _dim, size: 18),
        const SizedBox(width: 10),
        const Icon(Icons.bluetooth_connected, color: _dim, size: 18),
        const SizedBox(width: 10),
        const Icon(Icons.wifi, color: _dim, size: 18),
        const SizedBox(width: 10),
        Icon(Icons.notifications, color: v.tyreWarning ? _warn : _dim, size: 18),
      ]),
    );
  }
}

Widget _chip(String s, Color c) => Container(
      padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 3),
      decoration: BoxDecoration(border: Border.all(color: c), borderRadius: BorderRadius.circular(12)),
      child: Text(s, style: TextStyle(color: c, fontSize: 12, fontWeight: FontWeight.w700, letterSpacing: 1.2)),
    );

BoxDecoration _card() => BoxDecoration(
      color: _panel,
      borderRadius: BorderRadius.circular(18),
      border: Border.all(color: const Color(0xFF222B36)),
    );

// --------------------------------------------------------------- cluster ---
class Cluster extends StatelessWidget {
  final VehicleState v;
  final Color accent;
  const Cluster({super.key, required this.v, required this.accent});
  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: _card(),
      padding: const EdgeInsets.all(12),
      child: Column(children: [
        // tell-tales
        Row(mainAxisAlignment: MainAxisAlignment.spaceBetween, children: [
          Icon(Icons.arrow_back, size: 30, color: v.leftSignal && v.blink ? _ok : const Color(0xFF26303B)),
          Row(children: [
            _tt(Icons.light_mode, v.headlights, _ok),
            _tt(Icons.airline_seat_recline_normal, !v.seatbelt, _alert),
            _tt(Icons.tire_repair, v.tyreWarning, _warn),
            _tt(Icons.sensor_door, v.doorOpen, _alert),
            _tt(Icons.ac_unit, v.ac, accent),
          ]),
          Icon(Icons.arrow_forward, size: 30, color: v.rightSignal && v.blink ? _ok : const Color(0xFF26303B)),
        ]),
        const SizedBox(height: 4),
        Expanded(
          child: Row(children: [
            Expanded(
              child: CustomPaint(
                painter: GaugePainter(
                  value: v.speed, max: 240, step: 20, label: 'km/h', accent: accent, big: true),
                child: Center(
                  child: Column(mainAxisSize: MainAxisSize.min, children: [
                    Text(v.speed.round().toString(),
                        style: const TextStyle(color: _text, fontSize: 54, fontWeight: FontWeight.w300, height: 1)),
                    const Text('km/h', style: TextStyle(color: _dim, fontSize: 13)),
                  ]),
                ),
              ),
            ),
            SizedBox(
              width: 150,
              child: CustomPaint(
                painter: GaugePainter(value: v.rpm / 1000, max: 7, step: 1, label: 'x1000 rpm', accent: _warn, big: false, red: 6),
                child: Center(
                  child: Column(mainAxisSize: MainAxisSize.min, children: [
                    Text(v.gear == 'D' && v.gearNo > 0 ? 'D${v.gearNo}' : v.gear,
                        style: TextStyle(color: accent, fontSize: 32, fontWeight: FontWeight.w600)),
                    Text('${(v.rpm).round()} rpm', style: const TextStyle(color: _dim, fontSize: 11)),
                  ]),
                ),
              ),
            ),
          ]),
        ),
        // gear strip
        Row(mainAxisAlignment: MainAxisAlignment.center, children: [
          for (final g in const ['P', 'R', 'N', 'D'])
            Padding(
              padding: const EdgeInsets.symmetric(horizontal: 10),
              child: Text(g,
                  style: TextStyle(
                      fontSize: 20,
                      fontWeight: FontWeight.w700,
                      color: v.gear == g ? accent : const Color(0xFF3A4654))),
            ),
        ]),
        const SizedBox(height: 8),
        Row(children: [
          Expanded(child: _bar(Icons.local_gas_station, v.fuel, v.fuel < 0.15 ? _alert : accent, '${(v.fuel * 620).round()} km')),
          const SizedBox(width: 12),
          Expanded(child: _bar(Icons.device_thermostat, v.coolant, v.coolant > 0.8 ? _alert : _dim, '${(60 + v.coolant * 70).round()}°C')),
          const SizedBox(width: 12),
          Expanded(child: _bar(Icons.battery_charging_full, v.battery, _ok, '${(v.battery * 100).round()}%')),
        ]),
        const SizedBox(height: 8),
        Row(mainAxisAlignment: MainAxisAlignment.spaceBetween, children: [
          Text('ODO ${v.odometer.toStringAsFixed(1)} km', style: const TextStyle(color: _dim, fontSize: 12)),
          Text('TRIP ${v.trip.toStringAsFixed(2)} km', style: const TextStyle(color: _dim, fontSize: 12)),
          Text('${(5.2 + v.speed / 60).toStringAsFixed(1)} L/100km', style: const TextStyle(color: _dim, fontSize: 12)),
        ]),
      ]),
    );
  }

  Widget _tt(IconData i, bool on, Color c) =>
      Padding(padding: const EdgeInsets.symmetric(horizontal: 4), child: Icon(i, size: 20, color: on ? c : const Color(0xFF26303B)));

  Widget _bar(IconData i, double f, Color c, String label) => Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
        Row(children: [Icon(i, size: 14, color: _dim), const SizedBox(width: 4), Text(label, style: const TextStyle(color: _dim, fontSize: 11))]),
        const SizedBox(height: 4),
        ClipRRect(
          borderRadius: BorderRadius.circular(3),
          child: LinearProgressIndicator(value: f.clamp(0, 1), minHeight: 6, color: c, backgroundColor: const Color(0xFF26303B)),
        ),
      ]);
}

class GaugePainter extends CustomPainter {
  final double value, max, step;
  final String label;
  final Color accent;
  final bool big;
  final double? red;
  GaugePainter({required this.value, required this.max, required this.step, required this.label, required this.accent, required this.big, this.red});

  static const start = math.pi * 0.75, sweep = math.pi * 1.5;

  @override
  void paint(Canvas canvas, Size size) {
    final r = math.min(size.width, size.height) / 2 - 6;
    final c = Offset(size.width / 2, size.height / 2);
    final rect = Rect.fromCircle(center: c, radius: r);
    final track = Paint()
      ..color = const Color(0xFF222B36)
      ..style = PaintingStyle.stroke
      ..strokeWidth = big ? 12 : 8
      ..strokeCap = StrokeCap.round;
    canvas.drawArc(rect, start, sweep, false, track);
    if (red != null) {
      final rs = start + sweep * (red! / max);
      canvas.drawArc(rect, rs, start + sweep - rs, false, track..color = const Color(0x55FF4D4F));
    }
    final f = (value / max).clamp(0.0, 1.0);
    final arc = Paint()
      ..shader = SweepGradient(
        startAngle: start,
        endAngle: start + sweep,
        colors: [accent.withAlpha(60), accent],
        transform: const GradientRotation(0),
      ).createShader(rect)
      ..style = PaintingStyle.stroke
      ..strokeWidth = big ? 12 : 8
      ..strokeCap = StrokeCap.round;
    canvas.drawArc(rect, start, sweep * f, false, arc);
    // ticks and numbers
    final tick = Paint()..color = const Color(0xFF6B7785)..strokeWidth = 2;
    final n = (max / step).round();
    for (var i = 0; i <= n; i++) {
      final a = start + sweep * i / n;
      final o = c + Offset(math.cos(a), math.sin(a)) * (r - (big ? 14 : 10));
      final ii = c + Offset(math.cos(a), math.sin(a)) * (r - (big ? 24 : 16));
      canvas.drawLine(ii, o, tick);
      {
        final tp = TextPainter(
          text: TextSpan(text: (i * step).round().toString(), style: TextStyle(color: _dim, fontSize: big ? 11 : 9, fontFamily: 'Roboto')),
          textDirection: TextDirection.ltr,
        )..layout();
        final p = c + Offset(math.cos(a), math.sin(a)) * (r - (big ? 38 : 27));
        tp.paint(canvas, p - Offset(tp.width / 2, tp.height / 2));
      }
    }
    // needle
    final a = start + sweep * f;
    final needle = Paint()
      ..color = _text
      ..strokeWidth = big ? 3 : 2
      ..strokeCap = StrokeCap.round;
    canvas.drawLine(c + Offset(math.cos(a), math.sin(a)) * (r * 0.55), c + Offset(math.cos(a), math.sin(a)) * (r - 4), needle);
  }

  @override
  bool shouldRepaint(GaugePainter o) => o.value != value || o.accent != accent;
}

// ------------------------------------------------------------ navigation ---
class NavCard extends StatelessWidget {
  final VehicleState v;
  final Color accent;
  const NavCard({super.key, required this.v, required this.accent});
  @override
  Widget build(BuildContext context) {
    final remainKm = (1 - v.route) * 14.0;
    final etaMin = (remainKm / math.max(30, v.speed) * 60).round();
    return Container(
      decoration: _card(),
      clipBehavior: Clip.antiAlias,
      child: Stack(children: [
        Positioned.fill(child: CustomPaint(painter: MapPainter(progress: v.route, accent: accent, t: v.t))),
        Positioned(
          left: 14,
          top: 14,
          child: Container(
            padding: const EdgeInsets.all(12),
            decoration: BoxDecoration(color: _panel2.withAlpha(235), borderRadius: BorderRadius.circular(14)),
            child: Row(children: [
              Icon(v.route % 0.25 < 0.12 ? Icons.turn_right : Icons.straight, color: accent, size: 40),
              const SizedBox(width: 10),
              Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
                Text('${(400 - (v.route * 1600) % 400).round()} m', style: const TextStyle(color: _text, fontSize: 22, fontWeight: FontWeight.w600)),
                Text(v.route % 0.25 < 0.12 ? 'Turn right onto Harbour Road' : 'Continue on Ring Road', style: const TextStyle(color: _dim, fontSize: 13)),
              ]),
            ]),
          ),
        ),
        Positioned(
          right: 14,
          top: 14,
          child: Container(
            padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
            decoration: BoxDecoration(color: _panel2.withAlpha(235), borderRadius: BorderRadius.circular(12)),
            child: Column(children: [
              const Text('LIMIT', style: TextStyle(color: _dim, fontSize: 10)),
              Container(
                width: 44,
                height: 44,
                alignment: Alignment.center,
                decoration: BoxDecoration(shape: BoxShape.circle, color: Colors.white, border: Border.all(color: _alert, width: 5)),
                child: Text(v.route % 0.5 < 0.25 ? '80' : '50', style: const TextStyle(color: Colors.black, fontWeight: FontWeight.w800, fontSize: 16)),
              ),
            ]),
          ),
        ),
        Positioned(
          left: 14,
          right: 14,
          bottom: 14,
          child: Container(
            padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 10),
            decoration: BoxDecoration(color: _panel2.withAlpha(240), borderRadius: BorderRadius.circular(14)),
            child: Row(children: [
              const Icon(Icons.flag, color: _dim),
              const SizedBox(width: 8),
              const Expanded(child: Text('Science Park, Building 3', style: TextStyle(color: _text, fontSize: 15))),
              _kv('ETA', clock(v.t + etaMin * 60)),
              const SizedBox(width: 18),
              _kv('Distance', '${remainKm.toStringAsFixed(1)} km'),
              const SizedBox(width: 18),
              _kv('Time', '$etaMin min'),
            ]),
          ),
        ),
        Positioned(
          right: 14,
          top: 100,
          child: Column(children: [
            _mapBtn(Icons.add),
            const SizedBox(height: 8),
            _mapBtn(Icons.remove),
            const SizedBox(height: 8),
            _mapBtn(Icons.my_location),
          ]),
        ),
      ]),
    );
  }

  Widget _kv(String k, String v) => Column(crossAxisAlignment: CrossAxisAlignment.end, children: [
        Text(k, style: const TextStyle(color: _dim, fontSize: 11)),
        Text(v, style: const TextStyle(color: _text, fontSize: 16, fontWeight: FontWeight.w600)),
      ]);

  Widget _mapBtn(IconData i) => Material(
        color: _panel2,
        shape: const CircleBorder(),
        child: InkWell(customBorder: const CircleBorder(), onTap: () {}, child: Padding(padding: const EdgeInsets.all(8), child: Icon(i, color: _text, size: 20))),
      );
}

class MapPainter extends CustomPainter {
  final double progress, t;
  final Color accent;
  MapPainter({required this.progress, required this.accent, required this.t});

  @override
  void paint(Canvas canvas, Size s) {
    canvas.drawRect(Offset.zero & s, Paint()..color = const Color(0xFF101820));
    // blocks
    final rnd = math.Random(7);
    final block = Paint()..color = const Color(0xFF16212C);
    for (var x = -40.0; x < s.width; x += 90) {
      for (var y = -40.0; y < s.height; y += 80) {
        final w = 50 + rnd.nextDouble() * 30, h = 40 + rnd.nextDouble() * 30;
        canvas.drawRRect(RRect.fromRectAndRadius(Rect.fromLTWH(x + 12, y + 12, w, h), const Radius.circular(6)), block);
      }
    }
    // park and water
    canvas.drawRRect(RRect.fromRectAndRadius(Rect.fromLTWH(s.width * 0.62, s.height * 0.55, 120, 90), const Radius.circular(18)),
        Paint()..color = const Color(0xFF15301F));
    final water = Path()
      ..moveTo(0, s.height * 0.78)
      ..quadraticBezierTo(s.width * 0.3, s.height * 0.70, s.width * 0.5, s.height)
      ..lineTo(0, s.height)
      ..close();
    canvas.drawPath(water, Paint()..color = const Color(0xFF0E2236));
    // roads
    final road = Paint()
      ..color = const Color(0xFF2A3644)
      ..strokeWidth = 10
      ..style = PaintingStyle.stroke;
    for (var x = 0.0; x < s.width; x += 180) {
      canvas.drawLine(Offset(x + 50, 0), Offset(x + 20, s.height), road);
    }
    for (var y = 30.0; y < s.height; y += 160) {
      canvas.drawLine(Offset(0, y), Offset(s.width, y + 30), road);
    }
    // route
    final route = Path()
      ..moveTo(s.width * 0.12, s.height * 0.92)
      ..cubicTo(s.width * 0.25, s.height * 0.6, s.width * 0.35, s.height * 0.55, s.width * 0.5, s.height * 0.5)
      ..cubicTo(s.width * 0.68, s.height * 0.44, s.width * 0.72, s.height * 0.25, s.width * 0.88, s.height * 0.12);
    canvas.drawPath(route, Paint()
      ..color = accent.withAlpha(70)
      ..strokeWidth = 16
      ..style = PaintingStyle.stroke
      ..strokeCap = StrokeCap.round);
    canvas.drawPath(route, Paint()
      ..color = accent
      ..strokeWidth = 6
      ..style = PaintingStyle.stroke
      ..strokeCap = StrokeCap.round);
    // destination
    final m = route.computeMetrics().first;
    final end = m.getTangentForOffset(m.length)!.position;
    canvas.drawCircle(end, 9, Paint()..color = _alert);
    canvas.drawCircle(end, 4, Paint()..color = Colors.white);
    // car
    final tan = m.getTangentForOffset(m.length * progress)!;
    canvas.save();
    canvas.translate(tan.position.dx, tan.position.dy);
    canvas.rotate(-tan.angle + math.pi / 2);
    canvas.drawCircle(Offset.zero, 18 + 3 * math.sin(t * 4), Paint()..color = accent.withAlpha(50));
    final car = Path()
      ..moveTo(0, -14)
      ..lineTo(10, 11)
      ..lineTo(0, 6)
      ..lineTo(-10, 11)
      ..close();
    canvas.drawPath(car, Paint()..color = Colors.white);
    canvas.restore();
  }

  @override
  bool shouldRepaint(MapPainter o) => o.progress != progress || o.accent != accent || o.t != t;
}

// ----------------------------------------------------------------- media ---
class MediaCard extends StatelessWidget {
  final VehicleState v;
  final Color accent;
  const MediaCard({super.key, required this.v, required this.accent});
  @override
  Widget build(BuildContext context) {
    final tr = tracks[v.track];
    return Container(
      decoration: _card(),
      padding: const EdgeInsets.all(14),
      child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
        Row(children: [
          Container(
            width: 64,
            height: 64,
            decoration: BoxDecoration(
              borderRadius: BorderRadius.circular(12),
              gradient: LinearGradient(colors: [tr.c1, tr.c2], begin: Alignment.topLeft, end: Alignment.bottomRight),
            ),
            child: const Icon(Icons.music_note, color: Colors.white, size: 30),
          ),
          const SizedBox(width: 12),
          Expanded(
            child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
              Text(tr.title, style: const TextStyle(color: _text, fontSize: 17, fontWeight: FontWeight.w600), overflow: TextOverflow.ellipsis),
              Text(tr.artist, style: const TextStyle(color: _dim, fontSize: 13)),
              const SizedBox(height: 4),
              Row(children: [Icon(Icons.bluetooth_audio, size: 14, color: accent), const Text(' Phone audio', style: TextStyle(color: _dim, fontSize: 11))]),
            ]),
          ),
        ]),
        const Spacer(),
        SliderTheme(
          data: SliderTheme.of(context).copyWith(trackHeight: 3, thumbShape: const RoundSliderThumbShape(enabledThumbRadius: 6), overlayShape: SliderComponentShape.noOverlay),
          child: Slider(value: v.trackPos.clamp(0, 1), activeColor: accent, inactiveColor: const Color(0xFF26303B), onChanged: (x) => v.set(() => v.trackPos = x)),
        ),
        Row(mainAxisAlignment: MainAxisAlignment.spaceBetween, children: [
          Text(mmss(v.trackPos * tr.seconds), style: const TextStyle(color: _dim, fontSize: 11)),
          Text(mmss(tr.seconds.toDouble()), style: const TextStyle(color: _dim, fontSize: 11)),
        ]),
        Row(mainAxisAlignment: MainAxisAlignment.spaceEvenly, children: [
          IconButton(icon: const Icon(Icons.shuffle, color: _dim), onPressed: () {}),
          IconButton(
              icon: const Icon(Icons.skip_previous, color: _text, size: 30),
              onPressed: () => v.set(() {
                    v.track = (v.track + tracks.length - 1) % tracks.length;
                    v.trackPos = 0;
                  })),
          IconButton.filled(
              style: IconButton.styleFrom(backgroundColor: accent),
              icon: Icon(v.playing ? Icons.pause : Icons.play_arrow, color: Colors.black, size: 30),
              onPressed: () => v.set(() => v.playing = !v.playing)),
          IconButton(
              icon: const Icon(Icons.skip_next, color: _text, size: 30),
              onPressed: () => v.set(() {
                    v.track = (v.track + 1) % tracks.length;
                    v.trackPos = 0;
                  })),
          IconButton(icon: const Icon(Icons.repeat, color: _dim), onPressed: () {}),
        ]),
        Row(children: [
          const Icon(Icons.volume_down, color: _dim, size: 18),
          Expanded(
            child: SliderTheme(
              data: SliderTheme.of(context).copyWith(trackHeight: 3, thumbShape: const RoundSliderThumbShape(enabledThumbRadius: 6), overlayShape: SliderComponentShape.noOverlay),
              child: Slider(value: v.volume, activeColor: _text, inactiveColor: const Color(0xFF26303B), onChanged: (x) => v.set(() => v.volume = x)),
            ),
          ),
          const Icon(Icons.volume_up, color: _dim, size: 18),
        ]),
      ]),
    );
  }
}

// --------------------------------------------------------------- climate ---
class ClimateCard extends StatelessWidget {
  final VehicleState v;
  final Color accent;
  const ClimateCard({super.key, required this.v, required this.accent});
  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: _card(),
      padding: const EdgeInsets.all(14),
      child: Column(children: [
        Row(children: [
          const Text('Climate', style: TextStyle(color: _text, fontSize: 16, fontWeight: FontWeight.w600)),
          const Spacer(),
          _toggle('AUTO', v.sync, () => v.set(() => v.sync = !v.sync)),
        ]),
        const SizedBox(height: 10),
        Row(mainAxisAlignment: MainAxisAlignment.spaceBetween, children: [
          _temp('Driver', v.driverTemp, (d) => v.set(() {
                v.driverTemp = (v.driverTemp + d).clamp(16, 28);
                if (v.sync) v.passengerTemp = v.driverTemp;
              })),
          _temp('Passenger', v.passengerTemp, (d) => v.set(() => v.passengerTemp = (v.passengerTemp + d).clamp(16, 28))),
        ]),
        const SizedBox(height: 12),
        Row(children: [
          const Icon(Icons.air, color: _dim, size: 20),
          Expanded(
            child: Row(mainAxisAlignment: MainAxisAlignment.spaceEvenly, children: [
              for (var i = 1; i <= 6; i++)
                GestureDetector(
                  onTap: () => v.set(() => v.fan = i),
                  child: Container(
                    width: 22,
                    height: 8.0 + i * 4,
                    decoration: BoxDecoration(color: i <= v.fan ? accent : const Color(0xFF26303B), borderRadius: BorderRadius.circular(3)),
                  ),
                ),
            ]),
          ),
        ]),
        const Spacer(),
        Wrap(spacing: 8, runSpacing: 8, alignment: WrapAlignment.center, children: [
          _iconToggle(Icons.ac_unit, 'A/C', v.ac, () => v.set(() => v.ac = !v.ac)),
          _iconToggle(Icons.loop, 'Recirc', v.recirc, () => v.set(() => v.recirc = !v.recirc)),
          _iconToggle(Icons.event_seat, 'Seat L', v.seatHeatL, () => v.set(() => v.seatHeatL = !v.seatHeatL), hot: true),
          _iconToggle(Icons.event_seat, 'Seat R', v.seatHeatR, () => v.set(() => v.seatHeatR = !v.seatHeatR), hot: true),
        ]),
      ]),
    );
  }

  Widget _temp(String who, double t, void Function(double) change) => Column(children: [
        Text(who, style: const TextStyle(color: _dim, fontSize: 11)),
        Row(children: [
          _round(Icons.remove, () => change(-0.5)),
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 6),
            child: Text('${t.toStringAsFixed(1)}°', style: const TextStyle(color: _text, fontSize: 24, fontWeight: FontWeight.w500)),
          ),
          _round(Icons.add, () => change(0.5)),
        ]),
      ]);

  Widget _round(IconData i, VoidCallback f) => Material(
        color: _panel2,
        shape: const CircleBorder(),
        child: InkWell(customBorder: const CircleBorder(), onTap: f, child: Padding(padding: const EdgeInsets.all(5), child: Icon(i, color: _text, size: 16))),
      );

  Widget _toggle(String s, bool on, VoidCallback f) => GestureDetector(
        onTap: f,
        child: Container(
          padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 4),
          decoration: BoxDecoration(color: on ? accent.withAlpha(50) : _panel2, borderRadius: BorderRadius.circular(10), border: Border.all(color: on ? accent : _panel2)),
          child: Text(s, style: TextStyle(color: on ? accent : _dim, fontSize: 11, fontWeight: FontWeight.w700)),
        ),
      );

  Widget _iconToggle(IconData i, String s, bool on, VoidCallback f, {bool hot = false}) {
    final c = hot ? const Color(0xFFFF7A45) : accent;
    return GestureDetector(
      onTap: f,
      child: Container(
        width: 58,
        padding: const EdgeInsets.symmetric(vertical: 6),
        decoration: BoxDecoration(color: on ? c.withAlpha(45) : _panel2, borderRadius: BorderRadius.circular(12), border: Border.all(color: on ? c : _panel2)),
        child: Column(children: [Icon(i, color: on ? c : _dim, size: 20), Text(s, style: TextStyle(color: on ? c : _dim, fontSize: 10))]),
      ),
    );
  }
}

// --------------------------------------------------------------- vehicle ---
class VehicleCard extends StatelessWidget {
  final VehicleState v;
  final Color accent;
  const VehicleCard({super.key, required this.v, required this.accent});
  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: _card(),
      padding: const EdgeInsets.all(16),
      child: Row(children: [
        Expanded(
          child: Stack(children: [
            Positioned.fill(child: CustomPaint(painter: CarTopPainter(accent: accent, doorOpen: v.doorOpen))),
            for (var i = 0; i < 4; i++)
              Align(
                alignment: [const Alignment(-0.95, -0.55), const Alignment(0.95, -0.55), const Alignment(-0.95, 0.6), const Alignment(0.95, 0.6)][i],
                child: _tyre(i == 2 && v.tyreWarning ? 1.8 : v.tyres[i]),
              ),
          ]),
        ),
        const SizedBox(width: 16),
        SizedBox(
          width: 200,
          child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
            const Text('Vehicle', style: TextStyle(color: _text, fontSize: 18, fontWeight: FontWeight.w600)),
            const SizedBox(height: 12),
            _row(Icons.local_gas_station, 'Range', '${(v.fuel * 620).round()} km'),
            _row(Icons.speed, 'Avg speed', '${(38 + v.speed / 10).round()} km/h'),
            _row(Icons.build, 'Service', 'in 6 200 km'),
            _row(Icons.oil_barrel, 'Oil life', '68 %'),
            const SizedBox(height: 12),
            const Text('Drive mode', style: TextStyle(color: _dim, fontSize: 12)),
            const SizedBox(height: 6),
            Row(children: [
              for (final m in DriveMode.values)
                Expanded(
                  child: Padding(
                    padding: const EdgeInsets.only(right: 4),
                    child: GestureDetector(
                      onTap: () => v.set(() => v.mode = m),
                      child: Container(
                        padding: const EdgeInsets.symmetric(vertical: 8),
                        alignment: Alignment.center,
                        decoration: BoxDecoration(
                          color: v.mode == m ? accentOf(m).withAlpha(50) : _panel2,
                          borderRadius: BorderRadius.circular(10),
                          border: Border.all(color: v.mode == m ? accentOf(m) : _panel2),
                        ),
                        child: Text(modeName(m), style: TextStyle(color: v.mode == m ? accentOf(m) : _dim, fontSize: 10, fontWeight: FontWeight.w700)),
                      ),
                    ),
                  ),
                ),
            ]),
            const Spacer(),
            Material(type: MaterialType.transparency, child: SwitchListTile(
              dense: true,
              contentPadding: EdgeInsets.zero,
              title: const Text('Simulate a door open', style: TextStyle(color: _dim, fontSize: 12)),
              value: v.doorOpen,
              activeThumbColor: _alert,
              onChanged: (b) => v.set(() => v.doorOpen = b),
            )),
          ]),
        ),
      ]),
    );
  }

  Widget _tyre(double bar) {
    final low = bar < 2.1;
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 6),
      decoration: BoxDecoration(color: low ? _warn.withAlpha(40) : _panel2, borderRadius: BorderRadius.circular(10), border: Border.all(color: low ? _warn : _panel2)),
      child: Column(mainAxisSize: MainAxisSize.min, children: [
        Text('${bar.toStringAsFixed(1)} bar', style: TextStyle(color: low ? _warn : _text, fontWeight: FontWeight.w600)),
        Text(low ? 'LOW' : 'OK', style: TextStyle(color: low ? _warn : _ok, fontSize: 10)),
      ]),
    );
  }

  Widget _row(IconData i, String k, String val) => Padding(
        padding: const EdgeInsets.symmetric(vertical: 4),
        child: Row(children: [
          Icon(i, size: 16, color: _dim),
          const SizedBox(width: 8),
          Text(k, style: const TextStyle(color: _dim, fontSize: 13)),
          const Spacer(),
          Text(val, style: const TextStyle(color: _text, fontSize: 13, fontWeight: FontWeight.w600)),
        ]),
      );
}

class CarTopPainter extends CustomPainter {
  final Color accent;
  final bool doorOpen;
  CarTopPainter({required this.accent, required this.doorOpen});
  @override
  void paint(Canvas canvas, Size s) {
    final w = math.min(s.width * 0.42, 170.0), h = math.min(s.height * 0.9, 330.0);
    final r = Rect.fromCenter(center: s.center(Offset.zero), width: w, height: h);
    final body = RRect.fromRectAndRadius(r, Radius.circular(w * 0.35));
    canvas.drawRRect(body, Paint()..color = const Color(0xFF26303B));
    canvas.drawRRect(body, Paint()
      ..color = accent.withAlpha(140)
      ..style = PaintingStyle.stroke
      ..strokeWidth = 2);
    // glass
    final glass = Paint()..color = const Color(0xFF0E2236);
    canvas.drawRRect(RRect.fromRectAndRadius(Rect.fromLTWH(r.left + w * 0.14, r.top + h * 0.18, w * 0.72, h * 0.16), const Radius.circular(10)), glass);
    canvas.drawRRect(RRect.fromRectAndRadius(Rect.fromLTWH(r.left + w * 0.16, r.top + h * 0.70, w * 0.68, h * 0.11), const Radius.circular(10)), glass);
    // wheels
    final wheel = Paint()..color = const Color(0xFF0B0F14);
    for (final dy in [0.2, 0.75]) {
      for (final dx in [-0.04, 1.0]) {
        canvas.drawRRect(RRect.fromRectAndRadius(Rect.fromLTWH(r.left + w * dx - 6, r.top + h * dy, 16, h * 0.14), const Radius.circular(5)), wheel);
      }
    }
    if (doorOpen) {
      final door = Paint()
        ..color = _alert
        ..strokeWidth = 4
        ..strokeCap = StrokeCap.round;
      canvas.drawLine(Offset(r.left, r.top + h * 0.40), Offset(r.left - w * 0.28, r.top + h * 0.52), door);
    }
  }

  @override
  bool shouldRepaint(CarTopPainter o) => o.accent != accent || o.doorOpen != doorOpen;
}

// ----------------------------------------------------------------- phone ---
class PhoneCard extends StatelessWidget {
  final Color accent;
  const PhoneCard({super.key, required this.accent});
  static const contacts = [
    ('Alex Morgan', 'Mobile'), ('Office', 'Work'), ('Jordan Lee', 'Mobile'), ('Sam Rivera', 'Home'),
    ('Taylor Kim', 'Mobile'), ('Service Centre', 'Work'), ('Chris Novak', 'Mobile'), ('Robin Park', 'Mobile'),
    ('Dana Brooks', 'Home'), ('Morgan Ellis', 'Mobile'),
  ];
  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: _card(),
      padding: const EdgeInsets.all(14),
      child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
        Row(children: [
          const Text('Phone', style: TextStyle(color: _text, fontSize: 18, fontWeight: FontWeight.w600)),
          const Spacer(),
          Icon(Icons.bluetooth_connected, color: accent, size: 18),
          const Text('  Connected phone', style: TextStyle(color: _dim, fontSize: 12)),
        ]),
        const SizedBox(height: 8),
        Expanded(
          child: Material(
           type: MaterialType.transparency,
           child: ListView.separated(
            itemCount: contacts.length,
            separatorBuilder: (_, __) => const Divider(height: 1, color: Color(0xFF222B36)),
            itemBuilder: (context, i) => ListTile(
              dense: true,
              leading: CircleAvatar(
                backgroundColor: HSLColor.fromAHSL(1, (i * 47) % 360.0, 0.45, 0.35).toColor(),
                child: Text(contacts[i].$1[0], style: const TextStyle(color: Colors.white)),
              ),
              title: Text(contacts[i].$1, style: const TextStyle(color: _text)),
              subtitle: Text(contacts[i].$2, style: const TextStyle(color: _dim, fontSize: 12)),
              trailing: IconButton(icon: Icon(Icons.call, color: accent), onPressed: () {}),
            ),
          ),
          ),
        ),
      ]),
    );
  }
}

// -------------------------------------------------------------- settings ---
class SettingsCard extends StatelessWidget {
  final VehicleState v;
  final Color accent;
  const SettingsCard({super.key, required this.v, required this.accent});
  @override
  Widget build(BuildContext context) {
    return Container(
      decoration: _card(),
      padding: const EdgeInsets.all(14),
      child: Material(type: MaterialType.transparency, child: ListView(children: [
        const Text('Settings', style: TextStyle(color: _text, fontSize: 18, fontWeight: FontWeight.w600)),
        SwitchListTile(
          title: const Text('Demo drive', style: TextStyle(color: _text)),
          subtitle: const Text('Simulated drive cycle', style: TextStyle(color: _dim)),
          value: v.demo,
          activeThumbColor: accent,
          onChanged: (b) => v.set(() => v.demo = b),
        ),
        if (!v.demo)
          ListTile(
            title: const Text('Speed', style: TextStyle(color: _text)),
            subtitle: Slider(value: v.speed.clamp(0, 200), max: 200, activeColor: accent, onChanged: (x) => v.set(() => v.speed = x)),
          ),
        SwitchListTile(
          title: const Text('Headlights', style: TextStyle(color: _text)),
          value: v.headlights,
          activeThumbColor: accent,
          onChanged: (b) => v.set(() => v.headlights = b),
        ),
        SwitchListTile(
          title: const Text('Seatbelt fastened', style: TextStyle(color: _text)),
          value: v.seatbelt,
          activeThumbColor: accent,
          onChanged: (b) => v.set(() => v.seatbelt = b),
        ),
        ListTile(
          title: const Text('Drive mode', style: TextStyle(color: _text)),
          trailing: SegmentedButton<DriveMode>(
            segments: [for (final m in DriveMode.values) ButtonSegment(value: m, label: Text(modeName(m), style: const TextStyle(fontSize: 11)))],
            selected: {v.mode},
            showSelectedIcon: false,
            onSelectionChanged: (s) => v.set(() => v.mode = s.first),
          ),
        ),
        const ListTile(
          title: Text('About', style: TextStyle(color: _text)),
          subtitle: Text('Vehicle HMI demo for QNX -- Flutter on the QNX embedder, software rendering', style: TextStyle(color: _dim)),
        ),
      ])),
    );
  }
}

// ------------------------------------------------------------------ dock ---
class Dock extends StatelessWidget {
  final HmiPage page;
  final Color accent;
  final ValueChanged<HmiPage> onPage;
  final VehicleState v;
  const Dock({super.key, required this.page, required this.accent, required this.onPage, required this.v});
  @override
  Widget build(BuildContext context) {
    Widget item(IconData i, String s, HmiPage p) {
      final on = page == p;
      return GestureDetector(
        onTap: () => onPage(p),
        child: Container(
          width: 96,
          padding: const EdgeInsets.symmetric(vertical: 6),
          decoration: BoxDecoration(color: on ? accent.withAlpha(40) : Colors.transparent, borderRadius: BorderRadius.circular(14)),
          child: Column(mainAxisSize: MainAxisSize.min, children: [
            Icon(i, color: on ? accent : _dim, size: 26),
            Text(s, style: TextStyle(color: on ? accent : _dim, fontSize: 11)),
          ]),
        ),
      );
    }

    return Container(
      height: 64,
      margin: const EdgeInsets.fromLTRB(12, 0, 12, 8),
      decoration: _card(),
      child: Row(mainAxisAlignment: MainAxisAlignment.center, children: [
        item(Icons.navigation, 'Navigation', HmiPage.nav),
        item(Icons.directions_car, 'Vehicle', HmiPage.vehicle),
        item(Icons.phone, 'Phone', HmiPage.phone),
        item(Icons.settings, 'Settings', HmiPage.settings),
        const SizedBox(width: 30),
        Text('${v.driverTemp.toStringAsFixed(1)}°', style: const TextStyle(color: _text, fontSize: 18)),
        const SizedBox(width: 10),
        Icon(Icons.air, color: v.fan > 0 ? accent : _dim),
        const SizedBox(width: 10),
        Text('${v.passengerTemp.toStringAsFixed(1)}°', style: const TextStyle(color: _text, fontSize: 18)),
      ]),
    );
  }
}
