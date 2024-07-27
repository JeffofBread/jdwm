# jeff_dwm

A custom build of [dwm](https://dwm.suckless.org/) made by myself, JeffofBread. If you find any issues or would like to make a recommendation, feel free. This is a major work in progress and **will** change.

## Installation

**Notes before you install:**
 - Make sure you have all the necessary [dependencies](#dependencies), or issues could occur during installation.
 - For a more custom install process, check install flags (run `./install.sh -h` for more info) or edit paths in `install.sh`
 - To edit things like your terminal alias before installation, edit `/jeff_dwm/dwm/resources/jeff_dwm.aliases`. This can also be done after installation by going to `~/.config/jeff_dwm/jeff_dwm.aliases`
 - By default on install, `install.sh` will clone [jeff_dwm_wallpapers](https://github.com/JeffofBread/jeff_dwm_wallpapers) into `~/.config/jeff_dwm/wallpapers/`. These wallpapers are used by the themes in `jeff_dwm/dwm/themes/`. If you are bandwidth or storage limited, you may want to [disable this step in the install script](https://github.com/JeffofBread/jeff_dwm/blob/500536106a862473159736205ca6bbc159019f84/install.sh#L228) and remove/alter the wallpaper commands in the various theme files (`jeff_dwm/dwm/themes/`).

```bash
git clone https://github.com/JeffofBread/jeff_dwm.git
cd jeff_dwm
./install.sh
```

After your first install, you can use `Ctrl + Shift + Q` to open the recompile script (make sure to correctly defined in `/jeff_dwm/dwm/resources/jeff_dwm.aliases`). To reload jeff_dwm press `Super + Shift + Q`. To exit jeff_dwm press `Super + Ctrl + Shift + Q` or use `Super + P` to open a rofi power menu.

## Dependencies

- A terminal emulator of some kind (chosen in `/jeff_dwm/dwm/resources/jeff_dwm.aliases`, by default it is [kitty](https://sw.kovidgoyal.net/kitty/))
- [yajl](https://lloyd.github.io/yajl/) (dwm IPC)
- [imlib2](https://docs.enlightenment.org/api/imlib2/html/) (for program icons in the bar)
- [argp](https://www.gnu.org/software/libc/manual/html_node/Argp.html) (for cli arguments)
- [rofi](https://github.com/davatorium/rofi) (layout menu changer, window switcher, program launcher, etc. Technically not 100% dependent, but designed with it in mind)
- [grep](https://www.gnu.org/software/grep/manual/grep.html) && [sed](https://www.gnu.org/software/sed/manual/sed.html) (install script)

### Arch

```bash
sudo pacman -S yajl imlib2 gcc rofi grep sed
```
or

```bash
yay -S yajl imlib2 gcc rofi grep sed
```

## Recommended Programs

- A Compositor of some kind (I recommend https://github.com/FT-Labs/picom)
- [jq](https://github.com/jqlang/jq) (Rofi layout menu script)
- [xrandr](https://www.x.org/wiki/Projects/XRandR/) (Monitor resolution, refresh rate, etc. Used in jeff_dwm_setup.sh)
- [feh](https://feh.finalrewind.org/) (Background wallpaper. Used in jeff_dwm_setup.sh)
- [pactl](https://www.freedesktop.org/wiki/Software/PulseAudio/Documentation/User/CLI/#pactl) (Toggle mute on mic/sink binds)
- [playerctl](https://github.com/altdesktop/playerctl) (Media control binds)
- [pamixer](https://github.com/cdemoulins/pamixer) (Volume binds)
- [xbacklight](https://www.x.org/releases/X11R7.6/doc/man/man1/xbacklight.1.xhtml) (For dwmblocks sb-battery screen brightness functionality)
- [betterlockscreen](https://github.com/betterlockscreen/betterlockscreen) (For rofi powermenu script)

### Arch

(Lacks [betterlockscreen](https://github.com/betterlockscreen/betterlockscreen) because it's an AUR)
```bash
sudo pacman -S jq xorg-xrandr feh libpulse playerctl pamixer xorg-xbacklight
```

or

```bash
yay -S jq xorg-xrandr feh libpulse playerctl pamixer xorg-xbacklight betterlockscreen
```

These are all optional and easily changeable in their respective config files or scripts, these are just what are used out of the box.

## Patches

All the original .diff files are located in [jeff_dwm_patches repo](https://github.com/JeffofBread/jeff_dwm_patches). Needless to say that a lot of tweaks had to be made to many of these patches when implemented to make them all work together, so the .diff files may not represent exactly what is present in jeff_dwm. These changes are to be expected when adding 36 patches on top of each other, and is not a fault of the original authors. Also, many of these patches were pulled from [bakkeby's](https://github.com/bakkeby) incredible [flexipatch](https://github.com/bakkeby/dwm-flexipatch), but I frankly don't remember exactly which, though I have tried my best to provide links below. Also, major thanks to [bakkeby](https://github.com/bakkeby) and [FT-Labs](https://github.com/FT-Labs) for help with a few problems as well as for making their own builds of dwm, which I shamelessly pulled from.

**Patches integrated in jeff_dwm:**

- [alwayscenter](https://dwm.suckless.org/patches/alwayscenter/)
- [barpadding](https://dwm.suckless.org/patches/barpadding/)
- [bartoggle](https://dwm.suckless.org/patches/bartoggle/)
- [centeredwindowname](https://dwm.suckless.org/patches/centeredwindowname/)
- [colorbar](https://dwm.suckless.org/patches/colorbar/)
- [cursorwarp](https://dwm.suckless.org/patches/cursorwarp/)
- [decorhints](https://dwm.suckless.org/patches/decoration_hints/)
- [desktopicons](https://github.com/bakkeby/patches/blob/master/dwm/dwm-desktop_icons-6.5.diff)
- [dragmfacts](https://dwm.suckless.org/patches/dragmfact/)
- [ewmhtags](https://dwm.suckless.org/patches/ewmhtags/)
- [floatbordercolor](https://dwm.suckless.org/patches/float_border_color/)
- [floatborderwidth](https://dwm.suckless.org/patches/floatborderwidth/)
- [fullscreencompilation](https://github.com/bakkeby/patches/wiki/fullscreen-compilation)
- [hidevacanttags](https://dwm.suckless.org/patches/hide_vacant_tags/)
- [ignoretransientwindows](https://dwm.suckless.org/patches/ignore_transient_windows/)
- [ipc](https://github.com/mihirlad55/dwm-ipc)
- [layoutscroll](https://dwm.suckless.org/patches/layoutscroll/)
- [noborderflicker](https://dwm.suckless.org/patches/noborderflicker/)
- [pertag](https://dwm.suckless.org/patches/pertag/)
- [preserveonrestart](https://dwm.suckless.org/patches/preserveonrestart/)
- [resizecorners](https://dwm.suckless.org/patches/resizecorners/)
- [restartsig](https://dwm.suckless.org/patches/restartsig/)
- [rotatestack](https://dwm.suckless.org/patches/rotatestack/)
- [rulerefresher](https://dwm.suckless.org/patches/rulerefresher/)
- [scratchpad](https://dwm.suckless.org/patches/scratchpad/)
- [shiftools](https://dwm.suckless.org/patches/shift-tools/)
- [statuspadding](https://dwm.suckless.org/patches/statuspadding/)
- [steam](https://dwm.suckless.org/patches/steam/)
- [sticky](https://dwm.suckless.org/patches/sticky/)
- [stickyindicator](https://dwm.suckless.org/patches/stickyindicator/)
- [switchallmonitortags](https://dwm.suckless.org/patches/switch_all_monitor_tags/)
- [systray](https://dwm.suckless.org/patches/systray/)
- [underlinetags](https://dwm.suckless.org/patches/underlinetags/)
- [unmanaged](https://github.com/bakkeby/patches/blob/master/dwm/dwm-unmanaged-6.5.diff)
- [vanitygaps](https://dwm.suckless.org/patches/vanitygaps/)
- [winicon](https://dwm.suckless.org/patches/winicon/)

## Known Bugs

- Steam toast notifications not floating and screwing up tiling.
- There might be a conflict between dwm-ipc and pertag, where dwm-msg does not dump the correct values. I will look into it in the future. 

## Future

I still have many plans for changes and features, but currently don't have that much time to put towards this project, so don't expect too many updates, at least not rapid ones. 

