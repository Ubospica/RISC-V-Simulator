//
// Created by Ubospica on 2021/6/29.
//

#ifndef RISCV_BITOPERATIONS_HPP
#define RISCV_BITOPERATIONS_HPP

#include <cstddef>


namespace RISCV {
	template <typename T, uint32_t B>
	T signExtend(const T &val) {
		struct {T x:B;} s;
		s.x=val;
		return s.x;
	}
	
	template <uint32_t B>
	int32_t signedExtend (const int32_t &val) {
		return signExtend<int32_t, B>(val);
	}
	template <uint32_t B>
	uint32_t unsignedExtend (const uint32_t &val) {
		return signExtend<uint32_t, B>(val);
	}
	
	inline uint32_t getBitBetween(uint32_t val, uint32_t l, uint32_t r) {
		return (val >> l) & ((1 << (r - l + 1)) - 1);
	}
	
	union DivideType {
		uint32_t value;
		struct {
			unsigned opcode     :7;
			unsigned rd         :5;
			unsigned funct3     :3;
			unsigned rs1        :5;
			unsigned rs2        :5;
			unsigned funct7     :7;
		};
		DivideType(uint32_t value = 0) : value(value) {}
	};
	
	int32_t getImmIType(const DivideType &divide) {
		return signedExtend<12>((divide.funct7 << 5) | divide.rs2);
	}
	
	int32_t getImmSType(const DivideType &divide) {
		return signedExtend<12>((divide.funct7 << 5u) | divide.rd);
	}
	
	int32_t getImmBType(const DivideType &divide) {
		return signedExtend<13>((getBitBetween(divide.value, 8, 11) << 1) | (getBitBetween(divide.value, 25, 30) << 5) |
				(getBitBetween(divide.value, 7, 7) << 11) | (getBitBetween(divide.value, 31, 31) << 12));
	}
	
	int32_t getImmUType(const DivideType &divide) {
		return getBitBetween(divide.value, 12, 31) << 12;
	}
	
	int32_t getImmJType(const DivideType &divide) {
		return signedExtend<21>((getBitBetween(divide.value, 21, 30) << 1) | (getBitBetween(divide.value, 20, 20) << 11) |
		                        (getBitBetween(divide.value, 12, 19) << 12) | (getBitBetween(divide.value, 31, 31) << 20));
	}
	
	int32_t getImm(DivideType divide) {
		int32_t imm = 0;
		switch (divide.opcode) {
			case 0x03:
			case 0x13:
			case 0x67: //I Type
				imm = getImmIType(divide);
				break;
			case 0x23:
				imm = getImmSType(divide);
				break;
			case 0x37:
			case 0x17: //U Type
				imm = getImmUType(divide);
				break;
			case 0x6F:
				imm = getImmJType(divide);
				break;
			case 0x63:
				imm = getImmBType(divide);
				break;
			default:
				//R type
				break;
		}
		return imm;
	}
	
	enum class OperationType {
		R, I, S, B, U, J,
	};
	OperationType getType(DivideType divide) {
		switch (divide.opcode) {
			case 0x03:
			case 0x13:
			case 0x67: //I Type
				return OperationType::I;
			case 0x23:
				return OperationType::S;
			case 0x37:
			case 0x17: //U Type
				return OperationType::U;
			case 0x6F:
				return OperationType::J;
			case 0x63:
				return OperationType::B;
			default:
				return OperationType::R;
		}
	}
	
//	void divideRType(uint32_t val, DivideType &res) {
//		res.opcode = getBitBetween(val, 0, 6);
//		res.rd = getBitBetween(val, 7, 11);
//		res.funct3 = getBitBetween(val, 12, 14);
//		res.rs1 = getBitBetween(val, 15, 19);
//		res.rs2 = getBitBetween(val, 20, 24);
//		res.funct7 = getBitBetween(val, 25, 31);
//	}
//
//	void divideIType(uint32_t val, DivideType &res) {
//		res.opcode = getBitBetween(val, 0, 6);
//		res.rd = getBitBetween(val, 7, 11);
//		res.funct3 = getBitBetween(val, 12, 14);
//		res.rs1 = getBitBetween(val, 15, 19);
//		res.imm = signedExtend<12>(getBitBetween(val, 20, 31);
//	}
//
//	void divideSType(uint32_t val, DivideType &res) {
//		res.opcode = getBitBetween(val, 0, 6);
//		res.rd = getBitBetween(val, 7, 11);
//		res.funct3 = getBitBetween(val, 12, 14);
//		res.rs1 = getBitBetween(val, 15, 19);
//		res.imm = signedExtend<12>(getBitBetween(val, 20, 31);
//	}
}


#endif //RISCV_BITOPERATIONS_HPP
