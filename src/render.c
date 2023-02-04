#include <math.h>
#include "render.h"
#include "location.h"
#include "player.h"
#include "sprite.h"
#include "chunk.h"
#include "io.h"
#include "input.h"
#include "ui.h"
#include "enemy.h"

float m_trauma = 0.0f, m_decay = 0.0f;

void chunkAddToRender(struct Chunk_t* _chunk);

/// ///

void addTrauma(float _amount) {
  m_trauma += _amount;
  m_trauma *= -1;
  m_decay = _amount;
}

void render() {

  struct Player_t* p = getPlayer();

  if (true && m_decay > 0.0f) {
    m_decay -= TRAUMA_DECAY;
    m_trauma += (m_trauma > 0 ? -m_decay : m_decay);
    pd->display->setOffset(0, m_trauma * TRAUMA_AMPLIFICATION);
  } else {
    pd->display->setOffset(0, 0);
  }

  const float offX = -(p->m_x - (DEVICE_PIX_X/2));
  const float offY = -(p->m_y - (DEVICE_PIX_Y/2));

  pd->graphics->setDrawMode(kDrawModeCopy);
  pd->graphics->setDrawOffset(offX, offY);

  // Draw FPS indicator (dbg only)
  #ifdef DEV
  if (true || ALWAYS_FPS) {
    pd->system->drawFPS(0, 0);
  }
  #endif
}

bool checkBound(LCDSprite* _s, uint16_t _hw, uint16_t _hh, int16_t _sX, int16_t _sY) {
  float x, y;
  pd->sprite->getPosition(_s, &x, &y);
  const int16_t x_i = x, y_i = y;
  // pd->system->logToConsole(" -- -- -- sprite is at: %i %i, it has hw, hh:%i %i, x_i, y_i, _hw, _hh);
  // pd->system->logToConsole(" -- -- screen is at: %i %i", _sX, _sY);
  // pd->system->logToConsole(" -- -- bounds are: < %i  ^ %i > %i v %i ", 
  //   -(HALF_DEVICE_PIX_X),
  //   -(HALF_DEVICE_PIX_Y),
  //   HALF_DEVICE_PIX_X,
  //   HALF_DEVICE_PIX_Y);
  // pd->system->logToConsole(" -- veto if x %i <  %i - %s", (x_i + _hw - _sX), -(HALF_DEVICE_PIX_X), x_i + _hw - _sX < -(HALF_DEVICE_PIX_X) ? "VETO" : "OK");
  // pd->system->logToConsole(" -- veto if x %i >= %i - %s", (x_i - _hw - _sX), (HALF_DEVICE_PIX_X),  x_i - _hw - _sX >= (HALF_DEVICE_PIX_X) ? "VETO" : "OK");
  // pd->system->logToConsole(" -- veto if y %i <  %i - %s", (y_i + _hh - _sY), -(HALF_DEVICE_PIX_Y), y_i + _hh - _sY < -(HALF_DEVICE_PIX_Y) ? "VETO" : "OK");
  // pd->system->logToConsole(" -- veto if y %i >= %i - %s", (y_i - _hh - _sY), (HALF_DEVICE_PIX_Y),  y_i - _hh - _sY >= (HALF_DEVICE_PIX_Y) ? "VETO" : "OK");

  if (x_i + _hw - _sX < -(HALF_DEVICE_PIX_X)) return false;// { pd->system->logToConsole(" - V0"); return false; }
  if (x_i - _hw - _sX >= HALF_DEVICE_PIX_X) return false;//{ pd->system->logToConsole(" - V1"); return false; }
  //
  if (y_i + _hh - _sY < -(HALF_DEVICE_PIX_Y)) return false;//{ pd->system->logToConsole(" - V2"); return false; }
  if (y_i - _hh - _sY >= HALF_DEVICE_PIX_Y) return false;//{ pd->system->logToConsole(" - V3"); return false; }
  //pd->system->logToConsole(" - OK");
  return true;
}

void chunkAddToRender(struct Chunk_t* _chunk) {
  LCDSprite* bg = _chunk->m_bkgSprite;
  //struct Player_t* p = getPlayer();
  // 
  // Disabled for now as player can run faster than tick update
  //if (bg && checkBound(bg, CHUNK_PIX_X/2, CHUNK_PIX_Y/2, p->m_camera_pix_x, p->m_camera_pix_y)) {
  //  pd->sprite->addSprite(bg);
  //}
  //
  if (bg) pd->sprite->addSprite(bg);
  //
}

void updateRenderList() {

  pd->system->logToConsole("Update render list");

  struct Chunk_t* currentChunk = getCurrentChunk();
  pd->sprite->removeAllSprites();

  struct Player_t* player = getPlayer();
  pd->sprite->addSprite(player->m_sprite);

  const enum kSaveLoadRequest io = currentIOAction();

  if (io != kDoScreenShot) {
    addUIToSpriteList();
  }

  chunkAddToRender(currentChunk);
  for (uint32_t i = 0; i < CHUNK_NEIGHBORS_ALL; ++i) {
    chunkAddToRender(currentChunk->m_neighborsALL[i]);
  }

  enemyAddToRender();
}