#!/bin/bash

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
REQ_DEPS_MISSING=0
OPT_DEPS_MISSING=0
SKIP_DEPS=0

########################################################

ROFI_DIR="$PARENT_DIR/rofi"
ROFI_SCRIPTS_DIR="$ROFI_DIR/scripts"
ROFI_THEMES_DIR="$ROFI_DIR/themes"
ROFI_CONFIG_INSTALL_DIR="$USERS_DIR/.config/rofi"
ROFI_THEMES_INSTALL_DIR="$ROFI_CONFIG_INSTALL_DIR/themes"

########################################################

JDWM_VERSION=$(grep Makefile -e "VERSION = " | cut -b 11-)

JDWM_DIR="$PARENT_DIR/dwm"
JDWM_CONFIG_DIR="$JDWM_DIR/config"
JDWM_USER_CONFIG_DIR="$USERS_DIR/.config/jdwm"
JDWM_WALLPAPER_DIR="$JDWM_USER_CONFIG_DIR/wallpapers"
JDWM_THEMES_DIR="$JDWM_DIR/themes"
JDWM_SCRIPTS_DIR="$JDWM_DIR/scripts"
JDWM_RESOURCES_DIR="$JDWM_DIR/resources"

########################################################

DWM_BLOCKS_DIR="$PARENT_DIR/dwmblocks"
DWM_BLOCKS_CONFIG_DIR="$DWM_BLOCKS_DIR/config"
DWM_BLOCKS_SCRIPTS_DIR="$DWM_BLOCKS_DIR/scripts"

########################################################

JDWM_MAN_INSTALL_DIR=""
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    JDWM_MAN_INSTALL_DIR="$SHARE_DIR/man"
elif [[ "$OSTYPE" == "openbsd"* ]]; then
    JDWM_MAN_INSTALL_DIR="/usr/local/man"
fi

########################################################

check_and_link(){
    if [ ! -f "$1.h" ]; then
        response=y
        read -p "$1.h does not exist. Would you like to copy $1.def.h? [Y/n] " response
        if [[ "$response" =~ ^([nN][oO]|[nN])$ ]] then
            echo -e "No copy made, exiting script..."
            exit 0
        else
            if [ ! -f "$1.def.h" ]; then
                echo -e "Error: $1.def.h does not exist, nothing to copy"
                exit 1
            else
                echo -e "Copying $1.def.h to $1.h"
                cp $1.def.h $1.h
            fi
        fi
    fi

    echo "Symbolic link to $1.h being created in $JDWM_USER_CONFIG_DIR/"
    ln -s $1.h $JDWM_USER_CONFIG_DIR/$2.h &>/dev/null
}

check_dep(){
    if ! cmd="$(type -p "$1")" || [[ -z $cmd ]]; then
        if [[ $3 -eq 1 ]]; then
            echo "Required dependency '$1' not found, please install. Here is a link to help: $2"
            REQ_DEPS_MISSING=$(( $REQ_DEPS_MISSING + 1 ))
        else
            echo "Optional dependency '$1' not found, please install. Here is a link to help: $2"
            OPT_DEPS_MISSING=$(( $OPT_DEPS_MISSING + 1 ))
        fi
    else 
        if [[ $3 -eq 1 ]]; then
            echo "Required dependency '$1' successfully found"
        else 
            echo "Optional dependency '$1' successfully found"
        fi
    fi
}

check_opt_deps(){
    check_dep "jq" "https://github.com/jqlang/jq" "0"
    check_dep "xrandr" "https://www.x.org/wiki/Projects/XRandR/" "0"
    check_dep "dunst" "https://github.com/dunst-project/dunst" "0"
    check_dep "pactl" "https://www.freedesktop.org/wiki/Software/PulseAudio/Documentation/User/CLI/#pactl" "0"
    check_dep "playerctl" "https://github.com/altdesktop/playerctl" "0"
    check_dep "pamixer" "https://github.com/cdemoulins/pamixer" "0"
    check_dep "betterlockscreen" "https://github.com/betterlockscreen/betterlockscreen" "0"
    check_dep "firefox" "https://www.mozilla.org/en-US/firefox/new/" "0"
    check_dep "vscodium" "https://vscodium.com/" "0"
    check_dep "thunar" "https://docs.xfce.org/xfce/thunar/start" "0"

    echo -e "\nrofi calc is untestable without launching the rofi window itself, therefor is avoided in this script"
    echo "Please check yourself with the command 'rofi -show calc -modi calc -no-show-match -no-sort'"

    if [[ $OPT_DEPS_MISSING -eq 0 ]]; then
        echo -e "\nNo optional dependencies missing"
    elif [[ $OPT_DEPS_MISSING -eq 1 ]]; then
        echo -e "\nOptional dependency missing, please make sure you understand what features are effected this missing dependency."
        echo -e "To learn more, please check https://github.com/JeffofBread/jdwm/tree/master?tab=readme-ov-file#recommended-programs"
    else
        echo -e "\nMultiple optional dependencies missing, please make sure you understand what features are effected by these missing dependencies."
        echo -e "To learn more, please check https://github.com/JeffofBread/jdwm/tree/master?tab=readme-ov-file#recommended-programs"
    fi
}

check_req_deps(){
    check_dep "feh" "https://feh.finalrewind.org/" "1"
    check_dep "rofi" "https://github.com/davatorium/rofi" "1"
    check_dep "grep" "https://www.gnu.org/software/grep/manual/grep.html" "1"
    check_dep "sed" "https://www.gnu.org/software/sed/manual/sed.html" "1"
    check_dep "cut" "https://www.gnu.org/software/coreutils/cut" "1"

    if [[ $REQ_DEPS_MISSING -eq 0 ]]; then
        echo -e "\nNo required dependencies missing. Continuing install\n"
    elif [[ $REQ_DEPS_MISSING -eq 1 ]]; then
        echo -e "\nRequired dependency missing, please install it. Critical functions may not work otherwise."
        echo "To learn more, please check https://github.com/JeffofBread/jdwm/tree/master?tab=readme-ov-file#dependencies"
        echo -e "Cancelling install...\n"
        exit 1
    else
        echo -e "\nRequired dependencies missing, please install these. Critical functions may not work otherwise."
        echo "To learn more, please check https://github.com/JeffofBread/jdwm/tree/master?tab=readme-ov-file#dependencies"
        echo -e "Cancelling install...\n"
        exit 1
    fi
}

check_deps(){
    echo -e "\n|------ jdwm dependencies --------|\n"
    if [[ $SKIP_DEPS -eq 1 ]]; then
        echo -e "\nSkipping checking for dependencies, be warned, could lead to errors if missing dependencies aren't found"
    else
        check_req_deps
        check_opt_deps
    fi
    echo ""
}

copyexamplescripts(){
    examplefiles=( $(ls $1 | grep ".example") )
    if [[ ! ${#examplefiles[@]} -gt 0 ]]; then
        echo -e "No .example files found in $1, skipping copy"
    fi
    for (( i=0; i < ${#examplefiles[@]}; i++ )); do
        shellfile="${examplefiles[$i]::-7}sh"
        if [[ ! -f "$1/$shellfile" ]]; then
            echo "Copying ${examplefiles[$i]} to $shellfile in $1"
            cp $1/${examplefiles[$i]} $1/$shellfile
        else
            echo "$shellfile already exists in $1, not copying .example file"
        fi
    done
}

jdwm_aliases_install(){
    echo -e "\n|--------- jdwm aliases ----------|\n"

    jdwm_check_config_dir

    if [ ! -f "$JDWM_USER_CONFIG_DIR/jdwm.aliases" ]; then
        if [ ! -f "$JDWM_RESOURCES_DIR/jdwm.aliases" ]; then
            echo -e "Error: $JDWM_RESOURCES_DIR/jdwm.aliases does not exist, nothing to copy"
            exit 1
        else 
            echo -e "$JDWM_RESOURCES_DIR/jdwm.aliases to $JDWM_USER_CONFIG_DIR/jdwm.aliases"
            cp $JDWM_RESOURCES_DIR/jdwm.aliases $JDWM_USER_CONFIG_DIR/jdwm.aliases
        fi
    else 
        response=n
        read -p "Would you like to overwrite current jdwm.aliases file? [y/N] " response
        if [[ "$response" =~ ^([yY][eE][sS]|[yY])$ ]] then
            echo -e "$JDWM_RESOURCES_DIR/jdwm.aliases to $JDWM_USER_CONFIG_DIR/jdwm.aliases"
            cp $JDWM_RESOURCES_DIR/jdwm.aliases $JDWM_USER_CONFIG_DIR/jdwm.aliases
        else
            echo -e "Original jdwm.aliases not overwritten"
        fi
    fi
    if grep -q jdwm.aliases "$USERS_DIR/.bashrc"; then
        echo "jdwm.aliases already present in .bashrc, skipping addition"
    else
        echo "Adding link to $JDWM_USER_CONFIG_DIR/jdwm.aliases in .bashrc"
        echo -e "\n#jdwm.aliases\n. $JDWM_USER_CONFIG_DIR/jdwm.aliases" >> $USERS_DIR/.bashrc
    fi
    echo ""
}

jdwm_binaries_install(){
    echo -e "\n|--------- jdwm binaries ---------|\n"
    sudo -v
    make clean install   
    echo ""
}

jdwm_check_config_dir(){
    if [ -d "$JDWM_USER_CONFIG_DIR" ]; then
        echo "$JDWM_USER_CONFIG_DIR already exists, skipping creation"  
    else
        echo "Creating jdwm config directory in $JDWM_USER_CONFIG_DIR"
        mkdir -p $JDWM_USER_CONFIG_DIR
    fi
}

jdwm_link(){
    echo -e "\n|--------- jdwm symlinks ---------|\n"

    jdwm_check_config_dir

    check_and_link "$JDWM_CONFIG_DIR/autorun" "autorun"
    check_and_link "$JDWM_CONFIG_DIR/binds" "binds"
    check_and_link "$JDWM_CONFIG_DIR/config" "config"
    check_and_link "$JDWM_CONFIG_DIR/keydefs" "keydefs"
    check_and_link "$DWM_BLOCKS_CONFIG_DIR/blocks" "blocks"

    if [ -L "$SHARE_DIR/jdwm" ] && [ -d "$SHARE_DIR/jdwm" ]; then
        sudo rm -f $SHARE_DIR/jdwm
    fi

    echo "Symbolic link to $PARENT_DIR being created in $SHARE_DIR"
    sudo ln -s $PARENT_DIR $SHARE_DIR/jdwm  &>/dev/null
    
    echo ""
}

jdwm_desktop_file_install(){
    echo -e "\n|--------- jdwm .desktop ---------|\n"
    echo -e "jdwm.desktop being installed to /usr/share/xsessions/"
    sudo cp -f $JDWM_RESOURCES_DIR/jdwm.desktop /usr/share/xsessions/jdwm.desktop
    sudo /bin/sh -c "(echo "$JDWM_VERSION"; echo "Icon=$JDWM_RESOURCES_DIR/dwm.png") >> /usr/share/xsessions/jdwm.desktop"
    echo ""
}

jdwm_man_page_install(){
    echo -e "\n|------------ jdwm man -----------|\n"
    echo -e "jdwm manual being installed to $JDWM_MAN_INSTALL_DIR/man1 from $JDWM_RESOURCES_DIR/jdwm.1"
    sudo mkdir -p $JDWM_MAN_INSTALL_DIR/man1
    sudo /bin/sh -c "sed "s/VERSION/$JDWM_VERSION/g" < $JDWM_RESOURCES_DIR/jdwm.1 > $JDWM_MAN_INSTALL_DIR/man1/jdwm.1 && chmod 644 $JDWM_MAN_INSTALL_DIR/man1/jdwm.1"
    echo ""
}

jdwm_scripts_install(){
    echo -e "\n|---------- jdwm scripts ---------|\n"
    copyexamplescripts "$JDWM_SCRIPTS_DIR"
    echo -e "jdwm scripts are being installed to $BIN_INSTALL_DIR from $JDWM_SCRIPTS_DIR"
    echo -e "jdwm scripts being installed:\n"
    cd $JDWM_SCRIPTS_DIR 
    sudo chmod -R 755 ./
    sudo cp -f *.sh $BIN_INSTALL_DIR
    ls | grep -E '\.sh$' | sed -e 's/^/     #) /'
    echo ""
}

jdwm_wallpapers_install(){
    echo -e "\n|-------- jdwm wallpapers --------|\n"

    if [ -d "$JDWM_WALLPAPER_DIR" ]; then
        response=n
        read -p "Would you like to check for wallpaper updates (git pull)? [y/N] " response
        if [[ "$response" =~ ^([yY][eE][sS]|[yY])$ ]] then
            git pull
        else
            echo -e "Skipping checking for update"
        fi 
    else
        echo "Cloning jdwm_wallpapers repo into $JDWM_WALLPAPER_DIR"
        git clone https://github.com/JeffofBread/jdwm.git -b wallpapers $JDWM_WALLPAPER_DIR
    fi
}

dwm_blocks_scripts_install(){
    echo -e "\n|--------- dwmblocks scripts ---------|\n"
    copyexamplescripts "$DWM_BLOCKS_SCRIPTS_DIR"
    echo -e "\ndwmblocks scripts are being installed to $BIN_INSTALL_DIR from $DWM_BLOCKS_SCRIPTS_DIR"
    echo -e "dwmblocks scripts being installed:\n"
    cd $DWM_BLOCKS_SCRIPTS_DIR 
    sudo chmod -R 755 ./
    sudo cp -f *.sh $BIN_INSTALL_DIR
    ls | grep -E '\.sh$' | sed -e 's/^/     #) /'
    echo ""
}

rofi_config_install(){
    echo -e "\n|-------- Rofi config install --------|\n"
    echo -e "Rofi config being installed to $ROFI_CONFIG_INSTALL_DIR from $ROFI_DIR/config.rasi" 
    mkdir -p $ROFI_CONFIG_INSTALL_DIR 
    if [ -f "$ROFI_CONFIG_INSTALL_DIR/config.rasi" ]; then
        response=y
        read -p "Would you like to backup current config.rasi file? [Y/n] " response
        if [[ "$response" =~ ^([nN][oO]|[nN])$ ]] then
            echo -e "No backup being made"
        else
            # minute-hour-day-month-year, overkill but idc
            date="$(date '+%M-%H-%d-%m-%Y')"
            echo -e "Existing rofi config being backed up as config.rasi.$date.bak"
            cp $ROFI_CONFIG_INSTALL_DIR/config.rasi $ROFI_CONFIG_INSTALL_DIR/config.rasi.$date.bak
        fi
    fi
    cp -f $ROFI_DIR/config.rasi $ROFI_CONFIG_INSTALL_DIR
    echo ""
}

rofi_scripts_install(){
    echo -e "\n|-------- Rofi scripts install -------|\n"
    copyexamplescripts "$ROFI_SCRIPTS_DIR"
    echo -e "Rofi scripts are being installed to $BIN_INSTALL_DIR from $ROFI_SCRIPTS_DIR:"
    echo -e "Rofi scripts being installed:\n"
    cd $ROFI_SCRIPTS_DIR 
    sudo chmod -R 755 ./
    sudo cp -f *.sh $BIN_INSTALL_DIR
    ls | grep -E '\.sh$' | sed -e 's/^/     #) /'
    echo ""
}

rofi_theme_install(){
    echo -e "\n|-------- Rofi theme install ---------|\n"
    mkdir -p $ROFI_THEMES_INSTALL_DIR 
    cd $ROFI_THEMES_DIR && cp -f *.rasi $ROFI_THEMES_INSTALL_DIR 
    echo -e "Rofi themes are being installed to $ROFI_THEMES_INSTALL_DIR from $ROFI_THEMES_DIR.\nThemes being installed:\n"
    ls | grep -E '\.rasi$' | sed -e 's/^/     #) /'
    echo ""
}

print_help(){
    print_usage
    echo "This install script is a part of jdwm to assist in installing it"
    echo "along with any scripts, configs, or other necessary files or extras."
    echo "If you need help installing, want to report an issue, make a suggestion,"
    echo "or simply to take another look at the, codebase, head to"
    echo "https://github.com/JeffofBread/jdwm. By default (no flags), all"
    echo "installs will be run."
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
    echo "   -d,  --check-dependencies          Checks for the presence of any"
    echo "                                      requried or optional dependencies"
    echo ""
    echo "   -sd, --skip-dependencies           Skips checking for any requried"
    echo "                                      or optional dependencies"
    echo ""
    echo "   -ja, --jdwm-aliases                Installs jdwm alias file to"
    echo "                                      ~/.config/jdwm/"
    echo ""
    echo "   -jb, --jdwm-binaries               Installs all jdwm related"
    echo "                                      binaries to /usr/local/bin/ using"
    echo "                                      'sudo make install'"
    echo ""
    echo "   -jd, --jdesktop-file               Installs jdwm's desktop file to"
    echo "                                      /usr/share/xsessions/"
    echo ""
    echo "   -jl, --jdwm-link                   Installs symlinks linking config"
    echo "                                      header files in /jdwm/dwm/config/"
    echo "                                      to ~/.config/jdwm/. Also creates"
    echo "                                      symlink to jdwm source directory"
    echo "                                      as '/usr/share/jdwm'"
    echo ""
    echo "   -jm, --jdwm-manual                 Installs jdwn's-$JDWM_VERSION manual file"
    echo "                                      to $JDWM_MAN_INSTALL_DIR/man1/"
    echo ""
    echo "   -js, --jdwm-scripts                Installs jdwm's script files,"
    echo "                                      meaning any .sh files found in"
    echo "                                      /jdwm/dwm/scripts/, to"
    echo "                                      /usr/local/bin/"
    echo ""
    echo "   -jw, --jdwm-wallpapers             Clones jdwm's wallpapers repo to"
    echo "                                      ~/.config/jdwm/wallpapers and "
    echo "                                      symlinks to it in /jdwm/dwm/themes/"
    echo ""
    echo "   -bs, --dwmblocks-scripts           Installs jdwm's dwmblocks"
    echo "                                      script files, meaning any .sh files"
    echo "                                      found in/jdwm/dwmblocks/scripts/,"
    echo "                                      to /usr/local/bin/"
    echo ""
    echo "   -rc, --rofi-config                 Installs jdwm's rofi config file"
    echo "                                      (/jdwm/rofi/config.rasi) to"
    echo "                                      ~/.config/rofi/config.rasi"
    echo ""
    echo "   -rs, --rofi-scripts                Installs jdwm's rofi scripts,"
    echo "                                      meaning any .sh files found in"
    echo "                                      /jdwm/rofi/scripts/, to"
    echo "                                      /usr/local/bin/"
    echo ""
    echo "   -rt, --rofi-themes                 Installs jdwm's rofi themes,"
    echo "                                      meaning any .rasi files found in"
    echo "                                      both /jdwm/rofi/themes/, to"
    echo "                                      ~/.config/rofi/themes/"
    echo ""
}

print_usage(){
    echo ""
    echo "Usage: install.sh [-h] [--help] [-u] [--usage] [-v] [--version]"
    echo "       [-d] [--check-dependencies] [-sd] [--skip-dependencies]"
    echo "       [-ja] [--jdwm-aliases] [-jb] [--jdwm-binaries]"
    echo "       [-jd] [--jdesktop-file] [-jl] [--jdwm-link]"
    echo "       [-jm] [--jdwm-manual] [-js] [--jdwm-scripts]"
    echo "       [-jw] [--jdwm-wallpapers] [-bs] [--dwmblocks-scripts]"
    echo "       [-rc] [--rofi-config] [-rs] [--rofi-scripts] [-rt]"
    echo "       [--rofi-themes]"
    echo ""  
}

########################################################

DEFAULT_INSTALL=1

POSITIONAL_ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--check-dependencies)  # Check dependencies
            check_deps
            DEFAULT_INSTALL=0
            shift
            ;;
        -sd|--skip-dependencies)  # Ignore dependencies check
            SKIP_DEPS=1
            shift
            ;;
        -ja|--jdwm-aliases)  # Only installs the custom alias file
            jdwm_aliases_install
            DEFAULT_INSTALL=0
            shift
            ;;
        -jb|--jdwm-binaries)  # Only install binaries, aka just runs make install
            jdwm_binaries_install
            DEFAULT_INSTALL=0
            shift
            ;;
        -jd|--jdwm-desktop-file)  # Only installs the .desktop file for jdwm
            jdwm_desktop_file_install
            DEFAULT_INSTALL=0
            shift
            ;;
        -jl|--jdwm-link)  # Only installs jdwm symlinks
            jdwm_link
            DEFAULT_INSTALL=0
            shift
            ;;
        -jm|--jdwm-manual)  # Only installs jdwm manual
            jdwm_man_page_install
            DEFAULT_INSTALL=0
            shift
            ;;
        -js|--jdwm-scripts)  # Only installs dwm scripts
            jdwm_scripts_install
            DEFAULT_INSTALL=0
            shift
            ;;
        -jw|--jdwm-wallpapers)  # Only installs wallpapers and its symlink
            jdwm_wallpapers_install
            DEFAULT_INSTALL=0
            shift
            ;;
        -bs|--dwmblocks-scripts)  # Only installs dwmblocks scripts
            dwm_blocks_scripts_install
            DEFAULT_INSTALL=0
            shift
            ;;
        -rc|--rofi-config)  # Only installs rofi config
            rofi_config_install
            DEFAULT_INSTALL=0
            shift
            ;;
        -rs|--rofi-scripts)  # Only installs rofi scripts
            rofi_scripts_install
            DEFAULT_INSTALL=0
            shift
            ;;
        -rt|--rofi-themes)  # Only installs rofi themes
            rofi_theme_install
            DEFAULT_INSTALL=0
            shift
            ;;
        -v|--version)  # Prints jdwm's version
            echo "jdwm version: $JDWM_VERSION"
            exit 0
            ;;
        -h|--help)  # Help print message
            print_help
            exit 0
            ;;
        -u|--usage)  # Usage print message
            print_usage
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

if [[ $DEFAULT_INSTALL -eq 1 ]]; then
    check_deps
    jdwm_aliases_install
    jdwm_binaries_install
    jdwm_desktop_file_install
    jdwm_link
    jdwm_man_page_install
    jdwm_scripts_install
    jdwm_wallpapers_install
    dwm_blocks_scripts_install
    rofi_config_install
    rofi_scripts_install
    rofi_theme_install
fi