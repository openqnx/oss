#!/bin/sh
# mlpack's command-line programs on CSV files with known answers:
# mlpack_linear_regression trains on y = 1 + 2 x1 + 3 x2, saves the model,
# and a second run loads it and predicts exact values; mlpack_kmeans puts
# two separated point groups in two clusters and writes centroids at their
# centres; mlpack_knn finds the nearest grid points; --version and --help.
# Run the test itself in the background, its output to a log, and wait at
# most 170 s: nothing it leaves behind can hold the runner's output, and a
# hang ends as a FAIL with whatever the test printed so far.
if [ -z "$QNXPORT_INNER" ]; then
  QNXPORT_INNER=/tmp/qnxport-out.$$; export QNXPORT_INNER
  sh "$0" "$@" > "$QNXPORT_INNER" 2>&1 < /dev/null &
  qp=$!; qi=0
  while kill -0 $qp 2>/dev/null && [ $qi -lt 170 ]; do sleep 1; qi=$((qi+1)); done
  if kill -0 $qp 2>/dev/null; then
    for p in $(cat "$QNXPORT_INNER.pids" 2>/dev/null); do kill $p 2>/dev/null; done
    kill $qp 2>/dev/null; sleep 1; kill -KILL $qp 2>/dev/null
    cat "$QNXPORT_INNER"; echo "  FAIL watchdog: still running after 170 s -- stopped"
    echo "FAIL ${0##*/} (watchdog)"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit 1
  fi
  wait $qp; rc=$?; cat "$QNXPORT_INNER"; rm -f "$QNXPORT_INNER" "$QNXPORT_INNER.pids"; exit $rc
fi
N=toybox
N=mlpack_cli
echo "== $N =="
fail=0
ok() { echo "  ok   $1"; }; bad() { echo "  FAIL $1"; fail=$((fail+1)); }; info() { echo "  info $1"; }
L=/usr/local; [ -d "$L/bin" ] || L=/data/oss/usr/local
T=$(mktemp -d /tmp/qnxport-$N.XXXXXX) && cd "$T" || { echo "FAIL $N (no temp dir)"; exit 1; }
trap 'cd /; rm -rf "$T"' EXIT
to() { if command -v timeout >/dev/null 2>&1; then timeout "$@"; else shift; "$@"; fi; }
B=$L/bin

r=$(to 60 $B/mlpack_linear_regression --version 2>&1 | head -n 1)
case "$r" in *mlpack*4.8.0*) ok "--version: $r" ;; *) bad "--version: $r" ;; esac
to 60 $B/mlpack_kmeans --help 2>&1 | grep -q -- "--clusters" && ok "mlpack_kmeans --help lists --clusters" || bad "kmeans --help"

# linear regression: 20 points on y = 1 + 2 x1 + 3 x2
awk 'BEGIN { for (i = 0; i < 20; i++) { a = i % 5; b = int(i / 5) + 0.5 * (i % 3); printf "%g,%g\n", a, b } }' > x.csv
awk -F, '{ printf "%g\n", 1 + 2 * $1 + 3 * $2 }' x.csv > y.csv
printf '0,0\n1,1\n10,-2\n' > t.csv
to 60 $B/mlpack_linear_regression --training_file x.csv --training_responses_file y.csv --output_model_file lr.bin > lr.log 2>&1 \
  || to 60 $B/mlpack_linear_regression --training_file x.csv --training_responses y.csv --output_model_file lr.bin > lr.log 2>&1
[ -s lr.bin ] && ok "linear regression trained, model saved" || { bad "linear regression training"; cat lr.log; }
to 60 $B/mlpack_linear_regression --input_model_file lr.bin --test_file t.csv --output_predictions_file p.csv > lp.log 2>&1
r=$(awk '{ printf "%.6f ", $1 }' p.csv 2>/dev/null)
[ "$r" = "1.000000 6.000000 15.000000 " ] && ok "model loaded, predictions 1 6 15" || { bad "predictions: $r"; cat lp.log; }

# k-means: two groups around (0,0) and (20,20)
awk 'BEGIN { for (i = 0; i < 30; i++) { d = (i % 5) * 0.1; c = (i < 15) ? 0 : 20; printf "%g,%g\n", c + d, c - d } }' > k.csv
to 120 $B/mlpack_kmeans --input_file k.csv --clusters 2 --output_file ko.csv --centroid_file kc.csv --seed 1 > km.log 2>&1
r=$(awk -F, 'NR <= 15 { a[$3]++ } NR > 15 { b[$3]++ } END { printf "%d %d", length(a), length(b) }' ko.csv 2>/dev/null)
s=$(awk -F, 'NR == 1 { f = $3 } NR == 16 { g = $3 } END { print (f != g) }' ko.csv 2>/dev/null)
[ "$r" = "1 1" ] && [ "$s" = 1 ] && ok "k-means: each group in one cluster, two clusters" || { bad "k-means assignments: $r/$s"; cat km.log; }
r=$(awk -F, '{ printf "%.2f,%.2f;", $1, $2 }' kc.csv 2>/dev/null)
case "$r" in "0.20,-0.20;20.20,19.80;"|"20.20,19.80;0.20,-0.20;") ok "centroids $r" ;; *) bad "centroids: $r" ;; esac

# kNN: 3 nearest of (2.1, 2) on a 5x5 grid
awk 'BEGIN { for (i = 0; i < 25; i++) printf "%d,%d\n", i % 5, int(i / 5) }' > g.csv
printf '2.1,2\n' > q.csv
to 60 $B/mlpack_knn --reference_file g.csv --query_file q.csv --k 2 --neighbors_file n.csv --distances_file d.csv > kn.log 2>&1
r=$(cat n.csv 2>/dev/null); d=$(awk -F, '{ printf "%.3f,%.3f", $1, $2 }' d.csv 2>/dev/null)
[ "$r" = "12,13" ] && [ "$d" = "0.100,0.900" ] && ok "kNN: neighbours 12,13 at 0.1, 0.9" || { bad "kNN: $r / $d"; cat kn.log; }

[ $fail -eq 0 ] && { echo "PASS $N"; exit 0; }
echo "FAIL $N ($fail check(s) failed)"; exit 1
