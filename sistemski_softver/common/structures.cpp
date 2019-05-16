#include "structures.h"

SymbolTableID SymbolTable::InsertSymbol(string name, unsigned long sectionNumber, unsigned long value, unsigned long locationCounter, ScopeType scopeType, TokenType tokenType)
{
	map<SymbolTableID, SymbolTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
		if (it->second.name == name)
			throw AssemblerException("Symbol '" + name + "' already exists duplicated.", ErrorCodes::SYMBOL_EXISTS);	

	SymbolTableEntry entry(name, sectionNumber, value, locationCounter, scopeType, tokenType, counter);

	table.insert({ counter, entry });

	return counter++;
}

SymbolTableID SymbolTable::InsertSymbol(const SymbolTableEntry & e)
{
	SymbolTableEntry entry;

	entry.entryNo = e.entryNo;
	entry.name = e.name;
	entry.offset = e.offset;
	entry.scopeType = e.scopeType;
	entry.sectionNumber = e.sectionNumber;
	entry.tokenType = e.tokenType;
	entry.value = e.value;

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

SymbolTableEntry * SymbolTable::GetEntryByName(string name)
{
	map<SymbolTableID, SymbolTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
		if (it->second.name == name)
			return &it->second;

	return 0;
}

stringstream SymbolTable::GenerateTextualSymbolTable()
{
	stringstream output;

	output << left;
	output << setw(15) << "Name";
	output << setw(15) << "SectionNumber";
	output << setw(15) << "Value";
	output << setw(15) << "Offset";
	output << setw(15) << "ScopeType";
	output << setw(15) << "TokenType";
	output << setw(15) << "EntryNumber";
	output << endl;

	map<SymbolTableID, SymbolTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
	{
		output << left;
		output << setw(15) << it->second.name;
		if (it->second.sectionNumber != ASM_UNDEFINED)
			output << setw(15) << it->second.sectionNumber;
		else
			output << setw(15) << "N/A";
		if (it->second.value != ASM_UNDEFINED)
			output << setw(15) << it->second.value;
		else
			output << setw(15) << "N/A";
		if (it->second.offset != ASM_UNDEFINED)
			output << setw(15) << it->second.offset;
		else
			output << setw(15) << "N/A";
		output << setw(15) << it->second.scopeType;
		output << setw(15) << it->second.tokenType;
		output << setw(15) << it->second.entryNo;
		output << endl;
	}

	return output;
}

stringstream SymbolTable::Serialize()
{
	stringstream output;
	   
	char delimiter = ',';

	for (size_t i = 0; i < table.size(); i++)
	{
		SymbolTableEntry& entry = table.at((SymbolTableID)i);

		output.write(reinterpret_cast<char*>(&entry.entryNo), sizeof(entry.entryNo));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		size_t t = entry.name.size();
		output.write(reinterpret_cast<char*>(&t), sizeof(size_t));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		const char* c = entry.name.c_str();
		output.write(c, entry.name.size());
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		output.write(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		int a = entry.scopeType;
		output.write(reinterpret_cast<char*>(&a), sizeof(a));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		output.write(reinterpret_cast<char*>(&entry.sectionNumber), sizeof(entry.sectionNumber));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		int b = entry.tokenType;
		output.write(reinterpret_cast<char*>(&b), sizeof(b));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		output.write(reinterpret_cast<char*>(&entry.value), sizeof(entry.value));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));
	}

	return output;
}

SymbolTable SymbolTable::Deserialize(size_t numberOfElements, ifstream& input)
{
	SymbolTable result;

	size_t length = 0;
	char c;

	for (size_t i = 0; i < numberOfElements; i++)
	{
		SymbolTableEntry entry;

		input.read(reinterpret_cast<char*>(&entry.entryNo), sizeof(entry.entryNo));
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		input.read(reinterpret_cast<char*>(&length), sizeof(size_t));
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		if (length)
		{
			vector<char> tmp(length);
			input.read(tmp.data(), length);
			entry.name.assign(tmp.data(), length);
		}
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		input.read(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset));
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		int st;
		input.read(reinterpret_cast<char*>(&st), sizeof(st));
		entry.scopeType = static_cast<ScopeType>(st);
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		input.read(reinterpret_cast<char*>(&entry.sectionNumber), sizeof(entry.sectionNumber));
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		int tt;
		input.read(reinterpret_cast<char*>(&tt), sizeof(tt));
		entry.tokenType = static_cast<TokenType>(tt);
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		input.read(reinterpret_cast<char*>(&entry.value), sizeof(entry.value));
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		result.InsertSymbol(entry);
	}

	return result;
}

///////////////////////////////////////////////////////////
////////////////////// SECTION TABLE //////////////////////
///////////////////////////////////////////////////////////

SectionType StringToSectionType(string t = "text")
{
	if (t == ".start")
		return SectionType::ST_START;
	else if (t == ".text")
		return SectionType::ST_TEXT;
	else if (t == ".data")
		return SectionType::ST_DATA;
	else if (t == ".bss")
		return SectionType::ST_BSS;
	else
		return SectionType::ST_USER_SECTION;
}

string SectionTable::DefaultFlags(SectionType type)
{
	if (type == SectionType::ST_BSS)
		return "b";
	else if (type == SectionType::ST_DATA)
		return "wd";
	else if (type == SectionType::ST_TEXT)
		return "xr";
	else if (type == SectionType::ST_USER_SECTION)
		return "wx";
	else
		return "";
}

stringstream SectionTable::Serialize()
{
	stringstream output;

	char delimiter = ',';

	for (size_t i = 0; i < table.size(); i++)
	{
		SectionTableEntry& entry = table.at((SectionID)i);

		output.write(reinterpret_cast<char*>(&entry.entryNo), sizeof(entry.entryNo));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		output.write(reinterpret_cast<char*>(&entry.flags), sizeof(entry.flags));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		output.write(reinterpret_cast<char*>(&entry.length), sizeof(entry.length));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		size_t t = entry.name.size();
		output.write(reinterpret_cast<char*>(&t), sizeof(t));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		const char* tt = entry.name.c_str();
		output.write(tt, entry.name.size());
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		output.write(reinterpret_cast<char*>(&entry.symbolTableEntryNo), sizeof(entry.symbolTableEntryNo));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));
	}

	return output;
}

SectionTable SectionTable::Deserialize(size_t numberOfElements, ifstream & input)
{
	SectionTable result;

	size_t length = 0;
	char c;

	for (size_t i = 0; i < numberOfElements; i++)
	{
		SectionTableEntry entry;
		
		input.read(reinterpret_cast<char*>(&entry.entryNo), sizeof(entry.entryNo));
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		input.read(reinterpret_cast<char*>(&entry.flags), sizeof(entry.flags));
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		input.read(reinterpret_cast<char*>(&entry.length), sizeof(entry.length));
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		input.read(reinterpret_cast<char*>(&length), sizeof(length));
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		if (length)
		{
			vector<char> tmp(length);
			input.read(tmp.data(), length);
			entry.name.assign(tmp.data(), length);
		}
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		input.read(reinterpret_cast<char*>(&entry.symbolTableEntryNo), sizeof(entry.symbolTableEntryNo));
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		result.InsertSection(entry);
	}

	return result;
}

uint8_t SectionTable::ConvertStringFlagsToByte(string flags)
{
	uint8_t _flags = 0;

	for (int i = 0; i < flags.size(); i++)
	{
		char flag = flags[i];

		if (flag == 'b')
			_flags |= FLAG_BSS;
		else if (flag == 'n')
			_flags |= FLAG_NOT_LOADED;
		else if (flag == 'w')
			_flags |= FLAG_WRITABLE;
		else if (flag == 'd')
			_flags |= FLAG_DATA;
		else if (flag == 'r')
			_flags |= FLAG_READ_ONLY;
		else if (flag == 'x')
			_flags |= FLAG_EXECUTABLE;
	}

	return _flags;
}

SectionID SectionTable::InsertSection(string name, unsigned long length, string flags, unsigned long lineNumber)
{
	uint8_t _flags = ConvertStringFlagsToByte(flags);
	return InsertSection(name, length, _flags, lineNumber);
}

SectionID SectionTable::InsertSection(string name, unsigned long length, uint8_t _flags, unsigned long lineNumber)
{
	map<SectionID, SectionTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
		if (it->second.name == name)
			throw AssemblerException("Sections with the same name ('" + name + "') have been detected.", ErrorCodes::SECTION_ALREADY_DEFINED, lineNumber);

	if (((_flags & FLAG_BSS) && ((_flags & FLAG_DATA) || (_flags & FLAG_EXECUTABLE))) ||
		((_flags & FLAG_DATA) && ((_flags & FLAG_BSS) || (_flags & FLAG_EXECUTABLE))) ||
		((_flags & FLAG_EXECUTABLE) && ((_flags & FLAG_DATA) || (_flags & FLAG_BSS))) ||
		((_flags & FLAG_READ_ONLY) && (_flags & FLAG_WRITABLE)))
		throw AssemblerException("Invalid combination of flags while defining section.", ErrorCodes::INVALID_FLAGS, lineNumber);

	SectionTableEntry entry(name, length, counter, _flags);
	table.insert({ counter, entry });

	return counter++;;
}

SectionID SectionTable::InsertSection(const SectionTableEntry & e)
{
	SectionTableEntry entry;

	entry.entryNo = e.entryNo;
	entry.flags = e.flags;
	entry.length= e.length;
	entry.name = e.name;
	entry.symbolTableEntryNo = e.symbolTableEntryNo;

	table.insert({ counter, entry });

	return counter++;
}

SectionTableEntry* SectionTable::GetEntryByID(SectionID id)
{
	return &table.at(id);
}

SectionTableEntry * SectionTable::GetEntryByName(string name)
{
	map<SectionID, SectionTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
		if (it->second.name == name)
			return &it->second;

	return 0;
}

bool SectionTable::HasFlag(SectionID id, SectionPermissions permission)
{
	if (table.find(id) != table.end())
	{
		uint8_t flags = table.at(id).flags;

		return (flags & permission ? true : false);
	}
	else
		throw AssemblerException("Section not found in corresponding table.");
}

stringstream SectionTable::GenerateTextualSectionTable()
{
	stringstream output;

	output << left;
	output << setw(15) << "Name";
	output << setw(15) << "Length";
	output << setw(15) << "__bnwdrx";
	output << setw(15) << "EntryNumber";
	output << setw(15) << "SymbolTableEntryNumber";
	output << endl;

	map<SectionID, SectionTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
	{
		output << left;
		output << setw(15) << it->second.name;
		output << setw(15) << it->second.length;
		output << setw(15) << bitset<8>(it->second.flags);
		output << setw(15) << it->second.entryNo;
		output << setw(15) << it->second.symbolTableEntryNo;
		output << endl;
	}

	return output;
}

void RelocationTable::InsertRelocation(SectionID sectionNo, SymbolTableID symbolNo, unsigned long offset, RelocationType relocationType)
{
	RelocationTableEntry entry(sectionNo, symbolNo, offset, relocationType);
	table.push_back(entry);
}

void RelocationTable::InsertRelocation(const RelocationTableEntry & e)
{
	RelocationTableEntry entry;

	entry.offset = e.offset;
	entry.relocationType = e.relocationType;
	entry.sectionNo = e.sectionNo;
	entry.symbolNo = e.symbolNo;

	table.push_back(entry);
}

stringstream RelocationTable::GenerateTextualRelocationTable()
{
	stringstream output;

	output << left;
	output << setw(15) << "SectionNo";
	output << setw(15) << "SymbolNo";
	output << setw(15) << "Offset";
	output << setw(15) << "RelocationType";
	output << endl;

	vector<RelocationTableEntry>::iterator it;

	for (it = table.begin(); it != table.end(); it++)
	{
		output << left;
		output << setw(15) << it->sectionNo;
		output << setw(15) << it->symbolNo;
		output << setw(15) << it->offset;
		output << setw(15) << it->relocationType;
		output << endl;
	}

	return output;
}

stringstream RelocationTable::Serialize()
{
	stringstream output;

	char delimiter = ',';

	for (size_t i = 0; i < table.size(); i++)
	{
		RelocationTableEntry& entry = table.at(i);

		output.write(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		int t = entry.relocationType;
		output.write(reinterpret_cast<char*>(&t), sizeof(t));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		output.write(reinterpret_cast<char*>(&entry.sectionNo), sizeof(entry.sectionNo));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));

		output.write(reinterpret_cast<char*>(&entry.symbolNo), sizeof(entry.symbolNo));
		output.write(reinterpret_cast<char*>(&delimiter), sizeof(char));
	}

	return output;
}

RelocationTable RelocationTable::Deserialize(size_t numberOfElements, ifstream & input)
{
	RelocationTable result;

	char c;

	for (size_t i = 0; i < numberOfElements; i++)
	{
		RelocationTableEntry entry;
		
		input.read(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset));
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		int rt;
		input.read(reinterpret_cast<char*>(&rt), sizeof(rt));
		entry.relocationType = static_cast<RelocationType>(rt);
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		input.read(reinterpret_cast<char*>(&entry.sectionNo), sizeof(entry.sectionNo));
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		input.read(reinterpret_cast<char*>(&entry.symbolNo), sizeof(entry.symbolNo));
		input.read(reinterpret_cast<char*>(&c), sizeof(c));

		result.InsertRelocation(entry);
	}

	return result;
}
