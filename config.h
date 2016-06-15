#ifndef CONFIG_H
#define CONFIG_H

/* Mod (Mod1 == alt) */
#define MOD Mod1Mask

#define RESIZER 15

/* Colors */
#define FOCUS "rgb:1c/1c/1c"
#define UNFOCUS "rgb:9a/cc/79"
#define STATUS "rgb:1c/1c/1c"

typedef union {
    const char** com;
    const int i;
} Arg;

enum wm_command {MOVE_L, MOVE_D, MOVE_U, MOVE_R,
                 RESIZE_L, RESIZE_D, RESIZE_U, RESIZE_R,
                 RAISE_WIN, MAX_WIN, CLOSE_WIN, 
                 NEXT_WIN, PREV_WIN, SPAWN, QUIT_WM, NOP};

struct key {
    unsigned int mod;
    KeySym keysym;
    enum wm_command wm_cmd;
    const Arg arg;
};

struct client{
    struct client *next;
    struct client *prev;

    Window win;
	char name[256];
};

const char* dmenucmd[] = {"dmenu_run",NULL};
const char* termcmd[]  = {"urxvt",NULL};

/* Shortcuts */
static struct key keys[] = {
    /*MOD              KEY        FUNCTION      ARGS */
    { MOD,             XK_h,      MOVE_L,       {NULL}},
    { MOD,             XK_j,      MOVE_D,       {NULL}},
    { MOD,             XK_k,      MOVE_U,       {NULL}},
    { MOD,             XK_l,      MOVE_R,       {NULL}},
    { MOD|ShiftMask,   XK_h,      RESIZE_L,     {NULL}},
    { MOD|ShiftMask,   XK_j,      RESIZE_D,     {NULL}},
    { MOD|ShiftMask,   XK_k,      RESIZE_U,     {NULL}},
    { MOD|ShiftMask,   XK_l,      RESIZE_R,     {NULL}},
    { MOD,             XK_r,      RAISE_WIN,    {NULL}},
    { MOD,             XK_m,      MAX_WIN,      {NULL}},
    { MOD|ShiftMask,   XK_w,      CLOSE_WIN,    {NULL}},
    { MOD,             XK_Tab,    NEXT_WIN,     {NULL}},
    { MOD|ShiftMask,   XK_Tab,    PREV_WIN,     {NULL}},
    { MOD,             XK_p,      SPAWN,        {.com = dmenucmd}},
    { MOD,             XK_Return, SPAWN,        {.com = termcmd}},
    { MOD|ShiftMask,   XK_t,      QUIT_WM,      {NULL}}
};

#endif

