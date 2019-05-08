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

/* PUBLIC METHODS */

void Assembler::GenerateObjectFile()
{
	StripeOffCommentsAndLoadLocally();
	FirstPass();
	SecondPass();
}

void Assembler::FirstPass()
{
	unsigned long locationCounter = 0;
	SectionType currentSection = SectionType::START;

	unsigned long lineNumber = 0;

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

		switch (currentToken.GetTokenType())
		{
		case TokenType::LABEL:
		{
			string labelName = currentToken.GetValue();

			if (currentSection == SectionType::START)
				throw AssemblerException("Label '" + labelName + "' cannot be defined at the start of a file without prior specificating a section.", ErrorCodes::SYNTAX_NO_INITIAL_SECTION, lineNumber);

			symbolTable.InsertSymbol(labelName,
				locationCounter,
				currentToken.GetTokenType(),
				ScopeType::LOCAL,
				currentSection,
				true);

			if (currentLineTokens.empty())
				continue;
			else
				throw AssemblerException("Incorrect syntax after label '" + labelName + "' declaration.", ErrorCodes::SYNTAX_LABEL, lineNumber);

			break;
		}
		case TokenType::DIRECTIVE:
		{
			if (currentSection == SectionType::START || currentSection == SectionType::TEXT)
				throw AssemblerException("Directive '" + currentToken.GetValue() + "' cannot be defined in current section.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);

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
				while (locationCounter % (2 ^ value) != 0)
					locationCounter++;
			}
			else if (currentToken.GetValue() == SKIP_DIRECTIVE)
			{
				if (operand.GetTokenType() != TokenType::OPERAND_DECIMAL)
					throw AssemblerException("Directive '.skip' expects decimal operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				// skip <<operand>> bytes 
				locationCounter += strtoul(operand.GetValue().c_str(), NULL, 0);
			}
			else if (currentToken.GetValue() == CHAR_DIRECTIVE)
			{
				if (operand.GetTokenType() != TokenType::OPERAND_DECIMAL)
					throw AssemblerException("Directive '.char' expects decimal operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				// skip 1 * val_byte byte
				locationCounter += strtoul(operand.GetValue().c_str(), NULL, 0);
			}
			else if (currentToken.GetValue() == WORD_DIRECTIVE)
			{
				if (operand.GetTokenType() != TokenType::OPERAND_DECIMAL)
					throw AssemblerException("Directive '.word' expects decimal operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				// skip 2 * val_byte byte
				locationCounter += (strtoul(operand.GetValue().c_str(), NULL, 0) << 1);
			}
			else if (currentToken.GetValue() == LONG_DIRECTIVE)
			{
				if (operand.GetTokenType() != TokenType::OPERAND_DECIMAL)
					throw AssemblerException("Directive '.long' expects decimal operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				// skip 4 * val_byte byte
				locationCounter += (strtoul(operand.GetValue().c_str(), NULL, 0) << 2);
			}

			break;
		}
		case TokenType::SECTION:
		{
			if (currentSection != SectionType::START)
			{
				// TODO: if currentSection type is already present throw error
				sectionSizeMap.insert({ currentSection, locationCounter });
				symbolTable.GetEntry(SectionToString(currentSection))->size = locationCounter;
			}

			currentSection = StringToSectionType(currentToken.GetValue());
			locationCounter = 0;
			symbolTable.InsertSymbol(currentToken.GetValue(), locationCounter, currentToken.GetTokenType(), ScopeType::LOCAL, currentSection, true);

			break;
		}
		case TokenType::ACCESS_MODIFIER:
		{
			// if (currentSection != SectionType::START)
			//	 throw AssemblerException("Access modifier '" + currentToken.GetValue() + "' must be put outside of section.", ErrorCodes::INVALID_ACCESS_MODIFIER_SECTION, lineNumber);*/

			continue;
			break;
		}
		case TokenType::INSTRUCTION:
		{
			if (currentSection != SectionType::TEXT)
				throw AssemblerException("Instructions cannot be put outside of '.text' section.", ErrorCodes::INVALID_INSTRUCTION_SECTION, lineNumber);

			// TODO: check whether all instructions are four bytes long
			locationCounter += 4;

			break;
		}
		case TokenType::END_OF_FILE:
		{
			if (currentSection != SectionType::START)
			{
				// TODO: if currentSection type is already present throw error
				sectionSizeMap.insert({ currentSection, locationCounter });
				symbolTable.GetEntry(SectionToString(currentSection))->size = locationCounter;
			}

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
		case TokenType::ACCESS_MODIFIER:
		{
			Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
			currentLineTokens.pop();

			if (currentToken.GetValue() == PUBLIC_MODIFIER)
			{
				if (symbolTable.GetEntry(operand.GetValue()))
					symbolTable.GetEntry(operand.GetValue())->scope = ScopeType::GLOBAL;
			}
			else if (currentToken.GetValue() == EXTERN_MODIFIER)
			{
				/*symbolTable.InsertSymbol(operand.GetValue(),
										 locationCounter,
										 TokenType::SYMBOL,
										 ScopeType::EXTERN,
										 currentSection,
										 false)
										 */
			}
			else
				throw new AssemblerException("Access modifier directive not recognized.", ErrorCodes::SYNTAX_ACCESS_MODIFIER, lineNumber);

			// TODO: anything else here?
		}
		case TokenType::DIRECTIVE:
		{
		}
		case TokenType::SECTION:
		{
		}
		case TokenType::INSTRUCTION:
		{
		}
		default:
		{

		}
		}
	}
}