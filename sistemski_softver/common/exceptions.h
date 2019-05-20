#ifndef ASSEMBLER_EXCEPTION_ASSEMBLER_H_
#define ASSEMBLER_EXCEPTION_ASSEMBLER_H_

#include <cstring>
#include <iostream>
#include <string>
using namespace std;

enum ErrorCodes
{
	NOT_DEFINED = 0,
	IO_INPUT_EXCEPTION,
	IO_OUTPUT_EXCEPTION,

	SYMBOL_NOT_FOUND,
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
	INVALID_INSTRUCTION,
	SECTION_ALREADY_DEFINED,
	INVALID_FLAGS,
	SYNTAX_ERROR,

	LINKER_NO_FILES = 500,
	LINKER_CANNOT_OPEN,
	LINKER_SECTION_ADDRESS_UNSPECIFIED,
	LINKER_SYMBOL_NOT_DEFINED, 
	LINKER_NO_START,
	LINKER_UNCONSISTENT_FLAGS,
	LINKER_SECTION_OVERLAPPING,
	LINKER_MULTIPLE_SYMBOL_DEFINITION,
	LINKER_MEMORY_MAPPED_CONSTRAINT,
	LINKER_IVT_CONSTRAINT,
	LINKER_INVALID_RELOCATION_SCOPE,
	LINKER_SECTION_MISSING,
	LINKER_EQU_RESOLVE_ERROR,

	EMULATOR_UNKNOWN_INSTRUCTION = 1000,
	EMULATOR_UNKNOWN_ADDRESSING,
	EMULATOR_SEGMENTATION_FAULT,
	EMULATOR_NON_EXECUTABLE_SECTION,
	EMULATOR_STACK_UNDERFLOW,
	EMULATOR_SECTION_MISSING
};

class AssemblerException : public exception
{
private:
	string message;
	ErrorCodes errorCode = ErrorCodes::NOT_DEFINED;
	unsigned long location = -1;

	char** a = new char*[1];

public:
	AssemblerException(string message) : exception(), message(message) {}
	AssemblerException(string message, ErrorCodes errorCode) : exception(), message(message), errorCode(errorCode) {}
	AssemblerException(string message, ErrorCodes errorCode, unsigned long location) : exception(), message(message), errorCode(errorCode), location(location) {}
	~AssemblerException()
	{
		// TODO: check if deletion is OK
		delete[] a[0];
		delete a;
	}

	const char* what()
	{
		string report = "Error";
		if (errorCode != ErrorCodes::NOT_DEFINED)
			report += " (" + to_string(int(errorCode)) + ")";
		if (location != -1)
			report += " at line " + to_string(location) + ": ";
		else
			report += ": ";

		report += message;

		char *cstr = new char[report.length() + 1];
		strcpy(cstr, report.c_str());
		a[0] = cstr;

		// cannot return report.c_str() because it is stored on stack
		return cstr;
	}
};

class LinkerException : public exception
{
private:
	string message;
	ErrorCodes errorCode = ErrorCodes::NOT_DEFINED;

	char** a = new char*[1];

public:
	LinkerException(string message) : exception(), message(message) {}
	LinkerException(string message, ErrorCodes errorCode) : exception(), message(message), errorCode(errorCode) {}
	~LinkerException()
	{
		// TODO: check if deletion is OK
		delete[] a[0];
		delete a;
	}

	const char* what()
	{
		string report = "Error";
		if (errorCode != ErrorCodes::NOT_DEFINED)
			report += " (" + to_string(int(errorCode)) + "): ";
		else
			report += ": ";

		report += message;

		char *cstr = new char[report.length() + 1];
		strcpy(cstr, report.c_str());
		a[0] = cstr;

		// cannot return report.c_str() because it is stored on stack
		return cstr;
	}
};

class EmulatorException : public exception
{
private:
	string message;
	ErrorCodes errorCode = ErrorCodes::NOT_DEFINED;

	char** a = new char*[1];

public:
	EmulatorException(string message) : exception(), message(message) {}
	EmulatorException(string message, ErrorCodes errorCode) : exception(), message(message), errorCode(errorCode) {}
	~EmulatorException()
	{
		// TODO: check if deletion is OK
		delete[] a[0];
		delete a;
	}

	const char* what()
	{
		string report = "Error";
		if (errorCode != ErrorCodes::NOT_DEFINED)
			report += " (" + to_string(int(errorCode)) + "): ";
		else
			report += ": ";

		report += message;

		char *cstr = new char[report.length() + 1];
		strcpy(cstr, report.c_str());
		a[0] = cstr;

		// cannot return report.c_str() because it is stored on stack
		return cstr;
	}
};

#endif