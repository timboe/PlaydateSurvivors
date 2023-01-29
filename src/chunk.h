#pragma once
#include "game.h"
#include "location.h"

struct Chunk_t{
  // Persistent
  // None

  // Transient
  uint16_t m_x; // In chunk coords
  uint16_t m_y;

  LCDSprite* m_bkgSprite;
  LCDBitmap* m_bkgImage;

  struct Chunk_t* m_neighborsALL[CHUNK_NEIGHBORS_ALL];
};

struct Chunk_t* getChunk(int32_t _x, int32_t _y);

struct Chunk_t* getChunk_noCheck(const int32_t _x, const int32_t _y);

void chunkShiftTorus(bool _top, bool _left);

void chunkResetTorus(void);

void initChunk(void);

void resetChunk(void);
