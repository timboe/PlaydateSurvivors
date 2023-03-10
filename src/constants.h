
#define DEV

//#define SSHOT_ON_AUTOSAVE

//#define SLOW_LOAD

// Careful - can seem to break things 
//#define FAST_PROGRESS

//#define DEMO

//#define TITLE_LOGO_ONLY

//#define SYNCHRONOUS_SAVE_ENABLED

#define ALWAYS_FPS false

/// ///

#define TICK_FREQUENCY 32

#define TILE_PIX 32

#define HTILE_PIX 16


#define DEVICE_PIX_X 400
#define DEVICE_PIX_Y 240

#define HALF_DEVICE_PIX_X (DEVICE_PIX_X/2)
#define HALF_DEVICE_PIX_Y (DEVICE_PIX_Y/2)

#define TILES_PER_CHUNK_X 7
#define TILES_PER_CHUNK_Y 4
#define TILES_PER_CHUNK (TILES_PER_CHUNK_X*TILES_PER_CHUNK_Y)

#define SPAWN_X 9
#define SPAWN_Y 6

#define CHUNK_PIX_X (TILE_PIX * TILES_PER_CHUNK_X)
#define CHUNK_PIX_Y (TILE_PIX * TILES_PER_CHUNK_Y)

#define CHUNK_NEIGHBORS_ALL 8

#define SHEET32_SIZE_X 16
#define SHEET32_SIZE_Y 26
#define SHEET32_SIZE (SHEET32_SIZE_X*SHEET32_SIZE_Y)

#define SHEET36_SIZE_X 6
#define SHEET36_SIZE_Y 4
#define SHEET36_SIZE (SHEET36_SIZE_X*SHEET36_SIZE_Y)

#define TRAUMA_DECAY 0.05f;
#define TRAUMA_AMPLIFICATION 4

// How big is the world in X and Y
#define WORLD_CHUNKS_X 16
#define WORLD_CHUNKS_Y 16
#define TOT_CHUNKS (WORLD_CHUNKS_X*WORLD_CHUNKS_Y)

#define TOT_WORLD_PIX_X (WORLD_CHUNKS_X*CHUNK_PIX_X)
#define TOT_WORLD_PIX_Y (WORLD_CHUNKS_Y*CHUNK_PIX_Y)

#define TOT_TILES_X (TILES_PER_CHUNK_X*WORLD_CHUNKS_X)
#define TOT_TILES_Y (TILES_PER_CHUNK_Y*WORLD_CHUNKS_Y)
#define TOT_TILES (TOT_TILES_X*TOT_TILES_Y)

// Player acceleration and friction
#define PLAYER_A 3.5f
// 1 = np friction
#define PLAYER_FRIC 0.5f

#define SQRT_HALF 0.70710678118f

#define Z_INDEX_UI_TTT 32767
#define Z_INDEX_UI_TT 32766
#define Z_INDEX_UI_T 32765
#define Z_INDEX_UI_M 32764
#define Z_INDEX_UI_B 32763
#define Z_INDEX_UI_BB 32762
#define Z_INDEX_UI_BBB 32761
#define Z_INDEX_UI_BACK 32760

#define Z_INDEX_BACKGROUND -1
#define Z_INDEX_PLAYER 16000
#define Z_INDEX_PROJECTILE 15000

#define FLAG_PLAYER 0
#define FLAG_ENEMY 1
#define FLAG_PROJECTILE 2

// Shrink the collision boxes from the edge of the sprite, feels better
#define COFF32 4
#define COFF16 2

#define N_MUSIC_TRACKS 5

#define BUTTON_PRESSED_FRAMES 8

#define SSHOT_PIXELS_PER_FRAME 16384

#define VERSION "v0.1"

#define FLOOR_VARIETIES 8

#define MAX_ENEMIES 128
#define MAX_PROJECTILES 128