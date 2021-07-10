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
//	freopen("a.in", "r", stdin);
//	freopen("a.err", "w", stderr);
	RISCV::run();
	return 0;
}
