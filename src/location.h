#pragma once
#include "game.h"

struct Location_t{
  // Persistent
  // Chunk index - obtained from chunk
  // Building index - obtained from building

  // Transient

  // Populated at RESET
  struct Chunk_t* m_chunk;
  int16_t m_x; // In location coords
  int16_t m_y;
  int16_t m_pix_off_x; // Pixel offset
  int16_t m_pix_off_y; // Pixel offset
};

struct Location_t* getLocation(int32_t _x, int32_t _y);

struct Location_t* getLocation_noCheck(int32_t _x, int32_t _y);

bool clearLocation(struct Location_t* _loc);

void initLocation(void);

void resetLocation(void);

void serialiseLocation(struct json_encoder* je);

void deserialiseValueLocation(json_decoder* jd, const char* _key, json_value _value);

void* deserialiseStructDoneLocation(json_decoder* jd, const char* _name, json_value_type _type);