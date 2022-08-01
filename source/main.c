#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void *Initialise();

int main(int argc, char **argv) {

	xfb = Initialise();

	printf("\nHello World!\n");

	while(1) {

		// Call PAD_ScanPads each loop, this reads the latest controller states
		VIDEO_WaitVSync();
		PAD_ScanPads();

		// PAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		int buttonsDown = PAD_ButtonsDown(0);
		
		if( buttonsDown & PAD_BUTTON_A ) {
			printf("Button A pressed.\n");
		}

		if (buttonsDown & PAD_BUTTON_START) {
			exit(0);
		}
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
