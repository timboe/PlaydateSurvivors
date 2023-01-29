#pragma once

#include <stdio.h>
#include <stdbool.h> 

#include "pd_api.h"
#include "constants.h"

// Directions
enum kDir{SN, WE, NS, EW, kDirN};

extern PlaydateAPI* pd;

void setPDPtr(PlaydateAPI* _p);

int gameLoop(void* _data);

int getFrameCount(void);

uint16_t locToPix(uint16_t _loc);

uint16_t pixToLoc(uint16_t _pix);

void menuOptionsCallbackMenu(void*);

void populateMenuTitle(void);

void populateMenuGame(void);

void initGame(void);

void reset(bool _resetThePlayer);

char* ftos(float _value, int16_t _size, char* _dest);