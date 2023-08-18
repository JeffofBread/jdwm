#!/bin/sh

if ! [ $(id -u) = 0 ]; then
   echo "The script need to be run as root." >&2
   exit 1
fi

# Optional clear command to clear terminal for cleaner look #
# clear &&
echo -e "\n|-------- Compiling jeff_dwm ---------|\n" &&
make install
echo -e "\n|----------Install Complete-----------|\n"