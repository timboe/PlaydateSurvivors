#include "projectile.h"
#include "sprite.h"
#include "player.h"
#include "location.h"
#include "enemy.h"
#include "render.h"

const int32_t SIZE_PROJECTILES = MAX_PROJECTILES * sizeof(struct Projectile_t);

struct Projectile_t* m_projectiles = NULL;

uint16_t m_projectileSearchLocation = 0;
uint16_t m_nProjectiles = 0;

struct Projectile_t* projectileManagerNewProjectile(enum kProjectileType _type);

struct Projectile_t* projectileManagerGetByIndex(uint16_t _index);

void projectileManagerFreeProjectile(struct Projectile_t* _projectile);

void newProjectile(enum kProjectileType _type);

/// ///

struct Projectile_t* projectileManagerNewProjectile(enum kProjectileType _type) {
  for (uint8_t try = 0; try < 2; ++try) {
    const uint32_t start = (try == 0 ? m_projectileSearchLocation : 0);
    const uint32_t stop  = (try == 0 ? MAX_PROJECTILES : m_projectileSearchLocation);
    for (uint32_t i = start; i < stop; ++i) {
      if (m_projectiles[i].m_type == kNoProjectile) {
        ++m_nProjectiles;
        m_projectileSearchLocation = i+1;
        m_projectiles[i].m_type = _type;
        return &(m_projectiles[i]);
      }
    }
  }
  #ifdef DEV
  pd->system->error("Cannot allocate any more projectiles!");
  #endif
  return NULL;
}

struct Projectile_t* projectileManagerGetByIndex(uint16_t _index) {
  return &(m_projectiles[_index]);
}

void projectileManagerFreeProjectile(struct Projectile_t* _projectile) {
  _projectile->m_type = kNoProjectile;
  m_projectileSearchLocation = _projectile->m_index;
  pd->sprite->setVisible(_projectile->m_sprite, false);
  --m_nProjectiles;
}

void newProjectile(enum kProjectileType _type) {
  struct Projectile_t* p = projectileManagerNewProjectile(_type);


  pd->sprite->setImage(p->m_sprite, getProjectileBitmap(), kBitmapUnflipped);

  struct Player_t* pl = getPlayer();

  pd->sprite->moveTo(p->m_sprite, pl->m_x, pl->m_y);
  pd->sprite->getPosition(p->m_sprite, &(p->m_x), &(p->m_y));
  pd->sprite->setZIndex(p->m_sprite, Z_INDEX_PROJECTILE);
  pd->sprite->setVisible(p->m_sprite, true);

  const float v = 12.0f;
  const float a = ((rand() % 360) / 360.0f) * 2 * (float)M_PI;
  p->m_vx = v * sinf(a);
  p->m_vy = v * cosf(a); 
  p->m_damage = 1;

  pd->sprite->addSprite(p->m_sprite);

}

void projectileAddToRender() {
  struct Player_t* p = getPlayer();
  for (uint16_t i = 0; i < MAX_PROJECTILES; ++i) {
    struct Projectile_t* p = projectileManagerGetByIndex(i);
    if (p->m_type == kNoProjectile) continue;
    if (checkBound(p->m_x, p->m_y, 8, 8, p->m_x, p->m_y)) {
      pd->sprite->addSprite(p->m_sprite);
    }
  }
}

void moveProjectiles(int16_t _x, int16_t _y) {
  for (uint16_t i = 0; i < MAX_PROJECTILES; ++i) {
    struct Projectile_t* p = projectileManagerGetByIndex(i);
    if (p->m_type == kNoProjectile) continue;
    pd->sprite->moveTo(p->m_sprite, p->m_x + _x, p->m_y + _y);
    pd->sprite->getPosition(p->m_sprite, &(p->m_x), &(p->m_y));
  }
}

void tickProjectiles() {

  static uint32_t slowdown = 0;
  if (++slowdown % 2 && m_nProjectiles < MAX_PROJECTILES) {
    newProjectile(kDemoProjectile);
  }

  struct Player_t* pl = getPlayer();
  for (uint16_t i = 0; i < MAX_PROJECTILES; ++i) {
    struct Projectile_t* p = projectileManagerGetByIndex(i);
    if (p->m_type == kNoProjectile) continue;

    pd->sprite->getPosition(p->m_sprite, &(p->m_x), &(p->m_y));

    p->m_x += p->m_vx;
    p->m_y += p->m_vy;

    float tX, tY;

    int len;
    SpriteCollisionInfo* collInfo = pd->sprite->moveWithCollisions(p->m_sprite, p->m_x, p->m_y, &tX, &tY, &len);

    for (uint32_t c = 0; c < len; ++c) {
      SpriteCollisionInfo* info = &collInfo[c];
      if (pd->sprite->getTag(info->other) != FLAG_ENEMY) continue;

      struct Enemy_t* e = (struct Enemy_t*) pd->sprite->getUserdata(info->other);
      if (e->m_type == kNoEnemy) continue;

      e->m_health -= p->m_damage;

      if (e->m_health <= 0) {
        enemyManagerFreeEnemy(e);
      } else {
        pd->sprite->setDrawMode(e->m_sprite, kDrawModeInverted);
        //pd->sprite->markDirty(e->m_sprite);
      }

    }

    if (len) pd->system->realloc(collInfo, 0); // Free

    p->m_x = tX;
    p->m_y = tY;

    if (abs(pl->m_x - p->m_x) > CHUNK_PIX_X || abs(pl->m_y - p->m_y) > CHUNK_PIX_Y) {
      projectileManagerFreeProjectile(p);
    }

  }


}


void initProjectiles() {
  m_projectiles = pd->system->realloc(NULL, SIZE_PROJECTILES);
  memset(m_projectiles, 0, SIZE_PROJECTILES);
  #ifdef DEV
  pd->system->logToConsole("malloc: for projectiles %i", SIZE_PROJECTILES/1024);
  #endif
}


SpriteCollisionResponseType projectileLCDSpriteCollisionFilterProc(LCDSprite* _projectile, LCDSprite* _other) {
  return kCollisionTypeOverlap;
}

void resetProjectiles() {
  for (uint32_t i = 0; i < MAX_PROJECTILES; ++i) {
    if (m_projectiles[i].m_sprite) pd->sprite->freeSprite(m_projectiles[i].m_sprite);
  }
  memset(m_projectiles, 0, SIZE_PROJECTILES);
  for (uint32_t i = 0; i < MAX_PROJECTILES; ++i) {
    m_projectiles[i].m_index = i;
    m_projectiles[i].m_sprite = pd->sprite->newSprite();
    pd->sprite->setCollisionResponseFunction(m_projectiles[i].m_sprite, projectileLCDSpriteCollisionFilterProc);
    pd->sprite->setTag(m_projectiles[i].m_sprite, FLAG_PROJECTILE);

    static PDRect bound16 = {.x = 0, .y = 0, .width = TILE_PIX/2, .height = TILE_PIX/2};
    static PDRect cbound16 = {.x = COFF16, .y = COFF16, .width = (TILE_PIX/2)-2*COFF16, .height = (TILE_PIX/2)-2*COFF16};

    pd->sprite->setBounds(m_projectiles[i].m_sprite, bound16);
    pd->sprite->setCollideRect(m_projectiles[i].m_sprite, cbound16);

  }
  m_projectileSearchLocation = 0;
  m_nProjectiles = 0;
}
