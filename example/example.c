#include <stdint.h>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef intptr_t isize;

// TODO(khvorov) Remove
#include <stdlib.h>

#include <unistd.h>
#include <time.h>
#include <X11/Xlib.h>

int
main() {
    Display* dpy = XOpenDisplay(NULL);
    int      screen = DefaultScreen(dpy);
    int      width = 500;
    int      height = 500;
    Window   win = XCreateWindow(dpy, DefaultRootWindow(dpy), 0, 0, width, height, 0, 0, InputOutput, CopyFromParent, 0, 0);
    XMapWindow(dpy, win);

    XImage* img = XCreateImage(dpy, DefaultVisual(dpy, screen), 24, ZPixmap, 0, NULL, 500, 500, 32, 0);

    // TODO(khvorov) Arena
    img->data = (char*)malloc(width * height * sizeof(u32));

    XSelectInput(
        dpy,
        win,
        SubstructureNotifyMask | ExposureMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask
            | KeyPressMask | KeyReleaseMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask | ButtonMotionMask
            | KeymapStateMask | FocusChangeMask | PropertyChangeMask
    );

    // NOTE(khvorov) Get ClientMessage on window close/Alt+F4
    {
        Atom wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(dpy, win, &wm_delete_window, 1);
    }

    u8    redVal = 0;
    isize redvalDelta = 1;
    for (;;) {
        while (XPending(dpy)) {
            XEvent event = {};
            XNextEvent(dpy, &event);
            if (event.type == ClientMessage) {
                _exit(0);
            }
        }

        for (int i = 0; i < 500 * 500; i++) {
            ((u32*)img->data)[i] = (redVal << 16) | (0 << 8) | 0;
        }
        if (redVal == 0) {
            redvalDelta = 1;
        } else if (redVal == 255) {
            redvalDelta = -1;
        }
        redVal += redvalDelta;

        XPutImage(dpy, win, DefaultGC(dpy, screen), img, 0, 0, 0, 0, 500, 500);

        // NOTE(khvorov) Frame timing
        {
            struct timespec toSleep = {0, 16 * 1000 * 1000};
            nanosleep(&toSleep, 0);
        }
    }

    XCloseDisplay(dpy);
    return 0;
}
