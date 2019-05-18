#ifndef _EXECUTABLE_LINKER_H
#define _EXECUTABLE_LINKER_H

#define MEMORY_ADDRESS_SPACE 65536

#include "../common/structures.h"
#include <cstdint>

typedef map<string, uint16_t> LinkerSections;

class Executable
{

private:
	uint8_t memory[MEMORY_ADDRESS_SPACE];
	uint16_t initialPC;
	bool initialPCDefined = false;

	const LinkerSections sectionStartMap;
	SymbolTable symbolTable;
	SectionTable sectionTable;

public:
	Executable(const LinkerSections& sectionStartMap) : sectionStartMap(sectionStartMap) {}
	const uint8_t& MemoryRead(const uint16_t& address);
	void MemoryWrite(const uint16_t& address, const uint8_t& data, bool linker = true);
	
	bool CheckIfExecutable(uint16_t initialPC, uint16_t length);

	uint16_t& InitialPC() { return initialPC; }
	friend class Linker;
	friend class Emulator;
};

#endif