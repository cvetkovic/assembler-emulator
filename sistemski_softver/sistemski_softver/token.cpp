#include "token.h"

TokenType Token::GetTokenType() const
{
	return tokenType;
}

string Token::GetValue() const
{
	return value;
}

Token Token::ParseToken(string data, unsigned long lineNumber)
{
	if (data.size() == 0)
		Token(TokenType::INVALID, 0);

	data = regex_replace(data, regex("^sp"), "r6");
	data = regex_replace(data, regex("^pc"), "r7");
	data = regex_replace(data, regex("^psw"), "r15");

	bool memoryDirect = false;
	bool immediatelySymbol = false;
	bool pcRelativeSymbol = false;
	if (data.at(0) == '*')
	{
		data = data.substr(1, data.size() - 1);
		memoryDirect = true;
	}
	else if (data.at(0) == '&')
	{
		data = data.substr(1, data.size() - 1);
		immediatelySymbol = true;
	}
	else if (data.at(0) == '$')
	{
		data = data.substr(1, data.size() - 1);
		pcRelativeSymbol = true;
	}

	for (const regex& p : staticAssemblyParsers)
	{
		if (regex_match(data, p))
		{
			TokenType r1;
			string r2;

			if (&p == &staticAssemblyParsers[0])
			{
				r1 = TokenType::ACCESS_MODIFIER;
				r2 = data;
			}
			else if (&p == &staticAssemblyParsers[1])
			{
				r1 = TokenType::LABEL;
				r2 = data.substr(0, data.length() - 1);
			}
			else if (&p == &staticAssemblyParsers[2])
			{
				r1 = TokenType::SECTION;
				if (data == ".section")
					r2 = "";
				else
					r2 = data;
			}
			else if (&p == &staticAssemblyParsers[3])
			{
				r1 = TokenType::DIRECTIVE;
				r2 = data;
			}
			else if (&p == &staticAssemblyParsers[4])
			{
				r1 = TokenType::INSTRUCTION;
				r2 = data;
			}
			else if (&p == &staticAssemblyParsers[5])
			{
				r1 = TokenType::OPERAND_REGISTER_DIRECT;
				r2 = data;
			}
			else if (&p == &staticAssemblyParsers[6])
			{
				r1 = TokenType::END_OF_FILE;
				r2 = data;
			}
			else if (&p == &staticAssemblyParsers[7])
			{
				if (!memoryDirect)
					r1 = TokenType::OPERAND_IMMEDIATELY_DECIMAL;
				else
					r1 = TokenType::OPERAND_MEMORY_DIRECT_DECIMAL;

				r2 = data;
			}
			else if (&p == &staticAssemblyParsers[8])
			{
				if (!memoryDirect)
					r1 = TokenType::OPERAND_IMMEDIATELY_HEX;
				else
					r1 = TokenType::OPERAND_MEMORY_DIRECT_HEX;

				r2 = data;
			}
			else if (&p == &staticAssemblyParsers[9])	// OPERAND_REGISTER has to go before SYMBOL
			{
				if (immediatelySymbol)
					r1 = TokenType::OPERAND_IMMEDIATELY_SYMBOL;
				else if (pcRelativeSymbol)
					r1 = TokenType::OPERAND_PC_RELATIVE_SYMBOL;
				else
					r1 = TokenType::SYMBOL;

				r2 = data;
			}
			else if (&p == &staticAssemblyParsers[10])
			{
				r1 = TokenType::OPERAND_REGISTER_INDIRECT;
				r2 = data;
			}
			else if (&p == &staticAssemblyParsers[11])
			{
				r1 = TokenType::FLAGS;
				r2 = data.substr(1, data.size() - 2);
			}

			return Token(r1, r2);
		}
	}

	throw AssemblerException("Parser cannot process unrecognized token '" + data + "'.", ErrorCodes::SYNTAX_UNKNOWN_TOKEN, lineNumber);
}

ostream & operator<<(ostream && out, const Token & token)
{
	return out << token.GetValue();
}
