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
    m_currentLocation = getLocation(m_player.m_pix_x / TILE_PIX, m_player.m_pix_y / TILE_PIX);
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
  SpriteCollisionInfo* collInfo = pd->sprite->moveWithCollisions(m_player.m_sprite, _goalX, _goalY, &(m_player.m_pix_x), &(m_player.m_pix_y), &len);
  if (len) pd->system->realloc(collInfo, 0); // Free
  updatePlayerPosition();
}

void updatePlayerPosition() {
  pd->sprite->getPosition(m_player.m_sprite, &(m_player.m_pix_x), &(m_player.m_pix_y));
  //pd->system->logToConsole("P@ %f %f", m_player.m_pix_x , m_player.m_pix_y);
}


struct Chunk_t* computeCurrentChunk() {
  int16_t cX = (int16_t)m_player.m_pix_x / CHUNK_PIX_X;
  int16_t cY = (int16_t)m_player.m_pix_y / CHUNK_PIX_Y; 
  return getChunk(cX, cY);
}


void checkTorus() {
  // Torus splitting
  bool torusChanged = false;
  if (m_top && m_player.m_pix_y > TOT_WORLD_PIX_Y/2) {
    m_top = false;
    torusChanged = true;
  } else if (!m_top && m_player.m_pix_y <= TOT_WORLD_PIX_Y/2) {
    m_top = true;
    torusChanged = true;
  }

  if (m_left && m_player.m_pix_x > TOT_WORLD_PIX_X/2) {
    m_left = false;
    torusChanged = true;
  } else if (!m_left && m_player.m_pix_x <= TOT_WORLD_PIX_X/2) {
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
  float goalX = m_player.m_pix_x;
  float goalY = m_player.m_pix_y;

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
  if (m_player.m_pix_x > TOT_WORLD_PIX_X) {
    setPlayerPosition(m_player.m_pix_x - TOT_WORLD_PIX_X, m_player.m_pix_y, /*update current location = */ false);
  } else if (m_player.m_pix_x < 0) {
    setPlayerPosition(m_player.m_pix_x + TOT_WORLD_PIX_X, m_player.m_pix_y, /*update current location = */ false);
  }

  if (m_player.m_pix_y > TOT_WORLD_PIX_Y) {
    setPlayerPosition(m_player.m_pix_x, m_player.m_pix_y - TOT_WORLD_PIX_Y, /*update current location = */ false);
  } else if (m_player.m_pix_y < 0) {
    setPlayerPosition(m_player.m_pix_x, m_player.m_pix_y + TOT_WORLD_PIX_Y, /*update current location = */ false);
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
  m_currentLocation = getLocation(m_player.m_pix_x / TILE_PIX, m_player.m_pix_y / TILE_PIX);

}

SpriteCollisionResponseType playerLCDSpriteCollisionFilterProc(LCDSprite* _player, LCDSprite* _other) {
  return kCollisionTypeSlide;
}

void playerSpriteSetup() {
    m_player.m_sprite = pd->sprite->newSprite();
    PDRect pBound = {.x = 0, .y = 0, .width = TILE_PIX, .height = 18};
    PDRect cBound = {.x = (COLLISION_OFFSET_SMALL/2), .y = (COLLISION_OFFSET_SMALL/2), .width = (TILE_PIX - COLLISION_OFFSET_SMALL), .height = (TILE_PIX - COLLISION_OFFSET_SMALL)};
    pd->sprite->setBounds(m_player.m_sprite, pBound);
    pd->sprite->setImage(m_player.m_sprite, getSprite36(0, 3), kBitmapUnflipped);
    pd->sprite->setCollideRect(m_player.m_sprite, cBound);
    pd->sprite->setCollisionResponseFunction(m_player.m_sprite, playerLCDSpriteCollisionFilterProc);
    pd->sprite->setZIndex(m_player.m_sprite, Z_INDEX_PLAYER);

}

void forceTorus() { m_forceTorus = true; }

void resetPlayer() {
  m_player.m_saveTime = pd->system->getSecondsSinceEpoch(NULL);
  m_player.m_playTime = 0;
  m_player.m_animID = 0;
  m_player.m_pix_x = 0;
  m_player.m_pix_y = 0;
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
  // je->addTableMember(je, "player", 6);
  // je->startTable(je);
  // je->addTableMember(je, "sf", 2); // Save format
  // je->writeInt(je, CURRENT_SAVE_FORMAT);
  // je->addTableMember(je, "x", 1);
  // je->writeInt(je, (int) m_player.m_pix_x);
  // je->addTableMember(je, "y", 1);
  // je->writeInt(je, (int) m_player.m_pix_y);
  // je->addTableMember(je, "m", 1);
  // je->writeInt(je, m_player.m_money);
  // je->addTableMember(je, "mhwm", 4);
  // je->writeInt(je, m_player.m_moneyHighWaterMark);
  // je->addTableMember(je, "but", 3);
  // je->writeInt(je, m_player.m_buildingsUnlockedTo);
  // je->addTableMember(je, "mc", 2);
  // je->writeInt(je, m_player.m_moneyCumulative);
  // je->addTableMember(je, "st", 2);
  // je->writeInt(je, pd->system->getSecondsSinceEpoch(NULL));
  // je->addTableMember(je, "pt", 2);
  // je->writeInt(je, m_player.m_playTime);
  // je->addTableMember(je, "tutstage", 8);
  // je->writeInt(je, m_player.m_tutorialProgress);

  // je->addTableMember(je, "slot", 4);
  // je->writeInt(je, getSlot());

  // // Settings - not reset new games
  // je->addTableMember(je, "sets", 4);
  // je->writeInt(je, m_player.m_soundSettings);
  // je->addTableMember(je, "setm", 4);
  // je->writeInt(je, m_player.m_musicVol);
  // je->addTableMember(je, "setb", 4);
  // je->writeInt(je, m_player.m_autoUseConveyorBooster);
  // je->addTableMember(je, "setc", 4);
  // je->writeInt(je, m_player.m_enableConveyorAnimation); 
  // je->addTableMember(je, "sett", 4);
  // je->writeInt(je, m_player.m_enableTutorial);
  // je->addTableMember(je, "setd", 4);
  // je->writeInt(je, m_player.m_enableDebug);
  // je->addTableMember(je, "seta", 4);
  // je->writeInt(je, m_player.m_enableAutosave);
  // je->addTableMember(je, "setp", 4);
  // je->writeInt(je, m_player.m_enablePickupOnDestroy);
  // je->addTableMember(je, "seth", 4);
  // je->writeInt(je, m_player.m_enableScreenShake);
  // je->addTableMember(je, "seto", 4);
  // je->writeInt(je, m_player.m_enableExtractorOutlines);
  // je->addTableMember(je, "sete", 4);
  // je->writeInt(je, m_player.m_enableSteps);
  // je->addTableMember(je, "setz", 4);
  // je->writeInt(je, m_player.m_enableZoomWhenMove);
  // je->addTableMember(je, "setr", 4);
  // je->writeInt(je, m_player.m_enableCrankConveyor);
  // je->addTableMember(je, "setn", 4);
  // je->writeInt(je, m_player.m_enableCentreCamera);
  
  
  // je->addTableMember(je, "cargos", 6);
  // je->startArray(je);
  // for (int32_t i = 0; i < kNCargoType; ++i) {
  //   je->addArrayMember(je);
  //   je->writeInt(je, m_player.m_carryCargo[i]);
  // }
  // je->endArray(je);

  // je->addTableMember(je, "convs", 5);
  // je->startArray(je);
  // for (int32_t i = 0; i < kNConvSubTypes; ++i) {
  //   je->addArrayMember(je);
  //   je->writeInt(je, m_player.m_carryConveyor[i]);
  // }
  // je->endArray(je);

  // je->addTableMember(je, "plants", 6);
  // je->startArray(je);
  // for (int32_t i = 0; i < kNPlantSubTypes; ++i) {
  //   je->addArrayMember(je);
  //   je->writeInt(je, m_player.m_carryPlant[i]);
  // }
  // je->endArray(je);

  // je->addTableMember(je, "util", 4);
  // je->startArray(je);
  // for (int32_t i = 0; i < kNUtilitySubTypes; ++i) {
  //   je->addArrayMember(je);
  //   je->writeInt(je, m_player.m_carryUtility[i]);
  // }
  // je->endArray(je);

  // je->addTableMember(je, "xtrcts", 6);
  // je->startArray(je);
  // for (int32_t i = 0; i < kNExtractorSubTypes; ++i) {
  //   je->addArrayMember(je);
  //   je->writeInt(je, m_player.m_carryExtractor[i]);
  // }
  // je->endArray(je);


  // je->addTableMember(je, "facts", 5);
  // je->startArray(je);
  // for (int32_t i = 0; i < kNFactorySubTypes; ++i) {
  //   je->addArrayMember(je);
  //   je->writeInt(je, m_player.m_carryFactory[i]);
  // }
  // je->endArray(je);

  // je->addTableMember(je, "scargo", 6);
  // je->startArray(je);
  // for (int32_t i = 0; i < kNCargoType; ++i) {
  //   je->addArrayMember(je);
  //   je->writeInt(je, m_player.m_soldCargo[i]);
  // }
  // je->endArray(je);

  // je->addTableMember(je, "icargo", 6);
  // je->startArray(je);
  // for (int32_t i = 0; i < kNCargoType; ++i) {
  //   je->addArrayMember(je);
  //   je->writeInt(je, m_player.m_importedCargo[i]);
  // }
  // je->endArray(je);

  // for (int32_t w = 0; w < WORLD_SAVE_SLOTS; ++w) {
  //   char txt[12] = "";
  //   snprintf(txt, 12, "expw%02i", (int)w);
  //   je->addTableMember(je, txt, 6);
  //   je->startArray(je);
  //   for (int32_t i = 0; i < kNCargoType; ++i) {
  //     je->addArrayMember(je);
  //     je->writeDouble(je, m_player.m_exportPerWorld[w][i]);
  //   }
  //   je->endArray(je);
  // }

  // je->addTableMember(je, "spw", 3);
  // je->startArray(je);
  // for (int32_t w = 0; w < WORLD_SAVE_SLOTS; ++w) {
  //   je->addArrayMember(je);
  //   je->writeDouble(je, m_player.m_sellPerWorld[w]);
  // }
  // je->endArray(je);

  // je->addTableMember(je, "impc", 4);
  // je->startArray(je);
  // for (int32_t i = 0; i < kNCargoType; ++i) {
  //   je->addArrayMember(je);
  //   je->writeInt(je, m_player.m_importConsumers[i]);
  // }
  // je->endArray(je);

  // je->endTable(je);
}

void didDecodeTableValuePlayer(json_decoder* jd, const char* _key, json_value _value) {
  // if (strcmp(_key, "sf") == 0) {
  //   m_player.m_saveFormat = json_intValue(_value);
  // } else if (strcmp(_key, "x") == 0) {
  //   m_player.m_pix_x = (float) json_intValue(_value);
  // } else if (strcmp(_key, "y") == 0) {
  //   m_player.m_pix_y = json_intValue(_value);
  // } else if (strcmp(_key, "m") == 0) {
  //   m_player.m_money = json_intValue(_value);
  // } else if (strcmp(_key, "mhwm") == 0) {
  //   m_player.m_moneyHighWaterMark = json_intValue(_value);
  // } else if (strcmp(_key, "but") == 0) {
  //   m_player.m_buildingsUnlockedTo = json_intValue(_value);
  // } else if (strcmp(_key, "mc") == 0) {
  //   m_player.m_moneyCumulative = json_intValue(_value);
  // } else if (strcmp(_key, "st") == 0) {
  //   m_player.m_saveTime = json_intValue(_value);
  // } else if (strcmp(_key, "pt") == 0) {
  //   m_player.m_playTime = json_intValue(_value);
  // } else if (strcmp(_key, "tutstage") == 0) {
  //   m_player.m_tutorialProgress = json_intValue(_value);
  // } else if (strcmp(_key, "slot") == 0) {
  //   setSlot( json_intValue(_value) ); 
  // } else if (strcmp(_key, "sets") == 0) {
  //   m_player.m_soundSettings = json_intValue(_value); 
  // } else if (strcmp(_key, "setm") == 0) {
  //   m_player.m_musicVol = json_intValue(_value); 
  // } else if (strcmp(_key, "setb") == 0) {
  //   m_player.m_autoUseConveyorBooster = json_intValue(_value); 
  // } else if (strcmp(_key, "setc") == 0) {
  //   m_player.m_enableConveyorAnimation = json_intValue(_value);
  // } else if (strcmp(_key, "sett") == 0) {
  //   m_player.m_enableTutorial = json_intValue(_value);
  // } else if (strcmp(_key, "setd") == 0) {
  //   m_player.m_enableDebug = json_intValue(_value); 
  // } else if (strcmp(_key, "setp") == 0) {
  //   m_player.m_enablePickupOnDestroy = json_intValue(_value); 
  // } else if (strcmp(_key, "seth") == 0) {
  //   m_player.m_enableScreenShake = json_intValue(_value); 
  // } else if (strcmp(_key, "seto") == 0) {
  //   m_player.m_enableExtractorOutlines = json_intValue(_value); 
  // } else if (strcmp(_key, "sete") == 0) {
  //   m_player.m_enableSteps = json_intValue(_value); 
  // } else if (strcmp(_key, "setz") == 0) {
  //   m_player.m_enableZoomWhenMove = json_intValue(_value); 
  // } else if (strcmp(_key, "setr") == 0) {
  //   m_player.m_enableCrankConveyor = json_intValue(_value); 
  // } else if (strcmp(_key, "setn") == 0) {
  //   m_player.m_enableCentreCamera = json_intValue(_value); 
  // } else if (strcmp(_key, "seta") == 0) {
  //   m_player.m_enableAutosave = json_intValue(_value); 
  //   m_deserialiseArrayID = 0; // Note "one behind"
  // } else if (strcmp(_key, "cargos") == 0) {
  //   m_deserialiseArrayID = 1;
  // } else if (strcmp(_key, "convs") == 0) {
  //   m_deserialiseArrayID = 2;
  // } else if (strcmp(_key, "plants") == 0) {
  //   m_deserialiseArrayID = 3;
  // } else if (strcmp(_key, "util") == 0) {
  //   m_deserialiseArrayID = 4;
  // } else if (strcmp(_key, "xtrcts") == 0) {
  //   m_deserialiseArrayID = 5;
  // } else if (strcmp(_key, "facts") == 0) {
  //   m_deserialiseArrayID = 6;
  // } else if (strcmp(_key, "scargo") == 0) {
  //   m_deserialiseArrayID = 7;
  // } else if (strcmp(_key, "icargo") == 0) {
  //   m_deserialiseArrayID = 8;
  // } else if (strcmp(_key, "expw00") == 0) {
  //   m_deserialiseArrayID = 9;
  // } else if (strcmp(_key, "expw01") == 0) {
  //   m_deserialiseArrayID = 10;
  // } else if (strcmp(_key, "expw02") == 0) {
  //   m_deserialiseArrayID = 11;
  // } else if (strcmp(_key, "expw03") == 0) {
  //   m_deserialiseArrayID = 12;
  // } else if (strcmp(_key, "expw04") == 0) {
  //   m_deserialiseArrayID = 13;
  // } else if (strcmp(_key, "expw05") == 0) {
  //   m_deserialiseArrayID = 14;
  // } else if (strcmp(_key, "expw06") == 0) {
  //   m_deserialiseArrayID = 15;
  // } else if (strcmp(_key, "expw07") == 0) {
  //   m_deserialiseArrayID = 16;
  // } else if (strcmp(_key, "spw") == 0) {
  //   m_deserialiseArrayID = 17;
  // } else if (strcmp(_key, "impc") == 0) {
  //   // noop
  // } else if (strcmp(_key, "player") == 0) {
  //   jd->didDecodeSublist = deserialiseStructDonePlayer;
  // } else {
  //   pd->system->error("PLAYER DECODE ISSUE, %s", _key);
  // }
}

void deserialiseArrayValuePlayer(json_decoder* jd, int _pos, json_value _value) {
  // int v = json_intValue(_value);
  // float f = json_floatValue(_value);
  // int i = _pos - 1;
  // switch (m_deserialiseArrayID) {
  //   case 0: m_player.m_carryCargo[i] = v; break;
  //   case 1: m_player.m_carryConveyor[i] = v; break;
  //   case 2: m_player.m_carryPlant[i] = v; break;
  //   case 3: m_player.m_carryUtility[i] = v; break;
  //   case 4: m_player.m_carryExtractor[i] = v; break;
  //   case 5: m_player.m_carryFactory[i] = v; break;

  //   case 6: m_player.m_soldCargo[i] = v; break;
  //   case 7: m_player.m_importedCargo[i] = v; break;

  //   case 8:  m_player.m_exportPerWorld[0][i] = f; break;
  //   case 9:  m_player.m_exportPerWorld[1][i] = f; break;
  //   case 10: m_player.m_exportPerWorld[2][i] = f; break;
  //   case 11: m_player.m_exportPerWorld[3][i] = f; break;
  //   case 12: m_player.m_exportPerWorld[4][i] = f; break;
  //   case 13: m_player.m_exportPerWorld[5][i] = f; break;
  //   case 14: m_player.m_exportPerWorld[6][i] = f; break;
  //   case 15: m_player.m_exportPerWorld[7][i] = f; break;

  //   case 16: m_player.m_sellPerWorld[i] = f; break;

  //   case 17: m_player.m_importConsumers[i] = v; break;
  // }
}

void* deserialiseStructDonePlayer(json_decoder* jd, const char* _name, json_value_type _type) {
  // setPlayerPosition(m_player.m_pix_x, m_player.m_pix_y, /*update current location = */ true);
  // m_player.m_camera_pix_x = m_player.m_pix_x;
  // m_player.m_camera_pix_y = m_player.m_pix_y;

  // #ifdef DEV
  // pd->system->logToConsole("-- Player decoded to (%i, %i), current location (%i, %i), money:%i, unlock:%i", 
  //   (int32_t)m_player.m_pix_x, (int32_t)m_player.m_pix_y, m_currentLocation->m_x, m_currentLocation->m_y, m_player.m_money, m_player.m_buildingsUnlockedTo);
  // #endif

  // #ifdef DEMO
  // m_player.m_money = 0;
  // m_player.m_moneyCumulative = 0;
  // m_player.m_moneyHighWaterMark = 0;
  // m_player.m_buildingsUnlockedTo = getCactusUnlock(); // cactus
  // m_player.m_enableTutorial = 0;
  // for (int32_t i = 0; i < kNCargoType; ++i) if (m_player.m_carryCargo[i]) m_player.m_carryCargo[i] = rand() % 10;
  // for (int32_t i = 0; i < kNConvSubTypes; ++i) if (m_player.m_carryConveyor[i]) m_player.m_carryConveyor[i] = rand() % 10;
  // for (int32_t i = 0; i < kNUtilitySubTypes; ++i) if (m_player.m_carryUtility[i]) m_player.m_carryUtility[i] = (i >= kStorageBox ? 0 : rand() % 10);
  // for (int32_t i = 0; i < kNPlantSubTypes; ++i) if (m_player.m_carryPlant[i]) m_player.m_carryPlant[i] = rand() % 10;
  // for (int32_t i = 0; i < kNExtractorSubTypes; ++i) if (m_player.m_carryExtractor[i]) m_player.m_carryExtractor[i] = rand() % 10;
  // for (int32_t i = 0; i < kNFactorySubTypes; ++i) if (m_player.m_carryFactory[i]) m_player.m_carryFactory[i] = rand() % 10;
  // #endif

  // // SCHEMA EVOLUTION - V4 to V5 (v1.0 to v1.1)
  // // Might need to correct the building unlock progression due to new unlocks being added
  // // Need to set defaults for new options parameters
  // if (m_player.m_saveFormat == V1p0_SAVE_FORMAT) {
  //   m_player.m_saveFormat = V1p1_SAVE_FORMAT;
  //   const uint32_t prevUnlockedTo = m_player.m_buildingsUnlockedTo; 
  //   if (prevUnlockedTo >= 65) { // Dessert Factory from 1.0. Need to account for 2x Overflow, Rotavator & FacUp
  //     m_player.m_buildingsUnlockedTo += 4;
  //   } else if (prevUnlockedTo >= 46) { // Conveyor Grease from 1.0. Need to account for 2x Overflow & FacUp
  //     m_player.m_buildingsUnlockedTo += 3;
  //   } else if (prevUnlockedTo >= 26) { // Sign from 1.0. Need to account for 2x Overflow.
  //     m_player.m_buildingsUnlockedTo += 2;
  //   }
  //   m_player.m_enableCrankConveyor = 1; 
  //   m_player.m_enableCentreCamera = 0;
  //   pd->system->logToConsole("-- Performed player schema evolution from v%i to v%i, UnlockedTo:%i -> %i", V1p0_SAVE_FORMAT, V1p1_SAVE_FORMAT, prevUnlockedTo, m_player.m_buildingsUnlockedTo);
  // }

  // m_deserialiseArrayID = -1;

  return NULL;
}
