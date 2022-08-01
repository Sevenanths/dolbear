#include <grrlib.h>
#include <ogc/lwp_watchdog.h>   // Needed for gettime and ticks_to_millisecs
#include <fat.h>

#include "test_jpg_jpg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>

#define GRRLIB_WHITE   0xFFFFFFFF

int cursor_x = 300;
int cursor_y = 250;

int main(int argc, char **argv) {
    // Initialise the Graphics & Video subsystem
    GRRLIB_Init();

    // Initialise the Wiimotes
    PAD_Init();

    GRRLIB_texImg *tex_test_jpg = GRRLIB_LoadTexture(test_jpg_jpg);

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

        GRRLIB_DrawImg(cursor_x, cursor_y, tex_test_jpg, 0, 1, 1, GRRLIB_WHITE);  // Draw a jpeg

        GRRLIB_Render();  // Render the frame buffer to the TV
    }

    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB

    exit(0);  // Use exit() to exit a program, do not use 'return' from main()
}
