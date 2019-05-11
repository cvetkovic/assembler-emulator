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
	regex("^\\.(data|text|bss|section)$"),		// section
	regex("^\\.(align|byte|equ|skip|word)$"),	// directive
	regex("^(halt|ret|iret|(int|not|push|pop|jmp|jeq|jne|jgt|call|xchg|mov|add|sub|mul|div|cmp|and|or|xor|test|shl|shr)(b|w){0,1})$"),
	regex("^r[0-7](h|l){0,1}$"),				// register direct addressing
	regex("^.end$"),							// end of file
	regex("^[0-9]+$"),							// operand decimal
	regex("^0x[0-9a-fA-F]{1,}$"),				// operand hex
	regex("^[a-zA-Z_][a-zA-Z0-9_]*$"),			// symbol
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