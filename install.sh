#!/bin/sh

if ! [ $(id -u) != 0 ]; then
   echo "The script need to be run as non-root to start."
   exit 1
fi

rofi_config_install(){
   echo -e "Rofi config being installed:"
   mkdir -p ~/.config/rofi/
   cd rofi 
   cp -f config.rasi ~/.config/rofi/
   echo -e "   $(pwd)config.rasi -> ~/.config/rofi/config.rasi"
   mkdir -p ~/.config/rofi/themes/
   cd themes && cp -f *.rasi ~/.config/rofi/themes/
   echo -e "\nRofi themes being installed to ~/.config/rofi/themes/ \nThemes being installed from jeff_dwm/rofi/themes:\n"
   ls | grep -E '\.rasi$' | sed -e 's/^/#) /'
   cd ../../
}

# Optional clear command to clear terminal for cleaner look #
# clear &&

########################################################
echo -e "\n|--------- Install Started -----------|\n"  #
########################################################

rofi_config_install

########################################################
echo -e "\n|--Building and installing jeff-dwm---|\n"  #
########################################################

sudo make install

########################################################
echo -e "\n|----------Install Complete-----------|\n"  #
########################################################