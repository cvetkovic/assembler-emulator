#ifndef _EMULATOR_EMULATOR_H
#define _EMULATOR_EMULATOR_H

#include "cpu.h"
#include "executable.h"
#include "linker.h"

class Emulator
{

private:
	CPU processor;
	Executable* executable;

	inline void InitializeCPU();
	inline void Run();

public:
	Emulator(Executable* executable) : executable(executable) {}
	~Emulator();

	void Start();
};

#endif