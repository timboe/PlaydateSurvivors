#pragma once
#include "game.h"
#include "chunk.h"

struct Tile_t {
  // Transient
  uint8_t m_tile;
};

void generate(uint32_t _actionProgress);

void generateTitle(void);

int32_t getTile_idx(int32_t _x, int32_t _y);

struct Tile_t* getTile(int32_t _x, int32_t _y);

struct Tile_t* getTile_fromLocation(struct Location_t* _loc);

void setTile(uint16_t _i, uint8_t _tileValue);

void setTile_ptr(struct Tile_t* _tile, uint8_t _tileValue);

void renderChunkBackgroundImage(struct Chunk_t* _chunk);

void renderChunkBackgroundImageAround(struct Chunk_t* _chunk);

void renderChunkBackgroundImageAround3x3(struct Chunk_t* _chunk, struct Location_t* _loc);

void setChunkBackgrounds(bool _forTitles);

void initWorld(void);

void resetWorld(void);