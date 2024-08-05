#!/usr/bin/env bash

## Author : Aditya Shakya (adi1090x)
## Github : @adi1090x
#
## Rofi   : Power Menu
#
## Heavily modified for use in jdwm


# CMDs
uptime="`uptime -p | sed -e 's/up //g'`"

# Options
shutdown='⏻  Shutdown'
reboot='↺  Reboot'
lock='🖵  Lock'
suspend='⏾  Suspend'
logout='⏼   Logout'
yes='✓  Yes'
no='✗  No'

theme=$1

# Rofi CMD
rofi_cmd() {
	rofi -dmenu -i -p "Uptime: $uptime" -theme $theme
}

# Confirmation CMD
confirm_cmd() {
	rofi -dmenu -i -p 'Are you Sure?' -theme $theme
}

# Ask for confirmation
confirm_exit() {
	echo -e "$yes\n$no" | confirm_cmd
}

# Pass variables to rofi dmenu
run_rofi() {
	echo -e "$lock\n$suspend\n$logout\n$reboot\n$shutdown" | rofi_cmd
}

# Execute Command
run_cmd() {
	selected="$(confirm_exit)"
	if [[ "$selected" == "$yes" ]]; then
		if [[ $1 == '--shutdown' ]]; then
			systemctl poweroff
		elif [[ $1 == '--reboot' ]]; then
			systemctl reboot
		elif [[ $1 == '--suspend' ]]; then
			playerctl pause
			pactl set-sink-mute @DEFAULT_SINK@ 0
			systemctl suspend
		elif [[ $1 == '--logout' ]]; then
            jdwm-msg run_command quit
		fi
	else
		exit 0
	fi
}

# Actions
chosen="$(run_rofi)"
case ${chosen} in
    $shutdown)
		run_cmd --shutdown
        ;;
    $reboot)
		run_cmd --reboot
        ;;
    $lock)
	    if [[ -x '/usr/bin/betterlockscreen' ]]; then
		    betterlockscreen -l
        else
            echo -e "Ok, exit" | rofi -dmenu -p "Error: " -mesg "Could not find a lock screen tool! Check jdwm/rofi/scripts/rofi_powermenu.sh for what lock screens were tried." -theme $theme
		fi
        ;;
    $suspend)
		run_cmd --suspend
        ;;
    $logout)
		run_cmd --logout
        ;;
esac