#pragma once
#include "game.h"

void gameClickConfigHandler(uint32_t _buttonPressed);

void clickHandlerReplacement(void);

uint8_t getPressed(uint32_t _i);

bool getPressedAny(void);

bool bPressed(void);

void resetMomentum(void);
