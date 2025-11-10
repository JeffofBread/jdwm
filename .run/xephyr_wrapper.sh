#!/usr/bin/env bash

# Used by jetbrains as an external tool script.
# Intended to be used with `jdwm.run.xml`.

# To set it up, add a new external tool, set the executable to this script,
# disable file sync, and disable tool output, and finally add it to the
# "Before Launch" section of the jdwm build rule.

Xephyr :1 -br -ac -noreset -screen 1920x1080 -terminate & sleep 1
