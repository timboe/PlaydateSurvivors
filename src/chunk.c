#include "chunk.h"

const int32_t SIZE_CHUNK = TOT_CHUNKS * sizeof(struct Chunk_t);

uint32_t m_recursionCount;
#ifdef PRINT_MAX_RECURSION
uint32_t m_maxRecursion;
uint32_t m_totalRecursion;
#endif

struct Chunk_t* m_chunks;

void setChunkAssociations(void);

/// ///


struct Chunk_t* getChunk(int32_t _x, int32_t _y) {
  if (_x < 0) _x += WORLD_CHUNKS_X;
  else if (_x >= WORLD_CHUNKS_X) _x -= WORLD_CHUNKS_X;
  if (_y < 0) _y += WORLD_CHUNKS_Y;
  else if (_y >= WORLD_CHUNKS_Y) _y -= WORLD_CHUNKS_Y;
  return getChunk_noCheck(_x, _y);
}

struct Chunk_t* getChunk_noCheck(const int32_t _x, const int32_t _y) {
  const uint32_t l = (WORLD_CHUNKS_X * _y) + _x;
  return &m_chunks[l];
}

void setChunkSpriteOffsets(struct Chunk_t* _c, int16_t _x, int16_t _y) {
  // Move the background
  PDRect bound = {.x = 0, .y = 0, .width = CHUNK_PIX_X, .height = CHUNK_PIX_Y};
  if (_c->m_bkgSprite) {
    pd->sprite->moveTo(_c->m_bkgSprite, (CHUNK_PIX_X*_c->m_x + CHUNK_PIX_X/2.0 + _x), (CHUNK_PIX_Y*_c->m_y + CHUNK_PIX_Y/2.0 + _y));
  }
}


void chunkResetTorus() {
  for (int32_t x = 0; x < WORLD_CHUNKS_X; ++x) {
    struct Chunk_t* top = getChunk_noCheck(x, 0);
    struct Chunk_t* bottom = getChunk_noCheck(x, WORLD_CHUNKS_Y-1);
    setChunkSpriteOffsets(top, 0, 0);
    setChunkSpriteOffsets(bottom, 0, 0);
  }

  for (int32_t y = 0; y < WORLD_CHUNKS_Y; ++y) {
    struct Chunk_t* left = getChunk_noCheck(0, y);
    struct Chunk_t* right = getChunk_noCheck(WORLD_CHUNKS_X-1, y);
    setChunkSpriteOffsets(left, 0, 0);
    setChunkSpriteOffsets(right, 0, 0);
  }
}


void chunkShiftTorus(bool _top, bool _left) {

  #ifdef DEV
  pd->system->logToConsole("Shift Torus TOP:%i LEFT:%i", (int)_top, (int)_left);
  #endif

  for (int32_t x = 1; x < WORLD_CHUNKS_X - 1; ++x) {
    struct Chunk_t* top = getChunk_noCheck(x, 0);
    struct Chunk_t* bottom = getChunk_noCheck(x, WORLD_CHUNKS_Y-1);
    if (_top) {
      setChunkSpriteOffsets(top, 0, 0);
      setChunkSpriteOffsets(bottom, 0, -TOT_WORLD_PIX_Y);
    } else {
      setChunkSpriteOffsets(top, 0, TOT_WORLD_PIX_Y);
      setChunkSpriteOffsets(bottom, 0, 0);
    }
  }

  for (int32_t y = 1; y < WORLD_CHUNKS_Y - 1; ++y) {
    struct Chunk_t* left = getChunk_noCheck(0, y);
    struct Chunk_t* right = getChunk_noCheck(WORLD_CHUNKS_X-1, y);
    if (_left) {
      setChunkSpriteOffsets(left, 0, 0);
      setChunkSpriteOffsets(right, -TOT_WORLD_PIX_X, 0);
    } else {
      setChunkSpriteOffsets(left, TOT_WORLD_PIX_X, 0);
      setChunkSpriteOffsets(right, 0, 0);
    }
  }

  // Handle the corners
  struct Chunk_t* TL = getChunk_noCheck(0, 0);
  struct Chunk_t* TR = getChunk_noCheck(WORLD_CHUNKS_X-1, 0);
  struct Chunk_t* BL = getChunk_noCheck(0, WORLD_CHUNKS_Y-1);
  struct Chunk_t* BR = getChunk_noCheck(WORLD_CHUNKS_X-1, WORLD_CHUNKS_Y-1);

  if (_top && _left) {
    setChunkSpriteOffsets(TL, 0, 0);
    setChunkSpriteOffsets(TR, -TOT_WORLD_PIX_X, 0);
    setChunkSpriteOffsets(BL, 0, -TOT_WORLD_PIX_Y);
    setChunkSpriteOffsets(BR, -TOT_WORLD_PIX_X, -TOT_WORLD_PIX_Y);
  } else if (_top && !_left) {
    setChunkSpriteOffsets(TL, +TOT_WORLD_PIX_X, 0);
    setChunkSpriteOffsets(TR, 0, 0);
    setChunkSpriteOffsets(BL, +TOT_WORLD_PIX_X, -TOT_WORLD_PIX_Y);
    setChunkSpriteOffsets(BR, 0, -TOT_WORLD_PIX_Y);
  } else if (!_top && _left) {
    setChunkSpriteOffsets(TL, 0, +TOT_WORLD_PIX_Y);
    setChunkSpriteOffsets(TR, -TOT_WORLD_PIX_X, +TOT_WORLD_PIX_Y);
    setChunkSpriteOffsets(BL, 0, 0);
    setChunkSpriteOffsets(BR, -TOT_WORLD_PIX_X, 0);
  } else {
    setChunkSpriteOffsets(TL, +TOT_WORLD_PIX_X, +TOT_WORLD_PIX_Y);
    setChunkSpriteOffsets(TR, 0, +TOT_WORLD_PIX_Y);
    setChunkSpriteOffsets(BL, +TOT_WORLD_PIX_X, 0);
    setChunkSpriteOffsets(BR, 0, 0);
  }
}


// Each chunk should know who its neighbors are for fast rendering
void setChunkAssociations(void) {
  // Set neighbor associations
  for (uint16_t y = 0; y < WORLD_CHUNKS_Y; ++y) {
    for (uint16_t x = 0; x < WORLD_CHUNKS_X; ++x) {
      struct Chunk_t* c = getChunk_noCheck(x, y);
      c->m_neighborsALL[0] = getChunk(x + 0, y - 1);
      c->m_neighborsALL[1] = getChunk(x + 1, y - 1);
      c->m_neighborsALL[2] = getChunk(x + 1, y + 0);
      c->m_neighborsALL[3] = getChunk(x + 1, y + 1);
      c->m_neighborsALL[4] = getChunk(x + 0, y + 1);
      c->m_neighborsALL[5] = getChunk(x - 1, y + 1);
      c->m_neighborsALL[6] = getChunk(x - 1, y + 0);
      c->m_neighborsALL[7] = getChunk(x - 1, y - 1);
    }
  }
}

void resetChunk() {
  for (uint32_t x = 0; x < WORLD_CHUNKS_X; ++x) {
    for (uint32_t y = 0; y < WORLD_CHUNKS_Y; ++y) {
      // Owns background sprites and images
      struct Chunk_t* chunk = getChunk_noCheck(x, y);
      if (chunk->m_bkgSprite) pd->sprite->freeSprite(chunk->m_bkgSprite);
      if (chunk->m_bkgImage) pd->graphics->freeBitmap(chunk->m_bkgImage);
    }
  }
  memset(m_chunks, 0, SIZE_CHUNK);
  for (uint32_t x = 0; x < WORLD_CHUNKS_X; ++x) {
    for (uint32_t y = 0; y < WORLD_CHUNKS_Y; ++y) {
      struct Chunk_t* chunk = getChunk_noCheck(x, y);
      chunk->m_x = x;
      chunk->m_y = y;
    }
  }
  // The neighboring associations, including modulo behavior
  setChunkAssociations();
}

void initChunk() {
  m_chunks = pd->system->realloc(NULL, SIZE_CHUNK);
  memset(m_chunks, 0, SIZE_CHUNK);
  #ifdef DEV
  pd->system->logToConsole("malloc: for chunks %i", SIZE_CHUNK/1024);
  #endif
}