#ifndef XOB_REGISTER_H
#define XOB_REGISTER_H

#include <cstdint>
#include <vector>
#include <map>
#include <algorithm>
#include <string>


class Register {
private:
	std::uint64_t Val;
	std::string RegName;

public:
	Register() = default;
	Register(const std::string& Name, std::uint64_t Value);

	std::uint64_t GetReg() { return this->Val; }
	std::uint32_t Get32Bit() { return (std::uint32_t)this->Val; }
	std::uint16_t Get16Bit() { return (std::uint16_t)this->Val; }
	std::uint8_t GetHigher8() { return (this->Val >> 8) & 0xFF; }
	std::uint8_t GetLower8() { return (std::uint8_t)this->Val; };

	static bool IsValidRegister(const std::string& name) { return std::find(VALID_REGISTERS.begin(), VALID_REGISTERS.end(), name) != VALID_REGISTERS.end();  }
	
	static std::string GetRegisterPart(const std::string& reg, const std::uint8_t partIndex);

	static bool GetTwosCompliment8(std::uint8_t& val);
	static bool GetTwosCompliment32(std::uint32_t& val);

	const static std::vector<std::string> VALID_REGISTERS;
	const static std::map<std::string, std::vector<std::string>> REGISTER_PARTS;
	const static std::map<std::string, std::vector<std::string>> REGISTER_PARTS64;

	enum REGISTER_PART_INDEX {
		PART_EXTENDED64 = 0,
		PART_32 = 1,
		PART_16,
		PART_HIGHER_8,
		PART_LOWER_8,
	};
};


#endif //XOB_REGISTER_H
