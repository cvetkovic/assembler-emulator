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
	// initial stack pointer is set by interrupt vector #0
	// processor.sp = 0xFFFF;
	processor.pc = executable->initialPC;
	processor.initializationFinished = true;
	processor.halted = false;
	processor.StartThreads();
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