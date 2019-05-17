#ifndef _LINKER_EMULATOR_H
#define _LINKER_EMULATOR_H

#include "../common/structures.h"
#include "executable.h"

#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
using namespace std;

#define START_SYMBOL "_start"

typedef map<string, uint16_t> LinkerSections;

class ObjectFile
{

private:
	SymbolTable symbolTable;
	SectionTable sectionTable;
	RelocationTable relocationTable;

	// for raw data read from file
	size_t contentSize;
	uint8_t* content;

public:
	ObjectFile(string url);
	~ObjectFile();

	SymbolTable& GetSymbolTable() { return symbolTable; }
	SectionTable& GetSectionTable() { return sectionTable; }
	RelocationTable& GetRelocationTable() { return relocationTable; }

	const uint8_t& ContentRead(const unsigned long& address) const { return content[address]; }
};

class Linker
{

private:
	size_t numberOfFiles = 0;
	ObjectFile** objectFiles;
	Executable* executable;

	const LinkerSections sectionStartMap;

	void Initialize(vector<string>& inputFiles, LinkerSections& sections);
	void MergeAndLoadExecutable();
	void ResolveRelocations();
	void ResolveStartSymbol();

public:
	Linker(vector<string> inputFiles, LinkerSections sectionStart) : sectionStartMap(sectionStart) { Initialize(inputFiles, sectionStart); }
	~Linker();

	Executable* GetExecutable();

};

#endif