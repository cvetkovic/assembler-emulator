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
	ScopeType scopeType;
	SectionType sectionType;
	TokenType tokenType;
	bool defined;
	unsigned long size = 0;
	unsigned long entryNo;	// needed to link symbol with relocation table

	SymbolTableEntry(string name, unsigned long offset, TokenType tokenType, ScopeType scopeType, SectionType sectionType, bool defined, unsigned long entryNo) :
		name(name), offset(offset), tokenType(tokenType), scopeType(scopeType), sectionType(sectionType), defined(defined), entryNo(entryNo) {}
};

class SymbolTable
{

private:
	map<string, SymbolTableEntry> table;
	unsigned long counter = 1;

public:
	void InsertSymbol(string label, unsigned long locationCounter, TokenType tokenType, ScopeType scopeType, SectionType currentSection, bool defined);
	SymbolTableEntry* GetEntry(string name);
};

#endif