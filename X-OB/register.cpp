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
	{"ECX", {"RCX", "ECX", "CX", "DH", "CL"}},
	{"EDX", {"RDX", "EDX", "DX", "DH", "DL"}},
	{"ESI", {"RSI", "ESI", "SI", "SIH", "SIL"}},
	{"EDI", {"RDI", "EDI", "DI", "DIH", "DIL"}},
	{"EBP", {"RBP", "EBP", "BP", "BPH", "BPL"}},
	{"ESP", {"RSP", "ESP", "SP", "SPH", "SPL"}},

	{"R8D", {"R8", "R8D", "R8W", "R8H", "R8B"}},
	{"R9D", {"R9", "R9D", "R9W", "R9H", "R9B"}},
	{"R10D", {"R10", "R10D", "R10W", "R10H", "R10B"}},
	{"R11D", {"R11", "R11D", "R11W", "R11H", "R11B"}},
	{"R12D", {"R12", "R12D", "R12W", "R12H", "R12B"}},
	{"R13D", {"R13", "R13D", "R13W", "R13H", "R13B"}},
	{"R14D", {"R14", "R14D", "R14W", "R14H", "R14B"}},
	{"R15D", {"R15", "R15D", "R15W", "R15H", "R15B"}},
};

const std::map<std::string, std::vector<std::string>> Register::REGISTER_PARTS64 = {
	{"RAX", {"RAX", "EAX", "AX", "AH", "AL"}},
	{"RBX", {"RBX", "EBX", "BX", "BH", "BL"}},
	{"RCX", {"RCX", "ECX", "CX", "DH", "CL"}},
	{"RDX", {"RDX", "EDX", "DX", "DH", "DL"}},
	{"RSI", {"RSI", "ESI", "SI", "SIH", "SIL"}},
	{"RDI", {"RDI", "EDI", "DI", "DIH", "DIL"}},
	{"RBP", {"RBP", "EBP", "BP", "BPH", "BPL"}},
	{"RSP", {"RSP", "ESP", "SP", "SPH", "SPL"}},

	{"R8", {"R8", "R8D", "R8W", "R8H", "R8B"}},
	{"R9", {"R9", "R9D", "R9W", "R9H", "R9B"}},
	{"R10", {"R10", "R10D", "R10W", "R10H", "R10B"}},
	{"R11", {"R11", "R11D", "R11W", "R11H", "R11B"}},
	{"R12", {"R12", "R12D", "R12W", "R12H", "R12B"}},
	{"R13", {"R13", "R13D", "R13W", "R13H", "R13B"}},
	{"R14", {"R14", "R14D", "R14W", "R14H", "R14B"}},
	{"R15", {"R15", "R15D", "R15W", "R15H", "R15B"}},
};