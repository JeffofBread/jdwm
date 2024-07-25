#!/bin/sh

# Simple clang format command, uses .clang-format for style.
# Mainly here for easier binding and just to remember the exact command.
# I dont recommend using this on any of your configs, as it will often
# destroy the readability.
clang-format -i --style=file dwmsrc/*.c dwmsrc/*.h dwmblocks/*.c