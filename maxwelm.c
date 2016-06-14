/*
 * maxwelm - Maximizing Window element Manager
 * 
 * Maxwelm is written by Zach Sisco <sisco.z.d@gmail.com> in 2016.
 * This software is in the public domain
 * and is provided AS IS, with NO WARRANTY.
 */

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "config.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define TABLELENGTH(X) (sizeof(X) / sizeof(*X))

static void add_window(Window w);
static void buttonpress(XEvent *ev);
static void close_win();
static void destroynotify(XEvent *ev);
static unsigned long getcolor(const char* color);
static void grabinput();
static void keypress(XEvent *ev);
static void maprequest(XEvent *ev);
static void motionnotify(XEvent *ev);
static void next_win();
static void prev_win();
static void remove_win(Window w);
static void run();
static void send_kill_signal(Window w);
static void setup();
static void sigchld(int unused);
static void spawn(const Arg arg);
static void update_all_titles();
static void update_all_windows();
static void update_title(struct client *c);

static Display *dpy;
static XWindowAttributes attr;
static XButtonEvent start;
static Window root;
static int screen;
static int screen_w;
static int screen_h;
static struct client *head; 
static struct client *current; 
static unsigned int color_focus;
static unsigned int color_unfocus;
static unsigned int color_status;

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
        for (tmp = head; tmp->next; tmp = tmp->next);

        newclient->next = NULL;
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
        fprintf(stderr, "\n\t##############CLOSE WINDOW##################\n");
    }
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

    fprintf(stderr, "\t!!!!!!!!!!!!!!SEEK AND DESTROY!!!!!!!!!!!!!!\n");
    remove_win(dstr->window);
    update_all_titles();
    update_all_windows();
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

    for (i = 0; i < TABLELENGTH(keys); ++i) {
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
    /*KeySym keysym = XkbKeycodeToKeysym(dpy, ev->xkey.keycode, 
                                               0, ev->xkey.state & ShiftMask ? 1 : 0);*/
    KeySym keysym = XkbKeycodeToKeysym(dpy, ev->xkey.keycode, 0, 0);

    enum wm_command cmd = NOP;
    int i;
    for (i = 0; i < TABLELENGTH(keys); ++i) {
        if (keys[i].keysym == keysym && keys[i].mod == ev->xkey.state) {
            cmd = keys[i].wm_cmd;
            break;
        }
    }

    switch (cmd) {
    case MOVE_L:
        if (current != NULL && current->win != None && ev->xkey.state == (1<<3)) {
            XGetWindowAttributes(dpy, current->win, &attr);
            XMoveWindow(dpy, current->win, MAX(1, attr.x - RESIZER), attr.y);
        }
        break;
    case MOVE_D:
        if (current != NULL && current->win != None && ev->xkey.state == (1<<3)) {
            XGetWindowAttributes(dpy, current->win, &attr);
            XMoveWindow(dpy, current->win, attr.x, 
                    (screen_h - attr.height < attr.y + RESIZER ? screen_h - attr.height: attr.y + RESIZER));
        }
        break;
    case MOVE_U:
        if (current != NULL && current->win != None && ev->xkey.state == (1<<3)) {
            XGetWindowAttributes(dpy, current->win, &attr);
            XMoveWindow(dpy, current->win, attr.x, MAX(1, attr.y - RESIZER));
        }
        break;
    case MOVE_R:
        if (current != NULL && current->win != None && ev->xkey.state == (1<<3)) {
            XGetWindowAttributes(dpy, current->win, &attr);
            XMoveWindow(dpy, current->win, 
                    (screen_w - attr.width < attr.x + RESIZER ? screen_w - attr.width : attr.x + RESIZER),
                    attr.y);
        }
        break;
    case RESIZE_L:
        /*
        if (ev->xkey.subwindow != None && ev->xkey.state == (1<<3)) {
            XGetWindowAttributes(dpy, ev->xkey.subwindow, &attr);
            XResizeWindow(dpy, ev->xkey.subwindow, MAX(1, attr.width - RESIZER), attr.height);
        } 
        */
        if (current != NULL && current->win != None) {
            XGetWindowAttributes(dpy, current->win, &attr);
            XResizeWindow(dpy, current->win, MAX(1, attr.width - RESIZER), attr.height);
        }
        break;
    case RESIZE_D:
        if (current != NULL && current->win != None) {
            XGetWindowAttributes(dpy, current->win, &attr);
            XResizeWindow(dpy, current->win, attr.width, 
                    (screen_h - attr.y < attr.height + RESIZER ? screen_h - attr.y: attr.height + RESIZER));
        }
        break;
    case RESIZE_U:
        if (current != NULL && current->win != None) {
            XGetWindowAttributes(dpy, current->win, &attr);
            XResizeWindow(dpy, current->win, attr.width, MAX(1, attr.height - RESIZER));
        }
        break;
    case RESIZE_R:
        if (current != NULL && current->win != None) {
            XGetWindowAttributes(dpy, current->win, &attr);
            XResizeWindow(dpy, current->win, 
                    (screen_w - attr.x < attr.width + RESIZER ? screen_w - attr.x : attr.width + RESIZER),
                    attr.height);
        }
        break;
    case RAISE_WIN:
        if (ev->xkey.subwindow != None && ev->xkey.state == (1<<3)) {
            XRaiseWindow(dpy, ev->xkey.subwindow);
            /* set to current... don't bother unless I really want this */
        }
        break;
    case MAX_WIN:
        if (current != NULL && current->win != None && ev->xkey.state == (1<<3)) 
            XMoveResizeWindow(dpy, current->win, 0, 5, screen_w - 2, screen_h - 20);
        break;
    case CLOSE_WIN:
        close_win();
        break;
    case NEXT_WIN:
        next_win();
        break;
    case PREV_WIN:
        prev_win();
        break;
    case SPAWN:
        spawn(keys[i].arg);
        break;
    case QUIT_WM:
        break;
    default:
        break;
    }
}

void maprequest(XEvent *ev)
{
    XMapRequestEvent *mapev = &ev->xmaprequest;
    add_window(mapev->window);
    XMapWindow(dpy, mapev->window);
    update_all_windows();
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
    }
}

void remove_win(Window w)
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

void run()
{
    XEvent ev;

    start.subwindow = None;

    while (!XNextEvent(dpy, &ev)) {
        switch (ev.type) {
        case KeyPress:
            keypress(&ev);
            break;
        case MapRequest:
            maprequest(&ev);
            break;
        case ButtonPress:
            buttonpress(&ev);
            break;
        case DestroyNotify:
            destroynotify(&ev);
            break;
        case MotionNotify:
            motionnotify(&ev);
            break;
        case ButtonRelease:
            start.subwindow = None;
            break;
        default:
            break; 
        }
        /* DON'T PUT THIS HERE. IT FUCKS UP CLOSING WINDOWS.
        update_all_titles();
        update_all_windows();
        */
    }
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

void setup() 
{
    sigchld(0); 

    screen = DefaultScreen(dpy);
    screen_w = XDisplayWidth(dpy, screen);
    screen_h = XDisplayHeight(dpy, screen);
    root = RootWindow(dpy,screen);

    color_focus = getcolor(FOCUS);
    color_unfocus = getcolor(UNFOCUS);
    color_status = getcolor(STATUS);

    grabinput();

    head = NULL;
    current = NULL;

    XSelectInput(dpy,root,SubstructureNotifyMask|SubstructureRedirectMask|PropertyChangeMask);
}

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
            XSetWindowBorderWidth(dpy, c->win, 2);
            XSetWindowBorder(dpy, c->win, color_focus);
            XSetInputFocus(dpy, c->win, RevertToParent, CurrentTime);
            XRaiseWindow(dpy, c->win);
            /*drawbar();*/
            /*fprintf(stdout, "\tcurrent name: %s\n", c->name);*/
        } else {
            XSetWindowBorder(dpy, c->win, color_unfocus);
        }
    }
}

void update_title(struct client *c) 
{
    char *wname = NULL;
    if (XFetchName(dpy, c->win, &wname) > 0) {
        fprintf(stdout, "[ %s ]", wname);
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

