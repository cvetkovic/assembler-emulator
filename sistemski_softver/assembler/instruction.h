#ifndef INSTRUCTION_ASSEMBLER_H_
#define INSTRUCTION_ASSEMBLER_H_

#include "../common/token.h"

#include <fstream>
#include <iostream>
#include <map>
#include <queue>
using namespace std;

static map<string, InstructionDetails> instructionOperandMap = {
		{"halt", InstructionDetails(0, 1)},
		{"ret", InstructionDetails(0, 24, true)},
		{"iret", InstructionDetails(0, 25, true)},

		{"int", InstructionDetails(1, 3, true)},
		{"not", InstructionDetails(1, 10)},
		{"push", InstructionDetails(1, 17)},
		{"pop", InstructionDetails(1, 18)},
		{"jmp", InstructionDetails(1, 19)},			// absolute jumps -> false because of R_386_16
		{"jeq", InstructionDetails(1, 20, true)},
		{"jne", InstructionDetails(1, 21, true)},
		{"jgt", InstructionDetails(1, 22, true)},
		{"call", InstructionDetails(1, 23)},		// absolute jumps -> false because of R_386_16

		{"xchg", InstructionDetails(2, 2)},
		{"mov", InstructionDetails(2, 4)},
		{"add", InstructionDetails(2, 5)},
		{"sub", InstructionDetails(2, 6)},
		{"mul", InstructionDetails(2, 7)},
		{"div", InstructionDetails(2, 8)},
		{"cmp", InstructionDetails(2, 9)},
		{"and", InstructionDetails(2, 11)},
		{"or", InstructionDetails(2, 12)},
		{"xor", InstructionDetails(2, 13)},
		{"test", InstructionDetails(2, 14)},
		{"shl", InstructionDetails(2, 15)},
		{"shr", InstructionDetails(2, 16)}
};

class Instruction
{
private:
	uint8_t operationCode[7] = { 0,0,0,0,0,0,0 };
	uint8_t instructionSize = 0;
	
	unsigned long GenerateRelocation(string instructionMnemonic, const SymbolTableEntry& entry, unsigned long locationCounter, unsigned long writeToPosition, int instructionSize, SectionID currentSection, SymbolTable& symbolTable, RelocationTable& relocationTable);

public:
	Instruction(const Token& instruction, queue<Token> params, unsigned long lineNumber, unsigned long locationCounter, SectionID currentSection, SymbolTable& symbolTable, RelocationTable& relocationTable);

	static int GetInstructionSize(const Token& instruction, queue<Token> params, unsigned long lineNumber);
	friend class Assembler;
};

#endif