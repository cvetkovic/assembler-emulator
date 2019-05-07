#ifndef STRUCTURES_PARSER_H_
#define STRUCTURES_PARSER_H_

#include "exceptions.h"
#include "token.h"

#include <map>
#include <string>
using namespace std;

enum ScopeType
{
	GLOBAL,
	LOCAL
};

enum SectionType
{
	START,
	TEXT,
	RODATA,
	BSS,
	SYMTAB
};

enum TokenType : int;

struct SymbolTableEntry
{
	string name;
	unsigned long offset;
	ScopeType scope;
	SectionType sectionType;
	TokenType tokenType;
	bool defined;
	unsigned long size;

	SymbolTableEntry(string name, unsigned long offset, TokenType type, ScopeType scope, SectionType sectionType, bool defined) :
		name(name), offset(offset), tokenType(tokenType), scope(scope), sectionType(sectionType), defined(defined) {}
};

class SymbolTable
{

private:
	map<string, SymbolTableEntry> table;

public:
	void InsertSymbol(string label, unsigned long locationCounter, TokenType tokenType, ScopeType scopeType, SectionType currentSection, bool defined);
	SymbolTableEntry* GetEntry(string name);
};

#endif