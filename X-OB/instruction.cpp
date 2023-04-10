#include "instruction.h"


Instruction* Instruction::Decode(const std::vector<std::uint8_t>& bytes) {
	if (!bytes.size() /* || bytes.size() > INSTRUCTION_MAX_LENGTH*/) return {};

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
		instruction->m_LegacyPrefixes = LegacyPrefixes;
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
	instruction->m_Size = LegacyPrefixes.size() + IsRex + opcode.m_Bytes.size();

	if (opcode.m_Bytes.empty() || opcode.m_ToString->compare("Unknown opcode") == 0) {
		instruction->m_ValidInstruction = false;
	}

	return instruction;
}

Instruction::Opcode Instruction::Opcode::GetOpcode(const std::vector<std::uint8_t>& bytes, Instruction::Prefix::Rex* rex) {
	std::uint8_t opcode = bytes.at(0);
	std::string InstructionToString = "";
	std::size_t OpcodeSize = 1;

	if (Instruction::Opcode::OPCODE_NAME_LOOKUP_TABLE.find(opcode) == Instruction::Opcode::OPCODE_NAME_LOOKUP_TABLE.end()) {
		std::cout << "Unknown opcode";
		return {};
	}

	std::uint8_t DSet = opcode & 0x2;
	std::uint8_t SSet = opcode & 0x1;
	std::vector<std::uint8_t> operands = Instruction::Opcode::OPCODE_LOOKUP_TABLE.at(bytes.at(0));

	if (Opcode::OpcodeDBitOverride(opcode)) {
		DSet = !DSet;
	}

	std::uint8_t mod = bytes.at(1);
	std::uint8_t RegExtension = (opcode & 0x7);
	std::uint8_t ModRMRegMask = (mod & (0x7 << 3)) >> 3;
	std::uint8_t ModRMModMask = (mod & (0x3 << 6)) >> 6;
	std::uint8_t ModRMRmMask = (mod & 0x7);
	std::uint8_t Displacement = 0;

	bool NeedsModRmByte = Opcode::ModRM::NeedsModRMByte(opcode, operands);
	bool HasDoubleModRM = Opcode::ModRM::HasDoubleModRM(operands);
	bool HasSIB = false;

	std::string InstructionName = "";
	if (Opcode::OpcodeHasRegisterExtension(opcode) || !operands.size()) {
		InstructionName = Instruction::Opcode::OPCODE_NAME_LOOKUP_TABLE.at(opcode)[RegExtension];
	}
	else InstructionName = Instruction::Opcode::OPCODE_NAME_LOOKUP_TABLE.at(opcode)[ModRMRegMask];

	InstructionToString += InstructionName + " ";

	std::uint8_t BytesIndex = 1;
	for (int i = 0; i < operands.size(); i++) {
		if (operands.at(i) == OPERAND_MODRM) {
			if (HasDoubleModRM && ((i == 0 && DSet) || (i == 1 && !DSet))) { //MODR
				std::string RegName = Opcode::ModRM::MODRM_REGISTER_LOOKUP_TABLE.at({ ModRMRegMask, rex->RSet() });

				if (rex->WSet()) RegName = Register::GetRegisterPart(RegName, Register::PART_EXTENDED64);
				if (!SSet) RegName = Register::GetRegisterPart(RegName, Register::PART_LOWER_8);

				InstructionToString += RegName;
			}
			else if (NeedsModRmByte) {
				std::string RegName = rex->WSet() ? Register::GetRegisterPart(Opcode::ModRM::MODRM_REGISTER_LOOKUP_TABLE.at({ ModRMRmMask, rex->BSet() }), Register::PART_EXTENDED64) : Opcode::ModRM::MODRM_REGISTER_LOOKUP_TABLE.at({ ModRMRmMask, rex->BSet() });
				if (!SSet) RegName = Register::GetRegisterPart(RegName, Register::PART_LOWER_8);

				if (ModRMModMask != Opcode::ModRM::MOD_DIRECT_ADDRESSING) {
					std::vector<std::uint8_t> RemainingBytes = std::vector<std::uint8_t>(bytes.begin() + 1, bytes.end());
					std::string Register = Opcode::ModRM::GetDisplacement(ModRMModMask, ModRMRmMask, rex->WSet(), rex->XSet(), rex->BSet(), RemainingBytes);

					if (ModRMRmMask == 4) BytesIndex++; //Add byte for SIB

					BytesIndex += Opcode::ModRM::GetDisplacementInBytes(ModRMModMask);
					RegName = Register;
				}

				BytesIndex++;
				InstructionToString += RegName;
			}
			else {
				std::string RegName = Register::GetRegisterPart(Opcode::ModRM::MODRM_REGISTER_LOOKUP_TABLE.at({ RegExtension, rex->BSet() }), Register::PART_EXTENDED64);

				InstructionToString += RegName;
			}
		}
		else if (operands.at(i) == OPERAND_IMMEDIATE8) {
			InstructionToString += std::to_string(bytes.at(BytesIndex));
			BytesIndex++;
		}
		else if (operands.at(i) == OPERAND_IMMEDIATE32) {
			std::uint8_t ImmediateStart = BytesIndex;

			std::uint32_t ValMask = 0;
			for (int j = 3; j >= 0; j--) {
				ValMask |= (bytes.at(ImmediateStart + j) << (j * 8));
			}

			BytesIndex += 4;
			InstructionToString += std::to_string(ValMask);
		}
		else if (operands.at(i) == OPERAND_REL8) {
			//Jump condition should be based on flag
			InstructionToString += "RIP+" + std::to_string(bytes.at(BytesIndex));
		}
		else if (operands.at(i) == OPERAND_REL32) {
			//Jump condition should be based on flag
			std::uint8_t ImmediateStart = BytesIndex;

			std::uint32_t ValMask = 0;
			for (int j = 3; j >= 0; j--) {
				std::cout << std::hex << unsigned(bytes.at(ImmediateStart + j)) << std::endl;
				ValMask |= (bytes.at(ImmediateStart + j) << (j * 8));
			}

			InstructionToString += "RIP+" + std::to_string(ValMask);
		}

		//Add comma between operands
		if (i != operands.size() - 1) InstructionToString += ", ";
	}

	Opcode op;
	op.m_Opcode = opcode;
	op.m_Operands = operands;
	op.m_ToString = new std::string(InstructionToString);

	std::vector<std::uint8_t> NewBytes = std::vector<std::uint8_t>(bytes.begin(), bytes.begin() + BytesIndex);
	op.m_Bytes = NewBytes;

	return op;
}

const bool Instruction::Opcode::OpcodeHasRegisterExtension(std::uint8_t opcode) {
	const static std::vector<std::uint8_t> EXTENSION_TABLE = {
		0xb8, 0xb9, 0xba, 0xbb,
	};

	for (auto curr : EXTENSION_TABLE) {
		if (opcode == curr) return true;
	}

	return false;
}

const bool Instruction::Opcode::OpcodeDBitOverride(std::uint8_t opcode) {
	const static std::vector<std::uint8_t> opcodes = {
		0x8d,
	};

	for (auto curr : opcodes) {
		if (opcode == curr) return true;
	}

	return false;
}

//---------------------------------------------------------------------------------------------------------------

const std::string Instruction::Opcode::ModRM::GetDisplacement(std::uint8_t mod, std::uint8_t rm, std::uint8_t rexw, std::uint8_t rexx, std::uint8_t rexb, std::vector<std::uint8_t>& bytes) {
	if (mod >= 3) {
		return "";
	}

	std::string Displacement = "[";

	switch (mod) {
	case ModRM::MOD_INDIRECT_ADDRESSING: {
		if (rm == 4) { //NEEDS SIB
			std::uint8_t sib = bytes.at(1);

			std::string SIBToStr = Opcode::SIB::GetSIB(sib, rexx, rexb, rexx);
			Displacement += SIBToStr + "]";
		}
		else {
			std::string reg = ModRM::MODRM_REGISTER_LOOKUP_TABLE.at({ rm, rexb });
			reg = Register::GetRegisterPart(reg, Register::PART_EXTENDED64);

			Displacement += reg + "]";
		}

		return Displacement;
	}
	case ModRM::MOD_INDIRECT_DISP8: {
		std::uint8_t sib = bytes.at(1);

		if (rm == 4) {
			std::uint8_t DispAmmount = bytes.at(2);
			std::string DispAmmountToString = "";
			if (Register::GetTwosCompliment8(DispAmmount)) {
				DispAmmountToString = "-" + std::to_string(DispAmmount);
			}
			else DispAmmountToString = "+" + std::to_string(DispAmmount);

			Displacement += Opcode::SIB::GetSIB(sib, rexw, rexb, rexx);
			Displacement += DispAmmountToString;
		}
		else {
			std::string reg = ModRM::MODRM_REGISTER_LOOKUP_TABLE.at({ rm, rexb });
			std::uint8_t DispAmmount = bytes.at(1);
			std::string DispAmmountToString = "";

			if (Register::GetTwosCompliment8(DispAmmount)) {
				DispAmmountToString = "-" + std::to_string(DispAmmount);
			}
			else DispAmmountToString = "+" + std::to_string(DispAmmount);

			if (rexw) reg = Register::GetRegisterPart(reg, Register::PART_EXTENDED64);

			Displacement += reg;
			Displacement += DispAmmountToString;
		}

		Displacement += "]";
		return Displacement;
	}
	case ModRM::MOD_INDIRECT_DISP32: {
		if (rm == 4) {

		}
		else {
			std::string reg = ModRM::MODRM_REGISTER_LOOKUP_TABLE.at({ rm, rexb });
			reg = Register::GetRegisterPart(reg, Register::PART_EXTENDED64);

			std::uint32_t disp = 0;
			for (int i = 3; i >= 0; i--) {
				disp |= (bytes.at(i+1) << (8 * i));
			}
			
			if (Register::GetTwosCompliment32(disp)) {
				reg += "-" + std::to_string(disp);
			}
			else reg += "+" + std::to_string(disp);

			Displacement += reg + "]";
			return Displacement;
		}
	}
	}
	return "";
}

const std::uint8_t Instruction::Opcode::ModRM::GetDisplacementInBytes(std::uint8_t mod) {
	if (mod == 0) return 0;
	else if (mod == 1) return 1;
	else if (mod == 2) return 4;
	else return 0;
}

const bool Instruction::Opcode::ModRM::NeedsModRMByte(std::uint8_t opcode, const std::vector<std::uint8_t>& operands) {
	if (operands.size() <= 1 || Opcode::OpcodeHasRegisterExtension(opcode)) {
		return false;
	}
	else return true;
}

const bool Instruction::Opcode::ModRM::HasDoubleModRM(const std::vector<std::uint8_t>& operands) {
	if (operands.size() > 1) {
		if (operands.at(0) == OPERAND_MODRM && operands.at(1) == OPERAND_MODRM) {
			return true;
		}
	}

	return false;
}

const std::uint8_t Instruction::Opcode::ModRM::GetAdressingMode(std::uint8_t mod) {
	switch (mod) {
		case MOD_INDIRECT_ADDRESSING: return 0;
		case MOD_INDIRECT_DISP8: return 1;
	}
}

const std::map<std::pair<std::uint8_t, std::uint8_t>, std::string> Instruction::Opcode::ModRM::MODRM_REGISTER_LOOKUP_TABLE { //PAIR(REG/RM, REX.B/R)
	{{0x00, 0x00}, "EAX"},
	{{0x01, 0x00}, "ECX"},
	{{0x02, 0x00}, "EDX"},
	{{0x03, 0x00}, "EBX"},
	{{0x04, 0x00}, "ESP"},
	{{0x05, 0x00}, "EBP"},
	{{0x06, 0x00}, "ESI"},
	{{0x07, 0x00}, "EDI"},

	{{0x00, 0x01}, "R8D"},
	{{0x01, 0x01}, "R9D"},
	{{0x02, 0x01}, "R10D"},
	{{0x03, 0x01}, "R11D"},
	{{0x04, 0x01}, "R12D"},
	{{0x05, 0x01}, "R13D"},
	{{0x06, 0x01}, "R14D"},
	{{0x07, 0x01}, "R15D"},
};

//---------------------------------------------------------------------------------------------------------------

const std::vector<std::uint8_t> Instruction::Prefix::LEGACY_PREFIX = {
	0xF0, 0xF2, 0xF3, 0x2E, 0x36, 0x26, 0x64, 0x65, 0x2E, 0x3E, 0x66, 0x67
};

const std::vector<std::uint8_t> Instruction::Prefix::REX = {
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f
};

//-----------------------------------------------

std::string Instruction::Opcode::SIB::GetSIB(std::uint8_t sib, std::uint8_t rexw, std::uint8_t rexb, std::uint8_t rexx) {
	std::uint8_t SibScale = (sib & (0x3 << 6)) >> 6;
	std::uint8_t SibIndex = (sib & (0x7 << 3)) >> 3;
	std::uint8_t SibBase = (sib & 0x7);

	std::string BaseReg = SIB::SIB_BASE_LOOKUP.at({ SibBase, rexb });
	std::string IndexReg = SIB::SIB_INDEX_LOOKUP.at({ SibScale, SibIndex, rexx });

	if (rexw) {
		BaseReg = Register::GetRegisterPart(BaseReg, Register::PART_EXTENDED64);
	}

	if (SibIndex == 4) {
		return BaseReg;
	}
	else return BaseReg + "+" + IndexReg;
}

const std::map<std::pair<std::uint8_t, std::uint8_t>, std::string> Instruction::Opcode::SIB::SIB_BASE_LOOKUP = { //PAIR(sib.base, rex.b)
	{{0x00, 0x00}, "EAX"},
	{{0x01, 0x00}, "ECX"},
	{{0x02, 0x00}, "EDX"},
	{{0x03, 0x00}, "EBX"},
	{{0x04, 0x00}, "ESP"},
	{{0x05, 0x00}, "ESP"},
	{{0x06, 0x00}, "ESI"},
	{{0x07, 0x00}, "EDI"},

	{{0x00, 0x01}, "R8D"},
	{{0x01, 0x01}, "R9D"},
	{{0x02, 0x01}, "R10D"},
	{{0x03, 0x01}, "R11D"},
	{{0x04, 0x01}, "R12D"},
	{{0x05, 0x01}, "R12D"},
	{{0x06, 0x01}, "R14D"},
	{{0x07, 0x01}, "R15D"},
};

const std::map<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>, std::string> Instruction::Opcode::SIB::SIB_INDEX_LOOKUP = {  //PAIR(sib.scale, sib.index, rex.x)
	{{0x00, 0x00, 0x00}, "EAX"},
	{{0x00, 0x01, 0x00}, "ECX"},
	{{0x00, 0x02, 0x00}, "EDX"},
	{{0x00, 0x03, 0x00}, "EBX"},
	{{0x00, 0x04, 0x00}, ""},
	{{0x00, 0x05, 0x00}, "EBP"},
	{{0x00, 0x06, 0x00}, "ESI"},
	{{0x00, 0x07, 0x00}, "EDI"},

	{{0x00, 0x00, 0x01}, "R8D"},
	{{0x00, 0x01, 0x01}, "R9D"},
	{{0x00, 0x02, 0x01}, "R10D"},
	{{0x00, 0x03, 0x01}, "R11D"},
	{{0x00, 0x04, 0x01}, "R12D"},
	{{0x00, 0x05, 0x01}, "R13D"},
	{{0x00, 0x06, 0x01}, "R14D"},
	{{0x00, 0x07, 0x01}, "R15D"},

	{{0x01, 0x00, 0x00}, "EAX*2"},
	{{0x01, 0x01, 0x00}, "ECX*2"},
	{{0x01, 0x02, 0x00}, "EDX*2"},
	{{0x01, 0x03, 0x00}, "EBX*2"},
	{{0x01, 0x04, 0x00}, ""},
	{{0x01, 0x05, 0x00}, "EBP*2"},
	{{0x01, 0x06, 0x00}, "ESI*2"},
	{{0x01, 0x07, 0x00}, "EDI*2"},

	{{0x01, 0x00, 0x01}, "R8D*2"},
	{{0x01, 0x01, 0x01}, "R9D*2"},
	{{0x01, 0x02, 0x01}, "R10D*2"},
	{{0x01, 0x03, 0x01}, "R11D*2"},
	{{0x01, 0x04, 0x01}, "R12D*2"},
	{{0x01, 0x05, 0x01}, "R13D*2"},
	{{0x01, 0x06, 0x01}, "R14D*2"},
	{{0x01, 0x07, 0x01}, "R15D*2"},

	{{0x02, 0x00, 0x00}, "EAX*4"},
	{{0x02, 0x01, 0x00}, "ECX*4"},
	{{0x02, 0x02, 0x00}, "EDX*4"},
	{{0x02, 0x03, 0x00}, "EBX*4"},
	{{0x02, 0x04, 0x00}, ""},
	{{0x02, 0x05, 0x00}, "EBP*4"},
	{{0x02, 0x06, 0x00}, "ESI*4"},
	{{0x02, 0x07, 0x00}, "EDI*4"},

	{{0x02, 0x00, 0x01}, "R8D*4"},
	{{0x02, 0x01, 0x01}, "R9D*4"},
	{{0x02, 0x02, 0x01}, "R10D*4"},
	{{0x02, 0x03, 0x01}, "R11D*4"},
	{{0x02, 0x04, 0x01}, "R12D*4"},
	{{0x02, 0x05, 0x01}, "R13D*4"},
	{{0x02, 0x06, 0x01}, "R14D*4"},
	{{0x02, 0x07, 0x01}, "R15D*4"},

	{{0x03, 0x00, 0x00}, "EAX*8"},
	{{0x03, 0x01, 0x00}, "ECX*8"},
	{{0x03, 0x02, 0x00}, "EDX*8"},
	{{0x03, 0x03, 0x00}, "EBX*8"},
	{{0x03, 0x04, 0x00}, ""},
	{{0x03, 0x05, 0x00}, "EBP*8"},
	{{0x03, 0x06, 0x00}, "ESI*8"},
	{{0x03, 0x07, 0x00}, "EDI*8"},

	{{0x03, 0x00, 0x01}, "R8D*8"},
	{{0x03, 0x01, 0x01}, "R9D*8"},
	{{0x03, 0x02, 0x01}, "R10D*8"},
	{{0x03, 0x03, 0x01}, "R11D*8"},
	{{0x03, 0x04, 0x01}, "R12D*8"},
	{{0x03, 0x05, 0x01}, "R13D*8"},
	{{0x03, 0x06, 0x01}, "R14D*8"},
	{{0x03, 0x07, 0x01}, "R15D*8"},
};

//---------------------------------------------------------------------------------------------------------------

const std::map<std::uint8_t, std::vector<std::uint8_t>> Instruction::Opcode::OPCODE_LOOKUP_TABLE = {
	{0x00, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x01, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x02, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x03, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x04, {Instruction::Opcode::OPERAND_REGISTER_RAX, Instruction::Opcode::OPERAND_IMMEDIATE8}},
	{0x05, {Instruction::Opcode::OPERAND_REGISTER_RAX, Instruction::Opcode::OPERAND_IMMEDIATE32}},

	{0x08, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x09, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x0A, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x0B, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x0C, {Instruction::Opcode::OPERAND_REGISTER_RAX, Instruction::Opcode::OPERAND_IMMEDIATE8}},
	{0x0D, {Instruction::Opcode::OPERAND_REGISTER_RAX, Instruction::Opcode::OPERAND_IMMEDIATE32}},

	{0x2B, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},

	{0x32, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x33, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x38, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x39, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x3B, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x3D, {Instruction::Opcode::OPERAND_REGISTER_RAX, Instruction::Opcode::OPERAND_IMMEDIATE32}},

	{0x53, {Instruction::Opcode::OPERAND_MODRM}},
	{0x55, {Instruction::Opcode::OPERAND_MODRM}},
	{0x56, {Instruction::Opcode::OPERAND_MODRM}},
	{0x57, {Instruction::Opcode::OPERAND_MODRM}},
	{0x5D, {Instruction::Opcode::OPERAND_MODRM}},
	{0x5E, {Instruction::Opcode::OPERAND_MODRM}},

	{0x6B, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_IMMEDIATE8}},

	{0x73, {Instruction::Opcode::OPERAND_REL8}},
	{0x74, {Instruction::Opcode::OPERAND_REL8}},
	{0x75, {Instruction::Opcode::OPERAND_REL8}},

	{0x81, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_IMMEDIATE32}},
	{0x83, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_IMMEDIATE8}},
	{0x84, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x85, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},

	{0x88, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x89, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},

	{0x8A, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x8B, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},
	{0x8D, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_MODRM}},

	{0xB8, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_IMMEDIATE32}},
	{0xB9, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_IMMEDIATE32}},

	{0xC1, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_IMMEDIATE8}},
	{0xC2, {Instruction::Opcode::OPERAND_IMMEDIATE32}},
	{0xC3, {}},
	{0xC7, {Instruction::Opcode::OPERAND_MODRM, Instruction::Opcode::OPERAND_IMMEDIATE32}},

	{0xE8, {Instruction::Opcode::OPERAND_REL32}},
	{0xE9, {Instruction::Opcode::OPERAND_REL32}},
};

const std::map<std::uint8_t, std::vector<std::string>> Instruction::Opcode::OPCODE_NAME_LOOKUP_TABLE = { //opcode -> {ModRM.reg}
	{0x00, {"ADD", "ADD", "ADD", "ADD", "ADD", "ADD", "ADD", "ADD"}},
	{0x01, {"ADD", "ADD", "ADD", "ADD", "ADD", "ADD", "ADD", "ADD"}},
	{0x02, {"ADD", "ADD", "ADD", "ADD", "ADD", "ADD", "ADD", "ADD"}},
	{0x03, {"ADD", "ADD", "ADD", "ADD", "ADD", "ADD", "ADD", "ADD"}},
	{0x04, {"ADD", "ADD", "ADD", "ADD", "ADD", "ADD", "ADD", "ADD"}},
	{0x05, {"ADD", "ADD", "ADD", "ADD", "ADD", "ADD", "ADD", "ADD"}},

	{0x08, {"OR", "OR", "OR", "OR", "OR", "OR", "OR", "OR"}},
	{0x09, {"OR", "OR", "OR", "OR", "OR", "OR", "OR", "OR"}},
	{0x0A, {"OR", "OR", "OR", "OR", "OR", "OR", "OR", "OR"}},
	{0x0B, {"OR", "OR", "OR", "OR", "OR", "OR", "OR", "OR"}},
	{0x0C, {"OR", "OR", "OR", "OR", "OR", "OR", "OR", "OR"}},
	{0x0D, {"OR", "OR", "OR", "OR", "OR", "OR", "OR", "OR"}},

	{0x2B, {"SUB", "SUB", "SUB", "SUB", "SUB", "SUB", "SUB", "SUB"}},

	{0x32, {"XOR", "XOR", "XOR", "XOR", "XOR", "XOR", "XOR", "XOR"}},
	{0x33, {"XOR", "XOR", "XOR", "XOR", "XOR", "XOR", "XOR", "XOR"}},
	{0x38, {"CMP", "CMP", "CMP", "CMP", "CMP", "CMP", "CMP", "CMP"}},
	{0x39, {"CMP", "CMP", "CMP", "CMP", "CMP", "CMP", "CMP", "CMP"}},
	{0x3B, {"CMP", "CMP", "CMP", "CMP", "CMP", "CMP", "CMP", "CMP"}},
	{0x3D, {"CMP", "CMP", "CMP", "CMP", "CMP", "CMP", "CMP", "CMP"}},

	{0x53, {"PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH"}},
	{0x55, {"PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH"}},
	{0x56, {"PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH"}},
	{0x57, {"PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH", "PUSH"}},
	{0x5D, {"POP", "POP", "POP", "POP", "POP", "POP", "POP", "POP"}},
	{0x5E, {"POP", "POP", "POP", "POP", "POP", "POP", "POP", "POP"}},

	{0x6B, {"IMUL", "IMUL", "IMUL", "IMUL", "IMUL", "IMUL", "IMUL", "IMUL"}},

	{0x73, {"JNB/JAE/JNC", "JNB/JAE/JNC", "JNB/JAE/JNC", "JNB/JAE/JNC", "JNB/JAE/JNC", "JNB/JAE/JNC", "JNB/JAE/JNC", "JNB/JAE/JNC"}},
	{0x74, {"JZ/JE", "JZ/JE", "JZ/JE", "JZ/JE", "JZ/JE", "JZ/JE", "JZ/JE", "JZ/JE"}},
	{0x75, {"JNZ/JNE", "JNZ/JNE", "JNZ/JNE", "JNZ/JNE", "JNZ/JNE", "JNZ/JNE", "JNZ/JNE", "JNZ/JNE"}},

	{0x81, {"ADD", "OR", "ADC", "SBB", "AND", "SUB", "XOR", "CMP"}},
	{0x83, {"ADD", "OR", "ADC", "SBB", "AND", "SUB", "XOR", "CMP"}},
	{0x84, {"TEST", "TEST", "TEST", "TEST", "TEST", "TEST", "TEST", "TEST"}},
	{0x85, {"TEST", "TEST", "TEST", "TEST", "TEST", "TEST", "TEST", "TEST"}},

	{0x88, {"MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV"}},
	{0x89, {"MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV"}},

	{0x8A, {"MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV"}},
	{0x8B, {"MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV"}},
	{0x8D, {"LEA", "LEA", "LEA", "LEA", "LEA", "LEA", "LEA", "LEA"}},

	{0xB8, {"MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV"}},
	{0xB9, {"MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV"}},

	{0xC1, {"ROL", "ROR", "RCL", "RCR", "SHL/SAL", "SHR", "SAL/SHL", "SAR"}},
	{0xC2, {"RETN", "RETN", "RETN", "RETN", "RETN", "RETN", "RETN", "RETN"}},
	{0xC3, {"RETN", "RETN", "RETN", "RETN", "RETN", "RETN", "RETN", "RETN"}},
	{0xC7, {"MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV"}},

	{0xE8, {"CALL", "CALL", "CALL", "CALL", "CALL", "CALL", "CALL", "CALL"}},
	{0xE9, {"JMP", "JMP", "JMP", "JMP", "JMP", "JMP", "JMP", "JMP"}},
};