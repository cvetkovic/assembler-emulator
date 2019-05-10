#ifndef TOKEN_ASSEMBLER_H_
#define TOKEN_ASSEMBLER_H_

#define NUMBER_OF_PARSERS 13 + 1

#include <regex>
#include <string>
#include "enums.h"
#include "structures.h"

static const regex staticAssemblyParsers[NUMBER_OF_PARSERS] = {
	regex("^\\.(global|extern)$"),				// access modifier
	regex("^([a-zA-Z_][a-zA-Z0-9_]*_{0,}):$"),	// label (contains ':' on end; symbol is without ':')
	regex("^\\.(data|text|data|bss|section)$"),	// section
	regex("^\\.(align|char|long|skip|word)$"),	// directive
	regex("^(int|add|sub|mul|div|cmp|and|or|not|test|push|pop|call|iret|mov|shl|shr|ret|jmp)(eq|ne|gt|al)?$"),
	regex("^[a-zA-Z_][a-zA-Z0-9]*_{0,}$"),		// symbol
	regex("^.end$"),							// end of file
	regex("^[1-9][0-9]*$"),						// decimal operand
	regex(""),
	regex(""),
	regex(""),
	regex(""),
	regex(""),
	regex("")
};

class Token
{
private:
	TokenType tokenType;
	string value;
	
	Token(TokenType tokenType, string value) : tokenType(tokenType), value(value) {}

public:
	TokenType GetTokenType() const;
	string GetValue() const;

	static Token ParseToken(string data, unsigned long lineNumber);

	friend ostream& operator<<(ostream&& out, const Token& token);
};

#endif