#include "instruction.h"

Instruction::Instruction(const Token& instruction, queue<Token> params, unsigned long lineNumber, unsigned long locationCounter, SectionID currentSection, SymbolTable& symbolTable, RelocationTable& relocationTable)
{
	if (instruction.GetTokenType() != TokenType::INSTRUCTION)
		throw AssemblerException("Parameter 'instruction' is not of TokenType::INSTRUCTION.", ErrorCodes::SYNTAX_UNKNOWN_TOKEN, lineNumber);

	OperandSize operandSize = OperandSize::WORD;
	string instructionMnemonic = instruction.GetValue();
	// second condition because mnemonic can end with 'b' (e.g. sub)
	if ((instructionMnemonic[instructionMnemonic.size() - 1] == 'b') && (instructionOperandMap.find(instructionMnemonic) == instructionOperandMap.end()))
	{
		operandSize = OperandSize::BYTE;
		instructionMnemonic = instructionMnemonic.substr(0, instructionMnemonic.size() - 1);
	}
	else if (instructionMnemonic[instructionMnemonic.size() - 1] == 'w')
		instructionMnemonic = instructionMnemonic.substr(0, instructionMnemonic.size() - 1);

	map<string, InstructionDetails>::iterator it = instructionOperandMap.find(instructionMnemonic);

	if (it == instructionOperandMap.end())
		throw AssemblerException("Instruction '" + instruction.GetValue() + "' is not recognized.", ErrorCodes::INVALID_INSTRUCTION, lineNumber);
	else if (instructionOperandMap.at(instructionMnemonic).numberOfOperands != params.size())
		throw AssemblerException("Instruction '" + instruction.GetValue() + "' number of operands is not satisfied.", ErrorCodes::INVALID_OPERAND, lineNumber);

	operationCode[0] = instructionOperandMap.at(instructionMnemonic).opCode << 3;
	operationCode[0] = operationCode[0] | (operandSize << 2);
	instructionSize++;

	int size = GetInstructionSize(instruction, params, lineNumber);
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
		// NO RELOCATION FOR THIS ADDRESSING
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
		// ABSOLUTE RELOCATION FOR THIS ADDRESSING -> R_386_16
		case TokenType::OPERAND_IMMEDIATELY_SYMBOL:
		// NO RELOCATION FOR THIS ADDRESSING
		case TokenType::OPERAND_IMMEDIATELY_DECIMAL:
		// NO RELOCATION FOR THIS ADDRESSING
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
			{
				if (symbolTable.GetEntryByName(operand.GetValue()) == 0)
					throw AssemblerException("Symbol '" + operand.GetValue() + "' not found.", ErrorCodes::INVALID_OPERAND, lineNumber);
				
				const SymbolTableEntry& entry = *symbolTable.GetEntryByName(operand.GetValue());
				valueToWrite = GenerateRelocation(instructionMnemonic,
					entry,
					locationCounter,
					writeToPosition,
					size,
					currentSection,
					symbolTable,
					relocationTable);
			}
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
				if (valueToWrite > 0xFFFF)
					cout << "Warning at line " << lineNumber << ": instruction is specified to work with word operand(s), but provided operand is bigger than 0xFFFF. Least significant sixteen bits will be used." << endl;

				operationCode[writeToPosition++] = (uint8_t)(valueToWrite & 0xFF);
				operationCode[writeToPosition++] = (uint8_t)((valueToWrite >> 8) & 0xFF);

				instructionSize += 2;
			}

			break;
		}
		// ABSOLUTE RELOCATION FOR THIS SYMBOL ADDRESSING -> R_386_16,
		// OTHERWISE NOT
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
			string registerNumber = operand.GetValue().substr(1, operand.GetValue().find('[') - 1);;
			int c = stoi(registerNumber);

			if (c == 15)
				throw AssemblerException("Not allowed to use register indirect mode with PSW as base register.", ErrorCodes::INVALID_OPERAND, lineNumber);
			else if (c >= 8 || c < 0)
				throw AssemblerException("Specified register is not supported by the underlying processor architecture.", ErrorCodes::INVALID_OPERAND, lineNumber);
			
			long valueToWrite = -1;	// 16-bit long field
			if (offset.GetTokenType() == TokenType::OPERAND_IMMEDIATELY_HEX)
				valueToWrite = (unsigned long)strtol(offset.GetValue().c_str(), 0, 16);
			else if (offset.GetTokenType() == TokenType::SYMBOL)
			{
				if (symbolTable.GetEntryByName(offset.GetValue()))
				{
					const SymbolTableEntry& entry = *symbolTable.GetEntryByName(offset.GetValue());
					valueToWrite = GenerateRelocation(instructionMnemonic,
						entry,
						locationCounter,
						writeToPosition,
						size,
						currentSection,
						symbolTable,
						relocationTable);
				}
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
			else if ((unsigned long)valueToWrite <= 0xFF && offset.GetTokenType() != TokenType::SYMBOL)
			{
				operationCode[writeToPosition++] = (3 << 5) | (c << 1);
				instructionSize++;

				operationCode[writeToPosition++] = (uint8_t)(valueToWrite & 0xFF);
				instructionSize++;
			}
			else
			{
				if (valueToWrite > 0xFFFF)
					cout << "Warning at line " << lineNumber << ": instruction is specified to work with word operand(s), but provided operand is bigger than 0xFFFF. Least significant sixteen bits will be used." << endl;

				operationCode[writeToPosition++] = (4 << 5) | (c << 1);
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
		// ABSOLUTE RELOCATION FOR THIS SYMBOL ADDRESSING -> R_386_PC16,
		case TokenType::SYMBOL:
		// NO RELOCATION
		case TokenType::OPERAND_MEMORY_DIRECT_DECIMAL:
		// NO RELOCATION
		case TokenType::OPERAND_MEMORY_DIRECT_HEX:
		{
			// TODO: ?two memory accesses in the same instruction?
			operationCode[writeToPosition++] = 5 << 5; // set address mode to 0x00
			instructionSize++;

			long valueToWrite;	// 16-bit long field
			if (operand.GetTokenType() == TokenType::OPERAND_MEMORY_DIRECT_HEX)
				valueToWrite = (unsigned long)strtol(operand.GetValue().c_str(), 0, 16);
			else if (operand.GetTokenType() == TokenType::SYMBOL)
			{
				if (symbolTable.GetEntryByName(operand.GetValue()))
				{
					const SymbolTableEntry& entry = *symbolTable.GetEntryByName(operand.GetValue());
					valueToWrite = GenerateRelocation(instructionMnemonic,
						entry,
						locationCounter,
						writeToPosition,
						size,
						currentSection,
						symbolTable,
						relocationTable);
				}
				else
					throw AssemblerException("Symbol '" + operand.GetValue() + "' not found.", ErrorCodes::INVALID_OPERAND, lineNumber);
			}
			else
				valueToWrite = stoul(operand.GetValue());

			/*if (operandSize == OperandSize::BYTE)
			{
				operationCode[writeToPosition++] = (uint8_t)valueToWrite;

				instructionSize++;
			}
			else
			{*/
			operationCode[writeToPosition++] = (uint8_t)(valueToWrite & 0xFF);
			operationCode[writeToPosition++] = (uint8_t)((valueToWrite >> 8) & 0xFF);

			instructionSize += 2;
			//}

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

/* method for determining what to write in instruction argument fields and for
   adding entries to relocation table
*/
unsigned long Instruction::GenerateRelocation(string instructionMnemonic, 
	const SymbolTableEntry& entry, 
	unsigned long writeToPosition,
	unsigned long locationCounter,
	int instructionSize,
	SectionID currentSection, 
	SymbolTable& symbolTable, 
	RelocationTable& relocationTable)
{
	bool pcRelocation = false;
	map<string, InstructionDetails>::iterator it = instructionOperandMap.find(instructionMnemonic);
	if (it->second.jumpInstruction)
		pcRelocation = true;

	long result = 0;

	if (pcRelocation)	// R_386_PC16
	{
		if (entry.scopeType == ScopeType::EXTERN)
			result = -2; // -2 on 16-bit machine, because of data field length to next instruction
		else	// GLOBAL or LOCAL are both defined in the current file
		{
			if (currentSection == entry.sectionNumber)
				result = entry.offset - locationCounter - 2; // return without relocating wrong
			else
				result = entry.offset - 2;
		}

		relocationTable.InsertRelocation(currentSection,
			entry.entryNo,
			locationCounter + writeToPosition,
			RelocationType::R_386_PC16);
	}
	else				// R_386_16
	{
		// extern fields relocations are set at link time to absolute address
		// by just adding the starting address to 0 offset
		if (entry.scopeType == ScopeType::EXTERN)
			result = instructionSize;
		// fields in no matter what section in the current file should be set to
		// their offset and at link time just added with absolute address of their
		// respected section
		else
			result = entry.offset;

		relocationTable.InsertRelocation(currentSection,
			entry.entryNo,
			locationCounter + writeToPosition,
			RelocationType::R_386_16);
	}

	return result;
}

int Instruction::GetInstructionSize(const Token& instruction, queue<Token> params, unsigned long lineNumber)
{
	if (instruction.GetTokenType() != TokenType::INSTRUCTION)
		throw AssemblerException("Parameter 'instruction' is not of TokenType::INSTRUCTION.", ErrorCodes::SYNTAX_UNKNOWN_TOKEN, lineNumber);

	OperandSize operandSize = OperandSize::WORD;
	string instructionMnemonic = instruction.GetValue();
	// second condition because mnemonic can end with 'b' (e.g. sub)
	if ((instructionMnemonic[instructionMnemonic.size() - 1] == 'b') && (instructionOperandMap.find(instructionMnemonic) == instructionOperandMap.end()))
	{
		operandSize = OperandSize::BYTE;
		instructionMnemonic = instructionMnemonic.substr(0, instructionMnemonic.size() - 1);
	}
	else if (instructionMnemonic[instructionMnemonic.size() - 1] == 'w')
		instructionMnemonic = instructionMnemonic.substr(0, instructionMnemonic.size() - 1);

	map<string, InstructionDetails>::iterator it = instructionOperandMap.find(instructionMnemonic);

	if (it == instructionOperandMap.end())
		throw AssemblerException("Instruction '" + instruction.GetValue() + "' is not recognized.", ErrorCodes::INVALID_INSTRUCTION, lineNumber);
	else if (instructionOperandMap.at(instructionMnemonic).numberOfOperands != params.size())
		throw AssemblerException("Instruction '" + instruction.GetValue() + "' number of operands is not satisfied.", ErrorCodes::INVALID_OPERAND, lineNumber);

	int iteration = static_cast<int>(params.size());
	int result = 1;			// InstrDescr

	for (int i = 0; i < iteration; i++)
	{
		Token operand = params.front();
		params.pop();

		switch (operand.GetTokenType())
		{
		case TokenType::OPERAND_REGISTER_DIRECT:
		{
			result++;		// OpDescr
			break;
		}
		case TokenType::OPERAND_IMMEDIATELY_SYMBOL:
		{
			result++;		// OpDescr
			result += 2;	// Im/Di/Ad
			break;
		}
		case TokenType::OPERAND_IMMEDIATELY_DECIMAL:
		case TokenType::OPERAND_IMMEDIATELY_HEX:
		{
			result++;		// OpDescr
			if (operandSize == OperandSize::BYTE)
				result += 1;	// Im/Di/Ad
			else
				result += 2;	// Im/Di/Ad
			break;
		}
		case TokenType::OPERAND_REGISTER_INDIRECT:
		{
			Token baseRegister = Token::ParseToken(operand.GetValue().substr(0, operand.GetValue().find('[')), lineNumber);
			string offsetString = operand.GetValue();
			offsetString = string(offsetString.substr(offsetString.find('[') + 1, offsetString.size() - offsetString.find('[') - 2));
			Token offset = Token::ParseToken(offsetString, lineNumber);

			long valueToWrite = -1;	// 16-bit long field
			if (offset.GetTokenType() == TokenType::OPERAND_IMMEDIATELY_HEX)
				valueToWrite = (unsigned long)strtol(offset.GetValue().c_str(), 0, 16);
			else if (offset.GetTokenType() == TokenType::SYMBOL)
			{
				// nothing here because of second constraints bellow >>!= TokenType::SYMBOL<<
			}
			else
				valueToWrite = stoul(offset.GetValue());

			if (valueToWrite == 0 && offset.GetTokenType() != TokenType::SYMBOL)
			{
				result++;		// OpDescr
			}
			else if ((unsigned long)valueToWrite <= 0xFF && offset.GetTokenType() != TokenType::SYMBOL)
			{
				result++;		// OpDescr
				result += 1;	// Im/Di/Ad
			}
			else
			{
				result++;		// OpDescr
				result += 2;	// Im/Di/Ad
			}

			break;
		}
		case TokenType::OPERAND_PC_RELATIVE_SYMBOL:
		{
			result++;		// OpDescr
			result += 2;	// Im/Di/Ad
			break;
		}
		case TokenType::SYMBOL:
		case TokenType::OPERAND_MEMORY_DIRECT_DECIMAL:
		case TokenType::OPERAND_MEMORY_DIRECT_HEX:
		{
			result++;		// OpDescr
			result += 2;	// Im/Di/Ad
			break;
		}
		default:
		{
			throw AssemblerException("Addressing mode not recognized.", ErrorCodes::INVALID_OPERAND, lineNumber);
		}
		}
	}

	return result;
}