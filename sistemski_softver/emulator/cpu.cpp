#include "cpu.h"

const uint16_t CPU::memory_read_16(const uint16_t & address)
{
	uint16_t r = memory_read(address);
	r = r | (memory_read(address + 1) << 8);
	return r;
}

void CPU::ResolveAddressing(uint8_t rawData, Operand op)
{
	if (interruptRequests.size() != 0 && interruptRequests.top() == InterruptType::INT_INVALID_INSTRUCTION)
		return;

	uint16_t& operand = (op == Operand::FIRST_OPERAND ? operand1 : operand2);
	ByteSelector& byteSelector = (op == Operand::FIRST_OPERAND ? operand1ByteSelector : operand2ByteSelector);
	uint8_t& registerSelector = (op == Operand::FIRST_OPERAND ? registerSelector1 : registerSelector2);
	AddressingType addressingType = static_cast<AddressingType>(rawData >> 5);

	switch (addressingType)
	{
	case AddressingType::IMMEDIATELY:
	{
		operand = memory_read(pc++); // lower bytes
		if (operandSize == OperandSize::WORD)
			operand = (operand | (memory_read(pc++) << 8)); // higher bytes
		break;
	}
	case AddressingType::REGISTER_DIRECT:
	{
		registerSelector = ((rawData >> 1) & 0x0F);
		if (operandSize == OperandSize::BYTE)
			byteSelector = static_cast<ByteSelector>(rawData & 0x01);
		else
			byteSelector = ByteSelector::NOT_APPLICABLE;
		break;
	}
	case AddressingType::REGISTER_INDIRECT_NO_OFFSET:
	{
		registerSelector = ((rawData >> 1) & 0x0F);
		break;
	}
	case AddressingType::REGISTER_INDIRECT_8_BIT_OFFSET:
	{
		registerSelector = ((rawData >> 1) & 0x0F);
		operand = memory_read(pc++); // lower bytes
		break;
	}
	case AddressingType::REGISTER_INDIRECT_16_BIT_OFFSET:
	{
		registerSelector = ((rawData >> 1) & 0x0F);
		operand = memory_read(pc++); // lower bytes
		operand = (operand | (memory_read(pc++) << 8)); // higher bytes
		break;
	}
	case AddressingType::MEMORY_DIRECT:
	{
		operand = memory_read(pc++); // lower bytes
		operand = (operand | (memory_read(pc++) << 8)); // higher bytes
		break;
	}
	default:
		SetInterrupt(InterruptType::INT_INVALID_INSTRUCTION);
		break;
		//throw EmulatorException("Unknown addressing type.", ErrorCodes::EMULATOR_UNKNOWN_ADDRESSING);
	}

	if (op == Operand::FIRST_OPERAND)
		operand1AddressingType = addressingType;
	else
		operand2AddressingType = addressingType;
}

uint16_t& CPU::GetReference(Operand op)
{
	AddressingType& addressingType = (op == Operand::FIRST_OPERAND ? operand1AddressingType : operand2AddressingType);
	ByteSelector& byteSelector = (op == Operand::FIRST_OPERAND ? operand1ByteSelector : operand2ByteSelector);
	uint16_t& operand = (op == Operand::FIRST_OPERAND ? operand1 : operand2);
	uint8_t& registerSelector = (op == Operand::FIRST_OPERAND ? registerSelector1 : registerSelector2);

	switch (addressingType)
	{
	case AddressingType::IMMEDIATELY:
	{
		if (op == FIRST_OPERAND && cpuInstructionsMap.at(instructionMnemonic).numberOfOperands == 2)
			SetInterrupt(InterruptType::INT_INVALID_INSTRUCTION);
			//throw EmulatorException("Immediately addressed operand cannot be destination.");

		return operand;
	}
	case AddressingType::REGISTER_DIRECT:
		return registerFile[registerSelector];
	case AddressingType::REGISTER_INDIRECT_NO_OFFSET:
		return (uint16_t&)memory_read(registerFile[registerSelector]);
	case AddressingType::REGISTER_INDIRECT_8_BIT_OFFSET:
		return (uint16_t&)memory_read(registerFile[registerSelector] + (int8_t)(operand & 0xFF));
	case AddressingType::REGISTER_INDIRECT_16_BIT_OFFSET:
		return (uint16_t&)memory_read(registerFile[registerSelector] + (int16_t)operand);
	case AddressingType::MEMORY_DIRECT:
		return (uint16_t&)memory_read(operand);
	default:
		SetInterrupt(InterruptType::INT_INVALID_INSTRUCTION);
		break;
	}

	throw EmulatorException("Unknown addressing type.", ErrorCodes::EMULATOR_UNKNOWN_ADDRESSING);
}

void CPU::InstructionFetchAndDecode()
{
	uint8_t IP;
	int writeTo = 0;
	pcBeforeInstruction = pc;

	IP = memory_read(pc++);
	uint8_t instructionCode = ((IP >> 3) & 0x1F);
	uint8_t size = ((IP & 0x04) >> 2);
	if (instructionCode >= 1 && instructionCode <= 25)
	{
		instructionMnemonic = static_cast<InstructionMnemonic>(instructionCode);
		operandSize = static_cast<OperandSize>(size);
	}
	else
		SetInterrupt(InterruptType::INT_INVALID_INSTRUCTION);
		//throw EmulatorException("Unknown operation code detected.", ErrorCodes::EMULATOR_UNKNOWN_INSTRUCTION);

	InstructionDetails& details = cpuInstructionsMap.at(instructionMnemonic);

	switch (details.numberOfOperands)
	{
	case 0:
	{
		// nothing here
		break;
	}
	case 1:
	{
		uint8_t m = memory_read(pc++);
		ResolveAddressing(m, Operand::FIRST_OPERAND);
		break;
	}
	case 2:
	{
		uint8_t m1 = memory_read(pc++);
		ResolveAddressing(m1, Operand::FIRST_OPERAND);
		uint8_t m2 = memory_read(pc++);
		ResolveAddressing(m2, Operand::SECOND_OPERAND);
		break;
	}
	default:
		SetInterrupt(InterruptType::INT_INVALID_INSTRUCTION);
		break;
		//throw EmulatorException("Unknown instruction addressing field.", ErrorCodes::EMULATOR_UNKNOWN_INSTRUCTION);
	}
}

void CPU::InstructionExecute()
{
	if (!executable->CheckIfExecutable(pcBeforeInstruction, pc - pcBeforeInstruction))
		EmulatorException("Loaded code is not in executable section. Emulation aborted.", ErrorCodes::EMULATOR_NON_EXECUTABLE_SECTION);
	
	if (interruptRequests.size() != 0 && interruptRequests.top() == InterruptType::INT_INVALID_INSTRUCTION)
		return;

	switch (instructionMnemonic)
	{
	case InstructionMnemonic::HALT:
	{
		halted = true;
		break;
	}
	case InstructionMnemonic::XCHG:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		uint16_t temp = dst;
		dst = src;
		src = temp;
		break;
	}
	case InstructionMnemonic::INT:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);

		memory_push_16(psw);
		pc = memory_read((dst % 8) << 1);

		break;
	}
	case InstructionMnemonic::MOV:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst = src;
		SetFlagsZN(FLAG_Z | FLAG_N, (int16_t)dst);
		break;
	}
	case InstructionMnemonic::ADD:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		SetFlagO(src, dst, dst + src, InstructionMnemonic::ADD);
		SetFlagC(src, dst, dst + src, InstructionMnemonic::ADD);
		dst += src;
		SetFlagsZN(FLAG_Z | FLAG_N, (int16_t)dst);
		break;
	}
	case InstructionMnemonic::SUB:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		SetFlagO(src, dst, dst - src, InstructionMnemonic::SUB);
		SetFlagC(src, dst, dst - src, InstructionMnemonic::SUB);
		dst -= src;
		SetFlagsZN(FLAG_Z | FLAG_N, (int16_t)dst);
		break;
	}
	case InstructionMnemonic::MUL:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst *= src;
		SetFlagsZN(FLAG_Z | FLAG_N, (int16_t)dst);
		break;
	}
	case InstructionMnemonic::DIV:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst /= src;
		SetFlagsZN(FLAG_Z | FLAG_N, (int16_t)dst);
		break;
	}
	case InstructionMnemonic::CMP:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		uint16_t temp = dst - src;
		SetFlagsZN(FLAG_Z | FLAG_N, (int16_t)temp);
		SetFlagO(src, dst, temp, InstructionMnemonic::CMP);
		SetFlagC(src, dst, temp, InstructionMnemonic::CMP);
		break;
	}
	case InstructionMnemonic::NOT:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		dst = !dst;
		SetFlagsZN(FLAG_Z | FLAG_N, (int16_t)dst);
		break;
	}
	case InstructionMnemonic::AND:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst = dst & src;
		SetFlagsZN(FLAG_Z | FLAG_N, (int16_t)dst);
		break;
	}
	case InstructionMnemonic::OR:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst = dst | src;
		SetFlagsZN(FLAG_Z | FLAG_N, (int16_t)dst);
		break;
	}
	case InstructionMnemonic::XOR:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst = dst ^ src;
		SetFlagsZN(FLAG_Z | FLAG_N, (int16_t)dst);
		break;
	}
	case InstructionMnemonic::TEST:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		uint16_t temp = dst & src;
		SetFlagsZN(FLAG_Z | FLAG_N, (int16_t)temp);
		break;
	}
	case InstructionMnemonic::SHL:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		SetFlagC(src, dst, dst << src, InstructionMnemonic::CMP);
		dst = dst << src;
		SetFlagsZN(FLAG_Z | FLAG_N, (int16_t)dst);
		break;
	}
	case InstructionMnemonic::SHR:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		SetFlagC(src, dst, dst >> src, InstructionMnemonic::CMP);
		dst = dst >> src;
		SetFlagsZN(FLAG_Z | FLAG_N, (int16_t)dst);
		break;
	}
	case InstructionMnemonic::PUSH:
	{
		uint16_t& src = GetReference(Operand::FIRST_OPERAND);
		if (operandSize == OperandSize::BYTE)
			memory_push(src & 0xFF);
		else
			memory_push_16(src);
		break;
	}
	case InstructionMnemonic::POP:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		if (operandSize == OperandSize::BYTE)
			dst = memory_pop();
		else
			dst = memory_pop_16();
		break;
	}
	case InstructionMnemonic::JMP:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		pc = dst;
		break;
	}
	case InstructionMnemonic::JEQ:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		if (GetZ())
			pc = dst;
		break;
	}
	case InstructionMnemonic::JNE:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		if (!GetZ())
			pc = dst;
		break;
	}
	case InstructionMnemonic::JGT:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		if (GetZ() && (GetO() == !GetN()))	// ZERO=0 && (OVERFLOW=SIGNED)
			pc = dst;
		break;
	}
	case InstructionMnemonic::CALL:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		memory_push_16(pc);
		pc = dst;
		break;
	}
	case InstructionMnemonic::RET:
	{
		pc = memory_pop_16();
		break;
	}
	case InstructionMnemonic::IRET:
	{
		psw = memory_pop_16();
		pc = memory_pop_16();
		break;
	}
	default:
		SetInterrupt(InterruptType::INT_INVALID_INSTRUCTION);
		break;
		//throw EmulatorException("Unknown instruction.", ErrorCodes::EMULATOR_UNKNOWN_INSTRUCTION);
	}
}

void CPU::InstructionHandleInterrupt()
{
	char c;
	memoryMutex.lock();
	if (c = (char)memory_read(TERMINAL_DATA_OUT) != 0)
	{
		memory_write(TERMINAL_DATA_OUT, 0);

		cout << c;
		cout.flush();
	}
	memoryMutex.unlock();

	emulatorStatusMutex.lock();
	if (interruptRequests.size() == 0)
	{
		emulatorStatusMutex.unlock();
		return;
	}
	InterruptType itype = interruptRequests.top();
	// INT_INVALID_INSTRUCTION is non-maskable interrupt
	if (((itype != InterruptType::INT_INVALID_INSTRUCTION) && (!(psw & FLAG_I))) ||
		((itype == InterruptType::KEYBOARD) && (psw & FLAG_Tl)) ||
		((itype == InterruptType::TIMER) && (psw & FLAG_Tr)))
	{
		emulatorStatusMutex.unlock();
		return;
	}
	interruptRequests.pop();
	emulatorStatusMutex.unlock();
	
	memory_push_16(pc);
	memory_push_16(psw);

	psw = psw & !FLAG_I;
	pc = memory_read_16(IVT_START + 2 * (uint16_t)itype);
}

inline void CPU::SetFlagsZN(uint8_t flags, int16_t result)
{
	if ((flags & FLAG_Z) && (result == 0))
		psw = psw | FLAG_Z;
	else
		psw = psw & !FLAG_Z;

	if ((flags & FLAG_N) && (result < 0))
		psw = psw | FLAG_N;
	else
		psw = psw & !FLAG_N;
}

inline void CPU::SetFlagO(int16_t src, int16_t dst, int16_t r, InstructionMnemonic operation)
{
	switch (operation)
	{
	case InstructionMnemonic::ADD:
	{
		if ((src >= 0 && dst >= 0 && r < 0) || (src < 0 && dst < 0 && r >= 0))
			psw = psw | FLAG_O;
		else
			psw = psw & !FLAG_O;
		break;
	}
	case InstructionMnemonic::SUB:
	{
		// TODO: check this
		if ((src >= 0 && dst < 0 && r < 0) || (src < 0 && dst >= 0 && r >= 0))
			psw = psw | FLAG_O;
		else
			psw = psw & !FLAG_O;
		break;
	}
	}
}

inline void CPU::SetFlagC(int16_t src, int16_t dst, int16_t r, InstructionMnemonic operation)
{
	switch (operation)
	{
	case InstructionMnemonic::ADD:
	{
		// TODO: fix this
		/*if ((r >= 0 && (src1 < 0 || src2 < 0)) || (r < 0 && src1 < 0 && src2 < 0))
			psw = psw | FLAG_C;
		else
			psw = psw & !FLAG_C;*/
		break;
	}
	case InstructionMnemonic::SUB:
	case InstructionMnemonic::CMP:
	{
		break;
	}
	case InstructionMnemonic::SHL:
	{
		break;
	}
	case InstructionMnemonic::SHR:
	{
		break;
	}
	}
}

void CPU::StartThreads()
{
	keyboardThread = new thread(KeyboardHandler, this);
	timerThread = new thread(TimerHandler, this);
}

CPU::~CPU()
{
	timerThread->join();
	cout << "Press ENTER key to end..." << endl;
	keyboardThread->join();
	delete keyboardThread;
}

void CPU::WriteIO(const uint16_t & address, const uint8_t & data)
{
	if (data >= MEMORY_MAPPED_REGISTERS_START && data <= MEMORY_MAPPED_REGISTERS_END)
	{
		memoryMutex.lock();
		memory_write(address, data);
		memoryMutex.unlock();
	}
	else
		SetInterrupt(InterruptType::INT_INVALID_INSTRUCTION);
		//throw EmulatorException("Cannot write with this method outside of I/O space.");
}

void CPU::SetInterrupt(const InterruptType & type)
{
	emulatorStatusMutex.lock();
	interruptRequests.push(type);
	emulatorStatusMutex.unlock();
}