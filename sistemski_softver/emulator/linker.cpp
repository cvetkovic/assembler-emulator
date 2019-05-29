#include "linker.h"

ObjectFile * Linker::GetObjectFile(const SymbolTableEntry & symbol)
{
	for (size_t i = 0; i < numberOfFiles; i++)
	{
		ObjectFile& o = *objectFiles[i];
		SymbolTableEntry* e = o.GetSymbolTable().GetEntryByName(symbol.name);
		if (e)
		{
			return &o;
		}
		else
			continue;

	}

	return 0;
}

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
		totalTNSCount += objectFile.GetTNSTable().GetSize();

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
					if (symbol.tokenType == TokenType::DIRECTIVE &&
						!objectFile.GetTNSTable().GetEntryByName(symbol.name))
					{
						// nothing here 
						// all symbol tokens were in the same section so no need for relocating
					}
					else
						symbol.offset += addressToWriteTo;

					//symbol.tokenType = TokenType::SYMBOL; // TNS directive to symbol

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
				if (!objectFile.GetSectionTable().HasFlag((SectionID)j, SectionPermissions::BSS))
					executable->MemoryWrite(addressToWriteTo++, objectFile.ContentRead(readFrom++));
				else
					// do not read from object file because it doesn't contain data for .bss section
					executable->MemoryWrite(addressToWriteTo++, 0);

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

				if (symbol.sectionNumber != entry.sectionNo &&
					(&objectFile != GetObjectFile(symbol)) &&
					symbol.scopeType == ScopeType::LOCAL)
					throw LinkerException("Symbol '" + symbol.name + "' is not marked not marked as global or extern.", ErrorCodes::LINKER_INVALID_RELOCATION_SCOPE);

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
					uint8_t lowByte = executable->MemoryRead((uint16_t)entry.offset);
					uint8_t highByte = executable->MemoryRead((uint16_t)(entry.offset + 1));
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

void Linker::DeleteLocalSymbols()
{
	vector<SymbolTableID> v;

	for (size_t i = 0; i < executable->symbolTable.GetSize(); i++)
		if (executable->symbolTable.GetEntryByID((SymbolTableID)i)->scopeType == ScopeType::LOCAL)
			v.push_back((SymbolTableID)i);

	for (size_t i = 0; i < v.size(); i++)
		executable->symbolTable.DeleteSymbol(v.at((SymbolTableID)i));
}

void Linker::CheckForNotProvidedFiles()
{
	LinkerSections::const_iterator it;
	for (it = sectionStartMap.begin(); it != sectionStartMap.end(); it++)
	{
		if (!executable->sectionTable.GetEntryByName(it->first))
			throw LinkerException("Section '" + it->first + "' is missing from provided object files to be linked.", ErrorCodes::LINKER_SECTION_MISSING);
	}
}

unsigned long Linker::TNSSumSize()
{
	unsigned long size = 0;

	for (size_t obj = 0; obj < numberOfFiles; obj++)
	{
		TNSTable& tns = objectFiles[obj]->GetTNSTable();
		size += tns.GetSize();
	}

	return size;
}

Linker::~Linker()
{
	for (size_t i = 0; i < numberOfFiles; i++)
		delete objectFiles[i];
	delete[] objectFiles;
}

void Linker::ResolveTNS()
{
	bool end = false;

	while (!end)
	{
		end = true;
		int oldTNSSumSize = TNSSumSize();
		for (size_t obj = 0; obj < numberOfFiles; obj++)
		{
			TNSTable& tns = objectFiles[obj]->GetTNSTable();
			SymbolTable& symbolTable = executable->symbolTable;

			for (size_t i = 0; i < tns.GetSize(); i++)
			{
				try
				{
					TNSEntry& entry = *tns.GetEntryByID((unsigned)i);
					vector<Token> arithmeticTokens = ArithmeticParser::Parse(ArithmeticParser::TokenizeExpression(entry.expression));

					symbolTable.GetEntryByName(entry.name)->offset = ArithmeticParser::CalculateSymbolValue(arithmeticTokens, symbolTable, true);
					symbolTable.GetEntryByName(entry.name)->tokenType = TokenType::SYMBOL;

					// symbol is added, so table has changed
					tns.DeleteEntryByName(entry.name);
					totalTNSCount--;
					end = false;
					break; // break for because vector has change and iterator would throw exception
				}
				catch (const AssemblerException& ex)
				{
					throw ex;
				}
				catch (const exception& ex)
				{
					ex.what();
					// error because symbol is missing
				}
			}
		}

		if (TNSSumSize() == oldTNSSumSize && oldTNSSumSize != 0)			
			throw LinkerException("Some of .equ directive symbols are incalculatable.", ErrorCodes::LINKER_TNS_INCALCULATABLE);
	}

	if (totalTNSCount != 0)
		throw LinkerException("Linker couldn't not have calculated value of symbol generated by assembler '.equ' directive due to some symbols missing.", ErrorCodes::LINKER_EQU_RESOLVE_ERROR);
}

Executable* Linker::GetExecutable()
{
	MergeAndLoadExecutable();
	ResolveTNS();
	ResolveRelocations();
	ResolveStartSymbol();
	DeleteLocalSymbols();
	CheckForNotProvidedFiles();

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
	size_t tnsTableSize = 0;

	inputFile.read(reinterpret_cast<char*>(&symbolTableSize), sizeof(size_t));
	inputFile.read(reinterpret_cast<char*>(&sectionTableSize), sizeof(size_t));
	inputFile.read(reinterpret_cast<char*>(&relocationTableSize), sizeof(size_t));
	inputFile.read(reinterpret_cast<char*>(&tnsTableSize), sizeof(size_t));
	inputFile.read(reinterpret_cast<char*>(&contentSize), sizeof(size_t));

	symbolTable = SymbolTable::Deserialize(symbolTableSize, inputFile);
	sectionTable = SectionTable::Deserialize(sectionTableSize, inputFile);
	relocationTable = RelocationTable::Deserialize(relocationTableSize, inputFile);
	tns = TNSTable::Deserialize(tnsTableSize, inputFile);

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