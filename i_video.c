#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// Internal Doom resolution
#define DOOM_W 320
#define DOOM_H 200

// Upscaled window resolution (2x)
#define WINDOW_W 640
#define WINDOW_H 400

// Externs for Doom engine connectivity
typedef struct { unsigned char r, g, b; } doom_rgb_t;
typedef struct { int type; int data1; int data2; int data3; } event_t;
extern unsigned char* screens; 
extern void D_PostEvent(event_t* ev);

// Loom Port Globals
doom_rgb_t* game_palette;
Display*    X_display;
Window      X_window;
XImage*     X_image;

// Key Translator for Loom
int xlatekey(KeySym k) {
    switch(k) {
        case XK_Left:     return 0xac;
        case XK_Right:    return 0xae;
        case XK_Up:       return 0xad;
        case XK_Down:     return 0xaf;
        case XK_Control_L:
        case XK_Control_R: return 0x9d;
        case XK_Escape:    return 0x1b;
        case XK_space:     return ' ';
        case XK_Return:    return 0x0d;
        default:           return k & 0x7f;
    }
}

void I_InitGraphics(void) {
    X_display = XOpenDisplay(NULL);
    if (!X_display) { fprintf(stderr, "Loom Error: No X11 Display!\n"); exit(1); }

    int screen = DefaultScreen(X_display);
    
    // Create window at scaled resolution
    X_window = XCreateSimpleWindow(X_display, RootWindow(X_display, screen), 
                                   0, 0, WINDOW_W, WINDOW_H, 0, 0, 0);

    XSelectInput(X_display, X_window, KeyPressMask | KeyReleaseMask);
    XStoreName(X_display, X_window, "====Loom====");

    // Allocate 32-bit buffer for the upscaled image
    void* buffer = malloc(WINDOW_W * WINDOW_H * 4);
    X_image = XCreateImage(X_display, DefaultVisual(X_display, screen), 
                           DefaultDepth(X_display, screen), ZPixmap, 0, 
                           buffer, WINDOW_W, WINDOW_H, 32, 0);

    XMapWindow(X_display, X_window);
    XSync(X_display, False);
}

void I_StartTic(void) {
    XEvent ev;
    while (XPending(X_display)) {
        XNextEvent(X_display, &ev);
        event_t dev;
        if (ev.type == KeyPress || ev.type == KeyRelease) {
            dev.type = (ev.type == KeyPress) ? 0 : 1;
            dev.data1 = xlatekey(XLookupKeysym(&ev.xkey, 0));
            D_PostEvent(&dev);
        }
    }
}

void I_FinishUpdate(void) {
    if (!game_palette || !screens) return;
    uint8_t* src = screens;
    uint32_t* dest = (uint32_t*)X_image->data;

    for (int y = 0; y < DOOM_H; y++) {
        for (int x = 0; x < DOOM_W; x++) {
            // Get 8-bit pixel and translate to 32-bit RGB
            uint8_t color_idx = src[y * DOOM_W + x];
            uint32_t color32 = (game_palette[color_idx].r << 16) | 
                               (game_palette[color_idx].g << 8) | 
                                game_palette[color_idx].b;

            // Upscale: Paint 1 source pixel into a 2x2 block (4 pixels)
            int dx = x * 2;
            int dy = y * 2;
            dest[dy * WINDOW_W + dx] = color32;         // Top-left
            dest[dy * WINDOW_W + (dx + 1)] = color32;   // Top-right
            dest[(dy + 1) * WINDOW_W + dx] = color32;   // Bottom-left
            dest[(dy + 1) * WINDOW_W + (dx + 1)] = color32; // Bottom-right
        }
    }

    XPutImage(X_display, X_window, DefaultGC(X_display, 0), X_image, 0, 0, 0, 0, WINDOW_W, WINDOW_H);
    XFlush(X_display);
}

void I_SetPalette(unsigned char* pal) { game_palette = (doom_rgb_t*)pal; }
void I_ShutdownGraphics(void) { if (X_display) XCloseDisplay(X_display); }
void I_StartFrame(void) {}
void I_UpdateNoBlit(void) {}
void I_ReadScreen(unsigned char* s) {}
void I_PauseSong(int h) {}
void I_ResumeSong(int h) {}
void I_StopSong(int h) {}
void I_PlaySong(int h, int l) {}
int I_RegisterSong(void* d) { return 1; }
void I_UnRegisterSong(int h) {}
