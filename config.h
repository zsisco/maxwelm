#ifndef CONFIG_H
#define CONFIG_H

/* Mod (Mod1 == alt) */
#define MOD Mod1Mask

/* Colors */
#define FOCUS "rgb:1c/1c/1c"
#define UNFOCUS "rgb:9a/cc/79"
#define STATUS "rgb:1c/1c/1c"

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
    { MOD,             XK_1,      DESK_1,       {NULL}},
    { MOD,             XK_2,      DESK_2,       {NULL}},
    { MOD,             XK_3,      DESK_3,       {NULL}},
    { MOD,             XK_4,      DESK_4,       {NULL}},
    { MOD,             XK_5,      DESK_5,       {NULL}},
    { MOD,             XK_6,      DESK_6,       {NULL}},
    { MOD,             XK_7,      DESK_7,       {NULL}},
    { MOD,             XK_8,      DESK_8,       {NULL}},
    { MOD,             XK_9,      DESK_9,       {NULL}},
    { MOD|ShiftMask,   XK_1,      MOVEDESK_1,   {NULL}},
    { MOD|ShiftMask,   XK_2,      MOVEDESK_2,   {NULL}},
    { MOD,             XK_p,      SPAWN,        {.com = dmenucmd}},
    { MOD,             XK_Return, SPAWN,        {.com = termcmd}},
    { MOD|ShiftMask,   XK_t,      QUIT_WM,      {NULL}}
};

#endif

