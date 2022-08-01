#include <grrlib.h>
#include <ogc/lwp_watchdog.h>   // Needed for gettime and ticks_to_millisecs
#include <fat.h>

#include "background_png.h"
#include "wall_png.h"
#include "bear_png.h"
#include "star_png.h"
#include "fire_png.h"

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
#define OBJECT_SPEED 3
#define NUM_OBJECTS 3

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

struct BearObject {
	int type;
	int direction;
	int x;
	int y;
};

enum BearObjectDirection {
	UP_LEFT = 1,
	UP_RIGHT = 2,
	DOWN_LEFT = 3,
	DOWN_RIGHT = 4
};

enum BearObjectTypes {
	FIRE = 1,
	STAR = 2
};

int random_integer(int minimum_number, int max_number) {
	return rand() % (max_number + 1 - minimum_number) + minimum_number;
}

int random_coordinate_x() {
	return random_integer(OBJECT_WIDTH, GC_WIDTH - OBJECT_WIDTH);
}

int random_coordinate_y() {
	return random_integer(OBJECT_HEIGHT, GC_HEIGHT - OBJECT_HEIGHT);
}

int random_direction() {
	return random_integer(1, 4);
}

int main(int argc, char **argv) {
    // Initialise the Graphics & Video subsystem
    GRRLIB_Init();

    // Initialise the Wiimotes
    PAD_Init();

    GRRLIB_texImg *spr_wall = GRRLIB_LoadTexture(wall_png);
    GRRLIB_texImg *spr_bear = GRRLIB_LoadTexture(bear_png);
    GRRLIB_texImg *spr_fire = GRRLIB_LoadTexture(fire_png);
    GRRLIB_texImg *spr_star = GRRLIB_LoadTexture(star_png);
    GRRLIB_texImg *bg_background = GRRLIB_LoadTexture(background_png);

    /* 
    	Bear
    */
    struct Bear *obj_bear = malloc(sizeof(struct Bear));
    // Set bear coordinates to the middle of the screen
    obj_bear->x = (GC_WIDTH / 2) - (OBJECT_WIDTH / 2);
    obj_bear->y = (GC_HEIGHT / 2) - (OBJECT_HEIGHT / 2);

    /*
		Objects
    */
    struct BearObject objects[NUM_OBJECTS * 2];
    /* Set the first NUM_OBJECTS objects to fire */
    for (int i = 0; i < NUM_OBJECTS; ++i)
    {
    	objects[i].type = FIRE;
    }
    /* Set the first NUM_OBJECTS objects to fire */
    for (int i = NUM_OBJECTS; i < 2 * NUM_OBJECTS; ++i)
    {
    	objects[i].type = STAR;
    }

    /* Generate random coordinates and directions for all objects */
    for (int i = 0; i < NUM_OBJECTS * 2; ++i)
    {
    	objects[i].x = random_coordinate_x();
    	objects[i].y = random_coordinate_y();
    	objects[i].direction = random_direction();
    }

    // Loop forever
    while(1) {

        PAD_ScanPads();  // Scan the Wiimotes

        /*
			Bear controls
        */

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
			Bear collision (walls)
		*/
		if (obj_bear->x - BEAR_SPEED <= OBJECT_HEIGHT) {
			obj_bear->direction = BEAR_RIGHT;
		} else if (obj_bear->x + BEAR_SPEED >= GC_WIDTH - (OBJECT_WIDTH * 2)) {
			obj_bear->direction = BEAR_LEFT;
		} else if (obj_bear->y - BEAR_SPEED <= OBJECT_HEIGHT) {
			obj_bear->direction = BEAR_DOWN;
		} else if (obj_bear->y + BEAR_SPEED >= GC_HEIGHT - (OBJECT_HEIGHT * 2)) {
			obj_bear->direction = BEAR_UP;
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

		/*
			Object movement
		*/
		for (int i = 0; i < NUM_OBJECTS * 2; ++i)
		{
			// Left wall collision
			if (objects[i].x - OBJECT_SPEED <= OBJECT_WIDTH) {
				if (objects[i].direction == UP_LEFT) {
					objects[i].direction = UP_RIGHT;
				} else if (objects[i].direction == DOWN_LEFT) {
					objects[i].direction = DOWN_RIGHT;
				}
			// Right wall collision
			} else if (objects[i].x + OBJECT_SPEED >= GC_WIDTH - (OBJECT_WIDTH * 2)) {
				if (objects[i].direction == UP_RIGHT) {
					objects[i].direction = UP_LEFT;
				} else if (objects[i].direction == DOWN_RIGHT) {
					objects[i].direction = DOWN_LEFT;
				}
			// Top wall collision
			} else if (objects[i].y - OBJECT_SPEED <= OBJECT_HEIGHT) {
				if (objects[i].direction == UP_LEFT) {
					objects[i].direction = DOWN_LEFT;
				} else if (objects[i].direction == UP_RIGHT) {
					objects[i].direction = DOWN_RIGHT;
				}
			// Bottom wall
			} else if (objects[i].y + OBJECT_SPEED >= GC_HEIGHT - (OBJECT_HEIGHT * 2)) {
				if (objects[i].direction == DOWN_LEFT) {
					objects[i].direction = UP_LEFT;
				} else if (objects[i].direction == DOWN_RIGHT) {
					objects[i].direction = UP_RIGHT;
				}
			}

			if (objects[i].direction == UP_LEFT) {
				objects[i].x -= OBJECT_SPEED;
				objects[i].y -= OBJECT_SPEED;
			} else if (objects[i].direction == UP_RIGHT) {
				objects[i].x += OBJECT_SPEED;
				objects[i].y -= OBJECT_SPEED;
			} else if (objects[i].direction == DOWN_LEFT) {
				objects[i].x -= OBJECT_SPEED;
				objects[i].y += OBJECT_SPEED;
			} else if (objects[i].direction == DOWN_RIGHT) {
				objects[i].x += OBJECT_SPEED;
				objects[i].y += OBJECT_SPEED;
			}
		}

		/* 
			Bear and object collision
		*/
		for (int i = 0; i < NUM_OBJECTS * 2; ++i)
		{
			// All hail the mighty bounding box calculation 
			// I ported this from my LUA code in 2015! It might just work!
			if (obj_bear->x < objects[i].x + OBJECT_WIDTH &&
				obj_bear->x + OBJECT_WIDTH > objects[i].x &&
				obj_bear->y < objects[i].y + OBJECT_HEIGHT &&
				OBJECT_HEIGHT + obj_bear->y > objects[i].y) {

				objects[i].x = random_coordinate_x();
				objects[i].y = random_coordinate_y();
				objects[i].direction = random_direction();
			}
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

		// Draw objects
		for (int i = 0; i < NUM_OBJECTS * 2; ++i)
		{
			if (objects[i].type == FIRE) {
				GRRLIB_DrawImg(objects[i].x, objects[i].y, spr_fire, 0, 1, 1, GRRLIB_WHITE);
			} else if (objects[i].type == STAR) {
				GRRLIB_DrawImg(objects[i].x, objects[i].y, spr_star, 0, 1, 1, GRRLIB_WHITE);
			}
		}

        GRRLIB_Render();  // Render the frame buffer to the TV
    }

    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB

    exit(0);  // Use exit() to exit a program, do not use 'return' from main()
}
