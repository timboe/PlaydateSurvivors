#pragma once
#include "game.h"
#include "player.h"
 
enum kGameMode {
  kWanderMode, 
  kTitles, 
  kNGameModes
};

void addUIToSpriteList(void);

void updateUI(int _fc);

void updateUITitles(int _fc);

void modTitleCursor(bool _increment);

uint16_t getTitleCursorSelected(void);

void addSaveLoadProgressSprite(int32_t _doneX, int32_t _ofY);

LCDSprite* getSaveSprite(void);

LCDSprite* getLoadSprite(void);

LCDSprite* getGenSprite(void);

void snprintf_c(char* _buf, uint8_t _bufSize, int _n);

void setGameMode(enum kGameMode _mode);

enum kGameMode getGameMode(void);

void roundedRect(uint16_t _o, uint16_t _w, uint16_t _h, uint16_t _r, LCDColor _c);

void initiUI(void);

void resetUI(void);

