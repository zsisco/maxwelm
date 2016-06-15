# maxwelm (Maximizing Window Element Manager)

maxwelm is written by Zach Sisco `<sisco.z.d@gmail.com>`, 2016.

Maxwelm is a simple window manager based off of [TinyWM](https://github.com/mackstann/tinywm), my previous window manager [ShMOW](https://github.com/zsisco/ShMOW), and [dwm](http://dwm.suckless.org) when I got stuck. 
The main idea is to have a window manager suitable for small screens (and slower machines) that minimizes clutter and maximizes precious screen real estate.
All configuration is done in the config.h file. 

####Dependencies:
- Xlib.
- dmenu (optional).

####Functionality and default hotkeys:
```
Alt + Button1, drag:   interactive window move
Alt + Button3, drag:   interactive window resize
Alt + h/j/k/l:         move window (left/down/up/right)
Alt + Shift + h/j/k/l: resize window (left/down/up/right)
Alt + r:               raise window over pointer
Alt + m:               maximize focused window
Alt + Shift + w:       close focused window
Alt + Tab:             focus next window
Alt + Shift + Tab:     focus previous window
Alt + Enter:           spawn terminal
Alt + p:               spawn dmenu
Alt + Shift + t:       quit
```
