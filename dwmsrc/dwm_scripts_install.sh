#!/bin/sh

PARENT_DIR=$(pwd)
SCRIPTS_DIR="$PARENT_DIR/scripts"
INSTALL_DIR="/usr/local/bin"

if ! [ $(id -u) == 0 ]; then
   echo "The script need to be run as root."
   exit 1
fi

scripts_install(){


   # If you are using a custom setup file name, change these variables: 
   SETUPFILE="$SCRIPTS_DIR/jeff_dwm_setup.sh"
   EXAMPLE_SETUPFILE="$SCRIPTS_DIR/jeff_dwm_setup.example"

   if [ ! -f $SETUPFILE ]; then
      if [ ! -f $EXAMPLE_SETUPFILE ]; then
         echo -e "\n$EXAMPLE_SETUPFILE does not exist, no default config to copy.\n"
      else
         cp  $EXAMPLE_SETUPFILE $SETUPFILE
         echo -e "\nCopying default setup config.\n"
      fi
   fi

   echo -e "jeff_dwm scripts being installed:"
   cd $SCRIPTS_DIR && cp -f *.sh $INSTALL_DIR
   echo -e "\njeff_dwm scripts being installed to $INSTALL_DIR \nScripts being installed from $SCRIPTS_DIR:\n"
   ls | grep -E '\.sh$' | sed -e 's/^/#) /'
}

scripts_install
