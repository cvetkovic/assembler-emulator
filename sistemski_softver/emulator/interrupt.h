#ifndef _INTERRUPT_EMULATOR_H
#define _INTERRUPT_EMULATOR_H

#include <mutex>
#include "cpu.h"
#include "../common/enums.h"
using namespace std;

class CPU;

void KeyboardHandler(CPU* processor);
void TimerHandler(CPU* processor);

#endif