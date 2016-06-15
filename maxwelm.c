/*
 * maxwelm - Maximizing Window element Manager
 * 
 * Maxwelm is written by Zach Sisco <sisco.z.d@gmail.com>, 2016.
 * 
 * Borrowed interactive pointer move/resize code from TinyWM. 
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define LENGTH(X) (sizeof(X) / sizeof(*X))
#define RESIZER 15
#define TOPBAR 15

enum direction {LEFT, DOWN, UP, RIGHT};

typedef union {
    const char** com;
    const int i;
    const enum direction dir;
} Arg;

struct key {
    unsigned int mod;
    KeySym keysym;
    void (*function)(const Arg arg);
    const Arg arg;
};

struct client {
    struct client *next;
    struct client *prev;

    Window win;
	char name[256];
};

struct desktop {
    struct client *head;
    struct client *current;
};

/* declare functions */
static void add_window(Window w);
static void buttonpress(XEvent *ev);
static void buttonrelease(XEvent *ev);
static void change_desktop(const Arg arg);
static void client_to_desktop(const Arg arg);
static void close_win();
static void configurerequest(XEvent *e);
static void destroynotify(XEvent *ev);
static void drawbar();
static unsigned long getcolor(const char* color);
static void grabinput();
static void keypress(XEvent *ev);
static void maprequest(XEvent *ev);
static void max_win();
static void motionnotify(XEvent *ev);
static void move_win(const Arg arg);
static void next_win();
static void prev_win();
static void quit_wm();
static void remove_window(Window w);
static void resize_win(const Arg arg);
static void run();
static void save_desktop(int d);
static void select_desktop(int d);
static void send_kill_signal(Window w);
static GC setcolor(const char* col);
static void setup();
static void sigchld(int unused);
static void spawn(const Arg arg);
static void update_all_titles();
static void update_all_windows();
static void update_title(struct client *c);

/* variables */
static XWindowAttributes attr;
static Colormap cmap;
static XColor color;
static unsigned int color_focus;
static unsigned int color_unfocus;
static unsigned int color_status;
static struct client *current; 
static unsigned int currentdesktop;
static struct desktop desktops[10];
static Display *dpy;
static GC gc;
static void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = buttonpress,
	[ButtonRelease] = buttonrelease,
	[ConfigureRequest] = configurerequest,
	[DestroyNotify] = destroynotify,
	[KeyPress] = keypress,
	[MapRequest] = maprequest,
	[MotionNotify] = motionnotify
};
static struct client *head; 
static Window root;
static int screen;
static int screen_w;
static int screen_h;
static XButtonEvent start;
static char status_text[256];

/* include config here to use structs defined above */
#include "config.h"

void add_window(Window new_win)
{
    struct client *newclient, *tmp;

    if (!(newclient = (struct client *)calloc(1, sizeof(struct client)))) {
        fprintf(stderr, "calloc error!\n");
        exit(1);
    }

    newclient->win = new_win;

    if (head == NULL) {
        newclient->next = NULL;
        newclient->prev = NULL;
        head = newclient;
    } else {
        for (tmp = head; tmp->next; tmp = tmp->next)
            if (tmp == current)
                break;

        if (tmp->next != NULL)
            tmp->next->prev = newclient;

        newclient->next = tmp->next;
        newclient->prev = tmp;
        
        tmp->next = newclient;
    }

    update_title(newclient);

    current = newclient;
}

void buttonpress(XEvent *ev)
{
    if (ev->xbutton.subwindow != None) {
        XGetWindowAttributes(dpy, ev->xbutton.subwindow, &attr);
        start = ev->xbutton;
    }
}

void buttonrelease(XEvent *ev)
{
    start.subwindow = None;
}

void change_desktop(const Arg arg)
{
    int d = arg.i;
    struct client *c;

    if (d == currentdesktop)
        return;

    if (head != NULL)
        for (c = head; c; c = c->next)
            XUnmapWindow(dpy, c->win);

    save_desktop(currentdesktop);
    
    select_desktop(d);

    if (head != NULL)
        for (c = head; c; c = c->next)
            XMapWindow(dpy, c->win);

    update_all_windows();
    drawbar();
}

void client_to_desktop(const Arg arg)
{
    int d = arg.i;
    struct client *movec = current;
    int orig_desktop = currentdesktop;

    if (d == currentdesktop || current == NULL)
        return;

    save_desktop(orig_desktop);
    select_desktop(d);
    add_window(movec->win);
    save_desktop(d);

    select_desktop(orig_desktop);
    remove_window(movec->win);
    XUnmapWindow(dpy, movec->win);

    update_all_windows();
    drawbar();
}

void close_win()
{
    if (current != NULL) {
		/* send delete signal to window */
		XEvent ke;
		ke.type = ClientMessage;
		ke.xclient.window = current->win;
		ke.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
		ke.xclient.format = 32;
		ke.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
		ke.xclient.data.l[1] = CurrentTime;
		XSendEvent(dpy, current->win, False, NoEventMask, &ke);
        send_kill_signal(current->win);
    }
}

void configurerequest(XEvent *e) {
    /* Paste from dwm */
    XConfigureRequestEvent *ev = &e->xconfigurerequest;
    XWindowChanges wc;
    wc.x = ev->x;
    wc.y = ev->y;
    wc.width = ev->width;
    wc.height = ev->height;
    wc.border_width = ev->border_width;
    wc.sibling = ev->above;
    wc.stack_mode = ev->detail;
    XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
}

void destroynotify(XEvent *ev)
{
    int i = 0;
    struct client *c;
    XDestroyWindowEvent *dstr = &ev->xdestroywindow;

    for (c = head; c; c = c->next) {
        if (dstr->window == c->win) {
            i++;
        }
    }

    if (i == 0)
        return;

    remove_window(dstr->window);
    update_all_titles();
    update_all_windows();
    drawbar();
}

void drawbar()
{
    Window win = XCreateSimpleWindow(dpy, root, 0, 0, screen_w, TOPBAR, 0, color_focus, color_focus);
	XMapWindow(dpy, win);
	XDrawRectangle(dpy, win, setcolor(FOCUS), 0, 0, screen_w, TOPBAR - 1);
	XFillRectangle(dpy, win, setcolor(FOCUS), 0, 0, screen_w, TOPBAR);
    
    char desktoptext[4]; 
    snprintf(desktoptext, 4, "[%d]", currentdesktop);

    XDrawString(dpy, win, setcolor(STATUSTXT), 5, TOPBAR - 3, desktoptext, 5);

    XSync(dpy, False);
}

unsigned long getcolor(const char* color)
{
    XColor c;
    Colormap map = DefaultColormap(dpy, screen);

    if(!XAllocNamedColor(dpy, map, color, &c, &c)) {
        fprintf(stderr, "Error parsing color!");
        exit(1);
    }

    return c.pixel;
}

void grabinput() 
{
    int i;
    KeyCode code;

    for (i = 0; i < LENGTH(keys); ++i) {
        if ((code = XKeysymToKeycode(dpy, keys[i].keysym))) {
            XGrabKey(dpy, code, keys[i].mod, root, True, GrabModeAsync, GrabModeAsync);
        }
    }

    XGrabButton(dpy, 1, MOD, root, True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, MOD, root, True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
}


void keypress(XEvent *ev)
{
    KeySym keysym = XkbKeycodeToKeysym(dpy, ev->xkey.keycode, 0, 0);
    int i;
    for (i = 0; i < LENGTH(keys); ++i) {
        if (keys[i].keysym == keysym && keys[i].mod == ev->xkey.state) {
            keys[i].function(keys[i].arg);
            break;
        }
    }
}

void maprequest(XEvent *ev)
{
    static XWindowAttributes wa;
    XMapRequestEvent *mapev = &ev->xmaprequest;

    if (!XGetWindowAttributes(dpy, mapev->window, &wa))
        return;
    if (wa.override_redirect) /* for popups/dialogs */
        return;

    add_window(mapev->window);
    /* Map window and maximize, true to name */
    XMapWindow(dpy, mapev->window);
    max_win();
    update_all_titles();
    update_all_windows();
    drawbar();
}

void max_win()
{
    if (current != NULL && current->win != None) 
        XMoveResizeWindow(dpy, current->win, 0, TOPBAR, screen_w - 2, screen_h - (TOPBAR * 2) - 2);
}

void motionnotify(XEvent *ev)
{
    if (start.subwindow != None) {
        int xdiff = ev->xbutton.x_root - start.x_root;
        int ydiff = ev->xbutton.y_root - start.y_root;

        XMoveResizeWindow(dpy, start.subwindow,
            attr.x + (start.button==1 ? xdiff : 0),
            attr.y + (start.button==1 ? ydiff : 0),
            MAX(1, attr.width + (start.button==3 ? xdiff : 0)),
            MAX(1, attr.height + (start.button==3 ? ydiff : 0)));
    }
}

void move_win(const Arg arg)
{
    static int x, y;
    enum direction dir = arg.dir;

    if (current != NULL && current->win != None) {
        XGetWindowAttributes(dpy, current->win, &attr);
        switch (dir) {
        case LEFT:
            x = MAX(1, attr.x - RESIZER);
            y = attr.y;
            break;
        case DOWN:
            x = attr.x;
            y = (screen_h - attr.height < attr.y + RESIZER ? screen_h - attr.height: attr.y + RESIZER);
            break;
        case UP:
            x = attr.x;
            y = MAX(1, attr.y - RESIZER);
            break;
        case RIGHT:
            x = (screen_w - attr.width < attr.x + RESIZER ? screen_w - attr.width : attr.x + RESIZER);
            y = attr.y;
            break;
        default:
            break;
        }
        XMoveWindow(dpy, current->win, x, y);
    }
}

void next_win()
{
    struct client *c; 

    if (current != NULL && head != NULL) {
        if (current->next == NULL)
            c = head;
        else
            c = current->next;

        current = c;
        update_all_windows();
        drawbar();
    }
}

void prev_win()
{
    struct client *c; 

    if (current != NULL && head != NULL) {
        if (current->prev == NULL)
            for (c = head; c->next; c = c->next);
        else
            c = current->prev;

        current = c;
        update_all_windows();
        drawbar();
    }
}

void quit_wm()
{
    fprintf(stdout, "quitting...jk nm\n");
}

void remove_window(Window w)
{
    struct client *c;

    for (c = head; c != NULL; c = c->next) {
        if (c->win == w) {
            if (c->prev == NULL && c->next == NULL) {
                free(head);
                head = NULL;
                current = NULL;
                return;
            } else if (c->prev == NULL) {
                head = c->next;
                c->next->prev = NULL;
                current = c->next;
            } else if (c->next == NULL) {
                c->prev->next = NULL;
                current = c->prev;
            } else {
                c->prev->next = c->next;
                c->next->prev = c->prev;
                current = c->prev;
            }

            free(c);
            return;
        }
    }
}

void resize_win(const Arg arg)
{
    static int w, h;
    enum direction dir = arg.dir;

    if (current != NULL && current->win != None) {
        XGetWindowAttributes(dpy, current->win, &attr);
        switch (dir) {
        case LEFT:
            w = MAX(1, attr.width - RESIZER);
            h = attr.height;
            break;
        case DOWN:
            w = attr.width;
            h = (screen_h - attr.y < attr.height + RESIZER ? screen_h - attr.y: attr.height + RESIZER);
            break;
        case UP:
            w = attr.width;
            h = MAX(1, attr.height - RESIZER);
            break;
        case RIGHT:
            w = (screen_w - attr.x < attr.width + RESIZER ? screen_w - attr.x : attr.width + RESIZER);
            h = attr.height;
            break;
        default:
            break;
        }
        XResizeWindow(dpy, current->win, w, h);
    }
}

void run()
{
    XEvent ev;

    start.subwindow = None;

	XSync(dpy, False);

	/* Credit to dwm for the O(1)-time event loop */
	while (!XNextEvent(dpy, &ev))
		if (handler[ev.type])
			handler[ev.type](&ev); /* call handler */
}

void save_desktop(int d)
{
    desktops[d].head = head;
    desktops[d].current = current;
}

void select_desktop(int d)
{
    head = desktops[d].head;
    current = desktops[d].current;
    currentdesktop = d;
}

void send_kill_signal(Window w)
{
    XEvent ke;
    ke.type = ClientMessage;
    ke.xclient.window = w;
    ke.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
    ke.xclient.format = 32;
    ke.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
    ke.xclient.data.l[1] = CurrentTime;
    XSendEvent(dpy, w, False, NoEventMask, &ke);
}

GC setcolor(const char* col) 
{
	XAllocNamedColor(dpy, cmap, col, &color, &color);
	XSetForeground(dpy, gc, color.pixel);
	return gc;
}

void setup() 
{
    sigchld(0); 

    screen = DefaultScreen(dpy);
    screen_w = XDisplayWidth(dpy, screen);
    screen_h = XDisplayHeight(dpy, screen);
    root = RootWindow(dpy,screen);

    grabinput();

    head = NULL;
    current = NULL;

    int i;
    for (i = 0; i < 10; i++) {
        desktops[i].head = head;
        desktops[i].current = current;
    }

    /* Select first desktop as default */
    const Arg arg = {.i = 1};
    currentdesktop = arg.i;
    change_desktop(arg);

    /* init color stuff */
    color_focus = getcolor(FOCUS);
    color_unfocus = getcolor(UNFOCUS);
    color_status = getcolor(STATUS);
    cmap = DefaultColormap(dpy, screen);
    XGCValues val;
    val.font = XLoadFont(dpy, "fixed");
    gc = XCreateGC(dpy, root, GCFont, &val);

    strncpy(status_text, "maxwelm\0", sizeof(status_text));
    drawbar();

    XSelectInput(dpy,root,SubstructureNotifyMask|SubstructureRedirectMask|PropertyChangeMask);
}

/* Credit to dwm for this */
void sigchld(int unused) 
{
	if (signal(SIGCHLD, sigchld) == SIG_ERR) {
		fprintf(stderr, "Can't install SIGCHLD handler");
        exit(1);
    }
	while (0 < waitpid(-1, NULL, WNOHANG));
}

void spawn(const Arg arg)
{
    if (fork() == 0) {
        if (fork() == 0) {
            if (dpy)
                close(ConnectionNumber(dpy));

            setsid();
            execvp((char*)arg.com[0], (char**)arg.com);
        }
        exit(0);
    }
}

void update_all_titles()
{
    struct client *tmp;
    tmp = head;
    if (head == NULL)
        return;

    fprintf(stdout, "\n");
    update_title(tmp);
    while (tmp->next) {
        fprintf(stdout, " -> ");
        tmp = tmp->next;
        update_title(tmp);
    }
    fprintf(stdout, "\n");
}

void update_all_windows()
{
    struct client *c;

    for (c = head; c; c = c->next) {
        if (current == c) {
            XSetWindowBorderWidth(dpy, c->win, 1);
            XSetWindowBorder(dpy, c->win, color_focus);
            XSetInputFocus(dpy, c->win, RevertToParent, CurrentTime);
            XRaiseWindow(dpy, c->win);
        } else {
            XSetWindowBorder(dpy, c->win, color_unfocus);
        }
    }
}

void update_title(struct client *c) 
{
    char *wname = NULL;
    if (XFetchName(dpy, c->win, &wname) > 0) {
        fprintf(stdout, "[%d|%s]", currentdesktop, wname);
        strncpy(c->name, wname, sizeof(c->name));
        XFree(wname);
    }
}

int main(void) 
{
    
    if(!(dpy = XOpenDisplay(0x0))) return 1;

    setup();

    run();

    XCloseDisplay(dpy);

    return 0;
}


