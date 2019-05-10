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
	DATA,
	BSS,
	USER_SECTION
};

SectionType IntToSectionType(int t = 1);
SectionType StringToSectionType(string t);
string SectionToString(SectionType t);

enum TokenType : int;

struct SymbolTableEntry
{
	string name						// symbol name
	unsigned long sectionNumber;	// section identifier
	unsigned long value;			// symbol value
	unsigned long offset;			// offset from beginning of section
	ScopeType scopeType;			// scope type
	
	TokenType tokenType;			// type of token
	unsigned long size;				// symbol size
	unsigned long entryNo;			// needed to link symbol with relocation table

	SymbolTable(string name, unsigned long sectionNumber, unsigned long value, unsigned long offset, ScopeType scopeType, TokenType tokenType, unsigned long size = 0) :
		name(name), sectionNumber(sectionNumber), value(value), offset(offset), scopeType(scopeType), tokenType(tokenType), size(size) {}
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