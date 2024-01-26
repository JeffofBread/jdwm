#!/bin/sh
# Thanks to mateusz.kita on discord for helping me with some of the bash/rofi here

GET_LAYOUTS_CMD="dwm-msg get_layouts"
LAYOUTS_JSON=$($GET_LAYOUTS_CMD)

if jq -V ; then
    ARRAY_LENGTH=$($GET_LAYOUTS_CMD | jq -r 'length')
else
    rofi -e "Please install or verify that jq is working." 
    exit 0
fi

main() {
    for ((i = 0; i < ARRAY_LENGTH; i++ )); do
        LAYOUT_ADDRESS[i]=$(echo "$LAYOUTS_JSON" | jq -r --argjson val $i '.[$val] | .address')
        LAYOUT_NAME[i]=$(echo "$LAYOUTS_JSON" | jq -r --argjson val $i '.[$val] | .name')
        LAYOUT_SYMBOL[i]=$(echo "$LAYOUTS_JSON" | jq -r --argjson val $i '.[$val] | .symbol')
    done
}

main

choice=$(
    for ((snidx = 0; snidx < ARRAY_LENGTH; snidx++)); do
        echo "${LAYOUT_SYMBOL[snidx]} | ${LAYOUT_NAME[snidx]}"
    done  | rofi -no-fixed-num-lines -format 'i' -dmenu -i -fuzzy -p "Layout"
);

dwm-msg run_command setlayoutsafe $((${LAYOUT_ADDRESS[$choice]}))