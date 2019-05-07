#include "structures.h"

void SymbolTable::InsertSymbol(string label, unsigned long locationCounter, TokenType tokenType, ScopeType scopeType, SectionType currentSection, bool defined)
{
	auto searchForKey = table.find(label);
	if (searchForKey != table.end())
		throw AssemblerException("Symbol '" + label + "' with the same name already exists.", ErrorCodes::SYMBOL_EXISTS);

	SymbolTableEntry entry(label, locationCounter, tokenType, scopeType, currentSection, defined);

	table.insert({ label, entry });
}

SymbolTableEntry* SymbolTable::GetEntry(string name)
{
	if (table.find(name) != table.end())
		return &table.at(name);
	else
		return NULL;
}
