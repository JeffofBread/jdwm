#!/bin/sh

SCRIPT_DIR=$(dirname "$0")

ROOT_DIRECTORY="$SCRIPT_DIR"/..
BUILD_DIRECTORY="$ROOT_DIRECTORY"/build

if [ ! -d "$BUILD_DIRECTORY" ]; then
    meson setup --buildtype=debug "$BUILD_DIRECTORY"
fi

meson compile -C "$BUILD_DIRECTORY" || exit 1

# XEPHYR_TEST_COMMAND=""
XEPHYR_TEST_COMMAND="valgrind --leak-check=full --track-origins=yes"
# XEPHYR_TEST_COMMAND="heaptrack"
# XEPHYR_TEST_COMMAND="perf record"

Xephyr +xinerama -br -ac -noreset -screen 1920x1080 :1 &
XEPHYR_PID=$!
sleep 1
DISPLAY=:1 alacritty --hold -e $XEPHYR_TEST_COMMAND "$BUILD_DIRECTORY"/jdwm -N -L 1 -C "$ROOT_DIRECTORY"/resources/jdwm.conf
kill $XEPHYR_PID
