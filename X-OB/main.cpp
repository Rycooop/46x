#include <iostream>
#include <fstream>
#include <Windows.h>
#include <vector>
#include <string>

#include "instruction.h"



int main(int argc, char** argv) {
#ifdef _DEBUG
	std::vector<std::uint8_t> ExampleInstruction = { 0x48, 0x39, 0x85, 0x88, 0x00, 0x00, 0x00, 0x90, 0x08, 0x48, 0x8d, 0x40, 0xf8, 0x48, 0xc1, 0xe9, 0x08, 0x88, 0x48, 0x07, 0x48, 0xc1, 0xe9, 0x08, 0x90 };
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