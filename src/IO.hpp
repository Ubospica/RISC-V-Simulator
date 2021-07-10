//
// Created by Ubospica on 2021/6/29.
//

#ifndef RISCV_IO_HPP
#define RISCV_IO_HPP


#include <cstdio>
#include "CPU.hpp"


namespace RISCV {
	void readMemory(CPU::Memory<> &mem, FILE *input) {
		char str[20];
		uint32_t pos = 0;
		uint32_t val;
		while (fscanf(input, "%s", str) == 1) {
			if (str[0] == '@') {
				sscanf(str, "@%X", &val);
				pos = val;
			}
			else {
				sscanf(str, "%X", &val);
				mem[pos] = static_cast<uint8_t>(val);
				++pos;
			}
		}
	}
}



#endif //RISCV_IO_HPP
