#include <stdint.h>
#include <stdalign.h>

#define function static

// clang-format off
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define clamp(x, a, b) (((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x)))
#define arrayCount(arr) (isize)(sizeof(arr) / sizeof(arr[0]))
#define arenaAllocArray(arena, type, len) (type*)arenaAllocAndZero(arena, (len) * (isize)sizeof(type), alignof(type))
#define arenaAllocStruct(arena, type) (type*)arenaAllocAndZero(arena, sizeof(type), alignof(type))
#define isPowerOf2(x) (((x) > 0) && (((x) & ((x)-1)) == 0))
#define unused(x) ((x) = (x))
#define assert(cond) do {if (cond) {} else {__builtin_debugtrap();}} while (0)
// clang-format on

typedef uint8_t   u8;
typedef uint32_t  u32;
typedef intptr_t  isize;
typedef uintptr_t usize;

function isize
getOffsetForAlignment(void* ptr, isize align) {
    assert(isPowerOf2(align));
    usize ptrAligned = (usize)((u8*)ptr + (align - 1)) & (usize)(~(align - 1));
    assert(ptrAligned >= (usize)ptr);
    isize diff = (isize)(ptrAligned - (usize)ptr);
    assert(diff < align && diff >= 0);
    return diff;
}

typedef struct Arena {
    void* base;
    isize size;
    isize used;
    isize tempCount;
} Arena;

function void*
arenaFreePtr(Arena* arena) {
    void* result = (u8*)arena->base + arena->used;
    return result;
}

function isize
arenaFreeSize(Arena* arena) {
    isize result = arena->size - arena->used;
    return result;
}

function void
arenaChangeUsed(Arena* arena, isize byteDelta) {
    assert(arenaFreeSize(arena) >= byteDelta);
    arena->used += byteDelta;
}

function void
arenaAlignFreePtr(Arena* arena, isize align) {
    isize offset = getOffsetForAlignment(arenaFreePtr(arena), align);
    arenaChangeUsed(arena, offset);
}

function void*
arenaAllocAndZero(Arena* arena, isize size, isize align) {
    arenaAlignFreePtr(arena, align);
    void* result = arenaFreePtr(arena);
    arenaChangeUsed(arena, size);
    for (isize ind = 0; ind < size; ind++) {
        ((u8*)result)[ind] = 0;
    }
    return result;
}

function Arena
createArenaFromArena(Arena* parent, isize bytes) {
    Arena arena = {
        .base = arenaFreePtr(parent),
        .size = bytes,
        .used = 0,
        .tempCount = 0,
    };
    arenaChangeUsed(parent, bytes);
    return arena;
}

typedef struct TempMemory {
    Arena* arena;
    isize  usedAtBegin;
    isize  tempCountAtBegin;
} TempMemory;

function TempMemory
beginTempMemory(Arena* arena) {
    TempMemory temp = {.arena = arena, .usedAtBegin = arena->used, .tempCountAtBegin = arena->tempCount};
    arena->tempCount += 1;
    return temp;
}

function void
endTempMemory(TempMemory temp) {
    assert(temp.arena->tempCount == temp.tempCountAtBegin + 1);
    temp.arena->used = temp.usedAtBegin;
    temp.arena->tempCount -= 1;
}

#if (defined(linux) || defined(__linux) || defined(__linux__))

#include <unistd.h>
#include <time.h>
#include <sys/mman.h>

#undef function
#include <X11/Xlib.h>

int
main() {
    Arena arena = {};
    {
        isize bytes = 1 * 1024 * 1024 * 1024;
        void* ptr = mmap(0, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        assert(ptr != MAP_FAILED);
        arena = (Arena) {.base = ptr, .size = bytes};
    }

    Display* dpy = XOpenDisplay(NULL);
    int      screen = DefaultScreen(dpy);
    int      width = 500;
    int      height = 500;
    Window   win = XCreateWindow(dpy, DefaultRootWindow(dpy), 0, 0, width, height, 0, 0, InputOutput, CopyFromParent, 0, 0);
    XMapWindow(dpy, win);

    XImage* img = XCreateImage(dpy, DefaultVisual(dpy, screen), 24, ZPixmap, 0, NULL, width, height, 32, 0);
    img->data = (char*)arenaAllocArray(&arena, u32, width * height);

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

#endif  // linux
