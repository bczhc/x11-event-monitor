//
// Created by bczhc on 24/08/22.
//

// gcc -o xclipget xclipget.c -lX11
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include "clipboard.h"

typedef void (*Callback)(char *, size_t);

// https://stackoverflow.com/questions/8755471/x11-wait-for-and-get-clipboard-text
Bool PrintSelection(Display *display, Window window, const char *bufname, const char *fmtname, Callback callback) {
    char *result;
    unsigned long ressize, restail;
    int resbits;
    Atom bufid = XInternAtom(display, bufname, False),
            fmtid = XInternAtom(display, fmtname, False),
            propid = XInternAtom(display, "XSEL_DATA", False),
            incrid = XInternAtom(display, "INCR", False);
    XEvent event;

    XSelectInput(display, window, PropertyChangeMask);
    XConvertSelection(display, bufid, fmtid, propid, window, CurrentTime);
    do {
        XNextEvent(display, &event);
    } while (event.type != SelectionNotify || event.xselection.selection != bufid);

    if (event.xselection.property) {
        XGetWindowProperty(display, window, propid, 0, LONG_MAX / 4, True, AnyPropertyType,
                           &fmtid, &resbits, &ressize, &restail, (unsigned char **) &result);
        if (fmtid != incrid)
            callback(result, ressize);
        XFree(result);

        if (fmtid == incrid)
            do {
                do {
                    XNextEvent(display, &event);
                } while (event.type != PropertyNotify || event.xproperty.atom != propid ||
                         event.xproperty.state != PropertyNewValue);

                XGetWindowProperty(display, window, propid, 0, LONG_MAX / 4, True, AnyPropertyType,
                                   &fmtid, &resbits, &ressize, &restail, (unsigned char **) &result);
                callback(result, ressize);
                XFree(result);
            } while (ressize > 0);

        return True;
    } else // request failed, e.g. owner can't convert to the target format
        return False;
}

void WatchSelection(Display *display, Window window, const char *bufname, Callback callback) {
    int event_base, error_base;
    XEvent event;
    Atom bufid = XInternAtom(display, bufname, False);

    assert(XFixesQueryExtension(display, &event_base, &error_base));
    XFixesSelectSelectionInput(display, DefaultRootWindow(display), bufid, XFixesSetSelectionOwnerNotifyMask);

    while (True) {
        XNextEvent(display, &event);

        if (event.type == event_base + XFixesSelectionNotify &&
            ((XFixesSelectionNotifyEvent *) &event)->selection == bufid) {
            if (!PrintSelection(display, window, bufname, "UTF8_STRING", callback))
                PrintSelection(display, window, bufname, "STRING", callback);
        }
    }
}

void start(void (*callback)(char *data, size_t size)) {
    Display *display = XOpenDisplay(NULL);
    unsigned long color = BlackPixel(display, DefaultScreen(display));
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 1, 1, 0, color, color);

    WatchSelection(display, window, "CLIPBOARD", callback);
}

/*int main() {
    Display *display = XOpenDisplay(NULL);
    unsigned long color = BlackPixel(display, DefaultScreen(display));
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 1, 1, 0, color, color);
    //    Bool result = PrintSelection(display, window, "CLIPBOARD", "UTF8_STRING") ||
    //                  PrintSelection(display, window, "CLIPBOARD", "STRING");

    WatchSelection(display, window, "CLIPBOARD", printCallback);

    XDestroyWindow(display, window);
    XCloseDisplay(display);
    //    return !result;
    return 0;
}*/
