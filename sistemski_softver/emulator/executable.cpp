#include "executable.h"

const uint8_t& Executable::MemoryRead(const uint16_t & address)
{
	return memory[address];
}

void Executable::MemoryWrite(const uint16_t& address, const uint8_t& data, bool linker)
{
	// linker and emulator share this method
	// linker should be able to write everywhere
	if (linker)
	{
		memory[address] = data;
		return;
	}
	
	LinkerSections::const_iterator it;
	for (it = sectionStartMap.begin(); it != sectionStartMap.end(); it++)
	{
		const SectionTableEntry& entry = *sectionTable.GetEntryByName(it->first);

		if (((it->second <= address) && (address < it->second + entry.length)) &&
			(((entry.flags & FLAG_WRITABLE) == 0) || ((entry.flags & FLAG_READ_ONLY) == 1)))
			throw EmulatorException("Segmentation fault. Program tried to write to read-only section.", ErrorCodes::EMULATOR_SEGMENTATION_FAULT);
	}

	memory[address] = data;
}

bool Executable::CheckIfExecutable(uint16_t initialPC, uint16_t length)
{
	LinkerSections::const_iterator it;
	for (it = sectionStartMap.begin(); it != sectionStartMap.end(); it++)
	{
		if (sectionTable.GetEntryByName(it->first))
		{
			const SectionTableEntry& entry = *sectionTable.GetEntryByName(it->first);

			if (((it->second <= initialPC) && (initialPC + length < it->second + (uint16_t)entry.length)) &&
				((entry.flags & FLAG_EXECUTABLE) == 0))
				return false;
		}
		else
			throw EmulatorException("Section '" + it->first + "' not found in provided files.", ErrorCodes::EMULATOR_SECTION_MISSING);
	}

	return true;
}