#include <math.h>
#include "player.h"
#include "sprite.h"
#include "chunk.h"
#include "render.h"
#include "input.h"
#include "sound.h"
#include "io.h"
#include "ui.h"
#include "generate.h"
#include "projectile.h"
#include "enemy.h"

struct Player_t m_player;

int16_t m_offX, m_offY = 0; // Screen offsets

uint8_t m_facing = 0, m_wasFacing = 0, m_stepCounter = 0, m_animFrame = 0;

uint16_t m_deserialiseXPlayer = 0, m_deserialiseYPlayer = 0;
int16_t m_deserialiseArrayID = -1;

struct Chunk_t* m_currentChunk = NULL;

struct Location_t* m_currentLocation = NULL;

bool m_top = true;
bool m_left = true;
bool m_forceTorus = true;

void movePlayerPosition(float _goalX, float _goalY);

void updatePlayerPosition(void);

void playerSpriteSetup(void);

struct Chunk_t* computeCurrentChunk(void);

void checkTorus(void);

/// ///

struct Player_t* getPlayer() {
  return &m_player;
}

struct Chunk_t* getCurrentChunk() {
  return m_currentChunk;
}

struct Location_t* getPlayerLocation() {
  return m_currentLocation;
}

void setPlayerPosition(uint16_t _x, uint16_t _y, bool _updateCurrentLocation) {
  pd->sprite->moveTo(m_player.m_sprite, _x, _y);
  updatePlayerPosition();
  if (_updateCurrentLocation) {
    m_currentChunk = computeCurrentChunk();
    m_currentLocation = getLocation(m_player.m_x / TILE_PIX, m_player.m_y / TILE_PIX);
    // DON'T setTorus here (not 100% why, but it breaks building hitboxes)
    #ifdef DEV
    pd->system->logToConsole("CHUNKCHANGE (set) C:%i %i", m_currentChunk->m_x, m_currentChunk->m_y);
    #endif
    if (!IOOperationInProgress()) {
      updateRenderList();
    }
  }
}

void movePlayerPosition(float _goalX, float _goalY) {
  int len;
  SpriteCollisionInfo* collInfo = pd->sprite->moveWithCollisions(m_player.m_sprite, _goalX, _goalY, &(m_player.m_x), &(m_player.m_y), &len);

  bool coliding = false;
  for (uint32_t c = 0; c < len; ++c) {
    SpriteCollisionInfo* info = &collInfo[c];
    if (pd->sprite->getTag(info->other) != FLAG_ENEMY) continue;

    coliding = true;
    break;
  }

  pd->sprite->setDrawMode(m_player.m_sprite, coliding ? kDrawModeInverted : kDrawModeCopy);


  if (len) pd->system->realloc(collInfo, 0); // Free
  updatePlayerPosition();
}

void updatePlayerPosition() {
  pd->sprite->getPosition(m_player.m_sprite, &(m_player.m_x), &(m_player.m_y));
  //pd->system->logToConsole("P@ %f %f", m_player.m_x , m_player.m_y);
}


struct Chunk_t* computeCurrentChunk() {
  int16_t cX = (int16_t)m_player.m_x / CHUNK_PIX_X;
  int16_t cY = (int16_t)m_player.m_y / CHUNK_PIX_Y; 
  return getChunk(cX, cY);
}


void checkTorus() {
  // Torus splitting
  bool torusChanged = false;
  if (m_top && m_player.m_y > TOT_WORLD_PIX_Y/2) {
    m_top = false;
    torusChanged = true;
  } else if (!m_top && m_player.m_y <= TOT_WORLD_PIX_Y/2) {
    m_top = true;
    torusChanged = true;
  }

  if (m_left && m_player.m_x > TOT_WORLD_PIX_X/2) {
    m_left = false;
    torusChanged = true;
  } else if (!m_left && m_player.m_x <= TOT_WORLD_PIX_X/2) {
    m_left = true;
    torusChanged = true;
  }

  if (torusChanged || m_forceTorus) {
    chunkShiftTorus(m_top, m_left);
    m_forceTorus = false;
  }
}

void movePlayer(bool _forceUpdate) {

  //updatePlayerPosition();
  float goalX = m_player.m_x;
  float goalY = m_player.m_y;

  float acc = PLAYER_A;
  float fric = PLAYER_FRIC;
  if (bPressed()) acc *= 1.5f;

  float diffX = 0;
  float diffY = 0;

  if (getPressed(0)) diffX -= acc;
  if (getPressed(1)) diffX += acc;
  if (getPressed(2)) diffY -= acc;
  if (getPressed(3)) diffY += acc;

  // Note floating point == 0 check, hopefully work in this case
  if (diffX && diffY) {
    diffX *= SQRT_HALF;
    diffY *= SQRT_HALF;
  }

  m_player.m_vX = (m_player.m_vX + diffX) * fric;
  m_player.m_vY = (m_player.m_vY + diffY) * fric;

  static uint16_t movingTicksX = 0, movingTicksY = 0;
  if ((float)(fabs(m_player.m_vX) + fabs(m_player.m_vY)) > 0.1f) {
    if (m_player.m_vX) ++movingTicksX;
    if (m_player.m_vY) ++movingTicksY;
    if (fabs(m_player.m_vX) > fabs(m_player.m_vY)) {
      m_facing = (m_player.m_vX > 0 ? 1 : 0);
    } else {
      m_facing = (m_player.m_vY > 0 ? 3 : 2);
    }
  }
  if (!diffX) movingTicksX = 0;
  if (!diffY) movingTicksY = 0;

  goalX += m_player.m_vX;
  goalY += m_player.m_vY; 

  if (movingTicksX || movingTicksY || _forceUpdate) {
    if (++m_stepCounter * acc > PLAYER_ANIM_DELAY || m_facing != m_wasFacing) {
      m_animFrame = (m_animFrame + 1) % PLAYER_ANIM_FRAMES;
      m_stepCounter = 0;
      const uint16_t animY = m_facing;
      //if (m_animFrame % 3 == 0 && m_player.m_enableSteps) sfx(kFootstepDefault + rand() % N_FOOTSTEPS);
      m_player.m_animID = SPRITE36_ID(m_animFrame, animY);
      pd->sprite->setImage(m_player.m_sprite, getSprite36_byidx(m_player.m_animID), kBitmapUnflipped);
    }
    m_wasFacing = m_facing;
  }

  //pd->system->logToConsole("GOAL %f %f CURRENT %f %f", goalX, goalY, m_player.m_x, m_player.m_y);

  movePlayerPosition(goalX, goalY);

  // Don#t need to update position as the code below will pick up on this
  if (m_player.m_x > TOT_WORLD_PIX_X) {
    setPlayerPosition(m_player.m_x - TOT_WORLD_PIX_X, m_player.m_y, /*update current location = */ false);
    moveEnemies(-TOT_WORLD_PIX_X, 0);
    moveProjectiles(-TOT_WORLD_PIX_X, 0);
  } else if (m_player.m_x < 0) {
    setPlayerPosition(m_player.m_x + TOT_WORLD_PIX_X, m_player.m_y, /*update current location = */ false);
    moveEnemies(TOT_WORLD_PIX_X, 0);
    moveProjectiles(TOT_WORLD_PIX_X, 0);
  }

  if (m_player.m_y > TOT_WORLD_PIX_Y) {
    setPlayerPosition(m_player.m_x, m_player.m_y - TOT_WORLD_PIX_Y, /*update current location = */ false);
    moveEnemies(0, -TOT_WORLD_PIX_Y);
    moveProjectiles(0, -TOT_WORLD_PIX_Y);
  } else if (m_player.m_y < 0) {
    setPlayerPosition(m_player.m_x, m_player.m_y + TOT_WORLD_PIX_Y, /*update current location = */ false);
    moveEnemies(0, TOT_WORLD_PIX_Y);
    moveProjectiles(0, TOT_WORLD_PIX_Y);
  }

  // Check chunk change
  struct Chunk_t* cameraChunk = computeCurrentChunk();

  checkTorus();

  bool chunkChange = (cameraChunk != m_currentChunk);
  if (chunkChange) {
    m_currentChunk = cameraChunk;
    updateRenderList();
  }

  struct Location_t* wasAt = m_currentLocation;
  m_currentLocation = getLocation(m_player.m_x / TILE_PIX, m_player.m_y / TILE_PIX);

}

SpriteCollisionResponseType playerLCDSpriteCollisionFilterProc(LCDSprite* _player, LCDSprite* _other) {
  return kCollisionTypeOverlap;
}

void playerSpriteSetup() {
    m_player.m_sprite = pd->sprite->newSprite();
    PDRect pBound = {.x = 0, .y = 0, .width = TILE_PIX, .height = 18};
    PDRect cBound = {.x = 2*COFF16, .y = COFF16, .width = TILE_PIX - 4*COFF16, .height = TILE_PIX - 2*COFF16};
    pd->sprite->setBounds(m_player.m_sprite, pBound);
    pd->sprite->setImage(m_player.m_sprite, getSprite36(0, 3), kBitmapUnflipped);
    pd->sprite->setCollideRect(m_player.m_sprite, cBound);
    pd->sprite->setCollisionResponseFunction(m_player.m_sprite, playerLCDSpriteCollisionFilterProc);
    pd->sprite->setTag(m_player.m_sprite, FLAG_PLAYER);
    pd->sprite->setZIndex(m_player.m_sprite, Z_INDEX_PLAYER);

}

void forceTorus() { m_forceTorus = true; }

void resetPlayer() {
  m_player.m_saveTime = pd->system->getSecondsSinceEpoch(NULL);
  m_player.m_playTime = 0;
  m_player.m_animID = 0;
  m_player.m_x = 0;
  m_player.m_y = 0;
  setPlayerPosition((TOT_WORLD_PIX_X/2) + DEVICE_PIX_X/4, (TOT_WORLD_PIX_Y/2) + (3*DEVICE_PIX_Y)/4, /*update current location = */ true);
  m_facing = 0;
  m_wasFacing = 0;
  m_stepCounter = 0;
  m_animFrame = 0;
  m_deserialiseXPlayer = 0;
  m_deserialiseYPlayer = 0;
  m_deserialiseArrayID = -1;
}

void setPlayerVisible(bool _visible) {
  pd->sprite->setVisible(m_player.m_sprite, _visible);
  pd->sprite->setCollisionsEnabled(m_player.m_sprite, _visible);
}

void setDefaultPlayerSettings() {
}

void initPlayer() {
  playerSpriteSetup();
}

void serialisePlayer(struct json_encoder* je) {

}

void didDecodeTableValuePlayer(json_decoder* jd, const char* _key, json_value _value) {

}

void deserialiseArrayValuePlayer(json_decoder* jd, int _pos, json_value _value) {

}

void* deserialiseStructDonePlayer(json_decoder* jd, const char* _name, json_value_type _type) {
 
  return NULL;
}
