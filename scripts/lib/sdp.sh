# sdp.sh -- which SDP release this workspace's packages are for. Sourced by
# scripts that do not source env.sh; needs WS. Sets:
#   SDP      "8.0", or the version in the workspace's .sdp ("8.0.6")
#   SDP_SUB  "" for 8.0, "/<version>" otherwise: the sub-directory of
#            tests/results and tests/logs that holds this SDP's results
# docs/sdp-versions.md.
SDP=$(sed -n 's/^version=//p' "$WS/.sdp" 2>/dev/null)
SDP=${SDP:-${QNX_OSS_SDP_VERSION:-8.0}}
if [ "$SDP" = 8.0 ]; then SDP_SUB=; else SDP_SUB=/$SDP; fi
