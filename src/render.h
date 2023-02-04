#pragma once
#include "game.h"

void render(void);

void updateRenderList(void);

void addTrauma(float _amount);

bool checkBound(int16_t _x, int16_t _y, uint16_t _hw, uint16_t _hh, int16_t _sX, int16_t _sY);