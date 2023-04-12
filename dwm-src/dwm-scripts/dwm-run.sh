#!/bin/sh

# Apps that persist through a dwm restart can be added here
picom &
dwmblocks &

# Fix some java apps not liking dwm
export _JAVA_AWT_WM_NONREPARENTING=1
export AWT_TOOLKIT=MToolkit
wmname LG3D

# Area to add personal run options

# My Desktop Settings:
# xrandr --output DP-0 --off --output DP-1 --off --output DP-2 --off --output DP-3 --off --output HDMI-0 --off --output HDMI-1 --mode 3440x1440 --rate 60.00 --pos 4480x0 --rotate normal --output HDMI-2 --mode 1920x1080 --rate 75.00 --pos 0x180 --rotate normal --output DP-4 --primary --mode 2560x1440 --rate 240.00 --pos 1920x0 --rotate normal --output DP-5 --off &
# feh --bg-scale ~/Pictures/Wallpapers/main.jpg ~/Pictures/Wallpapers/ultrawide.jpg ~/Pictures/Wallpapers/main.jpg &

# My Laptop Settings
# xrandr --output eDP-1 --primary --mode 1920x1080 --rate 60.00 --pos 0x0 --rotate normal --output HDMI-1 --off

# while type dwm >/dev/null; do dwm && continue || break; done
dwm