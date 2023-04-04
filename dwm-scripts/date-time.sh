#!/bin/sh
while :; do
    xsetroot -name "$(acpi -b) | $(date '+%b %d (%a), %Y | %H:%M') "
    sleep 10s
done