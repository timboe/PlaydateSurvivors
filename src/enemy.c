#include "enemy.h"
#include "sprite.h"
#include "player.h"
#include "location.h"
#include "render.h"

const int32_t SIZE_ENEMIES = MAX_ENEMIES * sizeof(struct Enemy_t);

struct Enemy_t* m_enemies = NULL;

uint16_t m_enemySearchLocation = 0;
int16_t m_nEnemies = 0;

struct Enemy_t* enemyManagerNewEnemy(enum kEnemyType _type);

struct Enemy_t* enemyManagerGetByIndex(uint16_t _index);

void newEnemy(enum kEnemyType _type);

/// ///

struct Enemy_t* enemyManagerNewEnemy(enum kEnemyType _type) {
  for (uint8_t try = 0; try < 2; ++try) {
    const uint32_t start = (try == 0 ? m_enemySearchLocation : 0);
    const uint32_t stop  = (try == 0 ? MAX_ENEMIES : m_enemySearchLocation);
    for (uint32_t i = start; i < stop; ++i) {
      if (m_enemies[i].m_type == kNoEnemy) {
        ++m_nEnemies;
        m_enemySearchLocation = i+1;
        m_enemies[i].m_type = _type;
        return &(m_enemies[i]);
      }
    }
  }
  #ifdef DEV
  pd->system->error("Cannot allocate any more enemies! search loc %i", m_enemySearchLocation);
  #endif
  return NULL;
}

struct Enemy_t* enemyManagerGetByIndex(uint16_t _index) {
  return &(m_enemies[_index]);
}

void enemyManagerFreeEnemy(struct Enemy_t* _enemy) {
  _enemy->m_type = kNoEnemy;
  m_enemySearchLocation = _enemy->m_index;
  pd->sprite->setVisible(_enemy->m_sprite, false);
  --m_nEnemies;
}

void randomSpawn(struct Enemy_t* _enemy) {
  struct Location_t* loc = getPlayerLocation();
  int16_t spawnX = loc->m_x, spawnY = loc->m_y;
  switch (rand() % 4) {
    case 0: spawnX += SPAWN_X; spawnY += -SPAWN_Y + (rand() % SPAWN_Y * 2); break;
    case 1: spawnX -= SPAWN_X; spawnY += -SPAWN_Y + (rand() % SPAWN_Y * 2); break;
    case 2: spawnY += SPAWN_Y; spawnX += -SPAWN_X + (rand() % SPAWN_X * 2); break;
    case 3: spawnY -= SPAWN_Y; spawnX += -SPAWN_X + (rand() % SPAWN_X * 2); break;
  }
  pd->sprite->moveTo(_enemy->m_sprite, (spawnX * TILE_PIX) + TILE_PIX/2, (spawnY * TILE_PIX) + TILE_PIX/2);
  pd->sprite->getPosition(_enemy->m_sprite, &(_enemy->m_x), &(_enemy->m_y));
}

bool offScreen(struct Enemy_t* _enemy, struct Player_t* _p) {
  return (fabs(_p->m_x - _enemy->m_x) > 2*CHUNK_PIX_X || fabs(_p->m_y - _enemy->m_y) > 2*CHUNK_PIX_Y);
}

void newEnemy(enum kEnemyType _type) {
  struct Enemy_t* e = enemyManagerNewEnemy(_type);
  if (!e) return;

  pd->sprite->setImage(e->m_sprite, getSprite32(rand() % 9, 21 + rand() % 4), kBitmapUnflipped);
  e->m_v = 4.0f;
  e->m_health = 16;

  pd->sprite->addSprite(e->m_sprite);
  pd->sprite->setVisible(e->m_sprite, true);
  randomSpawn(e);

}

void enemyAddToRender() {
  struct Player_t* p = getPlayer();
  for (uint16_t i = 0; i < MAX_ENEMIES; ++i) {
    struct Enemy_t* e = enemyManagerGetByIndex(i);
    if (e->m_type != kNoEnemy) {
      if (checkBound(e->m_x, e->m_y, TILE_PIX/2, TILE_PIX/2, p->m_x, p->m_y)) {
        pd->sprite->addSprite(e->m_sprite);
      }
    }
  }
}

void tickEnemies() {

  static uint32_t slowdown = 0;
  if (++slowdown % 2 && m_nEnemies < MAX_ENEMIES) {
    pd->system->logToConsole("try new enemy, enemies = %i", m_nEnemies);
    newEnemy(kDemoEnemy);
  }

  struct Player_t* p = getPlayer();
  for (uint16_t i = 0; i < MAX_ENEMIES; ++i) {
    struct Enemy_t* e =enemyManagerGetByIndex(i);
    if (e->m_type == kNoEnemy) continue;

    pd->sprite->getPosition(e->m_sprite, &(e->m_x), &(e->m_y));
    pd->sprite->setDrawMode(e->m_sprite, kDrawModeCopy);

    if (offScreen(e, p)) {

      randomSpawn(e);

    } else {

      float a = atan2f(p->m_x - e->m_x, p->m_y - e->m_y);

      e->m_x += e->m_v * sinf(a);
      e->m_y += e->m_v * cosf(a);;

      float tX, tY;

      int len;
      SpriteCollisionInfo* collInfo = pd->sprite->moveWithCollisions(e->m_sprite, e->m_x, e->m_y, &tX, &tY, &len);
      if (len) pd->system->realloc(collInfo, 0); // Free

      e->m_x = tX;
      e->m_y = tY;

      pd->sprite->setZIndex(e->m_sprite, tY);

    }
  }




}


void initEnemies() {
  m_enemies = pd->system->realloc(NULL, SIZE_ENEMIES);
  memset(m_enemies, 0, SIZE_ENEMIES);
  #ifdef DEV
  pd->system->logToConsole("malloc: for enemies %i", SIZE_ENEMIES/1024);
  #endif
}


SpriteCollisionResponseType enemyLCDSpriteCollisionFilterProc(LCDSprite* _enemy, LCDSprite* _other) {
  if (pd->sprite->getTag(_other) == FLAG_ENEMY) {
    return kCollisionTypeSlide;
  }
  return kCollisionTypeOverlap;
}

void resetEnemies() {
  for (uint32_t i = 0; i < MAX_ENEMIES; ++i) {
    if (m_enemies[i].m_sprite) pd->sprite->freeSprite(m_enemies[i].m_sprite);
  }
  memset(m_enemies, 0, SIZE_ENEMIES);
  for (uint32_t i = 0; i < MAX_ENEMIES; ++i) {
    m_enemies[i].m_index = i;
    m_enemies[i].m_sprite = pd->sprite->newSprite();
    pd->sprite->setCollisionResponseFunction(m_enemies[i].m_sprite, enemyLCDSpriteCollisionFilterProc);
    pd->sprite->setTag(m_enemies[i].m_sprite, FLAG_ENEMY);
    pd->sprite->setUserdata(m_enemies[i].m_sprite, (void*) enemyManagerGetByIndex(i));

    static PDRect bound32 = {.x = 0, .y = 0, .width = TILE_PIX, .height = TILE_PIX};
    static PDRect cbound32 = {.x = COFF32, .y = COFF32, .width = TILE_PIX-2*COFF32, .height = TILE_PIX-2*COFF32};
    pd->sprite->setBounds(m_enemies[i].m_sprite, bound32);
    pd->sprite->setCollideRect(m_enemies[i].m_sprite, cbound32);

  }
  m_enemySearchLocation = 0;
  m_nEnemies = 0;
}
