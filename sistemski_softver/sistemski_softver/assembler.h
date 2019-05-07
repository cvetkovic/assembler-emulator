#ifndef ASSEMBLER_ASSEMBLER_H_
#define ASSEMBLER_ASSEMBLER_H_

#define COMMENT_START_SYMBOL '#'
#define TOKEN_DELIMITERS "\t\n, "
#define END_DIRECTIVE ".end"

#include "exceptions.h"
#include "structures.h"
#include "token.h"

#include <iostream>
#include <fstream>
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

	void StripeOffCommentsAndLoadLocally();
	void TokenizeCurrentLine(const string& line, vector<string>& collector);

	void FirstPass();

public:
	Assembler(string input_file_url, string output_file_url);
	void GenerateObjectFile();
};

#endif