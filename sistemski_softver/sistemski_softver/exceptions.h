#ifndef ASSEMBLER_EXCEPTION_ASSEMBLER_H_
#define ASSEMBLER_EXCEPTION_ASSEMBLER_H_

#include <iostream>
#include <string>
using namespace std;

enum ErrorCodes
{
	NOT_DEFINED = 0,
	IO_INPUT_EXCEPTION = 1,
	IO_OUTPUT_EXCEPTION = 2,

	SYMBOL_EXISTS,
	NOT_ALLOWED_TOKEN,
	SYNTAX_LABEL,
	SYNTAX_UNKNOWN_TOKEN,
	SYNTAX_NO_INITIAL_SECTION,
	DIRECTIVE_NOT_ALLOWED_IN_SECTION,
	INVALID_OPERAND,
	INVALID_INSTRUCTION_SECTION,
	INVALID_ACCESS_MODIFIER_SECTION,
	SYNTAX_ACCESS_MODIFIER,
	INVALID_INSTRUCTION
};

class AssemblerException : public exception
{
private:
	string message;
	ErrorCodes errorCode = ErrorCodes::NOT_DEFINED;
	unsigned long location = -1;

	char** a = new char*[1];

public:
	AssemblerException(string message) noexcept : exception(), message(message) {}
	AssemblerException(string message, ErrorCodes errorCode) noexcept : exception(), message(message), errorCode(errorCode) {}
	AssemblerException(string message, ErrorCodes errorCode, unsigned long location) noexcept : exception(), message(message), errorCode(errorCode), location(location) {}
	~AssemblerException()
	{
		// TODO: check if deletion is OK
		delete[] a[0];
		delete a;
	}

	const char* what() const noexcept override
	{
		string report = "Error";
		if (errorCode != ErrorCodes::NOT_DEFINED)
			report += " (" + to_string(int(errorCode)) + ")";
		if (location != -1)
			report += " at line " + to_string(location) + ": ";

		report += message;

		char *cstr = new char[report.length() + 1];
		strcpy(cstr, report.c_str());
		a[0] = cstr;

		// cannot return report.c_str() because it is stored on stack
		return cstr;
	}

	friend ostream& operator<<(ostream& out, const AssemblerException& ex)
	{
		return out << ex.what() << endl;
	}
};

#endif