#include <grrlib.h>
#include <ogc/lwp_watchdog.h>   // Needed for gettime and ticks_to_millisecs
#include <fat.h>

#include "background_png.h"
#include "wall_png.h"
#include "bear_png.h"

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

#define BEAR_SPEED 6

struct Bear {
	int x;
	int y;
	int direction;
};

enum BearDirection {
	BEAR_UP = 1,
	BEAR_DOWN = 2,
	BEAR_LEFT = 3,
	BEAR_RIGHT = 4
}; 

int main(int argc, char **argv) {
    // Initialise the Graphics & Video subsystem
    GRRLIB_Init();

    // Initialise the Wiimotes
    PAD_Init();

    GRRLIB_texImg *spr_wall = GRRLIB_LoadTexture(wall_png);
    GRRLIB_texImg *spr_bear = GRRLIB_LoadTexture(bear_png);
    GRRLIB_texImg *bg_background = GRRLIB_LoadTexture(background_png);

    struct Bear *obj_bear = malloc(sizeof(struct Bear));
    // Set bear coordinates to the middle of the screen
    obj_bear->x = (GC_WIDTH / 2) - (OBJECT_WIDTH / 2);
    obj_bear->y = (GC_HEIGHT / 2) - (OBJECT_HEIGHT / 2);

    // Loop forever
    while(1) {

        PAD_ScanPads();  // Scan the Wiimotes

        // If [HOME] was pressed on the first Wiimote, break out of the loop
        if (PAD_ButtonsDown(0) & PAD_BUTTON_START)  break;

        if (PAD_StickY(0) > 18) {
			obj_bear->direction = BEAR_UP;
		}
		if (PAD_StickY(0) < -18) {
			obj_bear->direction = BEAR_DOWN;
		}
		if (PAD_StickX(0) > 18) {
			obj_bear->direction = BEAR_RIGHT;
		}
		if (PAD_StickX(0) < -18) {
			obj_bear->direction = BEAR_LEFT;
		}

		/*
			Bear movement
		*/
		switch(obj_bear->direction) {
			case BEAR_UP:
				obj_bear->y -= BEAR_SPEED;
				break;
			case BEAR_DOWN:
				obj_bear->y += BEAR_SPEED;
				break;
			case BEAR_LEFT:
				obj_bear->x -= BEAR_SPEED;
				break;
			case BEAR_RIGHT:
				obj_bear->x += BEAR_SPEED;
				break;
		}

        // ---------------------------------------------------------------------
        // Place your drawing code here
        // ---------------------------------------------------------------------

		/*
			Draw window
		*/
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

		// Draw bear
		GRRLIB_DrawImg(obj_bear->x, obj_bear->y, spr_bear, 0, 1, 1, GRRLIB_WHITE);

        GRRLIB_Render();  // Render the frame buffer to the TV
    }

    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB

    exit(0);  // Use exit() to exit a program, do not use 'return' from main()
}
