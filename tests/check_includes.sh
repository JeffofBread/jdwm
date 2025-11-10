#!/bin/sh
set -e  # exit if any command fails

SCRIPT_DIR=$(dirname "$0")

ROOT_DIRECTORY="$SCRIPT_DIR"/../
BUILD_DIRECTORY="$ROOT_DIRECTORY"/build
SOURCE_DIRECTORY="$ROOT_DIRECTORY"/source

if [ ! -d "$BUILD_DIRECTORY" ]; then
    meson setup --buildtype=debug "$BUILD_DIRECTORY"
fi

meson compile -C "$BUILD_DIRECTORY"

iwyu_tool -p "$BUILD_DIRECTORY" "$SOURCE_DIRECTORY"