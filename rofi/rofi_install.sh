#!/bin/sh

PARENT_DIR=$(pwd)
SCRIPTS_DIR="$PARENT_DIR/scripts"
THEMES_DIR="$PARENT_DIR/themes"
SCRIPTS_INSTALL_DIR="/usr/local/bin"
CONFIG_INSTALL_DIR="$HOME/.config/rofi"
THEMES_INSTALL_DIR="$CONFIG_INSTALL_DIR/themes"


if ! [ $(id -u) != 0 ]; then
   echo "The script need to be run as non-root to start."
   exit 1
fi

config_install(){
   echo -e "Rofi config being installed to $CONFIG_INSTALL_DIR:" &&
   mkdir -p $CONFIG_INSTALL_DIR &&
   cp -f config.rasi $CONFIG_INSTALL_DIR &&
   echo -e "   $PARENT_DIR/config.rasi -> $CONFIG_INSTALL_DIR/config.rasi"
}

theme_install(){
   mkdir -p $THEMES_INSTALL_DIR &&
   cd $THEMES_DIR && cp -f *.rasi $THEMES_INSTALL_DIR &&
   echo -e "\nRofi themes being installed to $THEMES_INSTALL_DIR \nThemes being installed from $THEMES_DIR:\n" &&
   ls | grep -E '\.rasi$' | sed -e 's/^/#) /'
}

scripts_install(){
   echo -e "Rofi scripts being installed:"
   sudo /bin/sh -c "cd $SCRIPTS_DIR && cp -f *.sh $SCRIPTS_INSTALL_DIR"
   echo -e "\nRofi scripts being installed to $SCRIPTS_INSTALL_DIR \nScripts being installed from $SCRIPTS_DIR:\n"
   ls | grep -E '\.sh$' | sed -e 's/^/#) /'
}

config_install
theme_install
scripts_install