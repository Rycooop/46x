#ifndef XOB_INSTRUCTION_H
#define XOB_INSTRUCTION_H

#ifdef _DEBUG
#endif

#include <vector>
#include <map>
#include <string>
#include <tuple>
#include <iostream>

#include "register.h"

#define INSTRUCTION_MAX_LENGTH 15


class Instruction {
public:
	std::vector<unsigned char> m_Bytes;
	std::vector<std::uint8_t> m_LegacyPrefixes;
	std::uint8_t m_Size;

	//Mandatory prefix, rex prefix, opcode

	Instruction() {
		this->m_LegacyPrefixes = std::vector<std::uint8_t>(1);
		this->m_Size = 0;
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
		const static bool OpcodeHasRegisterExtension(std::uint8_t opcode);
		const static bool OpcodeDBitOverride(std::uint8_t opcode);

		const static std::map<std::uint8_t, std::vector<std::uint8_t>> OPCODE_LOOKUP_TABLE;
		const static std::map<std::uint8_t, std::vector<std::string>> OPCODE_NAME_LOOKUP_TABLE;

		enum OPERAND_TYPE {
			OPERAND_REGISTER_RAX,
			OPERAND_MODRM,
			OPERAND_IMMEDIATE8,
			OPERAND_IMMEDIATE32,
			OPERAND_REL8,
			OPERAND_REL32,
		};

		class ModRM {
		public:
			const static std::string GetDisplacement(std::uint8_t mod, std::uint8_t rm, std::uint8_t rexw, std::uint8_t rexx, std::uint8_t rexb, std::vector<std::uint8_t>& bytes);
			const static std::uint8_t GetDisplacementInBytes(std::uint8_t mod);

			const static bool NeedsModRMByte(std::uint8_t opcode, const std::vector<std::uint8_t>& operands);
			const static bool HasDoubleModRM(const std::vector<std::uint8_t>& operands);
			const static inline bool IsModRMArg(std::uint8_t arg) { return (arg == OPERAND_MODRM); }

			const static std::uint8_t GetAdressingMode(std::uint8_t mod);

			const static std::map<std::pair<std::uint8_t, std::uint8_t>, std::string> MODRM_REGISTER_LOOKUP_TABLE; //PAIR(REG/RM, REX.B/R)

			enum ADDRESSING_MODE {
				MOD_INDIRECT_ADDRESSING = 0,
				MOD_INDIRECT_DISP8,
				MOD_INDIRECT_DISP32,
				MOD_DIRECT_ADDRESSING,
			};
		};

		class SIB {
		public:
			static std::string GetSIB(std::uint8_t sib, std::uint8_t rexw, std::uint8_t rexb, std::uint8_t rexx);

			static const std::map<std::pair<std::uint8_t, std::uint8_t>, std::string> SIB_BASE_LOOKUP; //PAIR(sib.base, rex.b)
			static const std::map<std::tuple<std::uint8_t, std::uint8_t, std::uint8_t>, std::string> SIB_INDEX_LOOKUP; //PAIR(sib.scale, sib.index, rex.x)
		};
	};

	const static std::map<std::string, std::uint8_t> INSTRUCTION_CLASS;
	Instruction::Opcode* op;
	Instruction::Prefix::Rex* rex;

	enum INSTRUCTION_ENCODING {
		REX = 0,

	};
};


#endif //XOB_INSTRUCTION