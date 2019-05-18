#include "linker.h"

void Linker::Initialize(vector<string>& inputFiles, LinkerSections& sections)
{
	if (inputFiles.size() == 0)
		throw LinkerException("No input files provided.", ErrorCodes::LINKER_NO_FILES);

	numberOfFiles = inputFiles.size();
	objectFiles = new ObjectFile*[numberOfFiles];
	executable = new Executable(sectionStartMap);
	for (size_t i = 0; i < inputFiles.size(); i++)
		objectFiles[i] = new ObjectFile(inputFiles.at(i));
}

void Linker::MergeAndLoadExecutable()
{
	LinkerSections location = this->sectionStartMap;

	// for each object file
	for (size_t i = 0; i < numberOfFiles; i++)
	{
		ObjectFile& objectFile = *objectFiles[i];

		// objectFile number of section table entries
		size_t sectionTableSize = objectFile.GetSectionTable().GetSize();
		// counter for reading objectFile.content
		unsigned long readFrom = 0;

		// load each section
		for (size_t j = 0; j < sectionTableSize; j++)
		{
			SectionTableEntry& entry = *objectFile.GetSectionTable().GetEntryByID((SectionID)j);

			// continue if section in object files is marked that it should be loaded
			if (entry.flags & FLAG_NOT_LOADED)
				continue;

			// checking if linker caller specified where this section should be loaded
			if (location.find(entry.name) == location.end())
				throw LinkerException("Call linker with specifying loading address of '" + entry.name + "' section.", ErrorCodes::LINKER_SECTION_ADDRESS_UNSPECIFIED);
			else if (executable->sectionTable.GetEntryByName(entry.name))
			{
				if (executable->sectionTable.GetEntryByName(entry.name)->flags != entry.flags)
					throw LinkerException("Unconsistent flags detected while joining section named '" + entry.name + "'.", ErrorCodes::LINKER_UNCONSISTENT_FLAGS);
			}

			// getting address where section will be loaded
			uint16_t addressToWriteTo = location.find(entry.name)->second;

			if (addressToWriteTo >= MEMORY_MAPPED_REGISTERS_START)
				throw LinkerException("Section cannot be put into space reserved for memory mapped registers (0xFF00-0xFFFF).", ErrorCodes::LINKER_MEMORY_MAPPED_CONSTRAINT);
			else if ((entry.name != IVT_SECTION_NAME) && (addressToWriteTo >= IVT_START) && (addressToWriteTo < IVT_START + 2 * IVT_LENGTH))
				throw LinkerException("Section cannot be put into space reserved for interrupt vector table (0x0000 - 2 * IVT_LENGTH).", ErrorCodes::LINKER_IVT_CONSTRAINT);

			// add loadingAddress to offset of each symbol in current section symbol table
			for (SymbolTableID x = 0; x < objectFile.GetSymbolTable().GetSize(); x++)
			{
				SymbolTableEntry& symbol = *objectFile.GetSymbolTable().GetEntryByID((SymbolTableID)x);
				// only if current section is j
				if (symbol.sectionNumber == j &&
					symbol.scopeType != ScopeType::EXTERN &&
					symbol.tokenType != TokenType::SECTION)
				{
					symbol.offset += addressToWriteTo;

					if (executable->symbolTable.GetEntryByName(symbol.name))
						throw LinkerException("Multiple definition of symbol '" + symbol.name + "' in provided object files.", ErrorCodes::LINKER_MULTIPLE_SYMBOL_DEFINITION);
					
					executable->symbolTable.InsertSymbol(symbol);
				}
			}

			// add loadingAddress to offset of each relocation in current section relocation table
			for (RelocationID x = 0; x < objectFile.GetRelocationTable().GetSize(); x++)
			{
				RelocationTableEntry& relocation = *objectFile.GetRelocationTable().GetEntryByID((RelocationID)x);
				// only if current section is j
				if (relocation.sectionNo == j)
					relocation.offset += addressToWriteTo;
			}

			// check for section overlaping
			uint16_t newLength;
			map<string, uint16_t>::const_iterator it;
			if (!executable->sectionTable.GetEntryByName(entry.name))
				newLength = sectionStartMap.at(entry.name) + (uint16_t)entry.length;
			else
				newLength = (uint16_t)executable->sectionTable.GetEntryByName(entry.name)->length + (uint16_t)entry.length;
			for (it = sectionStartMap.begin(); it != sectionStartMap.end(); it++)
			{
				if ((it->second <= addressToWriteTo) && 
					(addressToWriteTo + newLength < location.find(it->first)->second) &&
					(it->first != entry.name))
					throw LinkerException("Cannot link section '" + entry.name + "' because it would overlap section '" + it->first + "'.", ErrorCodes::LINKER_SECTION_OVERLAPPING);
			}
			// copying from objectFile.content to executable.memory
			unsigned long k;	// added
			for (k = 0; k < entry.length; k++)
				executable->MemoryWrite(addressToWriteTo++, objectFile.ContentRead(readFrom++));

			// updating current section pointer for future merging
			location.find(entry.name)->second = addressToWriteTo;

			if (executable->sectionTable.GetEntryByName(entry.name))
				executable->sectionTable.GetEntryByName(entry.name)->length += k;
			else
			{
				SectionID sid = executable->sectionTable.InsertSection(entry.name, entry.length, entry.flags, 0);
				SymbolTableID no = executable->symbolTable.InsertSymbol(entry.name,
					sid,
					ASM_UNDEFINED,
					ASM_UNDEFINED,
					ScopeType::LOCAL,
					TokenType::SECTION);

				executable->sectionTable.GetEntryByID(sid)->symbolTableEntryNo = no;
			}
		}
	}
}

void Linker::ResolveRelocations()
{
	for (size_t i = 0; i < numberOfFiles; i++)
	{
		ObjectFile& objectFile = *objectFiles[i];
		RelocationTable& relocationTable = objectFile.GetRelocationTable();

		for (size_t j = 0; j < relocationTable.GetSize(); j++)
		{
			const RelocationTableEntry& entry = *relocationTable.GetEntryByID((RelocationID)j);

			if (executable->symbolTable.GetEntryByName(objectFile.GetSymbolTable().GetEntryByID(entry.symbolNo)->name))
			{
				SymbolTableEntry& symbol = *executable->symbolTable.GetEntryByName(objectFile.GetSymbolTable().GetEntryByID(entry.symbolNo)->name);

				switch (entry.relocationType)
				{
				case RelocationType::R_386_16:
				{
					uint16_t contentToWrite = (uint16_t)symbol.offset;

					executable->memory[entry.offset] = contentToWrite & 0xFF;
					executable->memory[entry.offset + 1] = ((contentToWrite >> 8) & 0xFF);
					break;
				}
				case RelocationType::R_386_PC16:
				{
					uint8_t lowByte = executable->MemoryRead((uint8_t)entry.offset);
					uint8_t highByte = executable->MemoryRead((uint8_t)(entry.offset + 1));
					int16_t contentToWrite = (highByte << 8) | lowByte;

					/* According to the ELF specification, the relocation value in R_386_PC32 mode
					is computed as S + A - P, where S is the symbol value, A is the value currently present
					at the address where the relocation takes place, and where P is the relocation address.
					Here, S - P the jumping offset from the point 0x401029 of relocation to the
					function entry point, and the addition of A=0xfcffffff causes a decrement by 4 bytes,
					accounting for the fact that the call instruction expects the offset measured from
					the next instruction. */
					contentToWrite = (int16_t)symbol.offset + contentToWrite - (int16_t)entry.offset;

					executable->memory[entry.offset] = contentToWrite & 0xFF;
					executable->memory[entry.offset + 1] = ((contentToWrite >> 8) & 0xFF);
					break;
				}
				default:
				{
					break;
				}
				}
			}
			else
				throw LinkerException("Symbol '" + objectFile.GetSymbolTable().GetEntryByID(entry.symbolNo)->name + "' is not defined in provided object files.", ErrorCodes::LINKER_SYMBOL_NOT_DEFINED);
		}
	}
}

void Linker::ResolveStartSymbol()
{
	// look for emulator entry point (START_SYMBOL)
	for (size_t i = 0; i < executable->symbolTable.GetSize(); i++)
	{
		if (executable->symbolTable.GetEntryByID((SymbolTableID)i)->name == START_SYMBOL)
		{
			executable->initialPCDefined = true;
			executable->initialPC = (uint16_t)executable->symbolTable.GetEntryByID((SymbolTableID)i)->offset;
			return;
		}
	}

	if (!executable->initialPCDefined)
		throw LinkerException("Linker cannot entry point symbol '_start' in provided files.", ErrorCodes::LINKER_NO_START);
}

Linker::~Linker()
{
	for (size_t i = 0; i < numberOfFiles; i++)
		delete objectFiles[i];
	delete[] objectFiles;
}

Executable* Linker::GetExecutable()
{
	MergeAndLoadExecutable();
	ResolveRelocations();
	ResolveStartSymbol();

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