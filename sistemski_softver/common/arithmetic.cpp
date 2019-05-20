#include "arithmetic.h"

bool ArithmeticParser::IsOperator(char c)
{
	switch (c)
	{
	case '+':
	case '-':
	case '*':
	case '/':
	case '^':
	case '(':
	case ')':
		return true;
	default:
		return false;
	}
}

int ArithmeticParser::GetPriority(char c, bool stack)
{
	switch (c)
	{
	case '+':
		return 2;
	case '-':
		return 2;
	case '*':
		return 3;
	case '/':
		return 3;
	case '^':
		if (stack)
			return 4;
		else
			return 5;
	case '(':
		if (stack)
			return 0;
		else
			return 6;
	case ')':
		return 1;
	default:
		return 0;
	}
}

int ArithmeticParser::GetRank(char c)
{
	return (c == '(' || c == ')') ? 0 : -1;
}

vector<Token> ArithmeticParser::Parse(vector<Token> input)
{
	vector<Token> result;
	stack<Token> stack;
	
	int rank = 0;

	for (Token next : input)
	{
		if (next.GetTokenType() != TokenType::ARITHMETIC_OPERATOR)
		{
			result.push_back(next);
			rank++;
		}
		else
		{
			while ((!stack.empty()) && (GetPriority(next.GetValue()[0], false) <= GetPriority(stack.top().GetValue()[0], true)))
			{
				Token c = stack.top();
				stack.pop();
				result.push_back(c);
				rank += GetRank(c.GetValue()[0]);

				if (rank < 1)
					throw AssemblerException("Error parsing.");
			}

			if (next.GetValue()[0] != ')')
				stack.push(next);
			else
				stack.pop();
		}
	}

	while (!stack.empty())
	{
		Token c = stack.top();
		stack.pop();
		result.push_back(c);
		rank += GetRank(c.GetValue()[0]);
	}

	if (rank < 1)
		throw AssemblerException("Error parsing.");

	return result;
}

Token ArithmeticParser::ReturnToken(string data)
{
	Token t = Token::ParseToken(data, 0);
	if (t.GetTokenType() == TokenType::ARITHMETIC_OPERATOR || 
		t.GetTokenType() == TokenType::SYMBOL ||
		t.GetTokenType() == TokenType::OPERAND_IMMEDIATELY_DECIMAL ||
		t.GetTokenType() == TokenType::OPERAND_IMMEDIATELY_HEX)
		return t;
	
	throw AssemblerException("");
}

vector<Token> ArithmeticParser::TokenizeExpression(string expression)
{
	vector<Token> result;
	string temp;

	for (char c : expression)
	{
		if (!IsOperator(c))
			temp += c;
		else
		{
			if (temp != "")
				result.push_back(ReturnToken(temp));
			result.push_back(ReturnToken(string(1, c)));
			temp = "";
		}
	}

	if (temp != "")
		result.push_back(ReturnToken(temp));
	
	return result;
}

unsigned long ArithmeticParser::CalculateSymbolValue(vector<Token> tokens, SymbolTable& symbolTable, bool linker, unsigned long section)
{
	stack<Token> tmp;
	unsigned long rez = 0;

	for (Token& t : tokens)
	{
		if (t.GetTokenType() == TokenType::SYMBOL || 
			t.GetTokenType() == TokenType::OPERAND_IMMEDIATELY_DECIMAL ||
			t.GetTokenType() == TokenType::OPERAND_IMMEDIATELY_HEX)
			tmp.push(t);
		else if (t.GetTokenType() == TokenType::ARITHMETIC_OPERATOR)
		{
			Token op2 = tmp.top();
			tmp.pop();
			Token op1 = tmp.top();
			tmp.pop();

			const SymbolTableEntry* s2 = symbolTable.GetEntryByName(op2.GetValue());
			const SymbolTableEntry* s1 = symbolTable.GetEntryByName(op1.GetValue());

			if ((s1 == 0 && op1.GetTokenType() == TokenType::SYMBOL) ||
				(s2 == 0 && op2.GetTokenType() == TokenType::SYMBOL) ||
				(linker == false && s1 != 0 && s2 != 0 && s1->sectionNumber != s2->sectionNumber) ||
				(linker == false && s1 != 0 && section != -1 && s1->sectionNumber != section) ||
				(linker == false && s2 != 0 && section != -1 && s2->sectionNumber != section))
				throw exception();

			unsigned long v2;
			if (s2)
				v2 = s2->offset;
			else
				v2 = strtoul(op2.GetValue().c_str(), NULL, 0);

			unsigned long v1;
			if (s1)
				v1 = s1->offset;
			else
				v1 = strtoul(op1.GetValue().c_str(), NULL, 0);

			if (t.GetValue() == "+")
				rez = v1 + v2;
			else if (t.GetValue() == "-")
				rez = v1 - v2;
			else if (t.GetValue() == "*")
				rez = v1 * v2;
			else if (t.GetValue() == "/")
				rez = v1 / v2;
			else if (t.GetValue() == "^")
				rez = (unsigned long)pow(v1, v2);

			char buffer[256];
			sprintf(buffer, "%lu", rez);

			tmp.push(Token(TokenType::OPERAND_IMMEDIATELY_DECIMAL, string(buffer)));
		}
	}

	rez = strtoul(tmp.top().GetValue().c_str(), NULL, 0);
	tmp.pop();

	if (tmp.empty())
		return rez;
	else
		throw AssemblerException("Invalid postfix arithmetic expression.");
}

stringstream ArithmeticParser::Serialize(map<string, vector<Token>> table)
{
	stringstream output;



	return output;
}