#ifndef _CPU_EMULATOR_H
#define _CPU_EMULATOR_H

#include <cstdint>
#include "emulator.h"

class CPU
{

private:
	// pointer to executable memory
	uint8_t* memory;

	// r0-r7 registers
	uint16_t registerFile[8];
	
	// pointer to registerFile[6]
	uint16_t& sp = registerFile[6];
	// pointer to registerFile76]
	uint16_t& pc = registerFile[7];
	// r15
	uint16_t psw;

	bool halted;

	void InstructionFetch();
	void InstructionDecode();
	void InstructionExecute();
	void InstructionHandleInterrupt();

public:

	bool GetN();
	bool GetZ();
	bool GetC();
	bool GetV();

	friend class Emulator;

};

#endif