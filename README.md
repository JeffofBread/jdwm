# jdwm_pure

A custom build of [dwm](https://dwm.suckless.org/) made by myself, JeffofBread. If you find any issues or would like to make a recommendation, feel free. This is a major work in progress and **will** change.

This is the **pure** build, so it is heavily stripped down, for a more traditional dwm experience. I would only recommend this if you are severely limited on performance, storage, or simply have no interest whatsoever in the scripting or integrations of [`jdwm_complete`](https://github.com/JeffofBread/jdwm/tree/jdwm_complete).

<img src="https://github.com/JeffofBread/jdwm/blob/screenshots/jdwm_storm.png"> 
<img src="https://github.com/JeffofBread/jdwm/blob/screenshots/jdwm_lush.png">

## Install

**Notes before you install:**
 - Make sure you have all the necessary [required dependencies](#required-dependencies), or issues could occur during installation.
 - jdwm is only compatable with X11, not Wayland. Please make sure you use X11.

```bash
git clone https://github.com/JeffofBread/jdwm.git -b jdwm_pure
cd jdwm
sudo make install
```

## Uninstall

```bash
cd jdwm
sudo make uninstall
```

## Required Dependencies

These are all programs or libraries jdwm relies on (specified in `()`) in some way or another. To not have them would severely reduce the functionality or result in the whole thing being non-functional.

Also, if anyone would like to add more package managers or distros to this list, I would appreciate it, please either reach out on discord @jeffofbread, start a discussion here on github, or create a pull request. 

- A terminal emulator of some kind (chosen in `/jdwm/binds.h`, by default it is [st](https://st.suckless.org/))
- A desktop notification handler of some kind (I recommend [dunst](https://github.com/dunst-project/dunst))
- [yajl](https://lloyd.github.io/yajl/) (dwm IPC)
- [imlib2](https://docs.enlightenment.org/api/imlib2/html/) (for program icons in the bar)
- [argp](https://www.gnu.org/software/libc/manual/html_node/Argp.html) (for cli arguments in jdwm and jdwm-msg)
- [sed](https://www.gnu.org/software/sed/manual/sed.html) (Makefile install)

<details><summary><b>Arch</b></summary>

```bash
sudo pacman -S git make gcc libx11 libxinerama libxft yajl imlib2 sed
```
or

```bash
yay -S git make gcc libx11 libxinerama libxft yajl imlib2 sed
```

</details>

<details><summary><b>Debain/Ubuntu</b></summary>

```bash
sudo apt install git build-essential libx11-dev libxinerama-dev libxft-dev libyajl-dev libimlib2-dev sed
```

</details>

<details><summary><b>Fedora</b></summary>

```bash
sudo dnf install git gcc make libX11-devel libXinerama-devel libXft-devel yajl-devel imlib2-devel sed
```

</details>


## Usage

This is not yet a comprehensive list, just a good place to start. All of the keybinds and button binds are found and edited in `/jdwm/binds.h`. 

<details><summary><b>Keybind Quickstart</b></summary><p><div>

These are just the keybinds I find myself using the most often, though "*quick*" might not be the best descriptor here. In the interest of keeping this a little shorter, I won't include them here, but I would also recommend checking out the media keybinds as well. 

Keybind | Action
----------------- | ----------
 <kbd>Win</kbd> + <kbd>Q</kbd> | Kill focused window 
 <kbd>Win</kbd> + <kbd>1</kbd> .. <kbd>9</kbd> | Navigate to `number` tag on the focused monitor
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>1</kbd> .. <kbd>9</kbd> | Send focused window and view to `number` tag on the focused monitor
 <kbd>Win</kbd> + <kbd>Enter</kbd> | Spawn st
 <kbd>Win</kbd> + <kbd>~</kbd> | Spawn an st scratchpad
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>~</kbd> | Spawn another st scratchpad
 <kbd>Win</kbd> + <kbd>&leftarrow;</kbd> | Move focus to the next left monitor
 <kbd>Win</kbd> + <kbd>&rightarrow;</kbd> | Move focus to the next right monitor
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>&leftarrow;</kbd> | Move focused window to next left monitor
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>&rightarrow;</kbd> | Move focused window to next right monitor
 <kbd>Win</kbd> + <kbd>&uparrow;</kbd> | Move focus up client stack
 <kbd>Win</kbd> + <kbd>&downarrow;</kbd> | Move focus down client stack
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>&uparrow;</kbd> | Move focused window up a tag
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>&downarrow;</kbd> | Move focused window down a tag
 <kbd>Win</kbd> + <kbd>F</kbd> | Make the focused window fullscreen, taking up the whole monitor
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>F</kbd> | Fake Fullscreen, makes the focused window respect layout even when, for example, a YouTube video is fullscreen. 
 <kbd>Win</kbd> + <kbd>Shift</kbd> + <kbd>Q</kbd> | Reload jdwm, same as sending `kill -HUP $(pidof jdwm)`
 <kbd>Win</kbd> + <kbd>Control</kbd> + <kbd>Shift</kbd> + <kbd>Q</kbd> | Quit jdwm, same as sending `kill -TERM $(pidof jdwm)` 

</div></p></details>

<details><summary><b>Media Keybinds (disabled by default in this build (jdwm_pure))</b></summary><p><div>

Keybind | Action
----------------- | ----------
 <kbd>Win</kbd> + <kbd>PageUp</kbd> | Increase volume by 5% using [pamixer](https://github.com/cdemoulins/pamixer)
 <kbd>Win</kbd> + <kbd>PageDown</kbd> | Decrease volume by 5% using [pamixer](https://github.com/cdemoulins/pamixer) 
 <kbd>Win</kbd> + <kbd>ScrollLock</kbd> | Play the next song in queue using [playerctl](https://github.com/altdesktop/playerctl)
 <kbd>Win</kbd> + <kbd>Print</kbd> | Play the previous song in queue using [playerctl](https://github.com/altdesktop/playerctl)
 <kbd>Win</kbd> + <kbd>Home</kbd> | Skip 10 seconds forward using [playerctl](https://github.com/altdesktop/playerctl)
 <kbd>Win</kbd> + <kbd>Home</kbd> | Rewind 10 seconds backwards using [playerctl](https://github.com/altdesktop/playerctl)
 <kbd>Win</kbd> + <kbd>Pause</kbd> | Pause/Play media using [playerctl](https://github.com/altdesktop/playerctl)
 <kbd>Win</kbd> + <kbd>Insert</kbd> | Toggle mute audio output using [pactl](https://www.freedesktop.org/wiki/Software/PulseAudio/Documentation/User/CLI/#pactl)
 <kbd>Win</kbd> + <kbd>Delete</kbd> | Toggle mute microphone using [pactl](https://www.freedesktop.org/wiki/Software/PulseAudio/Documentation/User/CLI/#pactl)

</div></p></details>


<details><summary><b>Button Binds</b></summary><p><div>

Button + Keybind | Bar Section | Action
---------------- | ----------- | ----------
 <kbd>LMB</kbd> | Layout Symbol | Toggle between most recent layouts
 <kbd>LMB</kbd> | Tag Number | Send focus to that tag
 <kbd>LMB</kbd> + <kbd>Win</kbd> | Tag Number | Send focus and focused window to that tag
 <kbd>LMB</kbd> + <kbd>Win</kbd> | Window | Click and drag floating windows around
 <kbd>LMB</kbd> + <kbd>Win</kbd> + <kbd>Control</kbd> | Window | Click and drag any non-floating window into floating
 <kbd>MMB</kbd> | Window Title | Spawn st
 <kbd>MMB</kbd> + <kbd>Win</kbd> | Window | Toggle window's floating state
 <kbd>RMB</kbd> | Tag Number | Focus that tag as well as your current tag (view both at the same time)
 <kbd>RMB</kbd> + <kbd>Win</kbd> | Window | Click and drag floating window's edge to resize the window

</div></p></details>

## Patches

All the original .diff files are located in [jdwm patches branch](https://github.com/JeffofBread/jdwm/tree/patches). Needless to say that a lot of tweaks had to be made to many of these patches when implemented to make them all work together, so the .diff files may not represent exactly what is present in jdwm. These changes are to be expected when adding 34 patches on top of each other, and is not a fault of the original authors. Also, many of these patches were pulled from [bakkeby's](https://github.com/bakkeby) incredible [flexipatch](https://github.com/bakkeby/dwm-flexipatch), but I frankly don't remember exactly which, though I have tried my best to provide links below. Also, major thanks to [bakkeby](https://github.com/bakkeby) and [FT-Labs](https://github.com/FT-Labs) for help with a few problems as well as for making their own builds of dwm, which I shamelessly pulled from.


<details><summary><b>Patches integrated in jdwm</b></summary>
<br>

- [alwayscenter](https://dwm.suckless.org/patches/alwayscenter/)
- [barpadding](https://dwm.suckless.org/patches/barpadding/)
- [bartoggle](https://dwm.suckless.org/patches/bartoggle/)
- [centeredwindowname](https://dwm.suckless.org/patches/centeredwindowname/)
- [colorbar](https://dwm.suckless.org/patches/colorbar/)
- [cursorwarp](https://dwm.suckless.org/patches/cursorwarp/)
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

</details>

## Known Bugs

- Steam (specifically toast notifications) causing tiling issues.
- There might be a conflict between dwm-ipc and pertag, where jdwm-msg does not always dump correct values. I will look into it in the future. 

## Future

I still have many plans for changes and features, but currently don't have that much time to put towards this project, so don't expect too many updates, at least not rapid ones. If you would like to suggest ideas, features, or contribute, please either reach out on discord @jeffofbread, start a discussion here on github, or create a pull request. 

