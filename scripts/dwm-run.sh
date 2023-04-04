#!/bin/sh

pasystray &
~/.local/share/JetBrains/Toolbox/bin/jetbrains-toolbox --minimize &
clipit &
blueman-applet &
nm-applet &
flameshot &
solaar -w hide &
/usr/lib/kdeconnectd &
picom &

# while type dwm >/dev/null; do dwm && continue || break; done
dwm