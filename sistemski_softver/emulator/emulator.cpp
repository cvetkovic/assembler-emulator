#include "emulator.h"

Emulator::~Emulator()
{
	delete executable;
}

inline void Emulator::InitializeCPU()
{
	processor.executable = this->executable;

	processor.pc = processor.memory_read_16(0);
	Run();
	// initial stack pointer is set by initialization code 
	// processor.sp = 0xFFFF;
	processor.pc = executable->initialPC;
	processor.halted = false;
}

inline void Emulator::Run()
{
	while (!processor.halted)
	{
		processor.InstructionFetchAndDecode();
		processor.InstructionExecute();
		processor.InstructionHandleInterrupt();
	}
}

void Emulator::Start()
{
	InitializeCPU();
	Run();
}