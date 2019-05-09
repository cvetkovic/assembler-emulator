#include "structures.h"

void SymbolTable::InsertSymbol(string label, unsigned long locationCounter, TokenType tokenType, ScopeType scopeType, SectionType currentSection, bool defined)
{
	auto searchForKey = table.find(label);
	if (searchForKey != table.end())
		throw AssemblerException("Symbol '" + label + "' with the same name already exists.", ErrorCodes::SYMBOL_EXISTS);

	SymbolTableEntry entry(label, locationCounter, tokenType, scopeType, currentSection, defined, counter++);

	table.insert({ label, entry });
}

SymbolTableEntry* SymbolTable::GetEntry(string name)
{
	if (table.find(name) != table.end())
		return &table.at(name);
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
		return SectionType::RODATA;
	case 3:
		return SectionType::BSS;
	default:
		return SectionType::TEXT;
	}
}

SectionType StringToSectionType(string t = "text")
{
	if (t == ".start")
		return SectionType::START;
	else if (t == ".text")
		return SectionType::TEXT;
	else if (t == ".rodata")
		return SectionType::RODATA;
	else if (t == ".bss")
		return SectionType::BSS;
	else
		return SectionType::TEXT;
}

string SectionToString(SectionType t)
{
	switch (t)
	{
	case SectionType::START:
		return ".start";
	case SectionType::TEXT:
		return ".text";
	case SectionType::RODATA:
		return ".rodata";
	case SectionType::BSS:
		return ".bss";
	default:
		return ".text";
	}
}
