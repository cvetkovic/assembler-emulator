#ifndef ASSEMBLER_ASSEMBLER_H_
#define ASSEMBLER_ASSEMBLER_H_

#define COMMENT_START_SYMBOL '#'
#define TOKEN_DELIMITERS "\t\n, "

#define END_DIRECTIVE ".end"

#define ALIGN_DIRECTIVE ".align"
#define SKIP_DIRECTIVE ".skip"
#define BYTE_DIRECTIVE ".byte"
#define WORD_DIRECTIVE ".word"
#define EQUIVALENCE_DIRECTIVE ".equ"

#define PUBLIC_MODIFIER ".global"
#define EXTERN_MODIFIER ".extern"

#define START_SECTION -1

#include "../common/exceptions.h"
#include "instruction.h"
#include "../common/structures.h"
#include "../common/token.h"
#include "../common/arithmetic.h"

#include <cstdint>
#include <cmath>
#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
#include <string.h>
using namespace std;

#define BYTES_INLINE 16

class Assembler
{

private:
	// opened -> constructor; closed -> LoadFileLocally()
	ifstream input_file;
	// opened -> constructor; TODO: closed -> destructor
	ofstream output_file;
	ofstream txt_output_file;
	stringstream binaryBuffer;

	vector<vector<string>> assemblyCode;
	
	SymbolTable symbolTable;
	SectionTable sectionTable;
	RelocationTable relocationTable;
	/* NOTE: in case that we want assembler without linker it is possible that all section
			 be one behind another, so to implement that it the locationCounter shouldn't be 
			 set to zero once section switch directive is encountered, but should just keep 
			 it's old value and sectionSizeMap should be extended that it keeps information
			 about section start position and size
	*/

	int currentBytesInline = 0;
	size_t contentLength = 0;

	TNSTable tns;

	void StripeOffCommentsAndLoadLocally();
	void TokenizeCurrentLine(const string& line, vector<string>& collector);
	inline void WriteToOutput(uint8_t byte);
	inline void WriteToOutput(const Instruction& instruction);
	inline void WriteToOutput(string text);

	void FirstPass();
	void ResolveIncalculatableSymbols();
	void SecondPass();

	void WriteBinaryFile();

public:
	Assembler(string input_file_url, string output_file_url);
	~Assembler();

	void GenerateObjectFile();
};

#endif