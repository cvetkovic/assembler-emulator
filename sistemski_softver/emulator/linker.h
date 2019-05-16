#ifndef _LINKER_EMULATOR_H
#define _LINKER_EMULATOR_H

#include "../common/structures.h"

#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
using namespace std;

struct LinkerSectionsEntry
{
	string sectionName;
	uint16_t startLocation;

	LinkerSectionsEntry(string sectionName, uint16_t startLocation) :
		sectionName(sectionName), startLocation(startLocation) {}
};

class Linker
{

public:
	Linker(vector<string> inputFiles, vector<LinkerSectionsEntry> sections);

};

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

};

#endif