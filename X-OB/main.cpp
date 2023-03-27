#include <iostream>
#include <fstream>
#include <Windows.h>
#include <vector>
#include <string>

#include "instruction.h"



int main(int argc, char** argv) {
#ifdef _DEBUG
	std::vector<std::uint8_t> ExampleInstruction = { 0x33, 0xc9, 0x48, 0x8b, 0x0e, 0x48, 0x8b, 0xd5, 0x00, 0x00 };
	Instruction* instruction = Instruction::Decode(ExampleInstruction);
	
	while (instruction->op->m_Opcode) {
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