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
	LOCAL,
	EXTERN
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

/////////////////////////////////////////////////////////
///////////////////// SYMBOL TABLE //////////////////////
/////////////////////////////////////////////////////////

typedef unsigned long SymbolTableID;

struct SymbolTableEntry
{
	string name;					// symbol name
	unsigned long sectionNumber;	// section identifier
	unsigned long value;			// symbol value
	unsigned long offset;			// offset from beginning of section
	ScopeType scopeType;			// scope type
	
	TokenType tokenType;			// type of token
	unsigned long size;				// symbol size
	SymbolTableID entryNo;			// needed to link symbol with relocation table

	SymbolTableEntry(string name, unsigned long sectionNumber, unsigned long value, unsigned long offset, ScopeType scopeType, TokenType tokenType, SymbolTableID entryNo, unsigned long size = 0) :
		name(name), sectionNumber(sectionNumber), value(value), offset(offset), scopeType(scopeType), tokenType(tokenType), size(size), entryNo(entryNo) {}
};

class SymbolTable
{

private:
	map<SymbolTableID, SymbolTableEntry> table;
	unsigned long counter = 0;

public:
	SymbolTableID InsertSymbol(string name, unsigned long sectionNumber, unsigned long value, unsigned long locationCounter, ScopeType scopeType, TokenType tokenType, unsigned long size);
	SymbolTableEntry* GetEntryByID(SymbolTableID id);
	SymbolTableEntry* GetEntryByName(string name);
};

/////////////////////////////////////////////////////////
///////////////////// SECTION TABLE /////////////////////
/////////////////////////////////////////////////////////

typedef unsigned long SectionID;

struct SectionTableEntry
{
	string name;
	unsigned long startAddress;
	unsigned long length;
	SectionID entryNo;
	SymbolTableID symbolTableEntryNo = -1;

	SectionTableEntry(string name, unsigned long startAddress, unsigned long length, SectionID entryNo) :
		name(name), startAddress(startAddress), length(length), entryNo(entryNo) {}
};

class SectionTable
{
private:
	map<SectionID, SectionTableEntry> table;
	SectionID counter = 0;

public:
	SectionID InsertSection(string name, unsigned long startAddress, unsigned long length);
	SectionTableEntry* GetEntryByID(SectionID id);
};

#endif