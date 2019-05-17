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
		processor.InstructionFetch();
		processor.InstructionDecode();
		processor.InstructionExecute();
		processor.InstructionHandleInterrupt();
	}
}
