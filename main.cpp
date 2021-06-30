#include <iostream>

#include "CPU.hpp"
#include "IO.hpp"


namespace RISCV {
	
	
	void run() {
		CPU cpu;
		readMemory(cpu.mem, stdin);
		cpu.run();
	}
	
}


int main() {
	RISCV::run();
	return 0;
}
