#include "interrupt.h"

void KeyboardHandler(CPU* processor)
{
	mutex& statusMutex = processor->GetEmulatorStatusMutex();

	while (1)
	{
		statusMutex.lock();
		// TODO: switch to pthreads once on linux to be able to cancel threads in ~CPU::CPU()
		if (processor->GetHaltedStatus() && processor->GetInitializationFinished())
		{
			statusMutex.unlock();
			break;	// exit from this loop
		}
		statusMutex.unlock();

		char input = getchar();
		bool interruptRequested = false;

		if (!processor->GetHaltedStatus())
		{
			// memory mutex is in CPU class
			processor->WriteIO(TERMINAL_DATA_IN, input);
			interruptRequested = true;
		}

		if (interruptRequested)
			processor->SetInterrupt(InterruptType::KEYBOARD);
	}
}

void TimerHandler(CPU* processor)
{
	uint16_t sleepTimeMs;
	mutex& statusMutex = processor->GetEmulatorStatusMutex();
		
	while (1)
	{
		switch (processor->memory_read(TIMER_CFG))
		{
		case 0x0:
			sleepTimeMs = 500;
			break;
		case 0x1:
			sleepTimeMs = 1000;
			break;
		case 0x2:
			sleepTimeMs = 1500;
			break;
		case 0x3:
			sleepTimeMs = 2000;
			break;
		case 0x4:
			sleepTimeMs = 5000;
			break;
		case 0x5:
			sleepTimeMs = 10000;
			break;
		case 0x6:
			sleepTimeMs = 30000;
			break;
		case 0x7:
			sleepTimeMs = 60000;
			break;
		}

		this_thread::sleep_for(chrono::milliseconds(sleepTimeMs));

		statusMutex.lock();
		if (processor->GetHaltedStatus() && processor->GetInitializationFinished())
		{
			statusMutex.unlock();
			break;	// exit from this loop
		}
		statusMutex.unlock();

		processor->SetInterrupt(InterruptType::TIMER);
	}
}