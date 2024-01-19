#!/bin/sh

# This is a temporary script to make my life easier debugging dwm, will eventually be removed

make
Xephyr -ac -screen 1920x1080 -br -reset -terminate 2> /dev/null :1 &
DISPLAY=:1 gdb -q -ex run --args ./dwmsrc/build/dwm --first-run &