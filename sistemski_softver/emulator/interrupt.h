#ifndef _INTERRUPT_EMULATOR_H
#define _INTERRUPT_EMULATOR_H

#include <mutex>
#include "cpu.h"
using namespace std;

class CPU;

void KeyboardHandler(CPU* processor);

#endif