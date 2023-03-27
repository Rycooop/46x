#include "register.h"


Register::Register(const std::string& Name, std::uint64_t Value) {
	this->RegName = Name;
	this->Val = Value;
}

std::string Register::GetRegisterPart(const std::string& reg, const std::uint8_t partIndex) {
	if (Register::REGISTER_PARTS.find(reg) != Register::REGISTER_PARTS.end()) {
		return Register::REGISTER_PARTS.at(reg)[partIndex];
	}
	else if (Register::REGISTER_PARTS64.find(reg) != Register::REGISTER_PARTS64.end()) {
		return Register::REGISTER_PARTS64.at(reg)[partIndex];
	}
	else return "Unknown_register";
}

bool Register::GetTwosCompliment8(std::uint8_t& val) {
	if (!(val & 0x80)) return false;
	

	val = ~val;
	val += 1;
	return true;
}

bool Register::GetTwosCompliment32(std::uint32_t& val) {
	if (!(val & 0x80000000)) return false;

	val = ~val;
	val += 1;
	return true;
}

const std::vector<std::string> Register::VALID_REGISTERS = {
		"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};

const std::map<std::string, std::vector<std::string>> Register::REGISTER_PARTS = {
	{"EAX", {"RAX", "EAX", "AX", "AH", "AL"}},
	{"EBX", {"RBX", "EBX", "BX", "BH", "BL"}},
	{"ECX", {"RCX", "ECX", "AX", "AH", "Cl"}},
	{"EDX", {"RDX", "EDX", "AX", "AH", "Dl"}},
	{"ESI", {"RSI", "ESI", "AX", "AH", "AL"}},
	{"EDI", {"RDI", "EDI", "AX", "AH", "AL"}},
	{"EBP", {"RBP", "EBP", "AX", "AH", "AL"}},
	{"ESP", {"RSP", "ESP", "AX", "AH", "AL"}},

	{"R8D", {"R8", "R8D", "AX", "AH", "AL"}},
	{"R9D", {"R9", "R9D", "AX", "AH", "AL"}},
	{"R10D", {"R10", "R10D", "AX", "AH", "AL"}},
	{"R11D", {"R11", "R11D", "AX", "AH", "AL"}},
	{"R12D", {"R12", "R12D", "AX", "AH", "AL"}},
	{"R13D", {"R13", "R13D", "AX", "AH", "AL"}},
	{"R14D", {"R14", "R14D", "AX", "AH", "AL"}},
	{"R15D", {"R15", "R15D", "AX", "AH", "AL"}},
};

const std::map<std::string, std::vector<std::string>> Register::REGISTER_PARTS64 = {
	{"RAX", {"RAX", "EAX", "AX", "AH", "AL"}},
	{"RBX", {"RBX", "EBX", "BX", "BH", "BL"}},
	{"RCX", {"RCX", "ECX", "AX", "AH", "CL"}},
	{"RDX", {"RDX", "EDX", "AX", "AH", "DL"}},
	{"RSI", {"RSI", "ESI", "AX", "AH", "AL"}},
	{"RDI", {"RDI", "EDI", "AX", "AH", "AL"}},
	{"RBP", {"RBP", "EBP", "AX", "AH", "AL"}},
	{"RSP", {"RSP", "ESP", "AX", "AH", "AL"}},

	{"R8", {"R8", "R8D", "AX", "AH", "AL"}},
	{"R9", {"R9", "R9D", "AX", "AH", "AL"}},
	{"R10", {"R10", "R10D", "AX", "AH", "AL"}},
	{"R11", {"R11", "R11D", "AX", "AH", "AL"}},
	{"R12", {"R12", "R12D", "AX", "AH", "AL"}},
	{"R13", {"R13", "R13D", "AX", "AH", "AL"}},
	{"R14", {"R14", "R14D", "AX", "AH", "AL"}},
	{"R15", {"R15", "R15D", "AX", "AH", "AL"}},
};