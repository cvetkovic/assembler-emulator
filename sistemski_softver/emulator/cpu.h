#ifndef _CPU_EMULATOR_H
#define _CPU_EMULATOR_H

#define MEMORY_ADDRESS_SIZE 2^16

class CPU
{
	char memory;

	short registry;
	short psw;

	bool halted;
};

#endif