#!/bin/sh

# Variable to hold themes path
JDWM_USER_CONFIG_DIR=$HOME/.config/jdwm
JDWM_DIR=/usr/local/share/jdwm
DWM_DIR=$JDWM_DIR/dwm
THEMES_DIR=$DWM_DIR/themes
CONFIG_DIR=$DWM_DIR/config
ALIAS_FOUND=0;


if [ -f "$JDWM_USER_CONFIG_DIR/jdwm.aliases" ]; then
    shopt -s expand_aliases
    source $JDWM_USER_CONFIG_DIR/jdwm.aliases
    ALIAS_FOUND=1
fi

# Array of theme filenames
themefilenames=( $(cd $THEMES_DIR && ls *.h | grep -v ".def.h") )

# Check for theme header files
if [[ ! ${#themefilenames[@]} -gt 0 ]]; then
    echo -e "Ok" | rofi -dmenu -p "Error: " -mesg "Could not find any theme header files in $THEMES_DIR. Exiting script..." -theme $1
    exit 1
fi

# Create and populate array with nice formatted names of themes,
# defined on the first line of each file.
themenames=()
for (( i=0; i < ${#themefilenames[@]}; i++ )); do
    tmp=$(sed '1q;d' $THEMES_DIR/${themefilenames[$i]})
    themenames[$i]=${tmp:3}
done

# Rofi chooser using all the formatted theme names
choice=$(
    for (( i=0; i < ${#themefilenames[@]}; i++ )); do 
        echo ${themenames[i]}
    done | rofi -dmenu -format 'i' -fuzzy -p 'Theme' -theme $1
)

if $choice &>/dev/null; then
    exit 0
fi

# Check that config.h exists, and go through steps to make it if not
if [ ! -f "$CONFIG_DIR/config.h" ]; then
    choiceyn="$(echo -e "Yes\nNo" | rofi -dmenu -p "Error: " -mesg "$CONFIG_DIR/config.h does not exist. Would you like to copy $CONFIG_DIR/config.def.h?" -theme $1)"
    if [[ "$choiceyn" == "Yes" ]]; then
        if [ ! -f "$CONFIG_DIR/config.def.h" ]; then
            echo -e "Ok" | rofi -dmenu -p "Error: " -mesg "$CONFIG_DIR/config.def.h does not exist, nothing to copy" -theme $1
            exit 1
        else
            cp $CONFIG_DIR/config.def.h $CONFIG_DIR/config.h
        fi
    else
        echo -e "Ok" | rofi -dmenu -p "Error: " -mesg "$CONFIG_DIR/config.h not being copied. Exiting script..." -theme $1
        exit 0
    fi
fi

# Finds the line in config.h that has `JDWM_THEME`, which is inline with the include.
# Though this isn't the best way, it was far simpler than other solutions and not horribly inconvenient.
sed -i "/JDWM_THEME/c\#include <${themefilenames[$choice]}> \\/\\/ JDWM_THEME" $CONFIG_DIR/config.h

RECOMPILE_TERM

kill -HUP $(pidof jdwm)