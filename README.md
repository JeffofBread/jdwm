# jdwm

jdwm is a window manager based on [dwm](https://dwm.suckless.org/), rebuilt from the ground up for ease of use, customizability, and hackability. It 
incorporates over [30 community patches](https://github.com/JeffofBread/jdwm/tree/patches) alongside extensive custom additions to turn the originally spartan dwm into a much 
more modern and compelling alternative to other mainstream window managers.

**Note:** jdwm is still in development and will sometimes have breaking changes or bugs. If you run into any issues, please let me
know through GitHub's [issue](https://github.com/JeffofBread/jdwm/issues) or [discussion](https://github.com/JeffofBread/jdwm/discussions) threads.

<p align="center">
<img src="https://raw.githubusercontent.com/JeffofBread/jdwm/refs/heads/screenshots/jdwm_storm.png" alt="Example image of jdwm 'storm' theme" style="display: block; margin-left: auto; margin-right: auto; width: 65%;"> 
<img src="https://raw.githubusercontent.com/JeffofBread/jdwm/refs/heads/screenshots/jdwm_lush.png" alt="Example image of jdwm 'lush' theme" style="display: block; margin-left: auto; margin-right: auto; width: 65%;"> 
</p>

## Dependencies

- [git](https://git-scm.com/) (assuming you are cloning the repository)
- C Compiler, like [gcc](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/)
- [meson](https://mesonbuild.com/)
- Dependency finder like [pkgconf](https://github.com/pkgconf/pkgconf), [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/), or [cmake](https://cmake.org/)
  ([more info](https://mesonbuild.com/Dependencies.html#dependency-detection-method))
- [libx11](https://gitlab.freedesktop.org/xorg/lib/libx11)
- [libxft](https://gitlab.freedesktop.org/xorg/lib/libxft)
- [imlib2](https://docs.enlightenment.org/api/imlib2/html/)
- [libconfig](https://hyperrealm.github.io/libconfig/)
- [libxinerama](https://gitlab.freedesktop.org/xorg/lib/libxinerama) (optional, highly recommended)
- [xterm](https://xterm.dev/) (Default terminal emulator, adjustable in `jdwm.conf`)
- [JetBrains Mono font](https://github.com/JetBrains/JetBrainsMono) (Default font, adjustable in `jdwm.conf`)

Below are some ready-to-go commands to install all the above programs for a few distributions. If you would
like another distribution or package managers added, I am happy to, just let me know. Also note that the below commands
may not be 100% foolproof across distribution versions or available repositories.

<details><summary><b>Arch</b></summary>

```bash
pacman -S sudo git gcc meson pkgconf libx11 libxft imlib2 libconfig libxinerama xterm ttf-jetbrains-mono
```
or

```bash
yay -S sudo git gcc meson pkgconf libx11 libxft imlib2 libconfig libxinerama xterm ttf-jetbrains-mono
```

</details>

<details><summary><b>Debian/Ubuntu</b></summary>

```bash
apt install sudo git gcc meson pkgconf libc-dev libx11-dev libxft-dev libimlib2-dev libconfig-dev libxinerama-dev xterm fonts-jetbrains-mono
```

</details>

<details><summary><b>Fedora</b></summary>

```bash
dnf install sudo git gcc meson pkgconf libX11-devel libXft-devel imlib2-devel libconfig-devel libXinerama-devel xterm jetbrains-mono-fonts
```

</details>

<details><summary><b>FreeBSD</b></summary>

```bash
pkg install sudo git meson pkgconf libX11 libXft imlib2 libconfig libXinerama xterm jetbrains-mono
```

</details>

<details><summary><b>openSUSE</b></summary>

```bash
zypper install sudo git gcc pkgconf-pkg-config libc-devel libX11-devel libXft-devel imlib2-devel libconfig-devel libXinerama-devel jetbrains-mono-fonts
```

</details>

## Install: Quickstart

**Notes before you install:**

- It is recommended to create a custom configuration to your liking. The default configuration file can be found at `resources/jdwm.conf` or in `/etc/jdwm/` after install. Your
  custom configuration file (named `jdwm.conf`) should be placed in either `~/.config/` or `~/.config/jdwm/` for jdwm to automatically find it at runtime, or use `-C` cli option.
- To edit what programs automatically start alongside jdwm, create a file called `.jdwmrc` in either `~`, `~/.config/`, or `~/.config/jdwm/`. This file will be treated like a
  standard shell script. Make sure it is executable and has a specified interpreter, ex `#!/bin/sh`.

```bash
git clone https://github.com/JeffofBread/jdwm.git
cd jdwm
meson setup --buildtype=release ./build/
meson install -C ./build/
```

## Future & TODO

I have quite a few things I still want to do before I consider jdwm to be at a `1.0` or release ready build. Here are some of the more major things that I know I want to do:

- [ ] Full documentation. I have a lot of the systems already build and fleshed out for this using doxygen, but simply need to sit down and document the remaining ~70% of
  the source code.
- [ ] Rewrite the scratchpad patch. See ["Known Issues"](#known-issues).

I am also have a few other plans, but I am not sure if they will be before a stable `1.x` release:

- [ ] Variable substitution in the libconfig configuration file.
- [ ] Handle status text internally. Currently, jdwm relies on other programs (like [dwmblocks](https://github.com/torrinfail/dwmblocks)) to perform nice, formatted, custom status
  text.
- [ ] Re-write and re-implement IPC. I had previously used [dwm-ipc](https://github.com/mihirlad55/dwm-ipc), but with the level of customization and changes I have made to this
  program, it did not integrate well and had issues. It also has portability issues I would like to overcome.

## Known Issues

- Clients reset to their default state when jdwm restarts. Not sure how/if I will deal with this.
- Notifications from certain more invasive programs are not handled well (ex. Zoom, Steam). Will need to look into methods to detect these notifications to handle them properly.
- Scratchpad sometimes incorrectly modifies visibility of other currently open clients. I intend to re-write the patch.
- Fullscreen windows incorrectly effect the state of a fullscreen window when move into the same layout as the fullscreen window. The
  [fake fullscreen patch](https://dwm.suckless.org/patches/fakefullscreen/) that controls this is a bit of a mess of logic, it needs work.
- When swapping themes during runtime, program tray icons do not update to the new theme. I have not come up with a way to force these to re-render yet, as X11 does not offer
  a way to do this natively. May need to re-think how the systray is themed instead, or use something like [the alpha patch](https://dwm.suckless.org/patches/alpha/) to make it
  completely clear.

## Reaching Out

If you would like to get in contact with me, please either message me on Discord @jeffofbread, or through email at jeffofbreadcoding@gmail.com.

## Credits

I just wanted to put a few special thanks here for a few key people that have helped more directly with jdwm:

- [bakkeby](https://github.com/bakkeby) for tons of help directly through DMs to solve issues and answer my questions. He also has a ton of great [dwm](https://dwm.suckless.org/)
  and [suckless](https://suckless.org/) related projects that I relied on throughout the project for inspiration and code.
- [FT-Labs](https://github.com/FT-Labs) for help answering many of my questions and for inspiring me to make jdwm with his amazing [pdwm](https://github.com/FT-Labs/pdwm) (which I
  also shamelessly copied ideas and code from).
- [Ddubs](https://github.com/dwilliam62) for help beta testing and providing a ton of great ideas and feedback.
