#include "assembler.h"

/* CONSTRUCTOR */

Assembler::Assembler(string input_file_url, string output_file_url)
{
	// opening input file in input mode and output file in output mode
	// truncating all the previously data in it
	this->input_file.open(input_file_url, ios::in);
	this->output_file.open(output_file_url, ios::out | ios::trunc);

	if (!input_file.is_open())
		throw AssemblerException("Cannot open input file.", ErrorCodes::IO_INPUT_EXCEPTION);
	else if (!output_file.is_open())
		throw AssemblerException("Cannot open output file.", ErrorCodes::IO_OUTPUT_EXCEPTION);
}

Assembler::~Assembler()
{
	output_file.close();
}

/* PRIVATE METHODS */

void Assembler::StripeOffCommentsAndLoadLocally()
{
	string line;
	unsigned long lineNumber = 0;

	while (std::getline(input_file, line))
	{
		lineNumber++;

		// find and stripe off comment in the current line
		size_t commentStart = line.find(COMMENT_START_SYMBOL);
		if (commentStart != string::npos)
			line = line.substr(0, commentStart);

		// split current line into separate tokens
		vector<string> currentLineTokens;
		TokenizeCurrentLine(line, currentLineTokens);

		/*if (currentLineTokens.size() == 0) -> removed to implement line numbering */
		if (currentLineTokens.size() != 0 && currentLineTokens[0] == END_DIRECTIVE)
			break;

		// save to local storage for assembler operation
		assemblyCode.push_back(currentLineTokens);
	}

	input_file.close();

	// add implicit end of file directive if it is not present
	if (assemblyCode.size() != 0)
	{
		vector<string> r = assemblyCode.at(assemblyCode.size() - 1);

		if (r.size() != 0 && (Token::ParseToken(r.at(0), lineNumber).GetTokenType() != TokenType::END_OF_FILE))
			assemblyCode.push_back({ END_DIRECTIVE });
	}
	else
		assemblyCode.push_back({ END_DIRECTIVE });

}

void Assembler::TokenizeCurrentLine(const string& line, vector<string>& collector)
{
	// TODO: _strdup -> strdup
	char* duplicate = _strdup(line.c_str());
	char* token = strtok(duplicate, TOKEN_DELIMITERS);

	while (token != NULL)
	{
		collector.push_back(string(token));
		token = strtok(NULL, TOKEN_DELIMITERS);
	}

	delete duplicate;
}

void Assembler::WriteToFile(uint8_t byte)
{
	throw AssemblerException("not yet implemented");
}

/* PUBLIC METHODS */

void Assembler::GenerateObjectFile()
{
	StripeOffCommentsAndLoadLocally();
	FirstPass();
	SecondPass();
}

void Assembler::FirstPass()
{
	unsigned long lineNumber = 0;

	unsigned long locationCounter = 0;
	SectionType currentSectionType = SectionType::START;
	SectionID currentSectionNo = -1;

	// get each line
	for (const vector<string>& line : assemblyCode)
	{
		// line numbering for output to user purpose
		lineNumber++;
		if (line.size() == 0)
			continue;

		// get vector of current line tokens
		queue<string> currentLineTokens;
		for (const string& s : line)
			currentLineTokens.push(s);

		// parse current token
		Token currentToken = Token::ParseToken(currentLineTokens.front(), lineNumber);
		currentLineTokens.pop();
		
		// TODO: don't allow tokens to be put in START section

		// necessary because label and instruction can be in the same line
		string labelName;
		if (currentToken.GetTokenType() == TokenType::LABEL)
		{
			labelName = currentToken.GetValue();

			if (currentSectionType == SectionType::START)
				throw AssemblerException("Label '" + labelName + "' cannot be defined at the start of a file without prior specificating a section.", ErrorCodes::SYNTAX_NO_INITIAL_SECTION, lineNumber);

			symbolTable.InsertSymbol(labelName,
				currentSectionNo,
				ASM_UNDEFINED,
				locationCounter,
				ScopeType::LOCAL,
				TokenType::LABEL,
				ASM_UNDEFINED);

			if (currentLineTokens.empty())
				continue;
			
			// prepare next token to analyze
			currentToken = Token::ParseToken(currentLineTokens.front(), lineNumber);
			currentLineTokens.pop();
		}

		switch (currentToken.GetTokenType())
		{
		case TokenType::LABEL:
		{
			throw AssemblerException("Incorrect syntax after label '" + labelName + "' declaration.", ErrorCodes::SYNTAX_LABEL, lineNumber);

			break;
		}
		case TokenType::DIRECTIVE:
		{
			if (currentSectionType == SectionType::START)
				throw AssemblerException("Directive '" + currentToken.GetValue() + "' cannot be defined outside of any section.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);

			Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
			currentLineTokens.pop();

			if (currentToken.GetValue() == ALIGN_DIRECTIVE)
			{
				if (operand.GetTokenType() != TokenType::OPERAND_DECIMAL)
					throw AssemblerException("Directive '.align' expects decimal operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				unsigned long value = strtoul(operand.GetValue().c_str(), NULL, 0);

				// TODO: optimize this with some math expression to calculate how much
				//		 to add to round so that location counter could be dividable 
				//		with 2 ^ value
				unsigned long divisibleBy = (unsigned long)pow(2.0, value);
				while ((locationCounter % divisibleBy) != 0)
					locationCounter++;
			}
			else if (currentToken.GetValue() == BYTE_DIRECTIVE)
			{
				// removed to allow user to implement new instructions
				/*if (currentSectionType != SectionType::DATA)
					throw AssemblerException("Directive '" + currentToken.GetValue() + "' cannot be defined in current section.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);*/
				
				if ((operand.GetTokenType() != TokenType::OPERAND_DECIMAL) &&
					(operand.GetTokenType() != TokenType::OPERAND_HEX))
					throw AssemblerException("Directive '.byte' expects decimal or hex operand.", ErrorCodes::INVALID_OPERAND, lineNumber);
				
				// skip 1 * val_byte byte
				locationCounter += 1; // strtoul(operand.GetValue().c_str(), NULL, 0);
			}
			else if (currentToken.GetValue() == SKIP_DIRECTIVE)
			{
				if (operand.GetTokenType() != TokenType::OPERAND_DECIMAL &&
					operand.GetTokenType() != TokenType::OPERAND_HEX)
					throw AssemblerException("Directive '.skip' expects decimal operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				// skip <<operand>> bytes 
				locationCounter += strtoul(operand.GetValue().c_str(), NULL, 0);
			}
			else if (currentToken.GetValue() == WORD_DIRECTIVE)
			{
				// removed to allow user to implement new instructions
				/*if (currentSectionType != SectionType::DATA)
					throw AssemblerException("Directive '" + currentToken.GetValue() + "' cannot be defined in current section.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);*/

				if ((operand.GetTokenType() != TokenType::OPERAND_DECIMAL) &&
					(operand.GetTokenType() != TokenType::OPERAND_HEX))
					throw AssemblerException("Directive '.word' expects decimal or hex operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				// skip 2 * val_byte byte
				locationCounter += 2; // (strtoul(operand.GetValue().c_str(), NULL, 0) << 1);
			}
			else if (currentToken.GetValue() == EQUIVALENCE_DIRECTIVE)
			{
				if (currentSectionType != SectionType::DATA)
					throw AssemblerException("Directive '" + currentToken.GetValue() + "' cannot be defined in current section.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);

				// Token operand holds equivalence name
				// Token equValue holds value to which label is equivalent

				if (operand.GetTokenType() != TokenType::SYMBOL)
					throw AssemblerException("Directive '.equ' requires label-like syntax for first operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				Token equValue = Token::ParseToken(currentLineTokens.front(), lineNumber);
				currentLineTokens.pop();

				unsigned long value;

				if (equValue.GetTokenType() == TokenType::OPERAND_DECIMAL)
					value = strtoul(equValue.GetValue().c_str(), NULL, 0);
				else if (equValue.GetTokenType() == TokenType::OPERAND_HEX)
					value = stoul(equValue.GetValue(), nullptr, 0);
				else
				{
					// TODO: implement equivalence expressions here if needed
				}
				
				symbolTable.InsertSymbol(operand.GetValue(),
					currentSectionNo,
					value,
					ASM_UNDEFINED,
					ScopeType::LOCAL,
					TokenType::DIRECTIVE,
					ASM_UNDEFINED);
			}

			break;
		}
		case TokenType::SECTION:
		{
			// update existing current section size
			if (currentSectionType != SectionType::START)
				sectionTable.GetEntryByID(currentSectionNo)->length = locationCounter;

			if (currentToken.GetValue() != "")	// GetValue() can return (.bss|.data|.text)
			{
				currentSectionType = StringToSectionType(currentToken.GetValue());
				currentSectionNo = sectionTable.InsertSection(currentToken.GetValue(), 0, 0);

				SymbolTableID symbolTableNo = symbolTable.InsertSymbol(currentToken.GetValue(),
					currentSectionNo,
					ASM_UNDEFINED,
					ASM_UNDEFINED,
					ScopeType::LOCAL,
					TokenType::SECTION,
					ASM_UNDEFINED);

				sectionTable.GetEntryByID(currentSectionNo)->symbolTableEntryNo = symbolTableNo;
			}				
			else
			{
				Token userDefinedSection = Token::ParseToken(currentLineTokens.front(), lineNumber);
				currentLineTokens.pop();

				if (userDefinedSection.GetTokenType() != TokenType::SECTION && userDefinedSection.GetTokenType() != TokenType::SYMBOL)
					throw new AssemblerException("After '.section' directive section name is required.", ErrorCodes::SYNTAX_UNKNOWN_TOKEN, lineNumber);
				
				currentSectionType = StringToSectionType(userDefinedSection.GetValue());
				currentSectionNo = sectionTable.InsertSection(userDefinedSection.GetValue(), 0, 0);

				SymbolTableID symbolTableNo = symbolTable.InsertSymbol(userDefinedSection.GetValue(),
					currentSectionNo,
					ASM_UNDEFINED,
					ASM_UNDEFINED,
					ScopeType::LOCAL,
					TokenType::SECTION,
					ASM_UNDEFINED);

				sectionTable.GetEntryByID(currentSectionNo)->symbolTableEntryNo = symbolTableNo;
			}

			// all new sections begin from zero
			locationCounter = 0;

			break;
		}
		case TokenType::ACCESS_MODIFIER:
		{
			// if (currentSection != SectionType::START)
			//	 throw AssemblerException("Access modifier '" + currentToken.GetValue() + "' must be put outside of section.", ErrorCodes::INVALID_ACCESS_MODIFIER_SECTION, lineNumber);*/

			continue;
		}
		case TokenType::INSTRUCTION:
		{
			if (currentSectionType != SectionType::TEXT)
				throw AssemblerException("Instructions cannot be put outside of '.text' section.", ErrorCodes::INVALID_INSTRUCTION_SECTION, lineNumber);

			queue<Token> params;

			while (!currentLineTokens.empty())
			{
				Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
				params.push(operand);

				currentLineTokens.pop();
			}

			Instruction instruction(currentToken, params);

			// TODO: if 1,4,7 are allowed for instruction size optimize by removing this 'vector<Token> params'

			locationCounter += instruction.GetInstructionSize();

			break;
		}
		case TokenType::END_OF_FILE:
		{
			// update length of the last section
			if (currentSectionType != SectionType::START)
				sectionTable.GetEntryByID(currentSectionNo)->length = locationCounter;

			break;
		}
		default:
			throw AssemblerException("Not allowed token detected in the first pass of assembler.", ErrorCodes::NOT_ALLOWED_TOKEN, lineNumber);
		}
	}
}

void Assembler::SecondPass()
{
	unsigned long lineNumber = 0;

	unsigned long locationCounter = 0;
	SectionType currentSectionType = SectionType::START;
	SectionID currentSectionNo = -1;

	for (vector<string> line : assemblyCode)
	{
		lineNumber++;
		if (line.size() == 0)
			continue;
		
		// get vector of current line tokens
		queue<string> currentLineTokens;
		for (const string& s : line)
			currentLineTokens.push(s);

		// parse current token
		Token currentToken = Token::ParseToken(currentLineTokens.front(), lineNumber);
		currentLineTokens.pop();

		switch (currentToken.GetTokenType())
		{
		case TokenType::LABEL:
		{
			break;
		}
		case TokenType::ACCESS_MODIFIER:
		{
			Token labelName = Token::ParseToken(currentLineTokens.front(), lineNumber);
			currentLineTokens.pop();

			if (currentToken.GetValue() == PUBLIC_MODIFIER)
			{
				if (symbolTable.GetEntryByName(labelName.GetValue()))
					symbolTable.GetEntryByName(labelName.GetValue())->scopeType = ScopeType::GLOBAL;
			}
			else if (currentToken.GetValue() == EXTERN_MODIFIER)
			{
				symbolTable.InsertSymbol(labelName.GetValue(),
					currentSectionNo,
					ASM_UNDEFINED,
					ASM_UNDEFINED,
					ScopeType::EXTERN,
					TokenType::LABEL,
					ASM_UNDEFINED);
			}
			else
				throw new AssemblerException("Access modifier directive not recognized.", ErrorCodes::SYNTAX_ACCESS_MODIFIER, lineNumber);			

			break;
		}
		case TokenType::DIRECTIVE:
		{
			if (currentSectionType == SectionType::START)
				throw AssemblerException("Directive '" + currentToken.GetValue() + "' cannot be defined outside of any section.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);

			Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
			currentLineTokens.pop();

			if (currentToken.GetValue() == ALIGN_DIRECTIVE)
			{
				if (operand.GetTokenType() != TokenType::OPERAND_DECIMAL)
					throw AssemblerException("Directive '.align' expects decimal operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				unsigned long value = strtoul(operand.GetValue().c_str(), NULL, 0);

				// TODO: optimize this with some math expression to calculate how much
				//		 to add to round so that location counter could be dividable 
				//		with 2 ^ value
				unsigned long divisibleBy = (unsigned long)pow(2.0, value);
				while ((locationCounter % divisibleBy) != 0)
				{
					locationCounter++;

					WriteToFile(0);
				}
			}
			else if (currentToken.GetValue() == BYTE_DIRECTIVE)
			{
				// removed to allow user to implement new instructions
				/*if (currentSectionType != SectionType::DATA)
					throw AssemblerException("Directive '" + currentToken.GetValue() + "' cannot be defined in current section.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);*/

				if ((operand.GetTokenType() != TokenType::OPERAND_DECIMAL) &&
					(operand.GetTokenType() != TokenType::OPERAND_HEX))
					throw AssemblerException("Directive '.byte' expects decimal or hex operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				uint8_t toWrite = stoi(operand.GetValue());
				WriteToFile(toWrite);

				// skip 1 * val_byte byte
				locationCounter += 1; // strtoul(operand.GetValue().c_str(), NULL, 0);
			}
			else if (currentToken.GetValue() == SKIP_DIRECTIVE)
			{
				if (operand.GetTokenType() != TokenType::OPERAND_DECIMAL &&
					operand.GetTokenType() != TokenType::OPERAND_HEX)
					throw AssemblerException("Directive '.skip' expects decimal operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				// skip <<operand>> bytes 
				locationCounter += strtoul(operand.GetValue().c_str(), NULL, 0);
			}
			else if (currentToken.GetValue() == WORD_DIRECTIVE)
			{
				// removed to allow user to implement new instructions
				/*if (currentSectionType != SectionType::DATA)
					throw AssemblerException("Directive '" + currentToken.GetValue() + "' cannot be defined in current section.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);*/

				if ((operand.GetTokenType() != TokenType::OPERAND_DECIMAL) &&
					(operand.GetTokenType() != TokenType::OPERAND_HEX))
					throw AssemblerException("Directive '.word' expects decimal or hex operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				// skip 2 * val_byte byte
				locationCounter += 2; // (strtoul(operand.GetValue().c_str(), NULL, 0) << 1);
			}
			else if (currentToken.GetValue() == EQUIVALENCE_DIRECTIVE)
			{
				if (currentSectionType != SectionType::DATA)
					throw AssemblerException("Directive '" + currentToken.GetValue() + "' cannot be defined in current section.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);

				// Token operand holds equivalence name
				// Token equValue holds value to which label is equivalent

				if (operand.GetTokenType() != TokenType::SYMBOL)
					throw AssemblerException("Directive '.equ' requires label-like syntax for first operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				Token equValue = Token::ParseToken(currentLineTokens.front(), lineNumber);
				currentLineTokens.pop();

				unsigned long value;

				if (equValue.GetTokenType() == TokenType::OPERAND_DECIMAL)
					value = strtoul(equValue.GetValue().c_str(), NULL, 0);
				else if (equValue.GetTokenType() == TokenType::OPERAND_HEX)
					value = stoul(equValue.GetValue(), nullptr, 0);
				else
				{
					// TODO: implement equivalence expressions here if needed
				}

				symbolTable.InsertSymbol(operand.GetValue(),
					currentSectionNo,
					value,
					ASM_UNDEFINED,
					ScopeType::LOCAL,
					TokenType::DIRECTIVE,
					ASM_UNDEFINED);
			}

			break;
		}
		case TokenType::SECTION:
		{
			// also reset location counter to zero in second-pass
			locationCounter = 0;

			currentSectionNo++;
			currentSectionType = StringToSectionType(sectionTable.GetEntryByID(currentSectionNo)->name);
		}
		case TokenType::INSTRUCTION:
		{
		}
		default:
		{
			// sdfsdf
		}
		}
	}

	/* NOTE: if TNS should be added this is the place to do it.
			 linked list of dependent symbols; removing of tail nodes until null returned
			 circular depencdency detection -> if nothing is removed throw exception
	*/
}