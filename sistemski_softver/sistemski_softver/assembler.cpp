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
	//SecondPass();
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
		}
		case TokenType::DIRECTIVE:
			if (currentSection == SectionType::START || SectionType::TEXT)
				throw runtime_error("Directive not allowed in current section.");

			// HandleDirective();

			break;
		case TokenType::SECTION:
			break;
		case TokenType::ACCESS_MODIFIER:
			break;
		case TokenType::INSTRUCTION:
			break;
		case TokenType::END_OF_FILE:
			break;
		default:
			throw AssemblerException("Not allowed token detected in the first pass of assembler.", ErrorCodes::NOT_ALLOWED_TOKEN, lineNumber);
		}
	}
}