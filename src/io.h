#pragma once
#include "game.h"

enum kSaveLoadRequest {kDoNothing, kDoSave, kDoLoad, kDoNewWorld, kDoSaveDelete, kDoScanSlots, kDoResetPlayer, kDoTitle, kDoScreenShot};

void synchronousSave(void);

void scanSlots(void);

bool hasSaveData(void);

void hardReset(void);

void doIO(enum kSaveLoadRequest _first, enum kSaveLoadRequest _andThen, enum kSaveLoadRequest _andFinally);

enum kSaveLoadRequest currentIOAction(void);

bool IOOperationInProgress(void);

void enactIO(void);