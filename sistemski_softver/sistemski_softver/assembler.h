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

#define ASM_UNDEFINED -1

#include "exceptions.h"
#include "instruction.h"
#include "structures.h"
#include "token.h"

#include <cstdint>
#include <cmath>
#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
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

	vector<vector<string>> assemblyCode;
	
	SymbolTable symbolTable;
	SectionTable sectionTable;
	/* NOTE: in case that we want assembler without linker it is possible that all section
			 be one behind another, so to implement that it the locationCounter shouldn't be 
			 set to zero once section switch directive is encountered, but should just keep 
			 it's old value and sectionSizeMap should be extended that it keeps information
			 about section start position and size
	*/

	int currentBytesInline = 0;

	void StripeOffCommentsAndLoadLocally();
	void TokenizeCurrentLine(const string& line, vector<string>& collector);
	inline void WriteToOutput(uint8_t byte);
	inline void WriteToOutput(const Instruction& instruction);
	inline void WriteToOutput(string text);

	void FirstPass();
	void SecondPass();

public:
	Assembler(string input_file_url, string output_file_url);
	~Assembler();

	void GenerateObjectFile();
};

#endif