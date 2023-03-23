#include <iostream>
#include <Windows.h>
#include <vector>
#include <string>

#include "instruction.h"



int main(int argc, char** argv) {
	std::vector<std::uint8_t> ExampleInstruction = { 0x48, 0x89, 0x6C, 0x24, 0x18 };

	Instruction* instruction = Instruction::Decode(ExampleInstruction);

	if (instruction->op->m_ToString->size()) {
		std::cout << "[+] Legacy prefixes: " << std::hex << unsigned(instruction->LegacyPrefixes.at(0)) << " REX byte: " << unsigned(instruction->rex->r) << " Opcode: " << unsigned(instruction->op->m_Opcode) << " - Instruction as string: " << instruction->op->m_ToString->c_str() << std::endl;
		delete instruction;
	}


	return 0;
}