#include <iostream>
#include <fstream>

#include "assembler.h"
#include "exceptions.h"

using namespace std;

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		cout << "Invalid program call parameters. Syntax is ./c_as -o output_file input_file" << endl;

		return 1;
	}

	try
	{
		if (string(argv[1]) == "-o")
		{
			if (argc % 2 != 0)
				cout << "Each input file has to have an output file.";

			int numberOfFiles = (argc - 2) / 2;
			for (int i = 0; i < numberOfFiles; i++)
			{
				// TODO: test this
				Assembler assembler(argv[2 + numberOfFiles + i], argv[2 + i]);
				assembler.GenerateObjectFile();
			}

			cout << "Output successfully generated." << endl;
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