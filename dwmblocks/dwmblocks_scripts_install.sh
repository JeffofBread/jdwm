#!/bin/sh

PARENT_DIR=$(pwd)
SCRIPTS_DIR="$PARENT_DIR/scripts"
INSTALL_DIR="/usr/local/bin"

if ! [ $(id -u) == 0 ]; then
   echo "The script need to be run as root."
   exit 1
fi

scripts_install(){
   echo -e "dwmblocks scripts being installed:"
   cd $SCRIPTS_DIR && cp -f *.sh $INSTALL_DIR
   echo -e "\ndwmblocks scripts being installed to $INSTALL_DIR \nScripts being installed from $SCRIPTS_DIR:\n"
   ls | grep -E '\.sh$' | sed -e 's/^/#) /'
}

scripts_install