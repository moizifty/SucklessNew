/* See LICENSE file for copyright and license details. */

#include "/home/moiz/.cache/wal/colors-wal-dwm.h"
#include <X11/XF86keysym.h>

/* appearance */
typedef enum
{
    CENTER,
    RIGHT,
    LEFT,
}HoriAlignment;

typedef enum
{
    TOP,
    BOTTOM,
}VertAlignment;

typedef enum
{
    TagsAllElem,
    TagsCurrentElem,
    LayoutCurrentElem,
    LayoutAllElem,
    TitleElem,
    StatusElem,
    SeperatorElem,
}ElementType;

typedef struct
{
    HoriAlignment align;
    ElementType type;
    unsigned int intData[10];
    const char *strData;
    unsigned int xpos;
    unsigned int w;
}BarElement;

//TODO: create a function to toggle this
//and allow the bar length formatted
//eg make char * variable
//and int variable
//lets say char * variable is "w" and int is 2
//this means the status bar will disaplay 2 words max
//TODO: FORMAT OPTIONS:
//w : defines number of words to display
//+: defines max length of title bar in px
//-: defines min length of title bar in px
//f: defines whether the bar is fixed length
//F: fixed length but if 'w' is also specified then it will display
//c: defines number of characters


static const unsigned int titlemaxwidth = 250;  // this sets the max width of title bar
                                                                                        //make these the same for fixed width
static const unsigned int titleminwidth = 150; // this sets minimum width of title bar
static const unsigned int titlewidthfittext = 0; //this makes it so the width of the title bar fits the text with respect to max length of bar

static const unsigned int titleintform = 6;  // this is the max number of words the title bar can display (max length over rides this if the words dont fit)
static const char titleFormat[] = "w"; // IGNORE THIS, this isnt implemented fully yet

// keybinding is available for this, to toggle it. function is called togglealttitle
static unsigned int titlestatus = 0;  //  PUT STATUS Text on title bar, this is functional but i haven't tested it in all cases

HoriAlignment tagIndHAlign = CENTER;  //the tagindicators horizontal alignment: see above for options
VertAlignment tagIndVAlign = TOP; //the tagindicators vertical alignment: see above for options

/* the tag indicators are centered in the element by default */
static const unsigned int tagindicatornormwidth = 3;
static const unsigned int tagindicatorselwidth = 100;
static const unsigned int tagindicatorheight = 3;
static const unsigned int statuspadpx = 8;   // this gives padding to the status text

/*Structuring the Elements and available types
 *
 * All elements have the same first 4 int data
 *
 * first: inverse colorscheme, 1 to enable 0 to disable
 * second: perminantly select the element (background will always be selected bg), 1 to enable 0 to disable
 * third: switch the normal color scheme with selected color scheme, 1 to enable 0 to disbale
 * fourth: isnt implemented yet leave as 0
 *
 * some element have specific int datas
 * 
 * Displays all tags (like default)
 * TagsAllElem,
 *
 * Displays all active tags only or tags with program 
 * TagsCurrentElem,
 *
 * Displays only current layout (like default)
 * LayoutCurrentElem,
 *
 * Displays all layouts 
 * LayoutAllElem,     fifth: should the current layout be displayed first in the list of layouts
 *
 * Displays title of focused window
 * TitleElem,         fifth: if this element is last, fill the remaining bar, 1 to enable, 0 to disable
 *
 * Displays status
 * StatusElem,
 *
 * Displays some text which is specified with strData, can be used as seperator
 * SeperatorElem,
 *
 */
static BarElement barOrder[] = 
{
    // IMPORTANT NOTE: only the last element can have RIGHT as the alignment
    // i havent finished implementing the alignment feature yet but it works for the last item
    // also see BarElement struct above.
    //
    //make sure to fill EVERY int data with 1 or 0, not filling some in will cause weird things
//  ALIGNMENT,  ELEMENTTYPE, INTDATA, STRDATA // SEE ABOVE FOR MORE INFO
    {LEFT, TitleElem,       .intData = {0, 1, 0, 0, 0}, .strData = "" }, //you dont have to put .intData or .strData but it helps for clarity
    {LEFT, TagsCurrentElem, .intData = {0 , 1, 0, 0},   .strData = "" }, //also some elements dont use strData so leave it empty
    {LEFT, SeperatorElem,   .intData = {0, 1, 0, 0},    .strData = "::" },
    {LEFT, LayoutAllElem,   .intData = {0, 0, 0, 0, 1}, .strData = "" },
    //{RIGHT, StatusElem, {0,0,0}},
};
static const unsigned int fontborder = 6;   // this gives the font some padding
static const unsigned int borderpx   = 6;        /* border pixel of windows */
static const unsigned int gappx      = 12;        /* gaps between windows */
static const unsigned int snap       = 32;       /* snap pixel */
static const int showbar             = 1;        /* 0 means no bar */
static const int topbar              = 1;        /* 0 means bottom bar */
static const char *fonts[]           = { "anonymous pro:size=15" };
static const char dmenufont[]        = "siji:size=15";
/* tagging */

static const char *tags[] = { "一", "二", "三", "四", "五", "六", "七", "八", "九" };
static const char *tagsalt[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.5; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	{ "⽥",      tile},
    { "〓",      NULL},    
	{ "⼞",      monocle},
	{ "⼯",      bstack},
	{ "⽬",      bstackhoriz},
    { "⾲",     centeredmaster},
    { "亜",    centeredfloatingmaster},
};

/*static const Layout layouts[] = {
    { "\ue002",*/


/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", norm_bg, "-nf", norm_fg, "-sb", sel_bg, "-sf", sel_fg, NULL };
static const char *termcmd[]  = { "st", NULL };
static const char *browsercmd[] = { "chromium", NULL };
static const char *termfmcmd[]  = { "st", "vifm", NULL };
static const char *graphfmcmd[] = { "pcmanfm", NULL };


static const char *backlightdown[] = {"notifBrightDown", NULL };
static const char *backlightup[] = {"notifBrightUp",NULL};
static const char *volup[]      = { "pamixer", "-i", "2", NULL };
static const char *voldown[]    = { "pamixer", "-d", "2", NULL };
static const char *voltoggle[]  = { "pamixer", "-t", NULL };
static const char *audiotoggle[] = { "notiMusicToggle",NULL };
static const char *audiostop[]  = { "playerctl", "stop", NULL };
static const char *audionext[]  = { "playerctl", "next", NULL };
static const char *audioprev[]  = { "playerctl", "previous", NULL };


static Key keys[] = {
	/* modifier                     key        function        argument */
    { 0,                            XF86XK_MonBrightnessUp,   spawn,          {.v = backlightup } },
    { 0,                            XF86XK_MonBrightnessDown, spawn,          {.v = backlightdown } },
    { 0,                            XF86XK_AudioMute,         spawn,          {.v = voltoggle } },
    { 0,                            XF86XK_AudioRaiseVolume,  spawn,          {.v = volup } },
    { 0,                            XF86XK_AudioLowerVolume,  spawn,          {.v = voldown } },
    { 0,                            XF86XK_AudioPlay,         spawn,          {.v = audiotoggle } },
    { 0,                            XF86XK_AudioStop,         spawn,          {.v = audiostop } },
    { 0,                            XF86XK_AudioNext,         spawn,          {.v = audionext } },
    { 0,                            XF86XK_AudioPrev,         spawn,          {.v = audioprev } },
    { MODKEY|ShiftMask,             XK_w,                     spawn,          {.v = browsercmd} },
    { MODKEY|ShiftMask,             XK_v,                     spawn,          {.v = termfmcmd } }, 
    { MODKEY|ShiftMask|ControlMask, XK_v,                     spawn,          {.v = graphfmcmd } }, 
	{ MODKEY,                       XK_p,                     spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return,                spawn,          {.v = termcmd } }, 
	{ MODKEY,                       XK_b,                     togglebar,      {0} }, 
	{ MODKEY,                       XK_j,                     focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,                     focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,                     incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,                     incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,                     setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,                     setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return,                zoom,           {0} },
	{ MODKEY,                       XK_Tab,                   view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,                     killclient,     {0} },
	{ MODKEY,                       XK_t,                     setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,                     setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,                     setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_y,                     setlayout,      {.v = &layouts[3]} },
	{ MODKEY|ControlMask,           XK_y,                     setlayout,      {.v = &layouts[4]} },
    { MODKEY,                       XK_u,                     setlayout,      {.v = &layouts[5]} },
    { MODKEY|ControlMask,           XK_u,                     setlayout,      {.v = &layouts[6]} },
	{ MODKEY,                       XK_space,                 setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,                 togglefloating, {0} }, 
	{ MODKEY|ShiftMask,             XK_f,                     togglefullscr,  {0} }, 
	{ MODKEY,                       XK_0,                     view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,                     tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,                 focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period,                focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,                 tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period,                tagmon,         {.i = +1 } },
    { MODKEY,                       XK_minus,                 setgaps,        {.i = -1 } },
    { MODKEY,                       XK_equal,                 setgaps,        {.i = +1 } },
	{ MODKEY,                       XK_n,                     togglealttag,   {0} },
    { MODKEY,                       XK_s,                     togglealttitle, {0} },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,                     quit,           {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0}},
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

