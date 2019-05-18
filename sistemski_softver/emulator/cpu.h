#ifndef _CPU_EMULATOR_H
#define _CPU_EMULATOR_H

#include <cstdint>
#include <map>
#include <mutex>
#include <thread>
#include "../common/structures.h"
#include "executable.h"
#include "interrupt.h"
#include "linker.h"

#define FLAG_Z	0x0001
#define FLAG_O	0x0002
#define FLAG_C	0x0004
#define FLAG_N	0x0008
#define FLAG_I	0x8000
#define FLAG_Tl 0x4000
#define FLAG_Tr 0x2000

#define TERMINAL_DATA_OUT 0xFF00
#define TERMINAL_DATA_IN  0xFF02

// DO NOT CHANGE THE ORDER HERE
enum InstructionMnemonic
{
	HALT = 1,
	XCHG,
	INT,
	MOV,
	ADD,
	SUB,
	MUL,
	DIV,
	CMP,
	NOT,
	AND,
	OR,
	XOR,
	TEST,
	SHL,
	SHR,
	PUSH,
	POP,
	JMP,
	JEQ,
	JNE,
	JGT,
	CALL,
	RET,
	IRET
};

static map<InstructionMnemonic, InstructionDetails> cpuInstructionsMap = {
		{HALT, InstructionDetails(0, 1)},
		{RET, InstructionDetails(0, 24, true)},
		{IRET, InstructionDetails(0, 25, true)},

		{INT, InstructionDetails(1, 3, true)},
		{NOT, InstructionDetails(1, 10)},
		{PUSH, InstructionDetails(1, 17)},
		{POP, InstructionDetails(1, 18)},
		{JMP, InstructionDetails(1, 19, true)},
		{JEQ, InstructionDetails(1, 20, true)},
		{JNE, InstructionDetails(1, 21, true)},
		{JGT, InstructionDetails(1, 22, true)},
		{CALL, InstructionDetails(1, 23, true)},

		{XCHG, InstructionDetails(2, 2)},
		{MOV, InstructionDetails(2, 4)},
		{ADD, InstructionDetails(2, 5)},
		{SUB, InstructionDetails(2, 6)},
		{MUL, InstructionDetails(2, 7)},
		{DIV, InstructionDetails(2, 8)},
		{CMP, InstructionDetails(2, 9)},
		{AND, InstructionDetails(2, 11)},
		{OR, InstructionDetails(2, 12)},
		{XOR, InstructionDetails(2, 13)},
		{TEST, InstructionDetails(2, 14)},
		{SHL, InstructionDetails(2, 15)},
		{SHR, InstructionDetails(2, 16)}
};

class CPU
{

private:
	// state of processor
	bool halted;

	Executable* executable;

	// memory access methods
	inline const uint8_t& memory_read(const uint16_t& address) { return executable->MemoryRead(address); }
	const uint16_t memory_read_16(const uint16_t& address);
	inline void memory_write(const uint16_t& address, const uint8_t& data) { executable->MemoryWrite(address, data, false); }
	
	// r0-r7 registers
	uint16_t registerFile[8];
	// pointer to registerFile[6]
	uint16_t& sp = registerFile[6];
	// pointer to registerFile[7]
	uint16_t& pc = registerFile[7];
	// r15
	uint16_t psw;

	InstructionMnemonic instructionMnemonic;
	OperandSize operandSize;
	
	// for checking if instruction is in executable section
	uint16_t pcBeforeInstruction = 0;

	AddressingType operand1AddressingType;
	ByteSelector operand1ByteSelector;
	uint16_t operand1;
	uint8_t registerSelector1;

	AddressingType operand2AddressingType;
	ByteSelector operand2ByteSelector;
	uint16_t operand2;
	uint8_t registerSelector2;

	void ResolveAddressing(uint8_t rawData, Operand op);
	uint16_t& GetReference(Operand op);

	void InstructionFetchAndDecode();
	void InstructionExecute();
	void InstructionHandleInterrupt();

	inline void memory_push(const uint8_t& data) 
	{ 
		if (sp > MEMORY_MAPPED_REGISTERS_START)
			throw EmulatorException("Stack underflow.", ErrorCodes::EMULATOR_STACK_UNDERFLOW);

		memory_write(--sp, data); 
	}
	inline void memory_push_16(const uint16_t& data) { memory_push((data >> 8) & 0xFF); memory_push(data & 0xFF); }
	inline uint8_t memory_pop() { return memory_read(sp++); }
	inline uint16_t memory_pop_16() { uint16_t r = memory_pop(); r = r | (memory_pop() << 8); return r; }

	inline void SetFlagsZN(uint8_t flags, int16_t result);
	inline void SetFlagO(int16_t src, int16_t dst, int16_t r, InstructionMnemonic operation);
	inline void SetFlagC(int16_t src, int16_t dst, int16_t r, InstructionMnemonic operation);

	// interrupts
	mutex emulatorStatusMutex;
	mutex memoryMutex;

	thread* keyboardThread;

	inline bool GetZ() { return psw & FLAG_Z; }
	inline bool GetO() { return psw & FLAG_O; }
	inline bool GetC() { return psw & FLAG_C; }
	inline bool GetN() { return psw & FLAG_N; }
	inline bool GetTr() { return psw & FLAG_Tr; }
	inline bool GetTl() { return psw & FLAG_Tl; }
	inline bool GetI() { return psw & FLAG_I; }


public:
	CPU();
	~CPU();

	inline thread& GetKeyboardThread() { return *keyboardThread; }
	inline mutex& GetEmulatorStatusMutex() { return emulatorStatusMutex; }
	inline mutex& GetMemoryMutex() { return memoryMutex; }
	inline const bool& GetHaltedStatus() { return halted; }
	void WriteIO(const uint16_t& address, const uint8_t& data);

	friend class Emulator;
};

#endif