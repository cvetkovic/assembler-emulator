#include "structures.h"

SymbolTableID SymbolTable::InsertSymbol(string name, unsigned long sectionNumber, unsigned long value, unsigned long locationCounter, ScopeType scopeType, TokenType tokenType)
{
	map<SymbolTableID, SymbolTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
		if (it->second.name == name)
			throw AssemblerException("Symbol '" + name + "' already exists duplicated.", ErrorCodes::SYMBOL_EXISTS);	

	SymbolTableEntry entry(name, sectionNumber, value, locationCounter, scopeType, tokenType, counter);

	table.insert({ counter, entry });

	return counter++;
}

SymbolTableEntry* SymbolTable::GetEntryByID(SymbolTableID id)
{
	if (table.find(id) != table.end())
		return &table.at(id);
	else
		return NULL;
}

SymbolTableEntry * SymbolTable::GetEntryByName(string name)
{
	map<SymbolTableID, SymbolTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
		if (it->second.name == name)
			return &it->second;

	return 0;
}

stringstream SymbolTable::GenerateTextualSymbolTable()
{
	stringstream output;

	output << left;
	output << setw(15) << "Name";
	output << setw(15) << "SectionNumber";
	output << setw(15) << "Value";
	output << setw(15) << "Offset";
	output << setw(15) << "ScopeType";
	output << setw(15) << "TokenType";
	output << setw(15) << "EntryNumber";
	output << endl;

	map<SymbolTableID, SymbolTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
	{
		output << left;
		output << setw(15) << it->second.name;
		if (it->second.sectionNumber != ASM_UNDEFINED)
			output << setw(15) << it->second.sectionNumber;
		else
			output << setw(15) << "N/A";
		if (it->second.value != ASM_UNDEFINED)
			output << setw(15) << it->second.value;
		else
			output << setw(15) << "N/A";
		if (it->second.offset != ASM_UNDEFINED)
			output << setw(15) << it->second.offset;
		else
			output << setw(15) << "N/A";
		output << setw(15) << it->second.scopeType;
		output << setw(15) << it->second.tokenType;
		output << setw(15) << it->second.entryNo;
		output << endl;
	}

	return output;
}

///////////////////////////////////////////////////////////
////////////////////// SECTION TABLE //////////////////////
///////////////////////////////////////////////////////////

SectionType StringToSectionType(string t = "text")
{
	if (t == ".start")
		return SectionType::ST_START;
	else if (t == ".text")
		return SectionType::ST_TEXT;
	else if (t == ".data")
		return SectionType::ST_DATA;
	else if (t == ".bss")
		return SectionType::ST_BSS;
	else
		return SectionType::ST_USER_SECTION;
}

string SectionTable::DefaultFlags(SectionType type)
{
	if (type == SectionType::ST_BSS)
		return "b";
	else if (type == SectionType::ST_DATA)
		return "wd";
	else if (type == SectionType::ST_TEXT)
		return "xr";
	else if (type == SectionType::ST_USER_SECTION)
		return "wx";
	else
		return "";
}

uint8_t SectionTable::ConvertStringFlagsToByte(string flags)
{
	uint8_t _flags = 0;

	for (int i = 0; i < flags.size(); i++)
	{
		char flag = flags[i];

		if (flag == 'b')
			_flags |= FLAG_BSS;
		else if (flag == 'n')
			_flags |= FLAG_NOT_LOADED;
		else if (flag == 'w')
			_flags |= FLAG_WRITABLE;
		else if (flag == 'd')
			_flags |= FLAG_DATA;
		else if (flag == 'r')
			_flags |= FLAG_READ_ONLY;
		else if (flag == 'x')
			_flags |= FLAG_EXECUTABLE;
	}

	return _flags;
}

SectionID SectionTable::InsertSection(string name, unsigned long length, string flags, unsigned long lineNumber)
{
	map<SectionID, SectionTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
		if (it->second.name == name)
			throw AssemblerException("Sections with the same name ('" + name + "') have been detected.", ErrorCodes::SECTION_ALREADY_DEFINED, lineNumber);

	uint8_t _flags = ConvertStringFlagsToByte(flags);

	if (((_flags & FLAG_BSS) && ((_flags & FLAG_DATA) || (_flags & FLAG_EXECUTABLE))) ||
		((_flags & FLAG_DATA) && ((_flags & FLAG_BSS) || (_flags & FLAG_EXECUTABLE))) ||
		((_flags & FLAG_EXECUTABLE) && ((_flags & FLAG_DATA) || (_flags & FLAG_BSS))) ||
		((_flags & FLAG_READ_ONLY) && (_flags & FLAG_WRITABLE)))
		throw AssemblerException("Invalid combination of flags while defining section.", ErrorCodes::INVALID_FLAGS, lineNumber);

	SectionTableEntry entry(name, length, counter, _flags);
	table.insert({ counter, entry });

	return counter++;;
}

SectionTableEntry* SectionTable::GetEntryByID(SectionID id)
{
	return &table.at(id);
}

bool SectionTable::HasFlag(SectionID id, SectionPermissions permission)
{
	if (table.find(id) != table.end())
	{
		uint8_t flags = table.at(id).flags;

		return (flags & permission ? true : false);
	}
	else
		throw AssemblerException("Section not found in corresponding table.");
}

stringstream SectionTable::GenerateTextualSectionTable()
{
	stringstream output;

	output << left;
	output << setw(15) << "Name";
	output << setw(15) << "Length";
	output << setw(15) << "__bnwdrx";
	output << setw(15) << "EntryNumber";
	output << setw(15) << "SymbolTableEntryNumber";
	output << endl;

	map<SectionID, SectionTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
	{
		output << left;
		output << setw(15) << it->second.name;
		output << setw(15) << it->second.length;
		output << setw(15) << bitset<8>(it->second.flags);
		output << setw(15) << it->second.entryNo;
		output << setw(15) << it->second.symbolTableEntryNo;
		output << endl;
	}

	return output;
}

void RelocationTable::InsertRelocation(SectionID sectionNo, SymbolTableID symbolNo, unsigned long offset, RelocationType relocationType)
{
	RelocationTableEntry entry(sectionNo, symbolNo, offset, relocationType);
	table.push_back(entry);
}

stringstream RelocationTable::GenerateTextualRelocationTable()
{
	stringstream output;

	output << left;
	output << setw(15) << "SectionNo";
	output << setw(15) << "SymbolNo";
	output << setw(15) << "Offset";
	output << setw(15) << "RelocationType";
	output << endl;

	vector<RelocationTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
	{
		output << left;
		output << setw(15) << it->sectionNo;
		output << setw(15) << it->symbolNo;
		output << setw(15) << it->offset;
		output << setw(15) << it->relocationType;
		output << endl;
	}

	return output;
}