#ifndef ASSEMBLER_ASSEMBLER_H_
#define ASSEMBLER_ASSEMBLER_H_

#define COMMENT_START_SYMBOL '#'
#define TOKEN_DELIMITERS "\t\n, "

#define END_DIRECTIVE ".end"

#define ALIGN_DIRECTIVE ".align"
#define SKIP_DIRECTIVE ".skip"
#define CHAR_DIRECTIVE ".char"
#define WORD_DIRECTIVE ".word"
#define LONG_DIRECTIVE ".long"

#define PUBLIC_MODIFIER ".global"
#define EXTERN_MODIFIER ".extern"

#include "exceptions.h"
#include "structures.h"
#include "token.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <string>
#include <vector>
using namespace std;

class Assembler
{

private:
	// opened -> constructor; closed -> LoadFileLocally()
	ifstream input_file;
	// opened -> constructor; TODO: closed ->
	ofstream output_file;

	vector<vector<string>> assemblyCode;
	
	SymbolTable symbolTable;
	map<SectionType, unsigned long> sectionSizeMap;

	void StripeOffCommentsAndLoadLocally();
	void TokenizeCurrentLine(const string& line, vector<string>& collector);

	void FirstPass();
	void SecondPass();

public:
	Assembler(string input_file_url, string output_file_url);
	void GenerateObjectFile();
};

#endif