#include "assembler.h"

/* CONSTRUCTOR */

Assembler::Assembler(string input_file_url, string output_file_url)
{
	// opening input file in input mode and output file in output mode
	// truncating all the previously data in it
	this->input_file.open(input_file_url, ios::in);
	this->output_file.open(output_file_url, ios::binary | ios::out | ios::trunc);
	this->txt_output_file.open(output_file_url + ".txt", ios::out | ios::trunc);

	if (!input_file.is_open())
		throw AssemblerException("Cannot open input file.", ErrorCodes::IO_INPUT_EXCEPTION);
	else if (!output_file.is_open())
		throw AssemblerException("Cannot open output file.", ErrorCodes::IO_OUTPUT_EXCEPTION);
	else if (!txt_output_file.is_open())
		throw AssemblerException("Cannot open textual output file.", ErrorCodes::IO_OUTPUT_EXCEPTION);
}

Assembler::~Assembler()
{
	output_file.close();
	txt_output_file.close();
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

		// convert all code to lowercase
		//transform(line.begin(), line.end(), line.begin(), tolower);
		for (int i = 0; line[i]; i++) {
			line[i] = tolower(line[i]);
		}

		// split current line into separate tokens
		vector<string> currentLineTokens;
		TokenizeCurrentLine(line, currentLineTokens);

		/*if (currentLineTokens.size() == 0) -> removed to implement line numbering */
		if (currentLineTokens.size() != 0 && currentLineTokens[0] == END_DIRECTIVE)
		{
			assemblyCode.push_back(currentLineTokens);
			break;
		}

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
	char* duplicate = strdup(line.c_str());
	char* token = strtok(duplicate, TOKEN_DELIMITERS);

	while (token != NULL)
	{
		collector.push_back(string(token));
		token = strtok(NULL, TOKEN_DELIMITERS);
	}

	delete duplicate;
}

inline void Assembler::WriteToOutput(uint8_t byte)
{
	// outputFile
	binaryBuffer.write(reinterpret_cast<char*>(&byte), sizeof(byte));
	contentLength++;

	txt_output_file << hex << ((byte >> 4) & 0xF);
	txt_output_file << hex << (byte & 0xF);
	if (++currentBytesInline == BYTES_INLINE)
	{
		currentBytesInline = 0;
		txt_output_file << endl;
	}
	else
		txt_output_file << " ";
	
}

inline void Assembler::WriteToOutput(const Instruction& instruction)
{
	for (int i = 0; i < instruction.instructionSize; i++)
	{
		// outputFile
		binaryBuffer << instruction.operationCode[i];
		contentLength++;

		// START DEBUG
		// txt_output_file << bitset<8>(instruction.operationCode[i]) << endl;
		// END DEBUG

		txt_output_file << hex << ((instruction.operationCode[i] >> 4) & 0xF);
		txt_output_file << hex << (instruction.operationCode[i] & 0xF);
		if (++currentBytesInline == BYTES_INLINE)
		{
			currentBytesInline = 0;
			txt_output_file << endl;
		}
		else
			txt_output_file << " ";
	}

	// START DEBUG
	// txt_output_file << endl << endl;
	// END DEBUG
}

inline void Assembler::WriteToOutput(string text)
{
	txt_output_file << text;
}

void Assembler::ResolveIncalculatableSymbols()
{
	bool end = false;

	while (!end)
	{
		end = true;

		for (size_t i = 0; i < tns.GetSize(); i++)
		{
			try
			{
				TNSEntry& entry = *tns.GetEntryByID((unsigned)i);
				vector<Token> arithmeticTokens = ArithmeticParser::Parse(ArithmeticParser::TokenizeExpression(entry.expression));
				
				unsigned long v = ArithmeticParser::CalculateSymbolValue(arithmeticTokens, symbolTable, false, entry.sectionNumber);
				symbolTable.GetEntryByName(entry.name)->offset = v;

				// symbol is added, so table has changed
				tns.DeleteEntryByName(entry.name);
				end = false;
				break; // break for because vector has change and iterator would throw exception
			}
			catch (const AssemblerException& ex)
			{
				throw ex;
			}
			catch (const exception& ex)
			{
				ex.what();
				// error because symbol is missing
			}
		}
	}
}

void Assembler::GenerateObjectFile()
{
	StripeOffCommentsAndLoadLocally();
	FirstPass();
	ResolveIncalculatableSymbols();
	SecondPass();

	// SYMBOL TABLE
	WriteToOutput("\n\n<!-- symbol table -->\n");
	WriteToOutput(symbolTable.GenerateTextualSymbolTable().str());

	// SECTION TABLE
	WriteToOutput("\n<!-- section table -->\n");
	WriteToOutput(sectionTable.GenerateTextualSectionTable().str());

	// RELOCATION TABLE
	WriteToOutput("\n<!-- relocation table -->\n");
	WriteToOutput(relocationTable.GenerateTextualRelocationTable().str());

	// TNS TABLE
	WriteToOutput("\n<!-- TNS table -->\n");
	WriteToOutput(tns.GenerateTextualTNSTable().str());

	WriteBinaryFile();
}

void Assembler::FirstPass()
{
	unsigned long lineNumber = 0;

	unsigned long locationCounter = 0;
	SectionID currentSectionNo = START_SECTION;

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
		
		// necessary because label and instruction can be in the same line
		string labelName;
		if (currentToken.GetTokenType() == TokenType::LABEL)
		{
			labelName = currentToken.GetValue();

			if (currentSectionNo == START_SECTION)
				throw AssemblerException("Label '" + labelName + "' cannot be defined at the start of a file without prior specificating a section.", ErrorCodes::SYNTAX_NO_INITIAL_SECTION, lineNumber);

			symbolTable.InsertSymbol(labelName,
				currentSectionNo,
				ASM_UNDEFINED,
				locationCounter,
				ScopeType::LOCAL,
				TokenType::LABEL);

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
			if (currentSectionNo == START_SECTION)
				throw AssemblerException("Directive '" + currentToken.GetValue() + "' cannot be defined outside of any section.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);

			if (currentToken.GetValue() == ALIGN_DIRECTIVE)
			{
				Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
				currentLineTokens.pop();

				if (operand.GetTokenType() != TokenType::OPERAND_IMMEDIATELY_DECIMAL)
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
				if (sectionTable.HasFlag(currentSectionNo, SectionPermissions::BSS))
					throw AssemblerException("Directive '.byte' cannot be put into secction with 'b' flag.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);

				do
				{
					Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
					currentLineTokens.pop();

					if ((operand.GetTokenType() != TokenType::OPERAND_IMMEDIATELY_DECIMAL) &&
						(operand.GetTokenType() != TokenType::OPERAND_IMMEDIATELY_HEX))
						throw AssemblerException("Directive '.byte' expects decimal or hex operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

					// skip 1 * val_byte byte
					locationCounter += 1;
				} while (!currentLineTokens.empty());
			}
			else if (currentToken.GetValue() == SKIP_DIRECTIVE)
			{
				Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
				currentLineTokens.pop();

				if (operand.GetTokenType() != TokenType::OPERAND_IMMEDIATELY_DECIMAL &&
					operand.GetTokenType() != TokenType::OPERAND_IMMEDIATELY_HEX)
					throw AssemblerException("Directive '.skip' expects decimal operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				// skip <<operand>> bytes 
				locationCounter += strtoul(operand.GetValue().c_str(), NULL, 0);
			}
			else if (currentToken.GetValue() == WORD_DIRECTIVE)
			{
				if (sectionTable.HasFlag(currentSectionNo, SectionPermissions::BSS))
					throw AssemblerException("Directive '.word' cannot be put into secction with 'b' flag.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);

				do
				{
					Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
					currentLineTokens.pop();

					if ((operand.GetTokenType() != TokenType::OPERAND_IMMEDIATELY_DECIMAL) &&
						(operand.GetTokenType() != TokenType::OPERAND_IMMEDIATELY_HEX) && 
						(operand.GetTokenType() != TokenType::SYMBOL))
						throw AssemblerException("Directive '.word' expects decimal, hex or symbol as operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

					// skip 2 * val_byte byte
					locationCounter += 2;
				} while (!currentLineTokens.empty());
			}
			else if (currentToken.GetValue() == EQUIVALENCE_DIRECTIVE)
			{
				bool canProceed = true;	// if equ symbol can be calculated now

				Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
				currentLineTokens.pop();

				if (!sectionTable.HasFlag(currentSectionNo, SectionPermissions::DATA))
					throw AssemblerException("Directive '" + currentToken.GetValue() + "' cannot be defined outside a section with data flag.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);

				if (operand.GetTokenType() != TokenType::SYMBOL)
					throw AssemblerException("Directive '.equ' requires label-like syntax for first operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				//if (operand.GetTokenType() != TokenType::ARITHMETIC_EXPRESSION)
				//	throw AssemblerException("Directive '.equ' requires constant or arithmetic expression for second operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				string expression;
				while (currentLineTokens.size())
				{
					expression += currentLineTokens.front();
					currentLineTokens.pop();
				}

				vector<Token> arithmeticTokens = ArithmeticParser::Parse(ArithmeticParser::TokenizeExpression(expression));
				
				for (const Token& t : arithmeticTokens)
				{
					if (t.GetTokenType() == TokenType::SYMBOL)
					{
						SymbolTableEntry* entry = symbolTable.GetEntryByName(t.GetValue());

						// because of circular dependency
						if (entry && entry->tokenType == TokenType::DIRECTIVE)
							canProceed = false;

						// only symbol in current section and file can be resolved
						if (!entry || entry->sectionNumber != currentSectionNo)
							canProceed = false;
					}
				}
				
				if (canProceed)
				{
					symbolTable.InsertSymbol(operand.GetValue(),
						currentSectionNo,
						ASM_UNDEFINED,
						ArithmeticParser::CalculateSymbolValue(arithmeticTokens, symbolTable),
						ScopeType::LOCAL,
						TokenType::DIRECTIVE);
				}
				else
				{
					symbolTable.InsertSymbol(operand.GetValue(),
						currentSectionNo,
						ASM_UNDEFINED,
						ASM_UNDEFINED,
						ScopeType::LOCAL,
						TokenType::DIRECTIVE);

					tns.InsertISymbol(operand.GetValue(), currentSectionNo, expression, ScopeType::LOCAL);
				}
			}

			break;
		}
		case TokenType::SECTION:
		{
			// update existing current section size
			if (currentSectionNo != START_SECTION)
				sectionTable.GetEntryByID(currentSectionNo)->length = locationCounter;

			string flags = SectionTable::DefaultFlags(StringToSectionType(currentToken.GetValue()));
			
			if (currentToken.GetValue() != "")	// GetValue() can return (.bss|.data|.text)
			{
				currentSectionNo = sectionTable.InsertSection(currentToken.GetValue(), 0, flags, lineNumber);

				SymbolTableID symbolTableNo = symbolTable.InsertSymbol(currentToken.GetValue(),
					currentSectionNo,
					ASM_UNDEFINED,
					ASM_UNDEFINED,
					ScopeType::LOCAL,
					TokenType::SECTION);

				sectionTable.GetEntryByID(currentSectionNo)->symbolTableEntryNo = symbolTableNo;

				if (!currentLineTokens.empty())
					cout << "Warning: flags in definition of section '" << currentToken.GetValue() << "' have been ignored. To include them use '.section' directive." << endl;
			}				
			else
			{
				if (currentLineTokens.size() == 0)
					throw AssemblerException("Directive '.section' required name of section as a parameter.", ErrorCodes::SYNTAX_LABEL, lineNumber);

				Token userDefinedSection = Token::ParseToken(currentLineTokens.front(), lineNumber);
				currentLineTokens.pop();

				if (!currentLineTokens.empty())
				{
					Token t = Token::ParseToken(currentLineTokens.front(), lineNumber);
					currentLineTokens.pop();

					if (t.GetTokenType() != TokenType::FLAGS)
						throw AssemblerException("Invalid flags in section definition.", ErrorCodes::INVALID_FLAGS, lineNumber);

					flags = t.GetValue();
				}

				if (userDefinedSection.GetTokenType() != TokenType::SECTION && userDefinedSection.GetTokenType() != TokenType::SYMBOL)
					throw new AssemblerException("After '.section' directive section name is required.", ErrorCodes::SYNTAX_UNKNOWN_TOKEN, lineNumber);
				
				currentSectionNo = sectionTable.InsertSection(userDefinedSection.GetValue(), 0, flags, lineNumber);

				SymbolTableID symbolTableNo = symbolTable.InsertSymbol(userDefinedSection.GetValue(),
					currentSectionNo,
					ASM_UNDEFINED,
					ASM_UNDEFINED,
					ScopeType::LOCAL,
					TokenType::SECTION);

				sectionTable.GetEntryByID(currentSectionNo)->symbolTableEntryNo = symbolTableNo;
			}

			// all new sections begin from zero
			locationCounter = 0;

			break;
		}
		case TokenType::ACCESS_MODIFIER:
		{
			if (currentToken.GetValue() == EXTERN_MODIFIER)
			{
				do
				{
					Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
					currentLineTokens.pop();

					if (operand.GetTokenType() != TokenType::SYMBOL)
						throw AssemblerException("Symbol expected after '.extern' directive", ErrorCodes::SYNTAX_LABEL, lineNumber);

					symbolTable.InsertSymbol(operand.GetValue(),
						ASM_UNDEFINED,
						ASM_UNDEFINED,
						ASM_UNDEFINED,
						ScopeType::EXTERN,
						TokenType::LABEL);

				} while (!currentLineTokens.empty());
			}

			break;
		}
		case TokenType::INSTRUCTION:
		{
			if (sectionTable.HasFlag(currentSectionNo, SectionPermissions::BSS))
				throw AssemblerException("Instructions cannot be put into section with 'b' flag.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);
			else if (!sectionTable.HasFlag(currentSectionNo, SectionPermissions::EXECUTABLE))
				throw AssemblerException("Instructions cannot be put into section without 'x' flag.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);

			queue<Token> params;
			while (!currentLineTokens.empty())
			{
				Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
				params.push(operand);

				currentLineTokens.pop();
			}

			locationCounter += Instruction::GetInstructionSize(currentToken, params, lineNumber);

			break;
		}
		case TokenType::END_OF_FILE:
		{
			// update length of the last section
			if (currentSectionNo != START_SECTION)
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
	SectionID currentSectionNo = START_SECTION;

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

		if (currentToken.GetTokenType() == TokenType::LABEL)
		{
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
			break;
		}
		case TokenType::ACCESS_MODIFIER:
		{
			do
			{
				Token labelName = Token::ParseToken(currentLineTokens.front(), lineNumber);
				currentLineTokens.pop();

				if (labelName.GetTokenType() != TokenType::SYMBOL)
					throw AssemblerException("Symbol expected after '.extern' directive", ErrorCodes::SYNTAX_LABEL, lineNumber);

				if (currentToken.GetValue() == PUBLIC_MODIFIER)
				{
					if (symbolTable.GetEntryByName(labelName.GetValue()))
						symbolTable.GetEntryByName(labelName.GetValue())->scopeType = ScopeType::GLOBAL;
					else if (tns.GetEntryByName(labelName.GetValue()))
						tns.GetEntryByName(labelName.GetValue())->scope = ScopeType::GLOBAL;
					else
						throw AssemblerException("Symbol '" + labelName.GetValue() + "' not found to be declared as global.", ErrorCodes::SYMBOL_NOT_FOUND, lineNumber);
				}
				else if (currentToken.GetValue() == EXTERN_MODIFIER)
				{
					// done in first pass
				}
				else
					throw new AssemblerException("Access modifier directive not recognized.", ErrorCodes::SYNTAX_ACCESS_MODIFIER, lineNumber);
			} while (!currentLineTokens.empty());

			break;
		}
		case TokenType::DIRECTIVE:
		{
			if (currentToken.GetValue() == ALIGN_DIRECTIVE)
			{
				Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
				currentLineTokens.pop();

				if (operand.GetTokenType() != TokenType::OPERAND_IMMEDIATELY_DECIMAL)
					throw AssemblerException("Directive '.align' expects decimal operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				unsigned long value = strtoul(operand.GetValue().c_str(), NULL, 0);

				// TODO: optimize this with some math expression to calculate how much
				//		 to add to round so that location counter could be dividable 
				//		with 2 ^ value
				unsigned long divisibleBy = (unsigned long)pow(2.0, value);
				while ((locationCounter % divisibleBy) != 0)
				{
					locationCounter++;
					WriteToOutput(0);
				}
			}
			else if (currentToken.GetValue() == BYTE_DIRECTIVE)
			{
				if (sectionTable.HasFlag(currentSectionNo, SectionPermissions::BSS))
					throw AssemblerException("Directive '.byte' cannot be put into secction with 'b' flag.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);
				
				do
				{
					Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
					currentLineTokens.pop();

					if ((operand.GetTokenType() != TokenType::OPERAND_IMMEDIATELY_DECIMAL) &&
						(operand.GetTokenType() != TokenType::OPERAND_IMMEDIATELY_HEX))
						throw AssemblerException("Directive '.byte' expects decimal or hex operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

					unsigned long toWrite = stoi(operand.GetValue());
					if (operand.GetTokenType() == TokenType::OPERAND_IMMEDIATELY_DECIMAL)
						toWrite = stoi(operand.GetValue());
					else if (operand.GetTokenType() == TokenType::OPERAND_IMMEDIATELY_HEX)
						toWrite = stoul(operand.GetValue(), nullptr, 0);

					if (toWrite > 0xFF)
						cout << "Warning at line " << lineNumber << ": directive should be provided with a single byte, but value provided is bigger than 0xFF. Least significant eight bits will be used." << endl;

					WriteToOutput((uint8_t)toWrite);

					// skip 1 * val_byte byte
					locationCounter += 1;
				} while (!currentLineTokens.empty());
			}
			else if (currentToken.GetValue() == SKIP_DIRECTIVE)
			{
				Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
				currentLineTokens.pop();

				if (operand.GetTokenType() != TokenType::OPERAND_IMMEDIATELY_DECIMAL &&
					operand.GetTokenType() != TokenType::OPERAND_IMMEDIATELY_HEX)
					throw AssemblerException("Directive '.skip' expects decimal or hex operand.", ErrorCodes::INVALID_OPERAND, lineNumber);

				// skip <<operand>> bytes 
				long howMany = strtoul(operand.GetValue().c_str(), NULL, 0);

				if (howMany < 0)
					throw AssemblerException("Directive '.skip' expects value greated than zero.", ErrorCodes::INVALID_OPERAND, lineNumber);

				// bss section should not contain data, just annotation in symbol table that it exists
				if (!sectionTable.HasFlag(currentSectionNo, SectionPermissions::BSS))
					for (long i = 0; i < howMany; i++)
						WriteToOutput(0);

				locationCounter += howMany;
			}
			else if (currentToken.GetValue() == WORD_DIRECTIVE)
			{
				if (sectionTable.HasFlag(currentSectionNo, SectionPermissions::BSS))
					throw AssemblerException("Directive '.word' cannot be put into secction with 'b' flag.", ErrorCodes::DIRECTIVE_NOT_ALLOWED_IN_SECTION, lineNumber);

				do
				{
					Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
					currentLineTokens.pop();

					unsigned long data;
					if ((operand.GetTokenType() == TokenType::OPERAND_IMMEDIATELY_DECIMAL) || 
						(operand.GetTokenType() == TokenType::OPERAND_IMMEDIATELY_HEX)	)	
						data = strtoul(operand.GetValue().c_str(), NULL, 0);
					else if (operand.GetTokenType() == TokenType::SYMBOL)
					{
						SymbolTableEntry* entry = symbolTable.GetEntryByName(operand.GetValue());

						relocationTable.InsertRelocation(currentSectionNo,
							entry->entryNo, 
							locationCounter, 
							RelocationType::R_386_16);

						if (entry)
							data = entry->offset;
						else
							throw AssemblerException("Symbol '" + entry->name + "' not found.", ErrorCodes::SYMBOL_NOT_FOUND, lineNumber);
					}

					if (data > 0xFFFF)
						cout << "Warning at line " << lineNumber << ": directive should be provided with a word, but value provided is bigger than 0xFFFF. Least significant sixteen bits will be used." << endl;

					WriteToOutput((uint8_t)(data & 0xFF));
					WriteToOutput((uint8_t)((data >> 8) & 0xFF));

					// skip 2 * val_byte byte
					locationCounter += 2;
				} while (!currentLineTokens.empty());
			}
			else if (currentToken.GetValue() == EQUIVALENCE_DIRECTIVE)
			{
				// nothing here
			}

			break;
		}
		case TokenType::SECTION:
		{
			// also reset location counter to zero in second-pass
			locationCounter = 0;

			if (currentSectionNo != -1)
			{
				WriteToOutput("\n\n");
				currentBytesInline = 0;
			}

			currentSectionNo++;

			WriteToOutput("<!-- section '");
			WriteToOutput(sectionTable.GetEntryByID(currentSectionNo)->name);
			WriteToOutput("' -->\n");

			break;
		}
		case TokenType::INSTRUCTION:
		{
			queue<Token> params;
			while (!currentLineTokens.empty())
			{
				Token operand = Token::ParseToken(currentLineTokens.front(), lineNumber);
				params.push(operand);

				currentLineTokens.pop();
			}

			Instruction instruction(currentToken, params, lineNumber, locationCounter, currentSectionNo, symbolTable, relocationTable);
			locationCounter += instruction.GetInstructionSize(currentToken, params, lineNumber);

			WriteToOutput(instruction);

			break;
		}
		case TokenType::END_OF_FILE:
			break;
		default:
		{
			throw AssemblerException("Not allowed token detected in the second pass of assembler.", ErrorCodes::NOT_ALLOWED_TOKEN, lineNumber);
		}
		}
	}
}

void Assembler::WriteBinaryFile()
{
	size_t sizes[4];
	
	sizes[0] = symbolTable.GetSize();
	sizes[1] = sectionTable.GetSize();
	sizes[2] = relocationTable.GetSize();
	sizes[3] = tns.GetSize();

	output_file.write(reinterpret_cast<char*>(sizes), 4 * sizeof(size_t));
	output_file.write(reinterpret_cast<char*>(&contentLength), sizeof(size_t));

	// doesn't work because string is not POD type
	/*for (int i = 0; i < sizes[0]; i++)
		output_file.write(reinterpret_cast<char*>(symbolTable.GetEntryByID(i)), sizeof(SymbolTableEntry));*/
	output_file << symbolTable.Serialize().str();
	output_file << sectionTable.Serialize().str();
	output_file << relocationTable.Serialize().str();
	output_file << tns.Serialize().str();
	output_file << binaryBuffer.str();
}