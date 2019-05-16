#ifndef _LINKER_EMULATOR_H
#define _LINKER_EMULATOR_H

#include <iostream>
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

#endif