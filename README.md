# maxwelm (Maximizing Window Element Manager)

maxwelm is written by Zach Sisco `<sisco.z.d@gmail.com>`, 2016.

Maxwelm is a simple window manager based off of [catwm](https://github.com/pyknite/catwm), my previous window manager [ShMOW](https://github.com/zsisco/ShMOW), and bits of [dwm](http://dwm.suckless.org) and [TinyWM](https://github.com/mackstann/tinywm) when I get stuck. 
The main idea is to have a window manager suitable for small screens (and slower machines) that minimizes clutter and maximizes precious screen real estate.

####Features
- All windows are maximized on creation (like monocle mode in dwm or other tiling window managers)
- Windows can be moved and resized with the mouse or through hotkeys
- Virtual desktops
- Status bar at the top of the screen displays current desktop number, focused window name, and custom status text

####Configuration
- All configuration is done in the config.h file (mostly key bindings, colors, and custom commands)
- Custom status text is set by "xsetroot -name $status" (similar to dwm) where $status is a string variable. A sample shell script is provided; while starting the window manager, run `./statusbar.sh`.

####Dependencies:
- Xlib.
- dmenu (optional).

####Functionality and default hotkeys:
```
Alt + Button1, drag:   interactive window move
Alt + Button3, drag:   interactive window resize
Alt + h/j/k/l:         move window (left/down/up/right)
Alt + Shift + h/j/k/l: resize window (left/down/up/right)
Alt + m:               maximize focused window
Alt + Shift + w:       close focused window
Alt + Tab:             focus next window
Alt + Shift + Tab:     focus previous window
Alt + (0 - 9)          focus virtual desktop (0 - 9)
Alt + Shift + (0 - 9): move focused window to virtual desktop (0 - 9)
Alt + Enter:           spawn terminal
Alt + p:               spawn dmenu
Alt + Shift + t:       quit
```
