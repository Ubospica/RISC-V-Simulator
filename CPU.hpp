//
// Created by Ubospica on 2021/6/29.
//

#ifndef RISCV_CPU_HPP
#define RISCV_CPU_HPP

#include <iostream>

#include "BitOperations.hpp"

namespace RISCV {
	template <size_t SIZE = 600000>
	struct Memory {
		uint8_t val[SIZE] {0};
		uint8_t &operator[] (uint32_t pos) {
			return val[pos];
		}
		const uint8_t &operator[] (uint32_t pos) const {
			return val[pos];
		}
		
		uint8_t& getPos8 (uint32_t pos) {
			return *(val + pos);
		}
		
		uint16_t& getPos16 (uint32_t pos) {
			return *(reinterpret_cast<uint16_t*>(val + pos));
		}
		
		uint32_t& getPos32 (uint32_t pos) {
			return *(reinterpret_cast<uint32_t*>(val + pos));
		}
	};
	
	
	template <size_t SIZE = 32>
	struct Register {
		uint32_t val[SIZE] {0};
		uint32_t &operator[] (uint32_t pos) {
			return val[pos];
		}
		const uint32_t &operator[] (uint32_t pos) const {
			return val[pos];
		}
		
	};
	
	class CPU {
	public:
		Memory<> mem;
		Register<> reg;
		uint32_t pc = 0;
		
		void run() {
			while (true) {
				uint32_t op = *reinterpret_cast<uint32_t*>(mem.val + pc);
				if (op == 0x0ff00513) {
					printf("%u\n", getBitBetween(reg[10], 0, 7));
					break;
				}
				DivideType divide(op);
//				std::cerr << std::hex << "operaton: " << divide.opcode << " rd/rs1/rs2: " << divide.rd << '/' << divide.rs1 << '/' << divide.rs2
//					<< " funct3/7: " << divide.funct3 << '/' << divide.funct7 << '\n';
				switch (divide.opcode) {
					case 0x03: { //load
						switch (divide.funct3) {
							case 0:
								lb(divide, getImmIType(divide));
								break;
							case 1:
								lh(divide, getImmIType(divide));
								break;
							case 2:
								lw(divide, getImmIType(divide));
								break;
							case 4:
								lbu(divide, getImmIType(divide));
								break;
							case 5:
								lhu(divide, getImmIType(divide));
								break;
						}
						break;
					}
					case 0x23: { //s
						switch (divide.funct3) {
							case 0:
								sb(divide, getImmSType(divide));
								break;
							case 1:
								sh(divide, getImmSType(divide));
								break;
							case 2:
								sw(divide, getImmSType(divide));
								break;
						}
						break;
					}
					case 0x33: {
						switch (divide.funct3) {
							case 0:
								if (divide.funct7 == 0) {
									add(divide);
								}
								else {
									sub(divide);
								}
								break;
							case 1:
								sll(divide);
								break;
							case 2:
								slt(divide);
								break;
							case 3:
								sltu(divide);
								break;
							case 4:
								getxor(divide);
								break;
							case 5:
								if (divide.funct7 == 0) {
									srl(divide);
								}
								else {
									sra(divide);
								}
								break;
							case 6:
								getor(divide);
								break;
							case 7:
								getand(divide);
								break;
								
						}
						break;
					}
					case 0x13: {
						switch (divide.funct3) {
							case 0:
								addi(divide, getImmIType(divide));
								break;
							case 2:
								slti(divide, getImmIType(divide));
								break;
							case 3:
								sltiu(divide, getImmIType(divide));
								break;
							case 4:
								xori(divide, getImmIType(divide));
								break;
							case 6:
								ori(divide, getImmIType(divide));
								break;
							case 7:
								andi(divide, getImmIType(divide));
								break;
							case 1:
								slli(divide);//
								break;
							case 5:
								if (divide.funct7 == 0){
									srli(divide);//
								}
								else {
									srai(divide);
								}
								break;
						}
						break;
					}
					case 0x37: {
						lui(divide, getImmUType(divide));
						break;
					}
					case 0x17: {
						auipc(divide, getImmUType(divide));
						break;
					}
					case 0x6F: {
						jal(divide, getImmJType(divide));
						break;
					}
					case 0x67: {
						jalr(divide, getImmIType(divide));
						break;
					}
					case 0x63: {
						switch (divide.funct3) {
							case 0:
								beq(divide, getImmBType(divide));
								break;
							case 1:
								bne(divide, getImmBType(divide));
								break;
							case 4:
								blt(divide, getImmBType(divide));
								break;
							case 5:
								bge(divide, getImmBType(divide));
								break;
							case 6:
								bltu(divide, getImmBType(divide));
								break;
							case 7:
								bgeu(divide, getImmBType(divide));
								break;
								
						}
						break;
					}
					case 0 : default : {
						goto out; // jump out of nested loops
					}
				}
				
				pc += 4;
//				break;
			}
			out:;
		}
		
		void lb(const DivideType &divide, int32_t imm) {
//			std::cout << "lb" << divide.rs1 << ' ' << divide.rd << ' ' << imm << '\n';
			reg[divide.rd] = signedExtend<8>(mem.getPos8(reg[divide.rs1] + imm));
		}
		void lh(const DivideType &divide, int32_t imm) {
			reg[divide.rd] = signedExtend<16>(mem.getPos16(reg[divide.rs1] + imm));
		}
		void lw(const DivideType &divide, int32_t imm) {
			reg[divide.rd] = mem.getPos32(reg[divide.rs1] + imm);
		}
		
		
		void lbu(const DivideType &divide, int32_t imm) {
//			std::cout << "lb" << divide.rs1 << ' ' << divide.rd << ' ' << imm << '\n';
			reg[divide.rd] = unsignedExtend<8>(mem.getPos8(reg[divide.rs1] + imm));
		}
		void lhu(const DivideType &divide, int32_t imm) {
			reg[divide.rd] = unsignedExtend<16>(mem.getPos16(reg[divide.rs1] + imm));
		}
		void lwu(const DivideType &divide, int32_t imm) {
			reg[divide.rd] = mem.getPos32(reg[divide.rs1] + imm);
		}
		
		void sb(const DivideType &divide, int32_t imm) {
//			std::cout << "sb" << divide.rs1 << ' ' << divide.rs2 << ' ' << imm << '\n';
			mem.getPos8(reg[divide.rs1] + imm) = getBitBetween(reg[divide.rs2], 0, 7);
		}
		void sh(const DivideType &divide, int32_t imm) {
			mem.getPos16(reg[divide.rs1] + imm) = getBitBetween(reg[divide.rs2], 0, 15);
		}
		void sw(const DivideType &divide, int32_t imm) {
			mem.getPos32(reg[divide.rs1] + imm) = reg[divide.rs2];
		}
		void add(const DivideType &divide) {
			reg[divide.rd] = reg[divide.rs1] + reg[divide.rs2];//ignoring overflow
		}
		void sub(const DivideType &divide) {
			reg[divide.rd] = reg[divide.rs1] - reg[divide.rs2];//ignoring overflow
		}
		void getxor(const DivideType &divide) {
			reg[divide.rd] = reg[divide.rs1] ^ reg[divide.rs2];
		}
		void getor(const DivideType &divide) {
			reg[divide.rd] = reg[divide.rs1] | reg[divide.rs2];
		}
		void getand(const DivideType &divide) {
			reg[divide.rd] = reg[divide.rs1] & reg[divide.rs2];
		}
		void sll(const DivideType &divide) {
			reg[divide.rd] = reg[divide.rs1] << getBitBetween(reg[divide.rs2], 0, 4);
		}
		void srl(const DivideType &divide) {
			reg[divide.rd] = reg[divide.rs1] >> getBitBetween(reg[divide.rs2], 0, 4);
		}
		void sra(const DivideType &divide) {
			reg[divide.rd] = static_cast<int32_t>(reg[divide.rs1]) >> static_cast<int32_t>(getBitBetween(reg[divide.rs2], 0, 4));
		}
		void slt(const DivideType &divide) {
			reg[divide.rd] = static_cast<bool>(static_cast<int32_t>(reg[divide.rs1]) < static_cast<int32_t>(reg[divide.rs2]));
		}
		void sltu(const DivideType &divide) {
			reg[divide.rd] = static_cast<bool>(reg[divide.rs1] < reg[divide.rs2]);
		}
		void addi(const DivideType &divide, int32_t imm) {
			reg[divide.rd] = reg[divide.rs1] + imm;//ignoring overflow
		}
		void xori(const DivideType &divide, int32_t imm) {
			reg[divide.rd] = reg[divide.rs1] ^ imm;
		}
		void ori(const DivideType &divide, int32_t imm) {
			reg[divide.rd] = reg[divide.rs1] | imm;
		}
		void andi(const DivideType &divide, int32_t imm) {
			reg[divide.rd] = reg[divide.rs1] & imm;
		}
		void slli(const DivideType &divide) { //assert(shamt[5] == 0)
			reg[divide.rd] = reg[divide.rs1] << divide.rs2;
		}
		void srli(const DivideType &divide) {
			reg[divide.rd] = reg[divide.rs1] >> divide.rs2;
		}
		void srai(const DivideType &divide) {
			reg[divide.rd] = static_cast<int32_t>(reg[divide.rs1]) >> static_cast<int32_t>(divide.rs2);
		}
		void slti(const DivideType &divide, int32_t imm) {
			reg[divide.rd] = static_cast<bool>(static_cast<int32_t>(reg[divide.rs1]) < imm);
		}
		void sltiu(const DivideType &divide, int32_t imm) {
			reg[divide.rd] = static_cast<bool>(reg[divide.rs1] < static_cast<uint32_t>(imm));
		}
		void lui(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
			reg[divide.rd] = imm;
		}
		void auipc(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
			reg[divide.rd] = pc + imm;
		}
		void beq(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
			if (reg[divide.rs1] == reg[divide.rs2]) {
				pc += imm - 4;
			}
		}
		void bne(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
			if (reg[divide.rs1] != reg[divide.rs2]) {
				pc += imm - 4;
			}
		}
		void blt(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
			if (static_cast<int32_t>(reg[divide.rs1]) < static_cast<int32_t>(reg[divide.rs2])) {
				pc += imm - 4;
			}
		}
		void bge(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
			if (static_cast<int32_t>(reg[divide.rs1]) >= static_cast<int32_t>(reg[divide.rs2])) {
				pc += imm - 4;
			}
		}
		void bltu(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
			if (reg[divide.rs1] < reg[divide.rs2]) {
				pc += imm - 4;
			}
		}
		void bgeu(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
			if (reg[divide.rs1] >= reg[divide.rs2]) {
				pc += imm - 4;
			}
		}
		void jal(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
			if (divide.rd) reg[divide.rd] = pc + 4;
			pc += imm - 4;
		}
		void jalr(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
			uint32_t t = pc + 4;
			pc = ((reg[divide.rs1] + imm) & (~1u)) - 4;
			if (divide.rd) reg[divide.rd] = t;
		}
	};
	
}

#endif //RISCV_CPU_HPP
