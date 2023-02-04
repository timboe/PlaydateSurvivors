#pragma once
#include "game.h"


enum kEnemyType {
  kNoEnemy,
  kDemoEnemy,
  kNEnemyType
};

struct Enemy_t{
  enum kEnemyType m_type;

  LCDSprite* m_sprite;

  //bool m_visible;

  bool m_boss;

  int16_t m_wave;

  int16_t m_health;

  float m_v;

  float m_x;
  float m_y;

  int16_t m_index;
};


void tickEnemies(void);

void initEnemies(void);

void resetEnemies(void);

void enemyAddToRender(void);

void enemyManagerFreeEnemy(struct Enemy_t* _enemy);