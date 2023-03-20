#include "instruction.h"


Instruction* Instruction::Decode(const std::vector<std::uint8_t>& bytes) {
	if (!bytes.size() || bytes.size() > INSTRUCTION_MAX_LENGTH) return {};

	std::vector<std::uint8_t> InstructionBytes;
	Instruction* instruction = new Instruction();

	//Get legacy prefixes
	std::vector<std::uint8_t> LegacyPrefixes;
	for (const auto curr : bytes) {
		if (Prefix::IsLegacyPrefix(curr)) {
			LegacyPrefixes.push_back(curr);
		}
		else break;
	}

	int index = 0;
	if (LegacyPrefixes.size()) {
		instruction->LegacyPrefixes = LegacyPrefixes;
		index = LegacyPrefixes.size() - 1;

		for (const auto& curr : LegacyPrefixes) {
			InstructionBytes.push_back(curr);
		}
	}

	bool IsRex = false;
	Instruction::Prefix::Rex* rex = new Instruction::Prefix::Rex();

	if (Prefix::Rex::IsREX(bytes[index])) {
		rex->r = bytes[index];
		IsRex = true; index++;
		InstructionBytes.push_back(bytes[index]);
	}
	instruction->rex = rex;

	std::vector<std::uint8_t> RemainingBytes = std::vector<std::uint8_t>(bytes.begin() + index, bytes.end());
	Opcode opcode = Instruction::Opcode::GetOpcode(RemainingBytes, rex);

	instruction->op = &opcode;

	return instruction;
}

Instruction::Opcode Instruction::Opcode::GetOpcode(const std::vector<std::uint8_t>& bytes, Instruction::Prefix::Rex* rex) {
	std::uint8_t opcode = bytes.at(0);
	std::string InstructionToString = "";

	if (Instruction::Opcode::OPCODE_NAME_LOOKUP_TABLE.find(opcode) == Instruction::Opcode::OPCODE_NAME_LOOKUP_TABLE.end()) {
		std::cout << "Unknown opcode";
		return {};
	}

	std::uint8_t DSet = opcode & 0x2;
	std::uint8_t SSet = opcode & 0x1;
	std::vector<std::uint8_t> operands = Instruction::Opcode::OPCODE_LOOKUP_TABLE.at(bytes.at(0));

	if (!Opcode::ModRM::NeedsModRMByte(operands)) {
		std::uint8_t RegExtension = (opcode & (0x7 << 3)) >> 3;
		std::uint8_t ModRMModMask = (opcode & (0x3 << 6)) >> 6;
		std::uint8_t ModRMRmMask = (opcode & 0x7);

		std::string InstructionName = Instruction::Opcode::OPCODE_NAME_LOOKUP_TABLE.at(opcode)[RegExtension];
		InstructionToString += InstructionName + " ";

		if (operands.size() && ModRM::IsModRMArg(operands.at(0))) {
			std::string RegisterName = Instruction::Opcode::ModRM::MODRM_NAME_LOOKUP_TABLE.at({ ModRMModMask, ModRMRmMask, rex->BSet() });
			InstructionToString += RegisterName;
		}
	}
	else {
		std::uint8_t mod = bytes.at(1);

		std::uint8_t ModRMRegMask = (mod & (0x7 << 3)) >> 3;
		std::uint8_t ModRMModMask = (mod & (0x3 << 6)) >> 6;
		std::uint8_t ModRMRmMask = (mod & 0x7);

		std::string InstructionName = Instruction::Opcode::OPCODE_NAME_LOOKUP_TABLE.at(opcode)[ModRMRegMask];
		InstructionToString += InstructionName + " ";

		for (int i = 0; i < operands.size(); i++) {
			if (operands.at(i) == Instruction::Opcode::OPERAND_MODRM) {

				std::string RegisterName = "";
				if ((DSet && i == 0) || (!DSet && i == 1)) {
					RegisterName = Instruction::Opcode::ModRM::MODRM_NAME_LOOKUP_TABLE.at({ ModRMModMask, ModRMRmMask, rex->BSet() });
					if (!rex->WSet()) {
						RegisterName = Register::GetRegisterPart(RegisterName, Register::PART_32);
					}
				}
				else {
					RegisterName = Instruction::Opcode::ModRM::MODR_NAME_LOOKUP_TABLE.at({ ModRMRegMask, rex->XSet() });
					if (!rex->WSet()) {
						RegisterName = Register::GetRegisterPart(RegisterName, Register::PART_32);
					}
				}

				InstructionToString += RegisterName + " ";
			}
			else if (operands.at(i) == Instruction::Opcode::OPERAND_IMMEDIATE8) {
				std::string toStr = std::to_string(bytes.at(i + 1));
				InstructionToString += toStr;
			}
		}
	}

	Opcode op;
	op.m_Opcode = opcode;
	op.m_Bytes = bytes;
	op.m_Operands = operands;
	op.m_ToString = new std::string(InstructionToString);

	return op;
}

const std::uint8_t Instruction::GetPrefix(const std::vector<std::uint8_t>& Bytes) {
	return 0;
} 

//---------------------------------------------------------------------------------------------------------------

const std::map<std::uint8_t, std::vector<std::uint8_t>> Instruction::Opcode::OPCODE_LOOKUP_TABLE = {
	{0x2B, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},

	{0x33, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},

	{0x55, {Instruction::Opcode::OPERAND_MODRM}},
	{0x56, {Instruction::Opcode::OPERAND_MODRM}},
	{0x57, {Instruction::Opcode::OPERAND_MODRM}},
	{0x5D, {Instruction::Opcode::OPERAND_MODRM}},
	{0x5E, {Instruction::Opcode::OPERAND_MODRM}},

	{0x83, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_IMMEDIATE8}},
	{0x84, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x85, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},

	{0x8A, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x8B, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},

	{0xC2, {Instruction::Opcode::OPERAND_IMMEDIATE32}},
	{0xC3, {}},
};

const std::map<std::uint8_t, std::vector<std::string>> Instruction::Opcode::OPCODE_NAME_LOOKUP_TABLE = { //opcode -> {ModRM.reg}
	{0x2B, {"SUB", "SUB", "SUB", "SUB", "SUB", "SUB", "SUB", "SUB"}},

	{0x33, {"XOR", "XOR", "XOR", "XOR", "XOR", "XOR", "XOR", "XOR"}},

	{0x55, {"PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH"}},
	{0x56, {"PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH"}},
	{0x57, {"PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH"}},
	{0x5D, {"POP", "POP", "POP", "POP", "POP", "POP", "POP", "POP"}},
	{0x5E, {"POP", "POP", "POP", "POP", "POP", "POP", "POP", "POP"}},

	{0x83, {"ADD", "OR", "ADC", "SBB", "AND", "SUB", "XOR", "CMP"}},
	{0x84, {"TEST", "TEST", "TEST", "TEST", "TEST", "TEST", "TEST", "TEST"}},
	{0x85, {"TEST", "TEST", "TEST", "TEST", "TEST", "TEST", "TEST", "TEST"}},

	{0x8A, {"MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV"}},
	{0x8B, {"MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV"}},

	{0xC2, {"RETN", "RETN", "RETN", "RETN", "RETN", "RETN", "RETN", "RETN"}},
	{0xC3, {"RETN", "RETN", "RETN", "RETN", "RETN", "RETN", "RETN", "RETN"}},
};

//---------------------------------------------------------------------------------------------------------------

const bool Instruction::Opcode::ModRM::ContainsModRM(const std::vector<std::uint8_t>& operands) {
	bool contains = false;
	for (const auto& currOp : operands) {
		if (currOp == Instruction::Opcode::OPERAND_MODRM) {
			contains = true;
			break;
		}
	}

	return contains;
}

const bool Instruction::Opcode::ModRM::NeedsModRMByte(const std::vector<std::uint8_t>& operands) {
	if (operands.size() > 1) {
		if (IsModRMArg(operands.at(0)) || IsModRMArg(operands.at(1))) return true;
		else return false;
	}
	else return false;
}

const std::map<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>, std::string> Instruction::Opcode::ModRM::MODRM_NAME_LOOKUP_TABLE = { //PAIR(MOD, RM, REX.B)
	{{0x0, 0x00, 0x00}, "[RAX]"},
	{{0x0, 0x01, 0x00}, "[RCX]"},
	{{0x0, 0x02, 0x00}, "[RDX]"},
	{{0x0, 0x03, 0x00}, "[RBX]"},
	{{0x0, 0x04, 0x00}, "[sib]"},
	{{0x0, 0x05, 0x00}, "[RIP]"},
	{{0x0, 0x06, 0x00}, "[RSI]"},
	{{0x0, 0x07, 0x00}, "[RDI]"},

	{{0x0, 0x00, 0x01}, "[R8]"},
	{{0x0, 0x01, 0x01}, "[R9]"},
	{{0x0, 0x02, 0x01}, "[R10]"},
	{{0x0, 0x03, 0x01}, "[R11]"},
	{{0x0, 0x04, 0x01}, "[sib]"},
	{{0x0, 0x05, 0x01}, "[R13]"},
	{{0x0, 0x06, 0x01}, "[R14]"},
	{{0x0, 0x07, 0x01}, "[R15]"},

	{{0x1, 0x00, 0x00}, "RAX"},
	{{0x1, 0x01, 0x00}, "RCX"},
	{{0x1, 0x02, 0x00}, "RDX"},
	{{0x1, 0x03, 0x00}, "RBX"},
	{{0x1, 0x04, 0x00}, "RSP"},
	{{0x1, 0x05, 0x00}, "RBP"},
	{{0x1, 0x06, 0x00}, "RSI"},
	{{0x1, 0x07, 0x00}, "RDI"},

	{{0x1, 0x00, 0x01}, "[R8]"},
	{{0x1, 0x01, 0x01}, "[R9]"},
	{{0x1, 0x02, 0x01}, "[R10]"},
	{{0x1, 0x03, 0x01}, "[R11]"},
	{{0x1, 0x04, 0x01}, "[SIB] + disp"},
	{{0x1, 0x05, 0x01}, "[R13]"},
	{{0x1, 0x06, 0x01}, "[R14]"},
	{{0x1, 0x07, 0x01}, "[R15]"},

	{{0x2, 0x00, 0x00}, "[RAX/EAX]"},
	{{0x2, 0x01, 0x00}, "[RCX/ECX]"},
	{{0x2, 0x02, 0x00}, "[RDX/EDX]"},
	{{0x2, 0x03, 0x00}, "[RBX/EBX]"},
	{{0x2, 0x04, 0x00}, "[sib]"},
	{{0x2, 0x05, 0x00}, "[RBP/EBP]"},
	{{0x2, 0x06, 0x00}, "[RSI/ESI]"},
	{{0x2, 0x07, 0x00}, "[RDI/EDI]"},

	{{0x3, 0x00, 0x00}, "RAX"},
	{{0x3, 0x01, 0x00}, "RCX"},
	{{0x3, 0x02, 0x00}, "RDX"},
	{{0x3, 0x03, 0x00}, "RBX"},
	{{0x3, 0x04, 0x00}, "RSP"},
	{{0x3, 0x05, 0x00}, "RBP"},
	{{0x3, 0x06, 0x00}, "RSI"},
	{{0x3, 0x07, 0x00}, "RDI"},

	{{0x3, 0x00, 0x01}, "R8"},
	{{0x3, 0x01, 0x01}, "R9"},
	{{0x3, 0x02, 0x01}, "R10"},
	{{0x3, 0x03, 0x01}, "R11"},
	{{0x3, 0x04, 0x01}, "R12"},
	{{0x3, 0x05, 0x01}, "R13"},
	{{0x3, 0x06, 0x01}, "R14"},
	{{0x3, 0x07, 0x01}, "R15"},
};

const std::map<std::pair<std::uint8_t, std::uint8_t>, std::string> Instruction::Opcode::ModRM::MODR_NAME_LOOKUP_TABLE = { //PAIR(REG, REX.R)
	{{0x00, 0x00}, "RAX"},
	{{0x01, 0x00}, "RCX"},
	{{0x02, 0x00}, "RDX"},
	{{0x03, 0x00}, "RBX"},
	{{0x04, 0x00}, "RSP"},
	{{0x05, 0x00}, "RBP"},
	{{0x06, 0x00}, "RSI"},
	{{0x07, 0x00}, "RDI"},

	{{0x00, 0x01}, "R8"},
	{{0x01, 0x01}, "R9"},
	{{0x02, 0x01}, "R10"},
	{{0x03, 0x01}, "R11"},
	{{0x04, 0x01}, "R12"},
	{{0x05, 0x01}, "R13"},
	{{0x06, 0x01}, "R14"},
	{{0x07, 0x01}, "R15"},
};

//---------------------------------------------------------------------------------------------------------------

const std::vector<std::uint8_t> Instruction::Prefix::LEGACY_PREFIX = {
	0xF0, 0xF2, 0xF3, 0x2E, 0x36, 0x26, 0x64, 0x65, 0x2E, 0x3E, 0x66, 0x67
};

const std::vector<std::uint8_t> Instruction::Prefix::REX = {
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f
};



