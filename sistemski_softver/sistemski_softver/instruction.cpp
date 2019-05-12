#include "instruction.h"

Instruction::Instruction(const Token & instruction, queue<Token>& params, unsigned long lineNumber, SymbolTable& symbolTable, bool firstPass)
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
		throw AssemblerException("Instruction '" + instruction.GetValue() + "' is not recognized.", ErrorCodes::INVALID_INSTRUCTION, lineNumber);
	else if (instructionOperandMap.at(instructionMnemonic).numberOfOperands != params.size())
		throw AssemblerException("Instruction '" + instruction.GetValue() + "' number of operands is not satisfied.", ErrorCodes::INVALID_OPERAND, lineNumber);

	operationCode[0] = instructionOperandMap.at(instructionMnemonic).opCode << 3;
	operationCode[0] = operationCode[0] | (operandSize << 2);
	instructionSize++;

	int writeToPosition = 1;

	// destination variable used for constraint checking on specific addressing modes
	bool destination = false;
	if (params.size() == 2)
		destination = true;
	int iteration = static_cast<int>(params.size());

	for (int i = 0; i < iteration; i++)
	{
		Token operand = params.front();
		params.pop();

		switch (operand.GetTokenType())
		{
		case TokenType::OPERAND_REGISTER_DIRECT:
		{

			char registerNumber = operand.GetValue().at(1);
			int c = registerNumber - '0';
			int mode = 0;

			if (destination && c == 15)
				throw AssemblerException("Not allowed to write to PSW register.", ErrorCodes::INVALID_OPERAND, lineNumber);

			if (operandSize == OperandSize::BYTE)
			{
				if (operand.GetValue().size() != 3)
					throw AssemblerException("Register higher or lower bytes to use is byte addressing mode is not specified.", ErrorCodes::INVALID_OPERAND, lineNumber);
				
				char hl = operand.GetValue().at(2);
				if (hl == 'l')
					mode = 0;
				else
					mode = 1;
			}
			else
			{
				if (operand.GetValue().size() == 3)
					throw AssemblerException("Register higher or lower bytes is not allowed to use if instruction addressing mode is not specified to be a byte one.", ErrorCodes::INVALID_OPERAND, lineNumber);
			}

			if (c >= 8 || c < 0)
				throw AssemblerException("Specified register is not supported by the underlying processor architecture.", ErrorCodes::INVALID_OPERAND, lineNumber);

			operationCode[writeToPosition++] = (1 << 5) | (c << 1) | mode;
			instructionSize++;

			break;
		}
		case TokenType::OPERAND_IMMEDIATELY_SYMBOL:
		case TokenType::OPERAND_IMMEDIATELY_DECIMAL:
		case TokenType::OPERAND_IMMEDIATELY_HEX:
		{
			if (destination)
				throw AssemblerException("Immediately addressing mode is not a valid mode for destination operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

			operationCode[writeToPosition++] = 0; // set address mode to 0x00
			instructionSize++;

			long valueToWrite;	// 16-bit long field
			if (operand.GetTokenType() == TokenType::OPERAND_IMMEDIATELY_HEX)
				valueToWrite = (unsigned long)strtol(operand.GetValue().c_str(), 0, 16);
			else if (operand.GetTokenType() == OPERAND_IMMEDIATELY_SYMBOL)
				valueToWrite = symbolTable.GetEntryByName(operand.GetValue())->offset;
			else
				valueToWrite = stoul(operand.GetValue());

			if (operandSize == OperandSize::BYTE)
			{
				operationCode[writeToPosition++] = (uint8_t)valueToWrite;

				if (valueToWrite > 0xFF)
					cout << "Warning at line " << lineNumber << ": instruction is specified to work with byte operand(s), but provided operand is bigger than 0xFF. Least significant eight bits will be used." << endl;

				instructionSize++;
			}
			else
			{
				operationCode[writeToPosition++] = (uint8_t)(valueToWrite & 0xFF);
				operationCode[writeToPosition++] = (uint8_t)((valueToWrite >> 8) & 0xFF);

				instructionSize += 2;
			}

			break;
		}
		case TokenType::OPERAND_REGISTER_INDIRECT:
		{
			Token baseRegister = Token::ParseToken(operand.GetValue().substr(0, operand.GetValue().find('[')), lineNumber);
			string offsetString = operand.GetValue();
			offsetString = string(offsetString.substr(offsetString.find('[') + 1, offsetString.size() - offsetString.find('[') - 2));
			Token offset = Token::ParseToken(offsetString, lineNumber);

			if (baseRegister.GetTokenType() != TokenType::OPERAND_REGISTER_DIRECT)
				throw AssemblerException("Invalid register indirect addressing mode syntax.", ErrorCodes::INVALID_OPERAND, lineNumber);

			if (offset.GetTokenType() != TokenType::OPERAND_IMMEDIATELY_DECIMAL &&
				offset.GetTokenType() != TokenType::OPERAND_IMMEDIATELY_HEX &&
				offset.GetTokenType() != TokenType::SYMBOL)
				throw AssemblerException("Invalid syntax offset of register indirect addressing mode.", ErrorCodes::INVALID_OPERAND, lineNumber);

			// operator encoding
			char registerNumber = operand.GetValue().at(1);
			int c = registerNumber - '0';

			if (c >= 8 || c < 0)
				throw AssemblerException("Specified register is not supported by the underlying processor architecture.", ErrorCodes::INVALID_OPERAND, lineNumber);
			else if (c == 15)
				throw AssemblerException("Not allowed to use register indirect mode with PSW as base register.", ErrorCodes::INVALID_OPERAND, lineNumber);

			long valueToWrite = -1;	// 16-bit long field
			if (offset.GetTokenType() == TokenType::OPERAND_IMMEDIATELY_HEX)
				valueToWrite = (unsigned long)strtol(offset.GetValue().c_str(), 0, 16);
			else if (offset.GetTokenType() == TokenType::SYMBOL)
			{
				if (!firstPass)
					if (symbolTable.GetEntryByName(offset.GetValue()))
						valueToWrite = symbolTable.GetEntryByName(offset.GetValue())->offset;
					else
						throw AssemblerException("Symbol '" + offset.GetValue() + "' not found.", ErrorCodes::INVALID_OPERAND, lineNumber);
			}
			else
				valueToWrite = stoul(offset.GetValue());

			if (valueToWrite == 0 && offset.GetTokenType() != TokenType::SYMBOL)
			{
				// write addressing opcode
				operationCode[writeToPosition++] = (2 << 5) | (c << 1);
				instructionSize++;
			}
			else if (valueToWrite >= -128 && valueToWrite <= 127 && offset.GetTokenType() != TokenType::SYMBOL)
			{
				operationCode[writeToPosition++] = (3 << 5) | (c << 1);
				instructionSize++;

				operationCode[writeToPosition++] = (uint8_t)(valueToWrite & 0xFF);
				instructionSize++;
			}
			else
			{
				operationCode[writeToPosition++] = (3 << 5) | (c << 1);
				instructionSize++;

				operationCode[writeToPosition++] = (uint8_t)(valueToWrite & 0xFF);
				operationCode[writeToPosition++] = (uint8_t)((valueToWrite >> 8) & 0xFF);

				instructionSize += 2;
			}

			break;
		}
		case TokenType::OPERAND_PC_RELATIVE_SYMBOL:
		{
			operationCode[writeToPosition++] = (4 << 5) | (7 << 1);
			instructionSize++;

			unsigned long valueToWrite = symbolTable.GetEntryByName(operand.GetValue())->offset;
			operationCode[writeToPosition++] = (uint8_t)(valueToWrite & 0xFF);
			operationCode[writeToPosition++] = (uint8_t)((valueToWrite >> 8) & 0xFF);

			instructionSize += 2;

			break;
		}
		case TokenType::SYMBOL:
		case TokenType::OPERAND_MEMORY_DIRECT_DECIMAL:
		case TokenType::OPERAND_MEMORY_DIRECT_HEX:
		{
			// TODO: ?two memory accesses in the same instruction?
			operationCode[writeToPosition++] = 5 << 5; // set address mode to 0x00
			instructionSize++;

			long valueToWrite;	// 16-bit long field
			if (operand.GetTokenType() == TokenType::OPERAND_MEMORY_DIRECT_HEX)
				valueToWrite = (unsigned long)strtol(operand.GetValue().c_str(), 0, 16);
			else if (operand.GetTokenType() == TokenType::SYMBOL)
				valueToWrite = symbolTable.GetEntryByName(operand.GetValue())->offset;
			else
				valueToWrite = stoul(operand.GetValue());

			if (operandSize == OperandSize::BYTE)
			{
				operationCode[writeToPosition++] = (uint8_t)valueToWrite;

				instructionSize++;
			}
			else
			{
				operationCode[writeToPosition++] = (uint8_t)(valueToWrite & 0xFF);
				operationCode[writeToPosition++] = (uint8_t)((valueToWrite >> 8) & 0xFF);

				instructionSize += 2;
			}

			break;
		}
		default:
		{
			throw AssemblerException("Addressing mode not recognized.", ErrorCodes::INVALID_OPERAND, lineNumber);
		}
		}

		destination = false;
	}
}

void Instruction::WriteToObjectFile(ofstream& output)
{
	for (int i = 0; i < instructionSize; i++)
		output << operationCode[i];
}