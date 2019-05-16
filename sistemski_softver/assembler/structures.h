#ifndef STRUCTURES_PARSER_H_
#define STRUCTURES_PARSER_H_

#include "enums.h"
#include "exceptions.h"
#include "token.h"

#include <bitset>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
using namespace std;

#define FLAG_BSS		0x20
#define FLAG_NOT_LOADED	0x10
#define FLAG_WRITABLE	0x08
#define FLAG_DATA		0x04
#define FLAG_READ_ONLY	0x02
#define FLAG_EXECUTABLE	0x01

enum ScopeType
{
	GLOBAL,
	LOCAL,
	EXTERN
};

// TODO: remove this because flags are sufficient to distinguish sections
enum SectionType
{
	ST_START = 0,
	ST_TEXT,
	ST_DATA,
	ST_BSS,
	ST_USER_SECTION
};

SectionType StringToSectionType(string t);

enum TokenType : int;

/////////////////////////////////////////////////////////
///////////////////// SYMBOL TABLE //////////////////////
/////////////////////////////////////////////////////////

typedef unsigned long SymbolTableID;
typedef unsigned long SectionID;
typedef unsigned long RelocationID;

enum SectionPermissions
{
	BSS = FLAG_BSS,
	NOT_LOADED = FLAG_NOT_LOADED,
	WRITABLE = FLAG_WRITABLE,
	DATA = FLAG_DATA,
	READ_ONLY = FLAG_READ_ONLY,
	EXECUTABLE = FLAG_EXECUTABLE
};

struct SymbolTableEntry
{
	string name;					// symbol name
	SectionID sectionNumber;		// section identifier
	unsigned long value;			// symbol value
	unsigned long offset;			// offset from beginning of section
	ScopeType scopeType;			// scope type
	
	TokenType tokenType;			// type of token
	SymbolTableID entryNo;			// needed to link symbol with relocation table

	SymbolTableEntry(string name, unsigned long sectionNumber, unsigned long value, unsigned long offset, ScopeType scopeType, TokenType tokenType, SymbolTableID entryNo) :
		name(name), sectionNumber(sectionNumber), value(value), offset(offset), scopeType(scopeType), tokenType(tokenType), entryNo(entryNo) {}
};

class SymbolTable
{

private:
	map<SymbolTableID, SymbolTableEntry> table;
	unsigned long counter = 0;

public:
	SymbolTableID InsertSymbol(string name, unsigned long sectionNumber, unsigned long value, unsigned long locationCounter, ScopeType scopeType, TokenType tokenType);
	SymbolTableEntry* GetEntryByID(SymbolTableID id);
	SymbolTableEntry* GetEntryByName(string name);

	stringstream GenerateTextualSymbolTable();
	unsigned long GetSize() { return table.size(); }
};

/////////////////////////////////////////////////////////
///////////////////// SECTION TABLE /////////////////////
/////////////////////////////////////////////////////////

struct SectionTableEntry
{
	string name;
	unsigned long length;
	SectionID entryNo;
	SymbolTableID symbolTableEntryNo = -1;
	uint8_t flags;

	SectionTableEntry(string name, unsigned long length, SectionID entryNo, uint8_t flags) :
		name(name), length(length), entryNo(entryNo), flags(flags) {}
};

class SectionTable
{
private:
	map<SectionID, SectionTableEntry> table;
	SectionID counter = 0;

	uint8_t ConvertStringFlagsToByte(string flags);

public:
	SectionID InsertSection(string name, unsigned long length, string flags, unsigned long lineNumber);
	SectionTableEntry* GetEntryByID(SectionID id);

	bool HasFlag(SectionID id, SectionPermissions permission);

	stringstream GenerateTextualSectionTable();

	static string DefaultFlags(SectionType type);
	unsigned long GetSize() { return table.size(); }
};

////////////////////////////////////////////////////////////
///////////////////// RELOCATION TABLE /////////////////////
////////////////////////////////////////////////////////////

enum RelocationType
{
	R_386_8 = 0,
	R_386_16,
	R_386_PC16
};

struct RelocationTableEntry
{
	SectionID sectionNo;
	SymbolTableID symbolNo;
	unsigned long offset;
	RelocationType relocationType;

	RelocationTableEntry(SectionID sectionNo, SymbolTableID symbolNo, unsigned long offset, RelocationType relocationType) :
		sectionNo(sectionNo), symbolNo(symbolNo), offset(offset), relocationType(relocationType) {}
};

class RelocationTable
{
private:
	vector<RelocationTableEntry> table;
	SectionID counter = 0;

public:
	void InsertRelocation(SectionID sectionNo, SymbolTableID symbolNo, unsigned long offset, RelocationType relocationType);
	
	RelocationTableEntry* GetEntryByID(int id) { return &table.at(id); }
	stringstream GenerateTextualRelocationTable();
	unsigned long GetSize() { return table.size(); }
};

#endif