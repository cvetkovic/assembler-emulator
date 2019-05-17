#include "emulator.h"

void Emulator::InitializeCPU()
{
	processor.memory = executable->memory;
	processor.sp = 0xFFFF;
	processor.pc = executable->initialPC;
}

void Emulator::Run()
{
	InitializeCPU();

	while (!processor.halted)
	{
		processor.InstructionFetchAndDecode();
		processor.InstructionExecute();
		processor.InstructionHandleInterrupt();
	}
}
