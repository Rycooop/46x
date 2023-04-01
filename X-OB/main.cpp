#include <iostream>
#include <fstream>
#include <Windows.h>
#include <vector>
#include <string>

#include "instruction.h"



int main(int argc, char** argv) {
#ifdef _DEBUG
	std::vector<std::uint8_t> ExampleInstruction = { 0x48, 0x89, 0xc3, 0x51, 0x90, 0x83, 0xec, 0x28, 0x48, 0x8d, 0x6c, 0x24, 0x20, 0x90, 0x05, 0x25, 0xB2, 0xAC, 0x01, 0xC7, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x9A, 0xFC, 0xFF, 0xFF, 0x48, 0x83, 0xC4, 0x28, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00 };
	Instruction* instruction = Instruction::Decode(ExampleInstruction);

	while (instruction->m_ValidInstruction) {
		std::cout << "[+] Instruction Size: " << std::hex << unsigned(instruction->m_Size) << " REX byte: " << unsigned(instruction->rex->r) << " Opcode: " << unsigned(instruction->op->m_Opcode) << " - Instruction as string: " << instruction->op->m_ToString->c_str() << std::endl;

		ExampleInstruction = std::vector<std::uint8_t>(ExampleInstruction.begin() + instruction->m_Size, ExampleInstruction.end());
		instruction = Instruction::Decode(ExampleInstruction);
	}
#else
	if (argc < 2) return -1;

	std::string arg = std::string(argv[1]);

	std::vector<std::uint8_t> Bytes;
	std::uint8_t curr;
	
#endif
	return 0;
}