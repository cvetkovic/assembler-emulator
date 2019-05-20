#ifndef _ARITHMETIC_ASSEMBLER_H
#define _ARITHMETIC_ASSEMBLER_H

#include "token.h"

#include <iostream>
#include <stack>
#include <string>
#include <vector>
using namespace std;

class ArithmeticParser
{
private: 
	static bool IsOperator(char c);
	static int GetPriority(char c, bool stack);
	static int GetRank(char c);
	static Token ReturnToken(string data);

public:
	static vector<Token> Parse(vector<Token> input);
	static vector<Token> TokenizeExpression(string expression);
	static unsigned long CalculateSymbolValue(vector<Token> tokens, SymbolTable& symbolTable, bool linker = false, unsigned long section = -1);
	static stringstream Serialize(map<string, vector<Token>> table);
};

#endif