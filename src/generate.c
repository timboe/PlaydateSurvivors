#include "generate.h"
#include "sprite.h"
#include "location.h"
#include "ui.h"
#include "io.h"
#include "sprite.h"
#include "render.h"
#include <math.h>


struct Tile_t* m_tiles = NULL;

const int32_t SIZE_GENERATE = TOT_TILES * sizeof(struct Tile_t);

void generateSpriteSetup(struct Chunk_t* _chunk);

struct Tile_t* getTileInChunk(struct Chunk_t* _chunk, int32_t _u, int32_t _v);

float pointDist(int32_t _x, int32_t _y, int32_t _x1, int32_t _y1, int32_t _x2, int32_t _y2);

uint8_t getNearbyBackground_Chunk(struct Chunk_t* _chunk, uint16_t _u, uint16_t _v);

uint8_t getNearbyBackground_Loc(struct Location_t* _loc);


/// ///

struct Tile_t* getTileInChunk(struct Chunk_t* _chunk, int32_t _u, int32_t _v) {
  return &m_tiles[ (WORLD_CHUNKS_X * TILES_PER_CHUNK_X)*((TILES_PER_CHUNK_Y * _chunk->m_y) + _v) + ((TILES_PER_CHUNK_X * _chunk->m_x) + _u) ];
}

int32_t getTile_idx(int32_t _x, int32_t _y) {
  if (_x < 0) _x += TOT_TILES_X;
  else if (_x >= TOT_TILES_X) _x -= TOT_TILES_X;
  if (_y < 0) _y += TOT_TILES_Y;
  else if (_y >= TOT_TILES_Y) _y -= TOT_TILES_Y;
  return TOT_TILES_X*_y + _x;
}

struct Tile_t* getTile(int32_t _x, int32_t _y) {
  return &m_tiles[ getTile_idx(_x, _y) ];
}

struct Tile_t* getTile_fromLocation(struct Location_t* _loc) {
  return getTile(_loc->m_x, _loc->m_y);
}

float pointDist(int32_t _x, int32_t _y, int32_t _x1, int32_t _y1, int32_t _x2, int32_t _y2) {

  int32_t A = _x - _x1;
  int32_t B = _y - _y1;
  int32_t C = _x2 - _x1;
  int32_t D = _y2 - _y1;

  float dot = A * C + B * D;
  int32_t len_sq = C * C + D * D;
  float param = -1;
  if (len_sq != 0) { //in case of 0 length line
    param = dot / len_sq;
  }

  int32_t xx, yy;

  if (param < 0) {
    xx = _x1;
    yy = _y1;
  } else if (param > 1) {
    xx = _x2;
    yy = _y2;
  } else {
    xx = _x1 + param * C;
    yy = _y1 + param * D;
  }

  int32_t dx = _x - xx;
  int32_t dy = _y - yy;
  return sqrtf(dx * dx + dy * dy);
}

void renderChunkBackgroundImageAround(struct Chunk_t* _chunk) {
  for (int32_t x = _chunk->m_x - 1; x < _chunk->m_x + 2; ++x) {
    for (int32_t y = _chunk->m_y - 1; y < _chunk->m_y + 2; ++y) {
      struct Chunk_t* toRedraw = getChunk(x,y);
      //pd->system->logToConsole("REDRAW %i %i", toRedraw->m_x, toRedraw->m_y);
      renderChunkBackgroundImage(toRedraw);
    }
  }
}

void renderChunkBackgroundImageAround3x3(struct Chunk_t* _chunk, struct Location_t* _loc) {
  const bool left = _loc->m_x % TILES_PER_CHUNK_X == 0;
  const bool right = _loc->m_x % TILES_PER_CHUNK_X == TILES_PER_CHUNK_X-1;
  const bool top = _loc->m_y % TILES_PER_CHUNK_Y == 0;
  const bool bot = _loc->m_y % TILES_PER_CHUNK_Y == TILES_PER_CHUNK_Y-1;
  const bool cornerVeto = false; // TODO - needed?
  for (int32_t x = -1; x < 2; ++x) {
    for (int32_t y = -1; y < 2; ++y) {
      if (x == -1 && y == -1) {
        if (!left || !top || cornerVeto) continue;
      } else if (x == 0 && y == -1) {
        if (!top) continue;
      } else if (x == 1 && y == -1) {
        if (!right || !top || cornerVeto) continue;
      } else if (x == -1 && y == 0) {
        if (!left) continue;
      } else if (x == 1 && y == 0) {
        if (!right) continue;
      } else if (x == -1 && y == 1) {
        if (!left || !bot || cornerVeto) continue;
      } else if (x == 0 && y == 1) {
        if (!bot) continue; 
      } else if (x == 1 && y == 1) {
        if (!right || !bot || cornerVeto) continue;
      }
      struct Chunk_t* toRedraw = getChunk(_chunk->m_x + x, _chunk->m_y + y);
      //pd->system->logToConsole("REDRAW %i %i", toRedraw->m_x, toRedraw->m_y);
      renderChunkBackgroundImage(toRedraw);
    }
  }
}

void renderChunkBackgroundImage(struct Chunk_t* _chunk) {
  // Possible to call this too early for neighboring chunks with the title screen
  if (!_chunk->m_bkgImage) {
    return;
  }
  struct Player_t* p = getPlayer();

  pd->graphics->pushContext(_chunk->m_bkgImage);
  pd->graphics->setDrawMode(kDrawModeCopy);
  pd->graphics->fillRect(0, 0, CHUNK_PIX_X, CHUNK_PIX_Y, kColorWhite);

  // Draw background
  for (uint16_t v = 0; v < TILES_PER_CHUNK_Y; ++v) {
    for (uint16_t u = 0; u < TILES_PER_CHUNK_X; ++u) {
      struct Tile_t* t = getTileInChunk(_chunk, u, v);
      LCDBitmapFlip flip = (u+v) % 2 ? kBitmapUnflipped : kBitmapFlippedX;
      pd->graphics->drawBitmap(getSprite32_byidx(t->m_tile), 
        u * TILE_PIX, v * TILE_PIX, flip);
    }
  }

  #ifdef DEV
  if (true) {
    setRoobert24();
    pd->graphics->drawRect(0, 0, CHUNK_PIX_X, CHUNK_PIX_Y, kColorBlack);
    static char text[16];
    snprintf(text, 16, "(%u,%u)", _chunk->m_x, _chunk->m_y);
    pd->graphics->drawText(text, 16, kASCIIEncoding, TILE_PIX, TILE_PIX);
  }
  #endif

  // Shift from Sprite to Bitmap draw coords
  const int16_t off16_x = (_chunk->m_x * CHUNK_PIX_X) + TILE_PIX/2;
  const int16_t off16_y = (_chunk->m_y * CHUNK_PIX_Y) + TILE_PIX/2;

  const int16_t off48_x = (_chunk->m_x * CHUNK_PIX_X) + 3*TILE_PIX/2;
  const int16_t off48_y = (_chunk->m_y * CHUNK_PIX_Y) + 3*TILE_PIX/2;

  pd->graphics->popContext();

}


void generateSpriteSetup(struct Chunk_t* _chunk) {
  if (_chunk->m_bkgImage == NULL) _chunk->m_bkgImage = pd->graphics->newBitmap(CHUNK_PIX_X, CHUNK_PIX_Y, kColorWhite);
  if (_chunk->m_bkgSprite == NULL) _chunk->m_bkgSprite = pd->sprite->newSprite();
  PDRect bound = {.x = 0, .y = 0, .width = CHUNK_PIX_X, .height = CHUNK_PIX_Y};
  pd->sprite->setBounds(_chunk->m_bkgSprite, bound);
  pd->sprite->setImage(_chunk->m_bkgSprite, _chunk->m_bkgImage, kBitmapUnflipped);
  pd->sprite->moveTo(_chunk->m_bkgSprite, (CHUNK_PIX_X*_chunk->m_x + CHUNK_PIX_X/2.0), (CHUNK_PIX_Y*_chunk->m_y + CHUNK_PIX_Y/2.0));
  pd->sprite->setZIndex(_chunk->m_bkgSprite, Z_INDEX_BACKGROUND);
  pd->sprite->setOpaque(_chunk->m_bkgSprite, true);
  pd->sprite->setCollisionsEnabled(_chunk->m_bkgSprite, false);
}

void setChunkBackgrounds(bool _forTitles) {
  const uint16_t maxY = (_forTitles ? 3 : WORLD_CHUNKS_Y);
  const uint16_t maxX = (_forTitles ? 3 : WORLD_CHUNKS_X);
  for (uint16_t y = 0; y < maxY; ++y) {
    for (uint16_t x = 0; x < maxX; ++x) {
      struct Chunk_t* chunk = getChunk_noCheck(x, y);
      generateSpriteSetup(chunk);
      renderChunkBackgroundImage(chunk);
    }
  }
}

void resetWorld() {
  memset(m_tiles, 0, SIZE_GENERATE);
}

void initWorld() {
  m_tiles = pd->system->realloc(NULL, SIZE_GENERATE);
  #ifdef DEV
  pd->system->logToConsole("malloc: for world %i", SIZE_GENERATE/1024);
  #endif
}

void setTile(uint16_t _i, uint8_t _tileValue) {
  setTile_ptr(&m_tiles[_i], _tileValue);
}

void setTile_ptr(struct Tile_t* _tile, uint8_t _tileValue) {
  _tile->m_tile = _tileValue;
}

void generate(uint32_t _actionProgress) {

  uint8_t floorMain = 0;

  if (_actionProgress == 1) {

    srand(pd->system->getSecondsSinceEpoch(NULL));

  } else if (_actionProgress == 2) {

    for (uint16_t i = 0; i < TOT_TILES; ++i) {
      setTile(i, (FLOOR_VARIETIES * floorMain) + rand() % FLOOR_VARIETIES);
    } 

  } else if (_actionProgress == 3) {

  } else if (_actionProgress == 4) {

  } else if (_actionProgress == 5) {

  } else if (_actionProgress == 6) {

  } else if (_actionProgress == 7) {

  } else if (_actionProgress == 8) {

    // Finished
    #ifdef SLOW_LOAD
    float f; for (int32_t i = 0; i < 10000; ++i) for (int32_t j = 0; j < 10000; ++j) { f*=i*j; }
    pd->system->logToConsole("Generated %s",  getWorldName(slot, /*mask*/ false));
    #endif

  }
}

void generateTitle() {

  uint8_t floorMain = 1;

  srand(0);

  for (uint16_t x = 0; x < TILES_PER_CHUNK_X*3; ++x) {
    for (uint16_t y = 0; y < TILES_PER_CHUNK_Y*3; ++y) {
      setTile( getTile_idx(x,y), rand() % FLOOR_VARIETIES );
    } 
  }

}
