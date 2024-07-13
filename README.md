# Jeff's dwm build

A build of dwm for myself and anyone who wishes to use it too. If you find any issues or would like to make a recommendation, feel free. This is a major work in progress and will be changing often and will be receiving major changes in the near future. Any recommendations or critique is appreciated.

## Installation

**Notes before you install:**
 - Make sure you have all the necessary [dependencies](#dependencies), or issues could occur during install
 - For a more custom install process, check install flags (run `./install.sh -h` for more info) or edit paths in install.sh
 - To edit things like your terminal alias before install, edit /jeff_dwm/dwmsrc/resources/jeff_dwm.aliases. This can also be done after install by going to ~/.config/jeff_dwm/jeff_dwm.aliases

```bash
git clone https://github.com/JeffofBread/jeff_dwm.git
cd jeff_dwm
./install.sh
```
After your first install, you can use `Ctrl + Shift + Q` to open the recompile script (make sure to correctly define in /jeff_dwm/dwmsrc/resources/jeff_dwm.aliases). To reload jeff_dwm press `Super + Shift + Q`. To exit dwm press `Super + Ctrl + Shift + Q`.

## Dependencies

- A terminal emulator of some kind (chosen in /jeff_dwm/dwmsrc/resources/jeff_dwm.aliases, by default it is [kitty](https://sw.kovidgoyal.net/kitty/))
- yajl (dwm IPC)
- imlib2 (for program icons in bar)
- argp (for cli arguments)
- rofi (layout menu changer, window switcher, program launcher, etc. Technically not 100% dependent but designed with it in mind)
- grep && sed (install script)

## Recommended Programs

- picom (Compositor, recommend https://github.com/FT-Labs/picom)
- jq (Rofi layout menu script)
- xrandr (Monitor resolution, refresh rate, etc. Used in jeff_dwm_setup.sh)
- feh (Background wallpaper. Used in jeff_dwm_setup.sh)
- pactl (Toggle mute on mic/sink binds)
- playerctl (Media control binds)
- pamixer (Volume binds)
- xbacklight (For dwmblocks sb-battery screen brightness functionality)

These are all optional and easily changeable in their respective config files or scripts, these are just what are used out of the box.

## Patches

All the original .diff files are located in jeff_dwm/dwmsrc/patches. Needless to say that a lot of tweaks had to be made to many of these patches when implemented to make them all work together. Not unexpected when adding almost 40 patches together. Also, many of these patches were pulled from [bakkeby's](https://github.com/bakkeby) incredible [flexipatch](https://github.com/bakkeby/dwm-flexipatch), but I frankly don't remember which. Also major thanks to [FT-Labs](https://github.com/FT-Labs) for helping me learn so much about dwm and x11 directly and indirectly through his [pdwm](https://github.com/FT-Labs/pdwm).

- alternativetags
- alwayscenter
- barpadding
- bartoggle
- centeredwindowname
- colorbar
- cursorwarp
- decorhints
- dragmfacts
- ewmhtags
- floatbordercolor
- floatborderwidth
- fullscreencompilation
- hidevacanttags
- ignoretransientwindows
- ipc
- layoutscroll
- noborderflicker
- pertag
- preserveonrestart
- resizecorners
- restartsig
- rotatestack
- rulerefresher
- scratchpad
- shiftools
- statuspadding
- steam
- sticky
- stickyindicator
- switchallmonitortags
- systray
- underlinetags
- vanitygaps
- winicon

## Known Bugs

- placemouse window placement is not the smoothest at the moment
- Steam toast notifications not floating and screwing up tiling
- mfacts does not stay on tag change
- There might be a conflict between dwm-ipc and pertag, where dwm-msg does not dump the correct values. Will look into in the future. 

## Future

I still have many plans for changes and features but currently dont have that much time to put towards this project, so dont expect too many updates, at least not rapid ones. 
