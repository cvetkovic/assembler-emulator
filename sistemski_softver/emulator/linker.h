#ifndef _LINKER_EMULATOR_H
#define _LINKER_EMULATOR_H

#include "../common/arithmetic.h"
#include "../common/structures.h"
#include "executable.h"

#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
using namespace std;

#define START_SYMBOL "_start"

#define IVT_START 0x0000
#define IVT_LENGTH 8
#define IVT_SECTION_NAME "iv_table"
#define MEMORY_MAPPED_REGISTERS_START 0xFF00
#define MEMORY_MAPPED_REGISTERS_END   0xFFFF

class ObjectFile
{

private:
	SymbolTable symbolTable;
	SectionTable sectionTable;
	RelocationTable relocationTable;
	TNSTable tns;

	// for raw data read from file
	size_t contentSize;
	uint8_t* content;

public:
	ObjectFile(string url);
	~ObjectFile();

	SymbolTable& GetSymbolTable() { return symbolTable; }
	SectionTable& GetSectionTable() { return sectionTable; }
	RelocationTable& GetRelocationTable() { return relocationTable; }
	TNSTable& GetTNSTable() { return tns; }

	const uint8_t& ContentRead(const unsigned long& address) const { return content[address]; }
};

class Linker
{

private:
	size_t numberOfFiles = 0;
	ObjectFile** objectFiles;
	Executable* executable;

	const LinkerSections sectionStartMap;

	size_t totalTNSCount = 0;

	ObjectFile* GetObjectFile(const SymbolTableEntry& symbol);

	void Initialize(vector<string>& inputFiles, LinkerSections& sections);
	void MergeAndLoadExecutable();
	void ResolveTNS();
	void ResolveRelocations();
	void ResolveStartSymbol();
	void DeleteLocalSymbols();
	void CheckForNotProvidedFiles();

public:
	Linker(vector<string> inputFiles, LinkerSections sectionStart) : sectionStartMap(sectionStart) { Initialize(inputFiles, sectionStart); }
	~Linker();

	Executable* GetExecutable();

};

#endif