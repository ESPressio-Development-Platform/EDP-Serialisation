#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BASE="${EDP_SERIALISATION_DEPENDENCY_ROOT:-$(cd "$ROOT/.." && pwd)}"
CLOCK="${EDP_CLOCK_SOURCE_DIR:-$BASE/EDP-Clock}"
for name in EDP-System EDP-Platform EDP-BoundedTypes EDP-BoundedTopology EDP-Memory EDP-Platform-Portable EDP-Localisation; do
    test -d "$BASE/$name/src" || { echo "Missing $name" >&2; exit 2; }
done
test -d "$CLOCK/src" || { echo "Missing EDP-Clock" >&2; exit 2; }
OUTPUT="$(mktemp)"
trap 'rm -f "$OUTPUT"' EXIT
"${CXX:-g++}" -std=c++20 -Wall -Wextra -Wpedantic -Werror \
    -I"$ROOT/src" \
    -I"$CLOCK/src" \
    -I"$BASE/EDP-System/src" \
    -I"$BASE/EDP-Platform/src" \
    -I"$BASE/EDP-BoundedTypes/src" \
    -I"$BASE/EDP-BoundedTopology/src" \
    -I"$BASE/EDP-Memory/src" \
    -I"$BASE/EDP-Platform-Portable/src" \
    -I"$BASE/EDP-Localisation/src" \
    "$ROOT/tests/host/clock_era_adapters.cpp" -o "$OUTPUT"
"$OUTPUT"
echo "Canonical optional ClockEra owner adapters: PASS"
