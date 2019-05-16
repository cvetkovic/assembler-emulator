#include "linker.h"

void Linker::Initialize(vector<string>& inputFiles, LinkerSections& sections)
{
	if (inputFiles.size() == 0)
		throw LinkerException("No input files provided.", ErrorCodes::LINKER_NO_FILES);

	numberOfFiles = inputFiles.size();
	objectFiles = new ObjectFile*[numberOfFiles];
	executable = new Executable();
	for (size_t i = 0; i < inputFiles.size(); i++)
		objectFiles[i] = new ObjectFile(inputFiles.at(i));
}

void Linker::MergeAndLoadToMemory()
{
	LinkerSections location = this->linkerSections;

	// for each object file
	for (size_t i = 0; i < numberOfFiles; i++)
	{
		ObjectFile& objectFile = *objectFiles[i];

		unsigned long currentFilePointer = 0;
		size_t sectionTableSize = objectFile.GetSectionTable().GetSize();

		unsigned long readFrom = 0;

		// for each section
		for (size_t j = 0; j < sectionTableSize; j++)
		{
			SectionTableEntry& entry = *objectFile.GetSectionTable().GetEntryByID((SectionID)j);

			if (location.find(entry.name) == location.end())
				throw LinkerException("Call linker with specifying loading address of '" + entry.name + "' section.", ErrorCodes::LINKER_SECTION_ADDRESS_UNSPECIFIED);

			uint16_t addressToWriteTo = location.find(entry.name)->second;

			for (unsigned long k = 0; k < entry.length; k++)
				executable->MemoryWrite(addressToWriteTo++, objectFile.ContentRead(readFrom++));

			location.find(entry.name)->second = addressToWriteTo;
		}
	}
}

Linker::~Linker()
{
	for (size_t i = 0; i < numberOfFiles; i++)
		delete objectFiles[i];
	delete[] objectFiles;

	delete executable;
}

Executable* Linker::GetExecutable()
{
	MergeAndLoadToMemory();

	return executable;
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

ObjectFile::~ObjectFile()
{
	delete content;
}
