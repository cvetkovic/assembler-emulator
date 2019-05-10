#include <iostream>
#include <fstream>

#include "assembler.h"
#include "exceptions.h"

using namespace std;

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		cerr << "Invalid main() invocation parameters" << endl;
		cout << "Invalid program call parameters. Syntax is ./c_as -o output_file input_file" << endl;

		return 1;
	}

	try
	{
		if (string(argv[1]) == "-o")
		{
			Assembler assembler(argv[2], argv[1]);
			assembler.GenerateObjectFile();
		}

		return 0;
	}
	catch (const AssemblerException& ex)
	{
		cout << ex << endl;
	}
	catch (const exception& ex)
	{
		cout << "Unknown assembly error: " << endl;
		cout << ex.what() << endl;
		cout << endl;
	}

	return 1;
}
