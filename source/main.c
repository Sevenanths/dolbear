#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void *Initialise();

int cursor_x = 300;
int cursor_y = 250;

void DrawHLine(int x1, int x2, int y, int color) {
    int i;
    y = 320 * y;
    x1 >>= 1;
    x2 >>= 1;
    for (i = x1; i <= x2; i++) {
        u32 *tmpfb = xfb;
        tmpfb[y+i] = color;
    }
}

void DrawVLine(int x, int y1, int y2, int color) {
    int i;
    x >>= 1;
    for (i = y1; i <= y2; i++) {
        u32 *tmpfb = xfb;
        tmpfb[x + (640 * i) / 2] = color;
    }
}

void DrawBox(int x1, int y1, int x2, int y2, int color) {
    DrawHLine (x1, x2, y1, color);
    DrawHLine (x1, x2, y2, color);
    DrawVLine (x1, y1, y2, color);
    DrawVLine (x2, y1, y2, color);
}

int main(int argc, char **argv) {

	xfb = Initialise();

	printf("\nHello World!\n");

	while(1) {

		// Call PAD_ScanPads each loop, this reads the latest controller states
		VIDEO_WaitVSync();
		PAD_ScanPads();

		// PAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		// Other possibilities: ButtonsHeld and ButtonsUp
		// first argument = controller number (0 = first controller!)
		int buttonsDown = PAD_ButtonsDown(0);
		
		if( buttonsDown & PAD_BUTTON_A ) {
			//printf("Button A pressed.\n");
		}

		if (buttonsDown & PAD_BUTTON_START) {
			exit(0);
		}

		// Analog movement
		if (PAD_StickY(0) > 18) {
			//printf("Joystick moved up.\n");
			cursor_y -= 20;
		}

		if (PAD_StickY(0) < -18) {
			//printf("Joystick moved down\n");
			cursor_y += 20;
		}

		if (PAD_StickX(0) > 18) {
			//printf("Joystick moved right.\n");
			cursor_x += 20;
		}

		if (PAD_StickX(0) < -18) {
			//printf("Joystick moved left\n");
			cursor_x -= 20;
		}

		// Clear the framebuffer to avoid "Mario Kart 8 test track" vibes
		VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);

		DrawBox (cursor_x, cursor_y, cursor_x + 1, cursor_y + 1, COLOR_WHITE);

		// Vsync, all good 👍
		VIDEO_WaitVSync();
	}

	return 0;
}

void * Initialise() {

	void *framebuffer;

	// Initialise the video system
	VIDEO_Init();

	// This function initialises the attached controllers
	PAD_Init();
	
	// Obtain the preferred video mode from the system
	rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	// Initialise the console, required for printf
	console_init(framebuffer,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	
	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);

	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(framebuffer);

	// Make the display visible
	VIDEO_SetBlack(FALSE);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	return framebuffer;

}