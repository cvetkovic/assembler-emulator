#include "linker.h"

Linker::Linker(vector<string> inputFiles, vector<LinkerSectionsEntry> sections)
{
	if (inputFiles.size() == 0)
		throw LinkerException("No input files provided.", ErrorCodes::LINKER_NO_FILES);

	for (string file : inputFiles)
	{
		ObjectFile objectFile(file);
	}
}

ObjectFile::ObjectFile(string url)
{
	ifstream inputFile;
	inputFile.open(url, ios::in | ios::binary);

	if (!inputFile.is_open())
		throw LinkerException("Linker cannot open input file '" + url + "'.", ErrorCodes::LINKER_CANNOT_OPEN);

	size_t symbolTableSize = 0;
	size_t sectionTableSize = 0;
	size_t relocationTableSize = 0;

	inputFile.read(reinterpret_cast<char*>(&symbolTableSize), sizeof(size_t));
	inputFile.read(reinterpret_cast<char*>(&sectionTableSize), sizeof(size_t));
	inputFile.read(reinterpret_cast<char*>(&relocationTableSize), sizeof(size_t));
	inputFile.read(reinterpret_cast<char*>(&contentSize), sizeof(size_t));

	symbolTable = SymbolTable::Deserialize(symbolTableSize, inputFile);
	sectionTable = SectionTable::Deserialize(sectionTableSize, inputFile);
	relocationTable = RelocationTable::Deserialize(relocationTableSize, inputFile);

	// load data content from file
	content = new uint8_t[contentSize];
	for (size_t i = 0; i < contentSize; i++)
		inputFile.read(reinterpret_cast<char*>(&content[i]), sizeof(uint8_t));

	inputFile.close();
}