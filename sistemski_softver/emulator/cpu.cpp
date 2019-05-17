#include "cpu.h"

void CPU::ResolveAddressing(uint8_t rawData, Operand op)
{
	uint16_t& operand = (op == Operand::FIRST_OPERAND ? operand1 : operand2);
	ByteSelector& byteSelector = (op == Operand::FIRST_OPERAND ? operand1ByteSelector : operand2ByteSelector);
	uint8_t& registerSelector = (op == Operand::FIRST_OPERAND ? registerSelector1 : registerSelector2);
	AddressingType addressingType = static_cast<AddressingType>(rawData >> 5);

	switch (addressingType)
	{
	case AddressingType::IMMEDIATELY:
	{
		operand = memory[pc++]; // lower bytes
		if (operandSize == OperandSize::WORD)
			operand = (operand | (memory[pc++] << 8)); // higher bytes
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
		operand = memory[pc++]; // lower bytes
		break;
	}
	case AddressingType::REGISTER_INDIRECT_16_BIT_OFFSET:
	{
		registerSelector = ((rawData >> 1) & 0x0F);
		operand = memory[pc++]; // lower bytes
		operand = (operand | (memory[pc++] << 8)); // higher bytes
		break;
	}
	case AddressingType::MEMORY_DIRECT:
	{
		operand = memory[pc++]; // lower bytes
		operand = (operand | (memory[pc++] << 8)); // higher bytes
		break;
	}
	default:
		throw EmulatorException("Unknown addressing type.", ErrorCodes::EMULATOR_UNKNOWN_ADDRESSING);
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
			throw EmulatorException("Immediately addressed operand cannot be destination.");

		return operand;
	}
	case AddressingType::REGISTER_DIRECT:
		return registerFile[registerSelector];
	case AddressingType::REGISTER_INDIRECT_NO_OFFSET:
		return (uint16_t&)memory[registerFile[registerSelector]];
	case AddressingType::REGISTER_INDIRECT_8_BIT_OFFSET:
		return (uint16_t&)memory[registerFile[registerSelector] + (int8_t)(operand & 0xFF)];
	case AddressingType::REGISTER_INDIRECT_16_BIT_OFFSET:
		return (uint16_t&)memory[registerFile[registerSelector] + (int16_t)operand];
	case AddressingType::MEMORY_DIRECT:
		return (uint16_t&)memory[operand];
	default:
	{
		break;
	}
	}
}

void CPU::InstructionFetchAndDecode()
{
	uint8_t IP;
	int writeTo = 0;

	IP = memory[pc++];
	uint8_t instructionCode = ((IP >> 3) & 0x1F);
	uint8_t size = ((IP & 0x04) >> 2);
	if (instructionCode >= 1 && instructionCode <= 25)
	{
		instructionMnemonic = static_cast<InstructionMnemonic>(instructionCode);
		operandSize = static_cast<OperandSize>(size);
	}
	else
		throw EmulatorException("Unknown operation code detected.", ErrorCodes::EMULATOR_UNKNOWN_INSTRUCTION);

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
		uint8_t m = memory[pc++];
		ResolveAddressing(m, Operand::FIRST_OPERAND);
		break;
	}
	case 2:
	{
		uint8_t m1 = memory[pc++];
		ResolveAddressing(m1, Operand::FIRST_OPERAND);
		uint8_t m2 = memory[pc++];
		ResolveAddressing(m2, Operand::SECOND_OPERAND);
		break;
	}
	default:
		throw EmulatorException("Unknown instruction addressing field.", ErrorCodes::EMULATOR_UNKNOWN_INSTRUCTION);
	}
}

void CPU::InstructionExecute()
{
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
		pc = memory[(dst % 8) << 1];

		break;
	}
	case InstructionMnemonic::MOV:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst = src;
		MemorizeLazyFlags(FLAG_Z | FLAG_N, dst, true, src);
		break;
	}
	case InstructionMnemonic::ADD:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst += src;
		MemorizeLazyFlags(FLAG_Z | FLAG_O | FLAG_C | FLAG_N, dst, true, src);
		break;
	}
	case InstructionMnemonic::SUB:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst -= src;
		MemorizeLazyFlags(FLAG_Z | FLAG_O | FLAG_C | FLAG_N, dst, true, src);
		break;
	}
	case InstructionMnemonic::MUL:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst *= src;
		MemorizeLazyFlags(FLAG_Z | FLAG_N, dst, true, src);
		break;
	}
	case InstructionMnemonic::DIV:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst /= src;
		MemorizeLazyFlags(FLAG_Z | FLAG_N, dst, true, src);
		break;
	}
	case InstructionMnemonic::CMP:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		uint16_t temp = dst - src;
		MemorizeLazyFlags(FLAG_Z | FLAG_O | FLAG_C | FLAG_N, dst, true, src);
		break;
	}
	case InstructionMnemonic::NOT:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		dst = !dst;
		MemorizeLazyFlags(FLAG_Z | FLAG_N, dst, dst);
		break;
	}
	case InstructionMnemonic::AND:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst = dst & src;
		MemorizeLazyFlags(FLAG_Z | FLAG_N, dst, true, src);
		break;
	}
	case InstructionMnemonic::OR:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst = dst | src;
		MemorizeLazyFlags(FLAG_Z | FLAG_N, dst, true, src);
		break;
	}
	case InstructionMnemonic::XOR:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst = dst ^ src;
		MemorizeLazyFlags(FLAG_Z | FLAG_N, dst, true, src);
		break;
	}
	case InstructionMnemonic::TEST:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		uint16_t temp = dst & src;
		MemorizeLazyFlags(FLAG_Z | FLAG_N, dst, true, src);
		break;
	}
	case InstructionMnemonic::SHL:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst = dst << src;
		MemorizeLazyFlags(FLAG_Z | FLAG_C | FLAG_N, dst, true, src);
		break;
	}
	case InstructionMnemonic::SHR:
	{
		uint16_t& dst = GetReference(Operand::FIRST_OPERAND);
		uint16_t& src = GetReference(Operand::SECOND_OPERAND);
		dst = dst >> src;
		MemorizeLazyFlags(FLAG_Z | FLAG_C | FLAG_N, dst, true, src);
		break;
	}
	case InstructionMnemonic::PUSH:
	{
		break;
	}
	case InstructionMnemonic::POP:
	{
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
		break;
	}
	case InstructionMnemonic::JNE:
	{
		break;
	}
	case InstructionMnemonic::JGT:
	{
		break;
	}
	case InstructionMnemonic::CALL:
	{
		memory_push_16(pc);
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
		throw EmulatorException("Unknown instruction.", ErrorCodes::EMULATOR_UNKNOWN_INSTRUCTION);
	}
}

void CPU::InstructionHandleInterrupt()
{
}

void CPU::MemorizeLazyFlags(uint8_t flags, uint16_t result, uint16_t dst, bool srcSet = true, uint16_t src = 0)
{
	/*if (flags & FLAG_Z)
	{
		lazyFlag.Z = dst;
		Z_RES = result;
		if (srcSet)
			Z_SRC = src;
	}

	if (flags & FLAG_O)
	{
		O_DST = dst;
		if (srcSet)
			O_SRC = src;
	}

	if (flags & FLAG_C)
	{
		O_DST = dst;
		if (srcSet)
			O_SRC = src;
	}

	if (flags & FLAG_N)
	{
		Z_DST = dst;
		if (srcSet)
			Z_SRC = src;
	}*/
}

void CPU::KeyboardInterruptRoutine()
{
	/*bool running = false;

	while (1)
	{
		emulatorStatusMutex.lock();
		if (halted)
		{
			emulatorStatusMutex.unlock();
			break;	// exit from this loop
		}
		emulatorStatusMutex.unlock();

		char input = getchar();

		memoryMutex.lock();
		if (!halted)
			memory[KEYBOARD_DATA_IN] = input;
		else
			running = false;
		memoryMutex.unlock();

		if (running)
			RegisterInterrupt();
	}*/
}