#!/bin/bash

# Check script is run as non root to get correct paths
if ! [ $(id -u) != 0 ]; then
    echo "The script need to be run as non-root to start."
    exit 1
fi

# Source needed path variables
. bash_paths.env

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
    echo ""
}

jdwm_aliases_uninstall(){
    echo "Removing jdwm's alias file: $JDWM_USER_CONFIG_DIR/jdwm.aliases and its reference in $USERS_DIR/.bashrc"
    sudo rm -f $JDWM_USER_CONFIG_DIR/jdwm.aliases
    sed -i '/jdwm.aliases/d' $USERS_DIR/.bashrc
    echo ""
}

jdwm_binaries_uninstall(){
    make uninstall
    echo ""
}

jdwm_rm_config_dir(){
    echo "Removing $JDWM_USER_CONFIG_DIR/"
    rm -rf $JDWM_USER_CONFIG_DIR 
    echo ""
}

jdwm_desktop_file_uninstall(){
    echo "Removing jdwm desktop file: /usr/share/xsessions/jdwm.desktop and jdwm's icon dir: /usr/share/icons/jdwm/"
    sudo rm -f /usr/share/xsessions/jdwm.desktop
    sudo rm -rf /usr/share/icons/jdwm/
    echo ""
}

jdwm_home_dir_uninstall(){
    echo ""
    echo "The only thing left now, to completely remove jdwm, is to remove the"
    echo "source directory you cloned jdwm into, and which contains this script."
    echo ""
    response=n
    read -p "Would you like this script to do it automatically? [y/N] " response
    if [[ "$response" =~ ^([yY][eE][sS]|[yY])$ ]] then
        response=n
        echo ""
        read -p "Ok. To confirm, you would like to remove $PARENT_DIR/ and all its subdirectories? [y/N] " response
        if [[ "$response" =~ ^([yY][eE][sS]|[yY])$ ]] then
            echo -e "\nOk, removing $PARENT_DIR/\nThank you for trying jdwm.\n"
            rm -rf $PARENT_DIR
            exit 0
        else
            echo -e "\nOk, exiting script. Thank you for trying jdwm.\n"
            exit 0
        fi
    else
        echo -e "\nOk, exiting script. Thank you for trying jdwm.\n"
        exit 0
    fi
}

jdwm_man_page_uninstall(){
    echo "Removing jdwm's manual: $JDWM_MAN_INSTALL_DIR/man1/jdwm.1"
    sudo rm -f $JDWM_MAN_INSTALL_DIR/man1/jdwm.1
    echo ""
}

jdwm_rm_pathing_symlink(){
    echo "Removing $SHARE_DIR/jdwm"
    sudo rm -f $SHARE_DIR/jdwm
    echo ""

}

jdwm_scripts_uninstall(){
    file_uninstaller "sh" "$JDWM_SCRIPTS_DIR" "$BIN_INSTALL_DIR" "jdwm script" "1"
}

jdwm_configs_unlink(){
    echo "Removing symlinks to autorun.h, binds.h, config.h, keydefs.h, and blocks.h from $JDWM_USER_CONFIG_DIR"
    rm -f $JDWM_USER_CONFIG_DIR/autorun.h $JDWM_USER_CONFIG_DIR/binds.h $JDWM_USER_CONFIG_DIR/config.h $JDWM_USER_CONFIG_DIR/keydefs.h $JDWM_USER_CONFIG_DIR/blocks.h
    echo ""
}

jdwm_wallpapers_uninstall(){
    echo "Removing jdwm wallpapers directory: $JDWM_WALLPAPER_DIR"
    rm -rf $JDWM_WALLPAPER_DIR
    echo ""
}

dwmblocks_scripts_uninstall(){
    file_uninstaller "sh" "$DWM_BLOCKS_SCRIPTS_DIR" "$BIN_INSTALL_DIR" "dwmblocks script" "1"
}

rofi_config_uninstall(){
    echo "Removing jdwm's rofi config file: $ROFI_CONFIG_INSTALL_DIR/config.rasi"
    rm -f $ROFI_CONFIG_INSTALL_DIR/config.rasi
    echo ""
}

rofi_scripts_uninstall(){
    file_uninstaller "sh" "$ROFI_SCRIPTS_DIR" "$BIN_INSTALL_DIR" "rofi script" "1"
}

rofi_theme_uninstall(){
    file_uninstaller "rasi" "$ROFI_THEMES_DIR" "$ROFI_THEMES_INSTALL_DIR" "jdwm rofi theme" "0"
}

print_help(){
    print_usage
    echo "This uninstall script is designed to help uninstall jdwm and any and"
    echo "all other files used by or along side of it. However, it DOES NOT remove"
    echo "any files you installed via a package manager and will not remove most"
    echo "files you might have added yourself. Exceptions include: any files with a"
    echo "'.sh' extension you put in any of the '/scripts' folders, and themes you"
    echo "added to '/rofi/themes' with a '.rasi' extension. All those files WILL be"
    echo "removed, as they are considered to have been apart of jdwm's install."
    echo "Refer to the script flags to see ways to avoid uninstalling specific sets"
    echo "of scripts or themes."
    echo ""
    echo "WARNING: By default (no flags) all files related to jdwm will be removed."
    echo "Please make sure you understand what will be removed, this cannot be un-done."
    echo ""
    echo "Flags (can use one or more, no values necessary):"
    echo ""
    echo "   -h,  --help                        Prints this help and usage message"
    echo ""
    echo "   -u,  --usage                       Prints jdwm's usage help message"
    echo ""
    echo "   -v,  --version                     Prints jdwm's version, same as"
    echo "                                      'jdwm -v'"
    echo ""
    echo "   -ja, --jdwm-aliases                Removes jdwm alias file from"
    echo "                                      ~/.config/jdwm/"
    echo ""
    echo "   -jb, --jdwm-binaries               Uninstalls all jdwm related"
    echo "                                      binaries from /usr/local/bin/"
    echo ""
    echo "   -jc, --jdwm-config-dir             Removes jdwm's config directory:"
    echo "                                      ~/.config/jdwm/"
    echo ""
    echo "   -jd, --jdwm-desktop-file           Removes jdwm's desktop file from"
    echo "                                      /usr/share/xsessions/"
    echo ""
    echo "   -jh, --jdwm-home-dir               Removes jdwm's entire source"
    echo "                                      directory. Be very careful with this,"
    echo "                                      make sure you really do want to"
    echo "                                      remove it all. Also note, if you do"
    echo "                                      just this flag, there will be a lot"
    echo "                                      zombie files left behind. I do not"
    echo "                                      recommend using this flag."
    echo ""
    echo "   -jm, --jdwm-manual                 Removes jdwn's manual file"
    echo "                                      from $JDWM_MAN_INSTALL_DIR/man1/"
    echo ""
    echo "   -jp, --jdwm-pathing-symlink        Removes jdwm's pathing symlinks"
    echo "                                      used to help it path to needed files"
    echo ""
    echo "   -js, --jdwm-scripts                Uninstalls jdwm's script files,"
    echo "                                      meaning any .sh files found in both"
    echo "                                      /jdwm/dwm/scripts/ and"
    echo "                                      /usr/local/bin/"
    echo ""
    echo "   -ju, --jdwm-config-unlink          Removes symlinks linking config"
    echo "                                      header files in /jdwm/dwm/config/"
    echo "                                      to ~/.config/jdwm/"
    echo ""
    echo "   -jw, --jdwm-wallpapers             Removes jdwm's wallpapers directory"
    echo "                                      in ~/.config/jdwm and symlink to it"
    echo "                                      in /jdwm/dwm/themes/"
    echo ""
    echo "   -bs, --dwmblocks-scripts           Uninstalls jdwm's dwmblocks"
    echo "                                      scripts, meaning any .sh files found"
    echo "                                      in both /jdwm/dwmblocks/scripts/"
    echo "                                      and /usr/local/bin/"
    echo ""
    echo "   -rc, --rofi-config                 Removes jdwm's rofi config file:"
    echo "                                      ~/.config/rofi/config.rasi"
    echo ""
    echo "   -rs, --rofi-scripts                Uninstalls jdwm's rofi scripts,"
    echo "                                      meaning any .sh files found in both"
    echo "                                      /jdwm/rofi/scripts/ and"
    echo "                                      /usr/local/bin/"
    echo ""
    echo "   -rt, --rofi-themes                 Removes jdwm's rofi themes,"
    echo "                                      meaning any .rasi files found in"
    echo "                                      both /jdwm/rofi/themes/ and"
    echo "                                      ~/.config/rofi/themes/"
    echo ""
}

print_usage(){
    echo ""
    echo "Usage: uninstall.sh [-h] [--help] [-u] [--usage] [-v] [--version]"
    echo "       [-ja] [--jdwm-aliases] [-jb] [--jdwm-binaries]"
    echo "       [-jc] [--jdwm-config-dir] [-jd] [--jdwm-desktop-file]"
    echo "       [-jh] [--jdwm-home-dir][-jm] [--j-dwm-manual] [-jp]"
    echo "       [--jdwm-pathing-symlink] [-js] [--jdwm-scripts]"
    echo "       [-ju] [--jdwm-config-unlink] [-jw] [--jdwm-wallpapers]"
    echo "       [-bs] [--dwmblocks-scripts] [-rc] [--rofi-config] [-rs]"
    echo "       [--rofi-scripts] [-rt] [--rofi-themes]"
    echo "" 
}

########################################################

echo ""

DEFAULT_UNINSTALL=1

POSITIONAL_ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -ja|--jdwm-aliases)  # Only removes the custom alias file
            jdwm_aliases_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -jb|--jdwm-binaries)  # Only uninstall jdwm binaries, aka just runs make uninstall
            jdwm_binaries_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -jc|--jdwm-config-dir)  # Only removes the jdwm config dir
            jdwm_rm_config_dir
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -jd|--jdwm-desktop-file)  # Only removes the .desktop file for jdwm
            jdwm_desktop_file_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -jh|--jdwm-home-dir)  # Removes jdwm home/source directory
            jdwm_home_dir_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -jm|--jdwm-manual)  # Only removes jdwm manual
            jdwm_man_page_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -jp|--jdwm-pathing-symlink)  # Only removes the jdwm pathing symlink
            jdwm_rm_pathing_symlink
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -js|--jdwm-scripts)  # Only uninstall jdwm scripts
            jdwm_scripts_uninstall
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -ju|--jdwm-config-unlink)  # Only removes config header file's symlinks
            jdwm_configs_unlink
            DEFAULT_UNINSTALL=0
            shift
            ;;
        -jw|--jdwm-wallpapers)  # Only removes wallpapers and its symlink
            jdwm_wallpapers_uninstall
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
        -v|--version)  # Prints jdwm's version
            echo "jdwm version: $JDWM_VERSION"
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
    jdwm_aliases_uninstall
    jdwm_binaries_uninstall
    jdwm_rm_config_dir
    jdwm_scripts_uninstall
    jdwm_desktop_file_uninstall
    jdwm_man_page_uninstall
    dwmblocks_scripts_uninstall
    rofi_config_uninstall
    rofi_theme_uninstall
    rofi_scripts_uninstall
    echo "Removing /usr/share/jdwm symlink"
    sudo rm -f $SHARE_DIR/jdwm
    jdwm_home_dir_uninstall
fi