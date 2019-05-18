#include "interrupt.h"

void KeyboardHandler(CPU* processor)
{
	bool running = false;

	mutex& statusMutex = processor->GetEmulatorStatusMutex();
	mutex& memoryMutex = processor->GetMemoryMutex();

	while (1)
	{
		statusMutex.lock();
		// TODO: add if processor has finished initialiation
		if (processor->GetHaltedStatus())
		{
			statusMutex.unlock();
			break;	// exit from this loop
		}
		statusMutex.unlock();

		char input = getchar();

		memoryMutex.lock();
		if (!processor->GetHaltedStatus())
			processor->WriteIO(TERMINAL_DATA_IN, input);
		else
			running = false;
		memoryMutex.unlock();

		//if (running)
			//RegisterInterrupt();
	}
}
