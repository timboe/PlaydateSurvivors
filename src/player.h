#pragma once
#include "game.h"

#define PLAYER_ANIM_FRAMES 6
#define PLAYER_ANIM_DELAY 16

struct Player_t{
  // Persistent
  uint8_t m_saveFormat;

  float m_pix_x; // Note: Centre
  float m_pix_y;

  uint32_t m_saveTime;
  uint32_t m_playTime;

  // Transient   
  LCDSprite* m_sprite;

  float m_vX;
  float m_vY;
  uint16_t m_animID;
};

struct Player_t* getPlayer(void);

void forceTorus(void);

struct Chunk_t* getCurrentChunk(void);

struct Location_t* getPlayerLocation(void);

void setPlayerPosition(uint16_t _x, uint16_t _y, bool _updateCurrentLocation);

bool tutorialEnabled(void);

void movePlayer(bool _forceUpdate);

void initPlayer(void);

void resetPlayer(void);

void setPlayerVisible(bool _visible);

void setDefaultPlayerSettings(void);

void serialisePlayer(struct json_encoder* je); 

void didDecodeTableValuePlayer(json_decoder* jd, const char* _key, json_value _value);

void* deserialiseStructDonePlayer(json_decoder* jd, const char* _name, json_value_type _type);

void deserialiseArrayValuePlayer(json_decoder* jd, int _pos, json_value _value);