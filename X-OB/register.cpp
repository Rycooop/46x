#include "register.h"

Register::Register(const std::string& Name, std::uint64_t Value) {
	this->RegName = Name;
	this->Val = Value;
}

std::string Register::GetRegisterPart(const std::string& reg, const std::uint8_t partIndex) {
	return Register::REGISTER_PARTS.at(reg)[partIndex];
}

const std::vector<std::string> Register::VALID_REGISTERS = {
		"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};

const std::map<std::string, std::vector<std::string>> Register::REGISTER_PARTS = {
	{"RAX", {"EAX", "AX", "AH", "AL"}},
	{"RBX", {"EBX", "BX", "BH", "BL"}},
	{"RCX", {"ECX", "AX", "AH", "AL"}},
	{"RDX", {"EDX", "AX", "AH", "AL"}},
	{"RSI", {"ESI", "AX", "AH", "AL"}},
	{"RDI", {"EDI", "AX", "AH", "AL"}},
	{"RBP", {"EBP", "AX", "AH", "AL"}},
	{"RSP", {"ESP", "AX", "AH", "AL"}},
};