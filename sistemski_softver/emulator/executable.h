#ifndef _EXECUTABLE_LINKER_H
#define _EXECUTABLE_LINKER_H

#define MEMORY_ADDRESS_SPACE 65536

#include "../common/structures.h"
#include <cstdint>

class Executable
{

private:
	uint8_t memory[MEMORY_ADDRESS_SPACE];
	uint16_t initialPC;
	bool initialPCDefined = false;

	SymbolTable symbolTable;
	SectionTable sectionTable;

public:
	inline uint8_t MemoryRead(const uint16_t& address) { return memory[address]; }
	inline void MemoryWrite(const uint16_t& address, const uint8_t& data) {	memory[address] = data;	}
	
	uint16_t& InitialPC() { return initialPC; }
	friend class Linker;
	friend class Emulator;
};

#endif