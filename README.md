# maxwelm (Maximizing Window Element Manager)

maxwelm is written by Zach Sisco `<sisco.z.d@gmail.com>`, 2016.

Maxwelm is a simple window manager based off of [catwm](https://github.com/pyknite/catwm), my previous window manager [ShMOW](https://github.com/zsisco/ShMOW), and bits of [dwm](http://dwm.suckless.org) and [TinyWM](https://github.com/mackstann/tinywm) when I get stuck. 
The main idea is to have a window manager suitable for small screens (and slower machines) that minimizes clutter and maximizes precious screen real estate.

####Features
- All windows are maximized on creation (like monocle mode in dwm or other tiling window managers)
- Windows can be moved and resized with the mouse or through hotkeys
- Status bar at the top of the screen displays current desktop number, focused window name, and custom status text
- Virtual desktops

####Configuration
- All configuration is done in `config.h` (mostly key bindings, colors, and custom commands)
- Custom status text is set by `xsetroot -name $status` (similar to dwm) where `$status` is a string variable. A sample shell script is provided -- `statusbar.sh`. It may use programs not installed on your machine; change it to output whatever you like. 

####Dependencies
- Xlib.
- dmenu (optional).

####Installation
- `$ vi config.h`
- `# make install clean`

####Functionality and default hotkeys:
```
Alt + Button1, drag:   interactive window move
Alt + Button3, drag:   interactive window resize
Alt + h/j/k/l:         move window (left/down/up/right)
Alt + Shift + h/j/k/l: resize window (left/down/up/right)
Alt + m:               toggle maximize for focused window
Alt + Shift + w:       close focused window
Alt + Tab:             focus next window
Alt + Shift + Tab:     focus previous window
Alt + (0 - 9)          focus virtual desktop (0 - 9)
Alt + Shift + (0 - 9): move focused window to virtual desktop (0 - 9)
Alt + Enter:           spawn terminal
Alt + p:               spawn dmenu
Alt + Control + t:     quit maxwelm
```

####Screenshots
![floating windows](http://i.imgur.com/qpB3On5.png "floating windows")

![maximized window](http://i.imgur.com/o1xECpD.png "maximized window")
