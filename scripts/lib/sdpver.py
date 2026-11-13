"""sdpver -- which SDP release this workspace's packages are for.

SDP is "8.0", or the version in the workspace's .sdp ("8.0.6"); SUB is "" for
8.0 and "<version>" otherwise, the sub-directory of tests/results and
tests/logs with that SDP's results (results_dir(), logs_dir()).
docs/sdp-versions.md.
"""
import os

WS = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


def _read():
    try:
        for line in open(os.path.join(WS, ".sdp")):
            if line.startswith("version="):
                return line.split("=", 1)[1].strip()
    except OSError:
        pass
    return os.environ.get("QNX_OSS_SDP_VERSION") or "8.0"


SDP = _read()
SUB = "" if SDP == "8.0" else SDP


def results_dir(rel=True):
    p = os.path.join("tests", "results", SUB) if SUB else os.path.join("tests", "results")
    return p if rel else os.path.join(WS, p)


def logs_dir(rel=True):
    p = os.path.join("tests", "logs", SUB) if SUB else os.path.join("tests", "logs")
    return p if rel else os.path.join(WS, p)
