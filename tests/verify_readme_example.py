#!/usr/bin/env python3

from pathlib import Path
import sys

REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
README_PATH = REPOSITORY_ROOT / "README.MD"
SOURCE_PATH = REPOSITORY_ROOT / "examples/json-numeric-round-trip/Portable_CMake/main.cpp"
ARDUINO_IDE_PATH = REPOSITORY_ROOT / "examples/json-numeric-round-trip/Arduino_IDE/json_numeric_round_trip.ino"
PIOARDUINO_PATH = REPOSITORY_ROOT / "examples/json-numeric-round-trip/PlatformIO_Arduino/src/main.cpp"
BEGIN_MARKER = "<!-- JSON_NUMERIC_ROUND_TRIP_SOURCE_BEGIN -->\n```cpp\n"
END_MARKER = "\n```\n<!-- JSON_NUMERIC_ROUND_TRIP_SOURCE_END -->"


def fail(message: str) -> None:
    """Reports a README example validation failure and exits unsuccessfully."""
    print(f"README example validation failed: {message}", file=sys.stderr)
    raise SystemExit(1)


readme = README_PATH.read_text()
source = SOURCE_PATH.read_text().rstrip("\n")

if readme.count(BEGIN_MARKER) != 1:
    fail("expected exactly one source begin marker")
if readme.count(END_MARKER) != 1:
    fail("expected exactly one source end marker")
start = readme.index(BEGIN_MARKER) + len(BEGIN_MARKER)
end = readme.index(END_MARKER, start)
embedded_source = readme[start:end]

if embedded_source != source:
    fail(
        "the complete README source block differs from "
        "examples/json-numeric-round-trip/Portable_CMake/main.cpp"
    )

if ARDUINO_IDE_PATH.read_text() != PIOARDUINO_PATH.read_text():
    fail(
        "the Arduino IDE source differs from the PIOArduino Arduino source "
        "used by executable validation"
    )

required_output = '{"2":"sensor-A","5":2350}'
if required_output not in readme:
    fail("the documented canonical JSON output is missing")

print("README JSON Numeric round-trip source matches the runnable example.")
