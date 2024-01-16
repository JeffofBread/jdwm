#!/bin/sh

PARENT_DIR=$(pwd)
ROFI_DIR="rofi"
DWM_DIR="dwmsrc"
DWM_BLOCKS_DIR="dwmblocks"

if ! [ $(id -u) != 0 ]; then
   echo "The script need to be run as non-root to start."
   exit 1
fi

# Optional clear command to clear terminal for cleaner look #
clear &&

########################################################
echo -e "\n|--------- Install Started -----------|\n"  #
########################################################

########################################################
echo -e "\n|---- Rofi config and scripts --------|\n"  #
########################################################

cd $ROFI_DIR && bash rofi_theme_install.sh && cd $PARENT_DIR

########################################################
echo -e "\n|--------- dwmblocks scripts----------|\n"  #
########################################################

cd $DWM_BLOCKS_DIR && sudo bash dwmblocks_scripts_install.sh && cd $PARENT_DIR

########################################################
echo -e "\n|-------- jeff_dwm scripts ----------|\n"   #
########################################################

cd $DWM_DIR && sudo bash dwm_scripts_install.sh && cd $PARENT_DIR

########################################################
echo -e "\n|--Building and installing jeff-dwm---|\n"  #
########################################################

cd $PARENT_DIR && sudo make install

########################################################
echo -e "\n|----------Install Complete-----------|\n"  #
########################################################