#!/bin/sh

# Fix some java apps not liking dwm
export _JAVA_AWT_WM_NONREPARENTING=1
export AWT_TOOLKIT=MToolkit

dwm --first-run