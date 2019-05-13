#include "structures.h"

SymbolTableID SymbolTable::InsertSymbol(string name, unsigned long sectionNumber, unsigned long value, unsigned long locationCounter, ScopeType scopeType, TokenType tokenType, unsigned long size)
{
	if (tokenType != TokenType::SECTION)
	{
		map<SymbolTableID, SymbolTableEntry>::iterator it;

		for (it = table.begin(); it != table.end(); it++)
			if (it->second.name == name)
				throw AssemblerException("Symbol '" + name + "' already is duplicated.", ErrorCodes::SYMBOL_EXISTS);
	}		

	SymbolTableEntry entry(name, sectionNumber, value, locationCounter, scopeType, tokenType, counter, size);

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
	output << setw(15) << "Size";
	output << setw(15) << "EntryNumber";
	output << endl;

	map<SymbolTableID, SymbolTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
	{
		output << left;
		output << setw(15) << it->second.name;
		output << setw(15) << it->second.sectionNumber;
		output << setw(15) << it->second.value;
		output << setw(15) << it->second.offset;
		output << setw(15) << it->second.scopeType;
		output << setw(15) << it->second.tokenType;
		output << setw(15) << it->second.size;
		output << setw(15) << it->second.entryNo;
		output << endl;
	}

	return output;
}

///////////////////////////////////////////////////////////
////////////////////// SECTION TABLE //////////////////////
///////////////////////////////////////////////////////////

SectionType IntToSectionType(int t)
{
	switch (t)
	{
	case 0:
		return SectionType::START;
	case 1:
		return SectionType::TEXT;
	case 2:
		return SectionType::DATA;
	case 3:
		return SectionType::BSS;
	default:
		return SectionType::USER_SECTION;
	}
}

SectionType StringToSectionType(string t = "text")
{
	if (t == ".start")
		return SectionType::START;
	else if (t == ".text")
		return SectionType::TEXT;
	else if (t == ".data")
		return SectionType::DATA;
	else if (t == ".bss")
		return SectionType::BSS;
	else
		return SectionType::USER_SECTION;
}

string SectionToString(SectionType t)
{
	switch (t)
	{
	case SectionType::START:
		return ".start";
	case SectionType::TEXT:
		return ".text";
	case SectionType::DATA:
		return ".data";
	case SectionType::BSS:
		return ".bss";
	default:
		return "user_section";
	}
}

SectionID SectionTable::InsertSection(string name, unsigned long startAddress, unsigned long length)
{
	map<SectionID, SectionTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
		if (it->second.name == name)
			cout << "Warning: Sections with the same name ('" << name << "') have been detected. They will be assembled to independent and different sections." << endl;

	SectionTableEntry entry(name, startAddress, length, counter);
	table.insert({ counter, entry });

	return counter++;;
}

SectionTableEntry* SectionTable::GetEntryByID(SectionID id)
{
	return &table.at(id);
}

stringstream SectionTable::GenerateTextualSectionTable()
{
	stringstream output;

	output << left;
	output << setw(15) << "Name";
	output << setw(15) << "StartAddress";
	output << setw(15) << "Length";
	output << setw(15) << "EntryNumber";
	output << setw(15) << "SymbolTableEntryNumber";
	output << endl;

	map<SectionID, SectionTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
	{
		output << left;
		output << setw(15) << it->second.name;
		output << setw(15) << it->second.startAddress;
		output << setw(15) << it->second.length;
		output << setw(15) << it->second.entryNo;
		output << setw(15) << it->second.symbolTableEntryNo;
		output << endl;
	}

	return output;
}