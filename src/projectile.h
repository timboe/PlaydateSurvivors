#pragma once
#include "game.h"


enum kProjectileType {
  kNoProjectile,
  kDemoProjectile,
  kNProjectileType
};

struct Projectile_t{
  enum kProjectileType m_type;

  LCDSprite* m_sprite;

  float m_vx;
  float m_vy;

  float m_x;
  float m_y;

  int16_t m_index;
};


void tickProjectiles(void);

void initProjectiles(void);

void resetProjectiles(void);

void projectileAddToRender(void);