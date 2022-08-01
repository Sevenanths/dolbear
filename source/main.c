#include <grrlib.h>
#include <ogc/lwp_watchdog.h>   // Needed for gettime and ticks_to_millisecs
#include <fat.h>

#include "background_png.h"
#include "wall_png.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>

#define GRRLIB_WHITE   0xFFFFFFFF

#define OBJECT_WIDTH 32
#define OBJECT_HEIGHT 32
#define GC_WIDTH 640
#define GC_HEIGHT 480

int cursor_x = 300;
int cursor_y = 250;

int main(int argc, char **argv) {
    // Initialise the Graphics & Video subsystem
    GRRLIB_Init();

    // Initialise the Wiimotes
    PAD_Init();

    GRRLIB_texImg *spr_wall = GRRLIB_LoadTexture(wall_png);
    GRRLIB_texImg *bg_background = GRRLIB_LoadTexture(background_png);

    // Loop forever
    while(1) {

        PAD_ScanPads();  // Scan the Wiimotes

        // If [HOME] was pressed on the first Wiimote, break out of the loop
        if (PAD_ButtonsDown(0) & PAD_BUTTON_START)  break;

        if (PAD_StickY(0) > 18) {
			cursor_y--;
		}
		if (PAD_StickY(0) < -18) {
			cursor_y++;
		}
		if (PAD_StickX(0) > 18) {
			cursor_x++;
		}
		if (PAD_StickX(0) < -18) {
			cursor_x--;
		}

        // ---------------------------------------------------------------------
        // Place your drawing code here
        // ---------------------------------------------------------------------

		GRRLIB_DrawImg(0, 0, bg_background, 0, 1, 1, GRRLIB_WHITE);

		// Horizontal walls
		for (int x = 0; x < 20; ++x)
		{
			// Top walls
			GRRLIB_DrawImg(x * OBJECT_WIDTH, 0, spr_wall, 0, 1, 1, GRRLIB_WHITE);
			// Bottom walls
			GRRLIB_DrawImg(x * OBJECT_WIDTH, GC_HEIGHT - OBJECT_HEIGHT, spr_wall, 0, 1, 1, GRRLIB_WHITE);
		}

		// Vertical walls
		for (int y = 0; y < 15; ++y)
		{
			// Top walls
			GRRLIB_DrawImg(0, y * OBJECT_HEIGHT, spr_wall, 0, 1, 1, GRRLIB_WHITE);
			// Bottom walls
			GRRLIB_DrawImg(GC_WIDTH - OBJECT_WIDTH, y * OBJECT_HEIGHT, spr_wall, 0, 1, 1, GRRLIB_WHITE);
		}

        // Vertical walls

        GRRLIB_Render();  // Render the frame buffer to the TV
    }

    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB

    exit(0);  // Use exit() to exit a program, do not use 'return' from main()
}
