#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/tests/.test-build"
SYSTEM_DIR="${EDP_SYSTEM_SOURCE_DIR:?EDP_SYSTEM_SOURCE_DIR must point at EDP-System}"
BOUNDED_DIR="${EDP_BOUNDED_TYPES_SOURCE_DIR:?EDP_BOUNDED_TYPES_SOURCE_DIR must point at EDP-BoundedTypes}"
PLATFORM_DIR="${EDP_PLATFORM_SOURCE_DIR:?EDP_PLATFORM_SOURCE_DIR must point at EDP-Platform}"
MEMORY_DIR="${EDP_MEMORY_SOURCE_DIR:?EDP_MEMORY_SOURCE_DIR must point at EDP-Memory}"
TOPOLOGY_DIR="${EDP_BOUNDED_TOPOLOGY_SOURCE_DIR:?EDP_BOUNDED_TOPOLOGY_SOURCE_DIR must point at EDP-BoundedTopology}"
PORTABLE_DIR="${EDP_PLATFORM_PORTABLE_SOURCE_DIR:?EDP_PLATFORM_PORTABLE_SOURCE_DIR must point at EDP-Platform-Portable}"

rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

CXX="${CXX:-g++}"
COMMON_FLAGS=(
    -std=c++20
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -I"${ROOT_DIR}/src"
    -I"${SYSTEM_DIR}/src"
    -I"${BOUNDED_DIR}/src"
    -I"${PLATFORM_DIR}/src"
    -I"${MEMORY_DIR}/src"
    -I"${TOPOLOGY_DIR}/src"
    -I"${PORTABLE_DIR}/src"
)

SANITIZER_MODE="${EDP_SERIALISATION_SANITIZER_MODE:-}"
EXTRA_FLAGS=()
if [[ "${SANITIZER_MODE}" == "undefined" ]]; then
    EXTRA_FLAGS=(-g -fno-omit-frame-pointer -fsanitize=undefined)
elif [[ "${SANITIZER_MODE}" == "address" ]]; then
    EXTRA_FLAGS=(-g -fno-omit-frame-pointer -fsanitize=address)
elif [[ -n "${SANITIZER_MODE}" ]]; then
    echo "Unsupported EDP_SERIALISATION_SANITIZER_MODE: ${SANITIZER_MODE}" >&2
    exit 2
fi

"${CXX}" "${COMMON_FLAGS[@]}" "${EXTRA_FLAGS[@]}" "${ROOT_DIR}/tests/host/main.cpp" -o "${BUILD_DIR}/host-tests"
ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0:abort_on_error=1}" \
UBSAN_OPTIONS="${UBSAN_OPTIONS:-halt_on_error=1:print_stacktrace=1}" \
    "${BUILD_DIR}/host-tests"

for source in "${ROOT_DIR}"/tests/compile_fail/*.cpp; do
    name="$(basename "${source}" .cpp)"
    if "${CXX}" "${COMMON_FLAGS[@]}" "${source}" -o "${BUILD_DIR}/${name}" >"${BUILD_DIR}/${name}.log" 2>&1; then
        echo "ERROR: compile-fail test unexpectedly succeeded: ${name}" >&2
        exit 1
    fi
    echo "Expected compile failure confirmed: ${name}"
done

echo "All EDP-Serialisation host tests passed."
