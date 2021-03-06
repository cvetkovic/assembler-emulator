#ifndef STRUCTURES_PARSER_H_
#define STRUCTURES_PARSER_H_

#include "enums.h"
#include "exceptions.h"
#include "token.h"

#include <bitset>
#include <iomanip>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#define FLAG_BSS		0x20
#define FLAG_NOT_LOADED	0x10
#define FLAG_WRITABLE	0x08
#define FLAG_DATA		0x04
#define FLAG_READ_ONLY	0x02
#define FLAG_EXECUTABLE	0x01

enum ScopeType : int
{
	GLOBAL = 0,
	LOCAL,
	EXTERN
};

enum SectionType : int
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

	SymbolTableEntry() {}
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
	SymbolTableID InsertSymbol(const SymbolTableEntry& e);
	SymbolTableEntry* GetEntryByID(SymbolTableID id);
	SymbolTableEntry* GetEntryByName(string name);

	stringstream GenerateTextualSymbolTable();
	size_t GetSize() { return table.size(); }

	void DeleteSymbol(const SymbolTableID& id) { table.erase(id); }

	stringstream Serialize();
	static SymbolTable Deserialize(size_t numberOfElements, ifstream& input);
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

	SectionTableEntry() {}
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
	SectionID InsertSection(string name, unsigned long length, uint8_t flags, unsigned long lineNumber);
	SectionID InsertSection(const SectionTableEntry& e);
	SectionTableEntry* GetEntryByID(SectionID id);
	SectionTableEntry* GetEntryByName(string name);

	bool HasFlag(SectionID id, SectionPermissions permission);

	stringstream GenerateTextualSectionTable();

	static string DefaultFlags(SectionType type);
	size_t GetSize() { return table.size(); }

	stringstream Serialize();
	static SectionTable Deserialize(size_t numberOfElements, ifstream& input);
};

////////////////////////////////////////////////////////////
///////////////////// RELOCATION TABLE /////////////////////
////////////////////////////////////////////////////////////

enum RelocationType : int
{
	R_386_16 = 0,
	R_386_PC16
};

struct RelocationTableEntry
{
	SectionID sectionNo;
	SymbolTableID symbolNo;
	unsigned long offset;
	RelocationType relocationType;

	RelocationTableEntry() {}
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
	void InsertRelocation(const RelocationTableEntry& e);

	RelocationTableEntry* GetEntryByID(int id) { return &table.at(id); }
	stringstream GenerateTextualRelocationTable();
	size_t GetSize() { return table.size(); }
	
	stringstream Serialize();
	static RelocationTable Deserialize(size_t numberOfElements, ifstream& input);
};

struct InstructionDetails
{
	uint8_t numberOfOperands;
	uint8_t opCode;
	//bool jumpInstruction;

	InstructionDetails(uint8_t numberOfOperands, uint8_t opCode) :
		numberOfOperands(numberOfOperands), opCode(opCode) {}
};

struct TNSEntry
{
	string name;
	SectionID sectionNumber;
	string expression;
	ScopeType scope;

	TNSEntry() {}
	TNSEntry(string name, SectionID sectionNumber, string expression, ScopeType scopeType) :
		name(name), sectionNumber(sectionNumber), expression(expression), scope(scopeType) {}
};

class TNSTable
{
private:
	vector<TNSEntry> table;

public:
	void InsertISymbol(string name, SectionID section, string expression, ScopeType scope);
	void InsertISymbol(const TNSEntry& e);

	TNSEntry* GetEntryByID(unsigned id);
	TNSEntry* GetEntryByName(string name);
	void DeleteEntryByName(string name);
	stringstream GenerateTextualTNSTable();
	size_t GetSize() { return table.size(); }

	stringstream Serialize();
	static TNSTable Deserialize(size_t numberOfElements, ifstream& input);
};

#endif