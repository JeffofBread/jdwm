#!/bin/sh

# Apps that persist through a dwm restart can be added here
picom &

# Fix some java apps not liking dwm
export _JAVA_AWT_WM_NONREPARENTING=1
export AWT_TOOLKIT=MToolkit
wmname LG3D

while type dwm >/dev/null; do dwm && continue || break; done
dwm