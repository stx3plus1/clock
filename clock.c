// settings
#define  WIDTH       432
#define  HEIGHT      270
#define  BACKGROUND  0x121212
#define  FOREGROUND1 0xEAEAEA
#define  FOREGROUND2 0xAAAAAA
#define  TIME_FONT   "7\\-Segment:size=96"
#define  DATE_FONT   "7\\-Segment:size=24"
#define  TIME_STR    "%H:%M:%S"
#define  DATE_STR    "%d . %m . %y"
#define  DATE_HEIGHT 64
#define  STR_MAX     128
#define  QUIT_KEY    XK_q

#define  SHOW_TIME
#define  SHOW_DATE
// end settings

#include <time.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include <fontconfig/fontconfig.h>

Display*     dpy;
int          scr;
int          width = WIDTH, height = HEIGHT;
Window       root, wnd;
XftFont*     timefont;
XftFont*     datefont;
XftColor     fg1, fg2;
XftDraw*     xftdraw;
Visual*      vis;
Colormap     cmap;
char         time_str[STR_MAX], date_str[STR_MAX];

void redraw_window(time_t time) {
	struct tm* tm = localtime(&time);
	strftime(time_str, STR_MAX, TIME_STR, tm);
	strftime(date_str, STR_MAX, DATE_STR, tm);

#ifdef SHOW_TIME
	XGlyphInfo time_extents;
	XftTextExtentsUtf8(dpy, timefont, (FcChar8*)time_str, strlen(time_str), &time_extents);

	int time_left = (width  - time_extents.width)  / 2; 
	int time_mid  = (height + time_extents.height) / 2;

	XClearArea(dpy, wnd, 0, time_mid - time_extents.height, 0, time_extents.height, False);
	XftDrawStringUtf8(
		xftdraw, &fg1, timefont,
		time_left, time_mid,
		(FcChar8*)time_str, strlen(time_str)
	);
#endif
#ifdef SHOW_DATE
	XGlyphInfo date_extents;
	XftTextExtentsUtf8(dpy, datefont, (FcChar8*)date_str, strlen(date_str), &date_extents);
	
	int date_left = (width - date_extents.width) / 2;
	int date_mid  = (DATE_HEIGHT + date_extents.height) / 2;

	XClearArea(dpy, wnd, 0, date_mid - date_extents.height, 0, date_extents.height, False);
	XftDrawStringUtf8(
		xftdraw, &fg2, datefont,
		date_left, date_mid,
		(FcChar8*)date_str, strlen(date_str)
	);
#endif
}

int main(int argc, char* argv[]) {
	if (!(dpy = XOpenDisplay(NULL))) {
		char* display = getenv("DISPLAY");
		fprintf(stderr, "Cannot open display %s\n", display ? display : "(DISPLAY is unset, is X running?)");
		return 1;
	}

	root = DefaultRootWindow(dpy);
	scr  = DefaultScreen(dpy);
	vis  = DefaultVisual(dpy, scr);
	cmap = DefaultColormap(dpy, scr);

	if (argc > 1 && !strcmp(argv[1], "-f")) {
		width  = DisplayWidth(dpy, scr);
		height = DisplayHeight(dpy, scr);
	}

	if (!(timefont = XftFontOpenName(dpy, scr, TIME_FONT))) {
		fprintf(stderr, "Cannot open time font / fallback for: %s\n", TIME_FONT);
		return 1;
	}
	if (!(datefont = XftFontOpenName(dpy, scr, DATE_FONT))) {
		fprintf(stderr, "Cannot open date font / fallback for: %s\n", DATE_FONT);
		return 1;
	}

	wnd = XCreateSimpleWindow(
		dpy, root,
		0, 0, width, height,
		0, 0, BACKGROUND
	);
	XStoreName(dpy, wnd, "clock");

	xftdraw = XftDrawCreate(dpy, wnd, vis, cmap);
	XRenderColor fg1_rc = (XRenderColor){
		.red   = ((FOREGROUND1 >> 16) & 0xFF) * 257,
		.green = ((FOREGROUND1 >>  8) & 0xFF) * 257,
		.blue  = ((FOREGROUND1 >>  0) & 0xFF) * 257,
		.alpha = 0xFFFF
	};
	XRenderColor fg2_rc = (XRenderColor){
		.red   = ((FOREGROUND2 >> 16) & 0xFF) * 257,
		.green = ((FOREGROUND2 >>  8) & 0xFF) * 257,
		.blue  = ((FOREGROUND2 >>  0) & 0xFF) * 257,
		.alpha = 0xFFFF
	};
	XftColorAllocValue(dpy, vis, cmap, &fg1_rc, &fg1);
	XftColorAllocValue(dpy, vis, cmap, &fg2_rc, &fg2);
	
	XSelectInput(dpy, wnd, ExposureMask);
	XGrabKey(dpy, XKeysymToKeycode(dpy, QUIT_KEY), AnyModifier, wnd, True, GrabModeAsync, GrabModeAsync);
	XMapWindow(dpy, wnd);

	time_t last = time(NULL), now;

	XEvent ev;
	bool running = true;
	while (running) {
		if ((now = time(NULL)) != last) redraw_window(++last);
		if (XPending(dpy)) {
			XNextEvent(dpy, &ev);
			switch (ev.type) {
			case Expose:   redraw_window(now); break;
			case KeyPress: running = false;    break; 
			}
		}
	}

	XCloseDisplay(dpy);
	return 0;
}
