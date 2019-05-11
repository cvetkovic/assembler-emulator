#include "instruction.h"

Instruction::Instruction(const Token & instruction, queue<Token>& params)
{
	if (instruction.GetTokenType() != TokenType::INSTRUCTION)
		throw AssemblerException("");

	OperandSize operandSize = OperandSize::WORD;
	string instructionMnemonic = instruction.GetValue();
	if (instructionMnemonic[instructionMnemonic.size() - 1] == 'b')
	{
		operandSize = OperandSize::BYTE;
		instructionMnemonic = instructionMnemonic.substr(0, instructionMnemonic.size() - 1);
	}
	else if (instructionMnemonic[instructionMnemonic.size() - 1] == 'w')
		instructionMnemonic = instructionMnemonic.substr(0, instructionMnemonic.size() - 1);

	map<string, InstructionDetails>::iterator it = instructionOperandMap.find(instructionMnemonic);
	
	// TODO: refactor exceptions
	if (it == instructionOperandMap.end())
		throw AssemblerException("instruction not recognized");
	else if (instructionOperandMap.at(instructionMnemonic).numberOfOperands != params.size())
		throw AssemblerException("number of parameters");

	operationCode[0] = instructionOperandMap.at(instructionMnemonic).opCode << 3;
	operationCode[0] = operationCode[0] | (operandSize << 2);

	// destination variable used for constraint checking on specific addressing modes
	bool destination = true;
	for (int i = 0; i < params.size(); i++)
	{
		Token operand = params.front();
		params.pop();

		/*switch (operand.GetTokenType())
		{
		case TokenType::OPERAND_IMMEDIATELY:
		{

		}
		case TokenType::OPERAND_REGISTER_DIRECT:
		{

		}
		case TokenType::OPERAND_REGISTER_INDIRECT:
		{

		}
		case TokenType::OPERAND_REGISTER_INDIRECT_8_SIGNED_BIT:
		{

		}
		case TokenType::OPERAND_REGISTER_INDIRECT_16_SIGNED_BIT:
		{

		}
		case TokenType::OPERAND_MEMORY:
		{

		}
		default:
		{
			throw AssemblerException("");
		}
		}*/

		destination = false;
	}


	// two operand
}

Instruction::~Instruction()
{
}

void Instruction::WriteCodeToOutput(ofstream& output)
{
	/*for (int i = 0; i < instructionSize; i++)
		output.write(operationCode[i]);*/
}
