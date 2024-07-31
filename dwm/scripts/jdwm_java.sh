#!/bin/sh

# I will likely remove this file in the future, temporary fix to execvp not working with it properly

# Fix some java apps not liking dwm
export _JAVA_AWT_WM_NONREPARENTING=1
export AWT_TOOLKIT=MToolkit