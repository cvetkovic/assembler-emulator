#ifndef STRUCTURES_PARSER_H_
#define STRUCTURES_PARSER_H_

#include "enums.h"
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
	START = 0,
	TEXT,
	RODATA,
	BSS
};

SectionType IntToSectionType(int t = 1);
SectionType StringToSectionType(string t);
string SectionToString(SectionType t);

enum TokenType : int;

struct SymbolTableEntry
{
	string name;
	unsigned long offset;
	ScopeType scope;
	SectionType sectionType;
	TokenType tokenType;
	bool defined;
	unsigned long size = 0;

	SymbolTableEntry(string name, unsigned long offset, TokenType tokenType, ScopeType scope, SectionType sectionType, bool defined) :
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