#ifndef XOB_INSTRUCTION_H
#define XOB_INSTRUCTION_H

#ifdef _DEBUG
#include <iostream>
#endif

#include <vector>
#include <map>
#include <string>
#include <tuple>

#include "register.h"

#define INSTRUCTION_MAX_LENGTH 15


class Instruction {
public:
	std::vector<unsigned char> Bytes;
	std::vector<std::uint8_t> LegacyPrefixes;
	std::uint8_t Length;

	//Mandatory prefix, rex prefix, opcode

	Instruction() {
		this->LegacyPrefixes = std::vector<std::uint8_t>(1);
		this->Length = 0;
		this->op = new Opcode();
	}

	static Instruction* Decode(const std::vector<std::uint8_t>& bytes);

	const static std::uint8_t GetPrefix(const std::vector<std::uint8_t>& Bytes);


	//Define all opcodes with their hexadecimal representation


	class Prefix {
	public:
		static bool IsLegacyPrefix(std::uint8_t b) { return (std::find(LEGACY_PREFIX.begin(), LEGACY_PREFIX.end(), b) != LEGACY_PREFIX.end()); }

		const static std::vector<std::uint8_t> LEGACY_PREFIX;
		const static std::vector<std::uint8_t> REX;
		
		class Rex {
		public:
			static bool IsREX(std::uint8_t b) { return (std::find(REX.begin(), REX.end(), b) != REX.end()); }

			std::uint8_t r;

			bool WSet() { return (r & 8); }
			bool RSet() { return (r & 4); }
			bool XSet() { return (r & 2); }
			bool BSet() { return (r & 1); }
		};
	};

	class Opcode {
	public:
		std::uint8_t m_Opcode;
		std::vector<std::uint8_t> m_Bytes;
		std::vector<std::uint8_t> m_Operands;
		std::string* m_ToString;

		static Opcode GetOpcode(const std::vector<std::uint8_t>& bytes, Instruction::Prefix::Rex* rex);
		inline static bool HasOpcodeEscapeSequence(const std::vector<std::uint8_t>& opcode) { return (opcode[0] == 0x0F); }
		
		const static std::map<std::uint8_t, std::vector<std::uint8_t>> OPCODE_LOOKUP_TABLE;
		const static std::map<std::uint8_t, std::vector<std::string>> OPCODE_NAME_LOOKUP_TABLE;

		enum OPERAND_TYPE {
			OPERAND_REGISTER,
			OPERAND_MODRM,
			OPERAND_IMMEDIATE8,
			OPERAND_IMMEDIATE32,
		};

		class ModRM {
		public:
			const static bool ContainsModRM(const std::vector<std::uint8_t>& operands);
			const static bool NeedsModRMByte(const std::vector<std::uint8_t>& operands);
			const static inline bool IsModRMArg(std::uint8_t arg) { return (arg == OPERAND_MODRM); }

			const static std::map<std::uint8_t, std::vector<std::uint8_t>> MODRM_LOOKUP_TABLE;

			const static std::map<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>, std::string> MODRM_NAME_LOOKUP_TABLE; //PAIR(MOD, RM, REX.B)
			const static std::map<std::pair<std::uint8_t, std::uint8_t>, std::string> MODR_NAME_LOOKUP_TABLE; //PAIR(REG, REX.R)
		};

		class SIB {
		public:

		};
	};

	const static std::map<std::string, std::uint8_t> INSTRUCTION_CLASS;
	Instruction::Opcode* op;
	Instruction::Prefix::Rex* rex;

	enum INSTRUCTION_ENCODING {
		REX = 0,

	};
};


#endif //XOB_INSTRUCTION_H