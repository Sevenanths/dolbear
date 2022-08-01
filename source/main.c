#include <grrlib.h>
#include <ogc/lwp_watchdog.h>   // Needed for gettime and ticks_to_millisecs
#include <fat.h>

#include "background_png.h"
#include "title_png.h"
#include "game_over_png.h"

#include "wall_png.h"
#include "bear_png.h"
#include "star_png.h"
#include "fire_png.h"

#include "button_start_png.h"

#include "dinbekbold_ttf.h"
#include "dinbekbold_png.h"

#include "bg_ogg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <asndlib.h>
#include "oggplayer.h"

#define GRRLIB_WHITE   0xFFFFFFFF
#define GRRLIB_BLACK   0x000000FF

#define OBJECT_WIDTH 32
#define OBJECT_HEIGHT 32
#define GC_WIDTH 640
#define GC_HEIGHT 480

#define BEAR_SPEED 6
#define OBJECT_SPEED 3
#define NUM_OBJECTS 3

#define FONT_TILESET_WIDTH 25

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

enum GameModes {
	TITLE, GAME, GAME_OVER
};

struct Game {
	struct Bear bear;
	struct BearObject objects[NUM_OBJECTS * 2];
	int score;
};

int random_integer(int minimum_number, int max_number) {
	return rand() % (max_number + 1 - minimum_number) + minimum_number;
}

int random_coordinate_x() {
	return random_integer(OBJECT_WIDTH, GC_WIDTH - (OBJECT_WIDTH * 2));
}

int random_coordinate_y() {
	return random_integer(OBJECT_HEIGHT, GC_HEIGHT - (OBJECT_WIDTH * 2));
}

int random_direction() {
	return random_integer(1, 4);
}

int score = 0;
int game_mode = TITLE;
int flicker_timer = 0;
bool show_button_prompt = true;

void init_game(struct Game *game) {
	/* 
    	Bear
    */

    // Set bear coordinates to the middle of the screen
    game->bear.x = (GC_WIDTH / 2) - (OBJECT_WIDTH / 2);
    game->bear.y = (GC_HEIGHT / 2) - (OBJECT_HEIGHT / 2);
    game->bear.direction = random_direction();

    /*
		Objects
    */
    /* Set the first NUM_OBJECTS objects to fire */
    for (int i = 0; i < NUM_OBJECTS; ++i)
    {
    	game->objects[i].type = FIRE;
    }
    /* Set the first NUM_OBJECTS objects to fire */
    for (int i = NUM_OBJECTS; i < 2 * NUM_OBJECTS; ++i)
    {
    	game->objects[i].type = STAR;
    }

    /* Generate random coordinates and directions for all objects */
    for (int i = 0; i < NUM_OBJECTS * 2; ++i)
    {
    	game->objects[i].x = random_coordinate_x();
    	game->objects[i].y = random_coordinate_y();
    	game->objects[i].direction = random_direction();
    }

    score = 0;
}

void draw_title(GRRLIB_texImg *bg_background,
				GRRLIB_texImg *spr_button_start,
				GRRLIB_texImg *bg_overlay,
				GRRLIB_ttfFont *fnt_score,
				char* message) {
	// Draw background
	GRRLIB_DrawImg(0, 0, bg_background, 0, 1, 1, GRRLIB_WHITE);
	// Draw overlay graphic
	GRRLIB_DrawImg((GC_WIDTH / 2) - (bg_overlay->w / 2),
				   (GC_HEIGHT / 2) - (bg_overlay->h / 2) - 30,
				   bg_overlay, 0, 1, 1, GRRLIB_WHITE);

	// Draw message 
	int start_prompt_x = 220;
	int start_prompt_y = 380;
	int start_prompt_size = 30;

	if (ticks_to_secs(gettime()) > flicker_timer + 0.5) {
		show_button_prompt = !show_button_prompt;
		flicker_timer = ticks_to_secs(gettime());
	}

	if (show_button_prompt) {
		GRRLIB_DrawImg(start_prompt_x, start_prompt_y, spr_button_start, 0, 1, 1, GRRLIB_WHITE);
		GRRLIB_PrintfTTF(start_prompt_x + spr_button_start->w + 10,
						 start_prompt_y - (spr_button_start->h / 2) - 2,
						 fnt_score, message, start_prompt_size, GRRLIB_WHITE);
	}
}

void draw_score(int score, GRRLIB_texImg *fnt_score_tile) {
	char str_score[32];
	itoa(score, str_score, 10);

	int score_width = strlen(str_score) * FONT_TILESET_WIDTH;

	// Draw score
	int score_x = (GC_WIDTH / 2) - (score_width / 2);
	int score_y = 1;

	GRRLIB_Printf(score_x + 1, score_y + 1, fnt_score_tile, GRRLIB_BLACK, 1, str_score);
	GRRLIB_Printf(score_x - 1, score_y + 1, fnt_score_tile, GRRLIB_BLACK, 1, str_score);
	GRRLIB_Printf(score_x - 1, score_y - 1, fnt_score_tile, GRRLIB_BLACK, 1, str_score);
	GRRLIB_Printf(score_x + 1, score_y - 1, fnt_score_tile, GRRLIB_BLACK, 1, str_score);
	GRRLIB_Printf(score_x, score_y, fnt_score_tile, GRRLIB_WHITE, 1, str_score);
}

int main(int argc, char **argv) {
    // Initialise the Graphics & Video subsystem
    GRRLIB_Init();

    // Initialise the Wiimotes
    PAD_Init();

	// Initialise the audio subsystem
	ASND_Init();

    GRRLIB_texImg *spr_wall = GRRLIB_LoadTexture(wall_png);
    GRRLIB_texImg *spr_bear = GRRLIB_LoadTexture(bear_png);
    GRRLIB_texImg *spr_fire = GRRLIB_LoadTexture(fire_png);
    GRRLIB_texImg *spr_star = GRRLIB_LoadTexture(star_png);

    GRRLIB_texImg *spr_button_start = GRRLIB_LoadTexture(button_start_png);

    GRRLIB_texImg *bg_background = GRRLIB_LoadTexture(background_png);
    GRRLIB_texImg *bg_title = GRRLIB_LoadTexture(title_png);
    GRRLIB_texImg *bg_game_over = GRRLIB_LoadTexture(game_over_png);

    GRRLIB_ttfFont *fnt_score = GRRLIB_LoadTTF(dinbekbold_ttf, dinbekbold_ttf_size);
    GRRLIB_texImg *fnt_score_tile = GRRLIB_LoadTexture(dinbekbold_png);
    GRRLIB_InitTileSet(fnt_score_tile, FONT_TILESET_WIDTH, 30, 32);

	struct Game* game = malloc(sizeof(struct Game));

    flicker_timer = ticks_to_secs(gettime());  

    PlayOgg(bg_ogg, bg_ogg_size, 0, OGG_INFINITE_TIME);

    // Loop forever
    while(1) {

        PAD_ScanPads();  // Scan the Wiimotes

        if (game_mode == TITLE) {
			draw_title(bg_background, spr_button_start, bg_title, fnt_score, "Press START");

			if (PAD_ButtonsDown(0) & PAD_BUTTON_START) {
				srand(gettime());
				init_game(game);
				game_mode = GAME;
			}

			if (PAD_ButtonsDown(0) & PAD_BUTTON_X) break;
        }
        else if (game_mode == GAME_OVER) {
			draw_title(bg_background, spr_button_start, bg_game_over, fnt_score, "Press START");

			if (PAD_ButtonsDown(0) & PAD_BUTTON_START) {
				game_mode = TITLE;
			}

			draw_score(score, fnt_score_tile);
        }
        else if (game_mode == GAME) {
        	/*
				Bear controls
        	*/

        	if (PAD_StickY(0) > 18 || (PAD_ButtonsDown(0) & PAD_BUTTON_UP)) {
				game->bear.direction = BEAR_UP;
			}
			if (PAD_StickY(0) < -18 || (PAD_ButtonsDown(0) & PAD_BUTTON_DOWN)) {
				game->bear.direction = BEAR_DOWN;
			}
			if (PAD_StickX(0) > 18 || (PAD_ButtonsDown(0) & PAD_BUTTON_RIGHT)) {
				game->bear.direction = BEAR_RIGHT;
			}
			if (PAD_StickX(0) < -18 || (PAD_ButtonsDown(0) & PAD_BUTTON_LEFT)) {
				game->bear.direction = BEAR_LEFT;
			}
	
			/* 
				Bear collision (walls)
			*/
			if (game->bear.x - BEAR_SPEED <= OBJECT_HEIGHT) {
				game->bear.direction = BEAR_RIGHT;
			} else if (game->bear.x + BEAR_SPEED >= GC_WIDTH - (OBJECT_WIDTH * 2)) {
				game->bear.direction = BEAR_LEFT;
			} else if (game->bear.y - BEAR_SPEED <= OBJECT_HEIGHT) {
				game->bear.direction = BEAR_DOWN;
			} else if (game->bear.y + BEAR_SPEED >= GC_HEIGHT - (OBJECT_HEIGHT * 2)) {
				game->bear.direction = BEAR_UP;
			}
	
			/*
				Bear movement
			*/
			switch(game->bear.direction) {
				case BEAR_UP:
					game->bear.y -= BEAR_SPEED;
					break;
				case BEAR_DOWN:
					game->bear.y += BEAR_SPEED;
					break;
				case BEAR_LEFT:
					game->bear.x -= BEAR_SPEED;
					break;
				case BEAR_RIGHT:
					game->bear.x += BEAR_SPEED;
					break;
			}
	
			/*
				Object movement
			*/
			for (int i = 0; i < NUM_OBJECTS * 2; ++i)
			{
				// Left wall collision
				if (game->objects[i].x - OBJECT_SPEED <= OBJECT_WIDTH) {
					if (game->objects[i].direction == UP_LEFT) {
						game->objects[i].direction = UP_RIGHT;
					} else if (game->objects[i].direction == DOWN_LEFT) {
						game->objects[i].direction = DOWN_RIGHT;
					}
				// Right wall collision
				} else if (game->objects[i].x + OBJECT_SPEED >= GC_WIDTH - (OBJECT_WIDTH * 2)) {
					if (game->objects[i].direction == UP_RIGHT) {
						game->objects[i].direction = UP_LEFT;
					} else if (game->objects[i].direction == DOWN_RIGHT) {
						game->objects[i].direction = DOWN_LEFT;
					}
				// Top wall collision
				} else if (game->objects[i].y - OBJECT_SPEED <= OBJECT_HEIGHT) {
					if (game->objects[i].direction == UP_LEFT) {
						game->objects[i].direction = DOWN_LEFT;
					} else if (game->objects[i].direction == UP_RIGHT) {
						game->objects[i].direction = DOWN_RIGHT;
					}
				// Bottom wall
				} else if (game->objects[i].y + OBJECT_SPEED >= GC_HEIGHT - (OBJECT_HEIGHT * 2)) {
					if (game->objects[i].direction == DOWN_LEFT) {
						game->objects[i].direction = UP_LEFT;
					} else if (game->objects[i].direction == DOWN_RIGHT) {
						game->objects[i].direction = UP_RIGHT;
					}
				}
	
				if (game->objects[i].direction == UP_LEFT) {
					game->objects[i].x -= OBJECT_SPEED;
					game->objects[i].y -= OBJECT_SPEED;
				} else if (game->objects[i].direction == UP_RIGHT) {
					game->objects[i].x += OBJECT_SPEED;
					game->objects[i].y -= OBJECT_SPEED;
				} else if (game->objects[i].direction == DOWN_LEFT) {
					game->objects[i].x -= OBJECT_SPEED;
					game->objects[i].y += OBJECT_SPEED;
				} else if (game->objects[i].direction == DOWN_RIGHT) {
					game->objects[i].x += OBJECT_SPEED;
					game->objects[i].y += OBJECT_SPEED;
				}
			}
	
			/* 
				Bear and object collision
			*/
			for (int i = 0; i < NUM_OBJECTS * 2; ++i)
			{
				// All hail the mighty bounding box calculation 
				// I ported this from my LUA code in 2015! It might just work!
				if (game->bear.x < game->objects[i].x + OBJECT_WIDTH &&
					game->bear.x + OBJECT_WIDTH > game->objects[i].x &&
					game->bear.y < game->objects[i].y + OBJECT_HEIGHT &&
					OBJECT_HEIGHT + game->bear.y > game->objects[i].y) {
	
					game->objects[i].x = random_coordinate_x();
					game->objects[i].y = random_coordinate_y();
					game->objects[i].direction = random_direction();
	
					if (game->objects[i].type == FIRE) {
						game_mode = GAME_OVER;
					} else if (game->objects[i].type == STAR) {
						score += 1000;
					}
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
			GRRLIB_DrawImg(game->bear.x, game->bear.y, spr_bear, 0, 1, 1, GRRLIB_WHITE);
	
			// Draw objects
			for (int i = 0; i < NUM_OBJECTS * 2; ++i)
			{
				if (game->objects[i].type == FIRE) {
					GRRLIB_DrawImg(game->objects[i].x, game->objects[i].y, spr_fire, 0, 1, 1, GRRLIB_WHITE);
				} else if (game->objects[i].type == STAR) {
					GRRLIB_DrawImg(game->objects[i].x, game->objects[i].y, spr_star, 0, 1, 1, GRRLIB_WHITE);
				}
			}
	
			draw_score(score, fnt_score_tile);
		}

        GRRLIB_Render();  // Render the frame buffer to the TV
    }

    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB

    exit(0);  // Use exit() to exit a program, do not use 'return' from main()
}
