#!/bin/bash

# Compile and run unit tests
CFLAGS=$(pkg-config --cflags glib-2.0 gstreamer-1.0)
LIBS=$(pkg-config --libs glib-2.0 gstreamer-1.0)

echo "=== Compiling Recording State Tests ==="
gcc -Wall -Wno-error=format-security -std=c99 -I. $CFLAGS \
  test/unit/test_recording_state.c \
  src/recording/recording_state.c \
  src/utils/timing.c \
  src/utils/logging.c \
  src/utils/memory.c \
  $LIBS -lpthread -o test_recording_state 2>&1 | grep -i "error" | head -5 || echo "OK"

if [ -f test_recording_state ]; then
  echo "Running Recording State Tests..."
  ./test_recording_state
  echo "Exit code: $?"
fi
