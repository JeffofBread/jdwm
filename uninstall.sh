#!/bin/sh

# Check script is run as non root to get correct paths
if ! [ $(id -u) != 0 ]; then
    echo "The script need to be run as non-root to start."
    exit 1
fi

########################################################

PARENT_DIR=$(pwd)
BIN_INSTALL_DIR="/usr/local/bin"
SHARE_DIR="/usr/local/share"
USERS_DIR=$HOME

########################################################

ROFI_DIR="$PARENT_DIR/rofi"
ROFI_SCRIPTS_DIR="$ROFI_DIR/scripts"
ROFI_THEMES_DIR="$ROFI_DIR/themes"
ROFI_CONFIG_INSTALL_DIR="$USERS_DIR/.config/rofi"
ROFI_THEMES_INSTALL_DIR="$ROFI_CONFIG_INSTALL_DIR/themes"

########################################################

JEFF_DWM_VERSION=$(jeff_dwm -v)

JEFF_DWM_DIR="$PARENT_DIR/dwm"
JEFF_DWM_CONFIG_DIR="$JEFF_DWM_DIR/config"
JEFF_DWM_USER_CONFIG_DIR="$USERS_DIR/.config/jeff_dwm"
JEFF_DWM_WALLPAPER_DIR="$JEFF_DWM_USER_CONFIG_DIR/wallpapers"
JEFF_DWM_THEMES_DIR="$JEFF_DWM_DIR/themes"
JEFF_DWM_SCRIPTS_DIR="$JEFF_DWM_DIR/scripts"

########################################################

DWM_BLOCKS_DIR="$PARENT_DIR/dwmblocks"
DWM_BLOCKS_SCRIPTS_DIR="$DWM_BLOCKS_DIR/scripts"

########################################################

DWM_MAN_INSTALL_DIR=""
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    DWM_MAN_INSTALL_DIR="$SHARE_DIR/man"
elif [[ "$OSTYPE" == "openbsd"* ]]; then
    DWM_MAN_INSTALL_DIR="/usr/local/man"
fi

########################################################

file_uninstaller(){
    files=( $(cd $2 && ls *.$1) )
    if [[ $5 -eq 1 ]]; then
        sudo -v
    fi
    echo -e "$4 files being removed:\n"
    for (( i=0; i < ${#files[@]}; i++ )); do
        echo "${files[$i]}" | sed -e 's/^/     #) /'
        sudo rm -f $3/${files[$i]}
    done
}

jeff_dwm_binaries_uninstall(){
    sudo make uninstall
}

jeff_dwm_rm_config_dir(){
    echo "Removing $JEFF_DWM_CONFIG_DIR/"
    rm -rf $JEFF_DWM_CONFIG_DIR 
}

jeff_dwm_rm_pathing_symlinks(){
    echo "Removing various pathing symlinks"
    sudo rm -f $JEFF_DWM_CONFIG_DIR/jeffdwmconfigdir $JEFF_DWM_THEMES_DIR/wallpapers $ROFI_DIR/roficonfigdir $SHARE_DIR/jeff_dwm

}

jeff_dwm_configs_unlink(){
    echo "Removing symlinks to autorun.h, binds.h, config.h, keydefs.h, and blocks.h from $JEFF_DWM_USER_CONFIG_DIR"
    rm -f $JEFF_DWM_USER_CONFIG_DIR/autorun.h $JEFF_DWM_USER_CONFIG_DIR/binds.h $JEFF_DWM_USER_CONFIG_DIR/config.h $JEFF_DWM_USER_CONFIG_DIR/keydefs.h $JEFF_DWM_USER_CONFIG_DIR/blocks.h
}

jeff_dwm_scripts_uninstall(){
    file_uninstaller "sh" "$JEFF_DWM_SCRIPTS_DIR" "$BIN_INSTALL_DIR" "jeff_dwm script" "1"
}

jeff_dwm_desktop_file_uninstall(){
    echo "Removing jeff_dwm desktop file: /usr/share/xsessions/jeff_dwm.desktop"
    sudo rm -f /usr/share/xsessions/jeff_dwm.desktop
}

jeff_dwm_wallpapers_uninstall(){
    echo "Removing jeff_dwm wallpapers directory: $JEFF_DWM_WALLPAPER_DIR"
}

jeff_dwm_aliases_uninstall(){
    echo "Removing jeff_dwm's alias file: $JEFF_DWM_USER_CONFIG_DIR/jeff_dwm.aliases"
    sudo rm -f $JEFF_DWM_USER_CONFIG_DIR/jeff_dwm.aliases
}

jeff_dwm_man_page_uninstall(){
    echo "Removing jeff_dwm's manual: $DWM_MAN_INSTALL_DIR/man1/jeff_dwm.1"
    sudo rm -f $DWM_MAN_INSTALL_DIR/man1/jeff_dwm.1
}

dwmblocks_scripts_uninstall(){
    file_uninstaller "sh" "$DWM_BLOCKS_SCRIPTS_DIR" "$BIN_INSTALL_DIR" "dwmblocks script" "1"
}

rofi_config_uninstall(){
    echo "Removing jeff_dwm's rofi config file: $ROFI_CONFIG_INSTALL_DIR/config.rasi"
    rm -f $ROFI_CONFIG_INSTALL_DIR/config.rasi
}

rofi_theme_uninstall(){
    file_uninstaller "rasi" "$ROFI_THEMES_DIR" "$ROFI_THEMES_INSTALL_DIR" "jeff_dwm rofi theme" "0"
}

rofi_scripts_uninstall(){
    file_uninstaller "sh" "$ROFI_SCRIPTS_DIR" "$BIN_INSTALL_DIR" "rofi script" "1"
}

print_help(){
    print_usage
    echo "This uninstall script is designed to help uninstall jeff_dwm and any and"
    echo "all other files used by or along side of it. However, it DOES NOT remove"
    echo "any files you installed via a package manager and will not remove most"
    echo "files you might have added yourself. Exceptions include: any files with a"
    echo "'.sh' extension you put in any of the '/scripts' folders, and themes you"
    echo "added to '/rofi/themes' with a '.rasi' extension. All those files WILL be"
    echo "removed, as they are considered to have been apart of jeff_dwm's install."
    echo "Refer to the script flags to see ways to avoid uninstalling specific sets"
    echo "of scripts or themes."
    echo ""
    echo "WARNING: By default (no flags) all files related to jeff_dwm will be removed."
    echo "Please make sure you understand what will be removed, this cannot be un-done."
    echo ""
    echo "Flags (can use one or more, no values necessary):"
    echo ""
    echo "   -h,  --help                        Prints this help and usage message"
    echo ""
    echo "   -u,  --usage                       Prints jeff_dwm's usage help message"
    echo ""
    echo "   -v,  --version                     Prints jeff_dwm's version, same as"
    echo "                                      'jeff_dwm -v'"
    echo ""
    echo "   -ja, --jeff-dwm-aliases            Removes jeff_dwm alias file from"
    echo "                                      ~/.config/jeff_dwm/"
    echo ""
    echo "   -jb, --jeff-dwm-binaries           Uninstalls all jeff_dwm related"
    echo "                                      binaries from /usr/local/bin/"
    echo ""
    echo "   -jc, --jeff-dwm-config-dir         Removes jeff_dwm's config directory:"
    echo "                                      ~/.config/jeff_dwm/"
    echo ""
    echo "   -jd, --jeff-desktop-file           Removes jeff_dwm's desktop file from"
    echo "                                      /usr/share/xsessions/"
    echo ""
    echo "   -jm, --jeff-dwm-manual             Removes jeff_dwn's manual file"
    echo "                                      from $DWM_MAN_INSTALL_DIR/man1/"
    echo ""
    echo "   -jp, --jeff-dwm-pathing-symlinks   Removes jeff_dwm's pathing symlinks"
    echo "                                      used to help it path to needed files"
    echo ""
    echo "   -js, --jeff-dwm-scripts            Uninstalls jeff_dwm's script files,"
    echo "                                      meaning any .sh files found in both"
    echo "                                      /jeff_dwm/dwm/scripts/ and"
    echo "                                      /usr/local/bin/"
    echo ""
    echo "   -ju, --jeff-dwm-config-unlink      Removes symlinks linking config"
    echo "                                      header files in /jeff_dwm/dwm/config/"
    echo "                                      to ~/.config/jeff_dwm/"
    echo ""
    echo "   -jw, --jeff-dwm-wallpapers         Removes jeff_dwm's wallpapers directory"
    echo "                                      in ~/.config/jeff_dwm and symlink to it"
    echo "                                      in /jeff_dwm/dwm/themes/"
    echo ""
    echo "   -bs, --dwmblocks-scripts           Uninstalls jeff_dwm's dwmblocks"
    echo "                                      scripts, meaning any .sh files found"
    echo "                                      in both /jeff_dwm/dwmblocks/scripts/"
    echo "                                      and /usr/local/bin/"
    echo ""
    echo "   -rc, --rofi-config                 Removes jeff_dwm's rofi config file:"
    echo "                                      ~/.config/rofi/config.rasi"
    echo ""
    echo "   -rs, --rofi-scripts                Uninstalls jeff_dwm's rofi scripts,"
    echo "                                      meaning any .sh files found in both"
    echo "                                      /jeff_dwm/rofi/scripts/ and"
    echo "                                      /usr/local/bin/"
    echo ""
    echo "   -rt, --rofi-themes                 Removes jeff_dwm's rofi themes,"
    echo "                                      meaning any .rasi files found in"
    echo "                                      both /jeff_dwm/rofi/themes/ and"
    echo "                                      ~/.config/rofi/themes/"
    echo ""
}

print_usage(){
    echo ""
    echo "Usage: uninstall.sh [-h] [--help] [-u] [--usage] [-v] [--version]"
    echo "       [-ja] [--jeff-dwm-aliases] [-jb] [--jeff-dwm-binaries]"
    echo "       [-jc] [--jeff-dwm-config-dir] [-jd] [--jeff-desktop-file]"
    echo "       [-jm] [--jeff-dwm-manual] [-jp] [--jeff-dwm-pathing-symlinks]"
    echo "       [-js] [--jeff-dwm-scripts] [-ju] [--jeff-dwm-config-unlink]"
    echo "       [-jw] [--jeff-dwm-wallpapers] [-bs] [--dwmblocks-scripts]"
    echo "       [-rc] [--rofi-config] [-rs] [--rofi-scripts] [-rt] [--rofi-themes]"
    echo "" 
}

########################################################

DEFAULT_UNINSTALL=1

POSITIONAL_ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -ja|--jeff-dwm-aliases)  # Only removes the custom alias file
            jeff_dwm_aliases_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -jb|--jeff-dwm-binaries)  # Only uninstall jeff_dwm binaries, aka just runs make uninstall
            jeff_dwm_binaries_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -jc|--jeff-dwm-config-dir)  # Only removes the jeff_dwm config dir
            jeff_dwm_rm_config_dir
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -jd|--jeff-desktop-file)  # Only removes the .desktop file for jeff_dwm
            jeff_dwm_desktop_file_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -jm|--jeff-dwm-manual)  # Only removes jeff_dwm manual
            jeff_dwm_man_page_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -jp|--jeff-dwm-pathing-symlinks)  # Only removes the jeff_dwm pathing symlinks
            jeff_dwm_rm_pathing_symlinks
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -js|--jeff-dwm-scripts)  # Only uninstall jeff_dwm scripts
            jeff_dwm_scripts_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -ju|--jeff-dwm-config-unlink)  # Only removes config header file's symlinks
            jeff_dwm_configs_unlink
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -jw|--jeff-dwm-wallpapers)  # Only removes wallpapers and its symlink
            jeff_dwm_wallpapers_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -bs|--dwmblocks-scripts)  # Only uninstalls dwmblocks scripts
            dwm_blocks_scripts_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -rc|--rofi-config)  # Only removes rofi config
            rofi_config_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -rs|--rofi-scripts)  # Only uninstalls rofi scripts
            rofi_scripts_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -rt|--rofi-themes)  # Only removes rofi themes
            rofi_theme_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -h|--help)  # Help print message
            print_help
            exit 0
            ;;
        -u|--usage)  # Usage print message
            print_usage
            exit 0
            ;;
        -v|--version)  # Prints jeff_dwm's version
            echo "jeff_dwm version: $JEFF_DWM_VERSION"
            exit 0
            ;;
        -*|--*)
            echo "Unknown flag $1"
            exit 1
            ;;
        *)
            echo "Unknown option $1"
            exit 1
            ;;
    esac
done

if [[ $DEFAULT_UNINSTALL -eq 1 ]]; then
    jeff_dwm_binaries_uninstall
    jeff_dwm_rm_config_dir
    jeff_dwm_rm_pathing_symlinks
    jeff_dwm_scripts_uninstall
    jeff_dwm_desktop_file_uninstall
    jeff_dwm_man_page_uninstall
    dwmblocks_scripts_uninstall
    rofi_config_uninstall
    rofi_theme_uninstall
    rofi_scripts_uninstall
fi

echo ""