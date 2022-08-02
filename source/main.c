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

#ifdef __gamecube__
#include "button_start_png.h"
#elif __wii__
#include "button_plus_png.h"
#endif

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

//#ifdef __wii__
#include <wiiuse/wpad.h>
//#endif

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
bool rumbling = false;

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
    /* Set the first group of NUM_OBJECTS objects to fire */
    for (int i = 0; i < NUM_OBJECTS; ++i)
    {
    	game->objects[i].type = FIRE;
    }
    /* Set the second group op NUM_OBJECTS objects to fire */
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

	// Every 0.5 seconds, we show/hide the button prompt
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

	// We use a tileset font in order to be able to pre-calculate the score's width
	// This way we can actually center it on screen
	int score_width = strlen(str_score) * FONT_TILESET_WIDTH;

	// Draw score
	int score_x = (GC_WIDTH / 2) - (score_width / 2);
	int score_y = 1;

	// The first four calls are used to draw the font outline
	GRRLIB_Printf(score_x + 1, score_y + 1, fnt_score_tile, GRRLIB_BLACK, 1, str_score);
	GRRLIB_Printf(score_x - 1, score_y + 1, fnt_score_tile, GRRLIB_BLACK, 1, str_score);
	GRRLIB_Printf(score_x - 1, score_y - 1, fnt_score_tile, GRRLIB_BLACK, 1, str_score);
	GRRLIB_Printf(score_x + 1, score_y - 1, fnt_score_tile, GRRLIB_BLACK, 1, str_score);
	// The final call is the actual centred font
	GRRLIB_Printf(score_x, score_y, fnt_score_tile, GRRLIB_WHITE, 1, str_score);
}

void start_game(struct Game *game) {
	srand(gettime());
	init_game(game);
	game_mode = GAME;
}

void rumble()
{
    static int tps=0;
    if (rumbling && ticks_to_millisecs(gettime())-tps>=250)
    {
    	#ifdef __wii__
        WPAD_Rumble(WPAD_CHAN_0, 0);
        #elif __gamecube__
    	PAD_ControlMotor(0, PAD_MOTOR_STOP);
        #endif

        tps = ticks_to_millisecs(gettime());
        rumbling = false;
    }

}

int main(int argc, char **argv) {
    // Initialise the Graphics & Video subsystem
    GRRLIB_Init();

    #ifdef __wii__
	// Initialise the Wiimotes
	WPAD_Init();
	#endif

    // Initialise GameCube controller input
    // I allow GameCube input for the Wii too, which is why this isn't in its own ifdef
    PAD_Init();

	// Initialise the audio subsystem
	ASND_Init();

	// Sprite loading
    GRRLIB_texImg *spr_wall = GRRLIB_LoadTexture(wall_png);
    GRRLIB_texImg *spr_bear = GRRLIB_LoadTexture(bear_png);
    GRRLIB_texImg *spr_fire = GRRLIB_LoadTexture(fire_png);
    GRRLIB_texImg *spr_star = GRRLIB_LoadTexture(star_png);

    #ifdef __gamecube__
    GRRLIB_texImg *spr_button_start = GRRLIB_LoadTexture(button_start_png);
    #elif __wii__
    GRRLIB_texImg *spr_button_start = GRRLIB_LoadTexture(button_plus_png);
    #endif

    GRRLIB_texImg *bg_background = GRRLIB_LoadTexture(background_png);
    GRRLIB_texImg *bg_title = GRRLIB_LoadTexture(title_png);
    GRRLIB_texImg *bg_game_over = GRRLIB_LoadTexture(game_over_png);

    // Font loading
    GRRLIB_ttfFont *fnt_score = GRRLIB_LoadTTF(dinbekbold_ttf, dinbekbold_ttf_size);
    GRRLIB_texImg *fnt_score_tile = GRRLIB_LoadTexture(dinbekbold_png);
    GRRLIB_InitTileSet(fnt_score_tile, FONT_TILESET_WIDTH, 30, 32);

    // Create a game instance
	struct Game* game = malloc(sizeof(struct Game));

    flicker_timer = ticks_to_secs(gettime());  

    PlayOgg(bg_ogg, bg_ogg_size, 0, OGG_INFINITE_TIME);

    #ifdef __gamecube__
    char* button_prompt = "Press START";
    #elif __wii__
    char* button_prompt = "Press PLUS";
    #endif

    // Loop forever
    while(1) {

        PAD_ScanPads();  // Scan for GameCube controller input

        #ifdef __wii__
        WPAD_ScanPads();  // Scan for Wii remote input

        // Scan for Wii remote extensions
		expansion_t e;
		WPAD_Expansion(0, &e);
        #endif

        if (rumbling) {
        	rumble();
        }

        if (game_mode == TITLE) {
			draw_title(bg_background, spr_button_start, bg_title, fnt_score, button_prompt);

			if (PAD_ButtonsDown(0) & PAD_BUTTON_START) {
				start_game(game);
			}

			if (PAD_ButtonsDown(0) & PAD_BUTTON_X) break;

			#ifdef __wii__
			// If Classic Controller
			if (e.type == WPAD_EXP_CLASSIC) {
				if (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_HOME) break;
	
				if (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_PLUS) {
					start_game(game);
				}
			// If no Classic Controller
			} else {
				if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) break;
	
				if (WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS) {
					start_game(game);
				}
			}
			#endif
        }
        else if (game_mode == GAME_OVER) {
			draw_title(bg_background, spr_button_start, bg_game_over, fnt_score, button_prompt);

			if (PAD_ButtonsDown(0) & PAD_BUTTON_START) game_mode = TITLE;

			#ifdef __wii__
			// If Classic Controller
			if (e.type == WPAD_EXP_CLASSIC) {
				if (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_PLUS) game_mode = TITLE;
			// If no Classic Controller
			} else {
				if (WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS) game_mode = TITLE;	
			}
			#endif

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

			#ifdef __wii__
			if(e.type == WPAD_EXP_NUNCHUK || e.type == WPAD_EXP_CLASSIC)
			{
				int nx;
				int ny;

				// Analog movement
				if (e.type == WPAD_EXP_NUNCHUK) {
					// Nunchuk movement
    				nx = e.nunchuk.js.pos.x - e.nunchuk.js.center.x;
					ny = e.nunchuk.js.pos.y - e.nunchuk.js.center.y;
				} else if (e.type == WPAD_EXP_CLASSIC) {
					// Classic controller movement
    				nx = e.classic.ljs.pos.x - e.classic.ljs.center.x;
					ny = e.classic.ljs.pos.y - e.classic.ljs.center.y;
				}

				if (ny > 18) {
					game->bear.direction = BEAR_UP;
				}
				if (ny < -18) {
					game->bear.direction = BEAR_DOWN;
				}
				if (nx > 18) {
					game->bear.direction = BEAR_RIGHT;
				}
				if (nx < -18) {
					game->bear.direction = BEAR_LEFT;
				}

				// Digital movement
				if (e.type == WPAD_EXP_CLASSIC) {
					// Classic Controller movement
					if (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_UP) {
						game->bear.direction = BEAR_UP;
					}
					if (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_DOWN) {
						game->bear.direction = BEAR_DOWN;
					}
					if (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_RIGHT) {
						game->bear.direction = BEAR_RIGHT;
					}
					if (WPAD_ButtonsDown(0) & WPAD_CLASSIC_BUTTON_LEFT) {
						game->bear.direction = BEAR_LEFT;
					}
				}
			} else {
				// Sideways remote movement
				if (WPAD_ButtonsDown(0) & WPAD_BUTTON_RIGHT) {
					game->bear.direction = BEAR_UP;
				}
				if (WPAD_ButtonsDown(0) & WPAD_BUTTON_LEFT) {
					game->bear.direction = BEAR_DOWN;
				}
				if (WPAD_ButtonsDown(0) & WPAD_BUTTON_DOWN) {
					game->bear.direction = BEAR_RIGHT;
				}
				if (WPAD_ButtonsDown(0) & WPAD_BUTTON_UP) {
					game->bear.direction = BEAR_LEFT;
				}
			}
			#endif
	
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
				// Bottom wall collision
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

					// Rumble (because why not?)
					#ifdef __wii__
					// Only rumble if no Classic Controller is attached
					if (e.type != WPAD_EXP_CLASSIC) {
						WPAD_Rumble(WPAD_CHAN_0, 1);
    					rumbling = true;
    				}
					#elif __gamecube__
    				PAD_ControlMotor(0, PAD_MOTOR_RUMBLE);
    				rumbling = true;
					#endif
	
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
				// Left walls
				GRRLIB_DrawImg(0, y * OBJECT_HEIGHT, spr_wall, 0, 1, 1, GRRLIB_WHITE);
				// Right walls
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
