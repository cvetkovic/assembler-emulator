#ifndef INSTRUCTION_ASSEMBLER_H_
#define INSTRUCTION_ASSEMBLER_H_

#include "token.h"

#include <fstream>
#include <iostream>
#include <map>
#include <queue>
using namespace std;

struct InstructionDetails
{
	uint8_t numberOfOperands;
	uint8_t opCode;

	InstructionDetails(uint8_t numberOfOperands, uint8_t opCode) :
		numberOfOperands(numberOfOperands), opCode(opCode) {}
};

static map<string, InstructionDetails> instructionOperandMap = {
		{"halt", InstructionDetails(0, 1)},
		{"ret", InstructionDetails(0, 24)},
		{"iret", InstructionDetails(0, 25)},

		{"int", InstructionDetails(1, 3)},
		{"not", InstructionDetails(1, 10)},
		{"push", InstructionDetails(1, 17)},
		{"pop", InstructionDetails(1, 18)},
		{"jmp", InstructionDetails(1, 19)},
		{"jeq", InstructionDetails(1, 20)},
		{"jne", InstructionDetails(1, 21)},
		{"jgt", InstructionDetails(1, 22)},
		{"call", InstructionDetails(1, 23)},

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
	unsigned char operationCode[7] = { 0,0,0,0,0,0,0 };
	unsigned instructionSize;

	enum OperandSize
	{
		BYTE = 0,
		WORD = 1
	};

public:
	Instruction(const Token& instruction, queue<Token>& params);
	~Instruction();

	unsigned GetInstructionSize() { return instructionSize; }
	void WriteCodeToOutput(ofstream& output);
};

#endif