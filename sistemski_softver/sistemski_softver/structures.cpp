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