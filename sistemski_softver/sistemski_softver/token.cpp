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
	for (const regex& p : staticAssemblyParsers)
	{
		if (regex_match(data, p))
		{
			TokenType r1;
			string r2;

			if (&p == &staticAssemblyParsers[0])
			{
				r1 = TokenType::ACCESS_MODIFIER;
				r2 = data.substr(1);				// from 1 to end
			}
			else if (&p == &staticAssemblyParsers[1])
			{
				r1 = TokenType::LABEL;
				r2 = data.substr(0, data.length() - 1);
			}
			else if (&p == &staticAssemblyParsers[2])
			{
				r1 = TokenType::SECTION;
				if (data == "section")
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
				r1 = TokenType::SYMBOL;
				r2 = data;
			}
			else if (&p == &staticAssemblyParsers[6])
			{
				r1 = TokenType::END_OF_FILE;
				r2 = data;
			}
			else if (&p == &staticAssemblyParsers[7])
			{
				r1 = TokenType::OPERAND_DECIMAL;
				r2 = data;
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
