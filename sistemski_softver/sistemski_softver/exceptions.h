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
	SYNTAX_NO_INITIAL_SECTION
};

class AssemblerException : public exception
{
private:
	string message;
	ErrorCodes errorCode = ErrorCodes::NOT_DEFINED;
	unsigned long location = -1;

public:
	AssemblerException(string message) noexcept : exception(), message(message) {}
	AssemblerException(string message, ErrorCodes errorCode) noexcept : exception(), errorCode(errorCode) {}
	AssemblerException(string message, ErrorCodes errorCode, unsigned long location) noexcept : exception(), errorCode(errorCode), location(location) {}

	const char* what() const noexcept override
	{
		string report = "Error";
		if (errorCode != ErrorCodes::NOT_DEFINED)
			report += " (" + to_string(int(errorCode)) + ")";
		if (location != -1)
			report += "at line " + to_string(location) + ": ";

		report += message;

		return report.c_str();
	}

	friend ostream& operator<<(ostream& out, const AssemblerException& ex)
	{
		return out << ex << endl;
	}
};

#endif