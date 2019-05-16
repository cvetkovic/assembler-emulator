#ifndef _MAIN_EMULATOR_H
#define _MAIN_EMULATOR_H

#include "linker.h"

#include <iostream>
#include <regex>
#include <vector>
using namespace std;

int main(int argc, char** argv)
{
	if (argc > 1)
	{
		vector<LinkerSectionsEntry> sections;
		vector<string> inputFiles;
		
		regex placeRegex("^-place=\\.{0,1}[a-zA-Z_][a-zA-Z0-9_]*@0x[0-9a-fA-F]{1,4}$");
		regex inputFileRegex("^[a-zA-Z_][a-zA-Z0-9_.]*\\.o$");

		for (int i = 1; i < argc; i++)
		{
			string input = argv[i];

			if (regex_match(input, placeRegex))
			{
				string sectionName = input.substr(input.find('=') + 1, input.find('@') - input.find('=') - 1);
				uint16_t location = (uint16_t)strtol(input.substr(input.find('@') + 1, input.size() - input.find('@')).c_str(), 0, 16);

				sections.push_back(LinkerSectionsEntry(sectionName, location));
			}
			else if (regex_match(input, inputFileRegex))
			{
				inputFiles.push_back(input);
			}
			else
				cout << "Invalid emulator calling parameters." << endl;
		}

		Linker linker(inputFiles, sections);
		//Executable executable = linker.GetExecutable();

		//Emulator emulator(executable);
	}
	else
		cout << "Emulator invocation required at least one parameter." << endl;
}

#endif