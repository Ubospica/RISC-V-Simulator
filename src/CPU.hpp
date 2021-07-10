//
// Created by Ubospica on 2021/6/29.
//

#ifndef RISCV_CPU_HPP
#define RISCV_CPU_HPP

#include <climits>
#include <iostream>

#include "BitOperations.hpp"

namespace RISCV {
	
//
//	template <size_t SIZE = 32>
//	struct Register {
//		uint32_t val[SIZE] {0};
//		uint32_t &operator[] (uint32_t pos) {
//			return val[pos];
//		}
//		const uint32_t &operator[] (uint32_t pos) const {
//			return val[pos];
//		}
//
//	};
	
	constexpr int MEM_SIZE = 600000, REG_SIZE = 32, FQ_SIZE = 32, rs_SIZE = 32, rob_SIZE = 32, slb_SIZE = 32;
	constexpr uint32_t INF32 = UINT32_MAX;
	constexpr uint8_t INF8 = UINT8_MAX;
	
	class CPU {
	public:
		template <size_t SIZE = MEM_SIZE>
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
		
		
		template <typename T, size_t SZ>
		struct Queue {
			T val[SZ] {};
			uint32_t head = 1, tail = 1;
			
			T &operator[] (uint32_t pos) {
				return val[pos % SZ];
			}
			const T &operator[] (uint32_t pos) const {
				return val[pos % SZ];
			}
			
			T& front() {
				return val[head % SZ];
			}
			uint32_t push(const T &v) {
				val[tail % SZ] = v;
				++tail;
				return tail;
			}
			void pop() {
				++head;
			}
			uint32_t size() {
				return tail - head;
			}
			void clear() {
				head = tail = 1;
			}
			bool empty() {
				return size() == 0;
			}
			bool full() {
				return size() >= SZ;
			}
		};
		
		struct execute_t {
			uint32_t reg = 0, pc = 0;
			bool jump = false;
		};
		
		struct FetchQueue_t {
			uint32_t op, pc;
		};
		
		struct rs_t {
			bool busy = false;
			DivideType divide = 0; //opcode funct7 funct3 rs1 rs2 rd
			int32_t imm;
			uint32_t rs1_data, rs1_rob, rs2_data, rs2_rob, rd_rob, time;
			uint32_t pc; //jump
		};
		
		struct ReservationStation {
			rs_t val[rs_SIZE];
			rs_t &operator[] (uint32_t pos) {
				return val[pos];
			}
			const rs_t &operator[] (uint32_t pos) const {
				return val[pos];
			}
		};
		
		struct ReorderBuffer_t {
			DivideType divide;
			uint32_t reg_id, rs_id, slb_id;
			bool ready;
			execute_t res;
		};
		
		struct Register_t {
			uint32_t rob = 0, data = 0;
		};
		
		struct Register {
			Register_t val[REG_SIZE];
			
			Register_t &operator[] (uint32_t pos) {
				return val[pos];
			}
			const Register_t &operator[] (uint32_t pos) const {
				return val[pos];
			}
		};
		
		struct SaveLoadBuffer_t {
			DivideType divide; //op, rd
			int32_t imm;
			uint32_t rs1_data, rs1_rob, rs2_data, rs2_rob, rd_rob, time;
		};
		
		
		Memory<> mem;
		uint32_t cycle = 0;
		
		
		Queue<FetchQueue_t, FQ_SIZE> fetchQueue_prev, fetchQueue_next;
		uint32_t pc_fetch = 0;
		
		
		Register reg_prev, reg_next;
		ReservationStation rs_prev, rs_next;
		Queue<ReorderBuffer_t, rob_SIZE> rob_prev, rob_next;
		Queue<SaveLoadBuffer_t, slb_SIZE> slb_prev, slb_next;
		
		
		
		//common data bus
		FetchQueue_t fetch_to_issue;
		ReorderBuffer_t issue_to_rob;
		SaveLoadBuffer_t issue_to_slb;
		
		struct issue_to_rs_t {
			DivideType divide;
			int32_t imm;
			uint32_t rs1, rs2, time, pc, rob_id, rs_id;
		} issue_to_rs;
		
		struct issue_to_regfile_t {
			uint32_t reg, rob;
		} issue_to_regfile;
		
		struct rs_to_ex_t {
			DivideType divide;
			int32_t imm;
			uint32_t v1, v2, pc, rob_id, time;
		} rs_to_ex;
		
		
		struct ex_to_rob_t {
			uint32_t rob;
			execute_t res;
		}ex_to_rob, slb_to_rob_prev, slb_to_rob_next;
		struct ex_to_rs_t {
			uint32_t rob, reg;
		}ex_to_rs, ex_to_slb, slb_to_rs_prev, slb_to_rs_next, slb_to_slb_prev, slb_to_slb_next;
		
		struct rob_to_commit_t {
			DivideType divide;
			uint32_t reg_id, rob_id, slb_id;
			execute_t res;
		} rob_to_commit;
		
		struct commit_to_regfile_t {
			uint32_t rob_id, reg_id, data;
		} commit_to_regfile;
		struct commit_to_slb_t {
			uint32_t slb_id;
		} commit_to_slb;
		
		
		//stall
		bool stall_fetch_queue = true, stall_fetch_to_issue = true;
		bool stall_issue_to_fetch = true, stall_issue_to_rob = true, stall_issue_to_rs = true, stall_issue_to_slb = true, stall_issue_to_regfile = true;
		bool stall_rs_to_ex = true;
		bool stall_ex_to_rob = true, stall_ex_to_rs = true, stall_ex_to_slb = true;
		bool stall_slb_to_rob_prev = true, stall_slb_to_rs_prev = true, stall_slb_to_slb_prev = true;
		bool stall_slb_to_rob_next = true, stall_slb_to_rs_next = true, stall_slb_to_slb_next = true;
		bool stall_rob_to_commit = true;
		bool stall_commit_to_regfile = true, stall_commit_to_slb = true;
		bool stall_clear_rob = true, stall_clear_slb = true, stall_clear_rs = true, stall_clear_fq = true, stall_clear_regfile = true;

		CPU() {
		
		}
		
		void run(){
			pc_fetch = 0;
			for (cycle = 0; ; ++cycle){
				/*在这里使用了两阶段的循环部分：
				  1. 实现时序电路部分，即在每个周期初同步更新的信息。
				  2. 实现组合电路部分，即在每个周期中如ex、issue的部分
				  已在下面给出代码
				*/
//				if (cycle > 100000) {
//					std::cerr << "inf loop\n";
//					exit(255);
//				}
				run_rob();
				
//				std::cerr << "rob_to_commit: " << !stall_rob_to_commit << ' ' << rob_to_commit.divide.value << '\n';
				
				run_slbuffer();
				run_reservation();
				run_regfile();
				run_inst_fetch_queue();
				update();
				
				run_ex();
				run_issue();
				run_commit();
				
				if (!stall_rob_to_commit && rob_to_commit.divide.value == 0x0ff00513) {
					printf("%u\n", getBitBetween(reg_prev[10].data, 0, 7));
					break;
				}
			}
		}
		
		void run_inst_fetch_queue(){
			/*
			在这一部分你需要完成的工作：
			1. 实现一个先进先出的指令队列
			2. 读取指令并存放到指令队列中
			3. 准备好下一条issue的指令
			tips: 考虑边界问题（满/空...）
			*/
			stall_fetch_to_issue = true;
			if (!stall_clear_fq) {
				fetchQueue_next.clear();
				return;
			}
			stall_fetch_queue = fetchQueue_prev.full();
			if (!stall_fetch_queue) {
				fetchQueue_next.push((FetchQueue_t){*reinterpret_cast<uint32_t*>(mem.val + pc_fetch), pc_fetch});
				pc_fetch += 4;
			}
			if (!stall_issue_to_fetch) {
				if (fetchQueue_prev.size() > 1) {
					fetch_to_issue = fetchQueue_prev[fetchQueue_prev.head + 1];
					stall_fetch_to_issue = false;
				}
				fetchQueue_next.pop();
			}
			else {
				if (!fetchQueue_prev.empty()) {
					fetch_to_issue = fetchQueue_prev.front();
					stall_fetch_to_issue = false;
				}
			}
		}
		
		
		void run_issue(){
			/*
			在这一部分你需要完成的工作：
			1. 从run_inst_fetch_queue()中得到issue的指令
			2. 对于issue的所有类型的指令向rob申请一个位置（或者也可以通过rob预留位置），并通知regfile修改相应的值
			2. 对于 非 Load/Store的指令，将指令进行分解后发到Reservation Station
			  tip: 1. 这里需要考虑怎么得到rs1、rs2的值，并考虑如当前rs1、rs2未被计算出的情况，参考书上内容进行处理
				   2. 在本次作业中，我们认为相应寄存器的值已在rob中存储但尚未commit的情况是可以直接获得的，即你需要实现这个功能
					  而对于rs1、rs2不ready的情况，只需要stall即可，有兴趣的同学可以考虑下怎么样直接从EX完的结果更快的得到计算结果
			3. 对于 Load/Store指令，将指令分解后发到slbuffer(需注意slbUFFER也该是个先进先出的队列实现)
			tips: 考虑边界问题（是否还有足够的空间存放下一条指令）
			*/
			stall_issue_to_fetch = stall_issue_to_regfile = stall_issue_to_rob =
					stall_issue_to_slb = stall_issue_to_rs = true;
			if (!stall_fetch_to_issue) {
				if (!rob_next.full()) {
					uint32_t nextrob = rob_prev.tail, nextslb = INF32, nextrs = INF32;
					
					//decode
					DivideType divide(fetch_to_issue.op);
					int32_t imm = getImm(divide);
					
					if (divide.opcode == 0x03 || divide.opcode == 0x23) {
						if (!slb_prev.full()) {
							stall_issue_to_fetch = stall_issue_to_slb = stall_issue_to_rob = false;
							nextslb = slb_prev.tail;
							issue_to_slb = (SaveLoadBuffer_t){divide, imm,
							                                   divide.rs1, 0, divide.rs2, 0, nextrob, 3};
							issue_to_rob = (ReorderBuffer_t) {divide, divide.rd, 0, nextslb};
							if (divide.opcode == 0x03) { //load
								if (divide.rd != 0) {
									stall_issue_to_regfile = false;
									issue_to_regfile = (issue_to_regfile_t) {divide.rd, nextrob};
								}
							}
						}
					}
					else { //arithmetic & jump
						for (int i = 0; i < rs_SIZE; ++i) {
							if (!rs_prev[i].busy) {
								nextrs = i;
								break;
							}
						}
						if (nextrs != INF32) {
							stall_issue_to_fetch = stall_issue_to_rs = stall_issue_to_rob = false;
							issue_to_rs = (issue_to_rs_t) {divide, imm, divide.rs1, divide.rs2,
							        1, fetch_to_issue.pc, nextrob, nextrs};
							issue_to_rob = (ReorderBuffer_t) {divide, divide.rd, nextrs, 0};
							if (divide.opcode != 0x63 && divide.rd != 0) { // not b operation
								stall_issue_to_regfile = false;
								issue_to_regfile = (issue_to_regfile_t) {divide.rd, nextrob};
							}
						}
					}
				}
			}
		}
		//		struct commit_to_regfile_t {
		//			uint32_t reg_id, data;
		//		} commit_to_regfile;
		void run_regfile(){
			/*
			每个寄存器会记录Q和V，含义参考ppt。这一部分会进行写寄存器，内容包括：根据issue和commit的通知修改对应寄存器的Q和V。
			tip: 请注意issue和commit同一个寄存器时的情况: 先commit后issue
			*/
			if (!stall_commit_to_regfile && commit_to_regfile.reg_id) {
				auto &reg = reg_next[commit_to_regfile.reg_id];
				reg.data =  commit_to_regfile.data;
				if (reg.rob == commit_to_regfile.rob_id) {
					reg.rob = 0;
				}
			}
			if (!stall_clear_regfile) {
				for (int i = 0; i < REG_SIZE; ++i) {
					reg_next[i].rob = 0;
				}
				return;
			}
			if (!stall_issue_to_regfile) {
				reg_next[issue_to_regfile.reg].rob = issue_to_regfile.rob;
			}
		}
		
		
		void run_reservation(){
			/*
			在这一部分你需要完成的工作：
			1. 设计一个Reservation Station，其中要存储的东西可以参考CAAQA或其余资料，至少需要有用到的寄存器信息等
			2. 如存在，从issue阶段收到要存储的信息，存进Reservation Station（如可以计算也可以直接进入计算）
			3. 从Reservation Station或者issue进来的指令中选择一条可计算的发送给EX进行计算
			4. 根据上个周期EX阶段或者slbUFFER的计算得到的结果遍历Reservation Station，更新相应的值
			*/
			//		struct rs_t {
			//			bool busy;
			//			DivideType divide = 0; //opcode funct7 funct3 rs1 rs2 rd
			//			int32_t imm;
			//			uint32_t rs1_data, rs1_rob, rs2_data, rs2_rob, rd_rob, time;
			//			uint32_t pc; //jump
			//		};
			//
			
			if (!stall_clear_rs) {
				stall_rs_to_ex = true;
				for (uint32_t i = 0; i < rs_SIZE; ++i) {
					rs_next[i].busy = false;
				}
				return;
			}
			
			if (!stall_issue_to_rs) {
				auto &next = rs_next[issue_to_rs.rs_id];
				next.busy = true;
				next.divide = issue_to_rs.divide;
				next.imm = issue_to_rs.imm;
				next.time = issue_to_rs.time;
				next.rd_rob = issue_to_rs.rob_id;
				next.pc = issue_to_rs.pc;
				if (reg_prev[issue_to_rs.rs1].rob == 0) {
					next.rs1_rob = 0;
					next.rs1_data = reg_prev[issue_to_rs.rs1].data;
				}
				else {
					auto rob_id = reg_prev[issue_to_rs.rs1].rob;
					if (!stall_ex_to_rs && ex_to_rs.rob == rob_id) {
						next.rs1_rob = 0, next.rs1_data = ex_to_rs.reg;
					}
					else if (!stall_slb_to_rs_prev && slb_to_rs_prev.rob == rob_id) {
						next.rs1_rob = 0, next.rs1_data = slb_to_rs_prev.reg;
					}
					else if (rob_prev[rob_id].ready) {
						next.rs1_rob = 0, next.rs1_data = rob_prev[rob_id].res.reg;
					}
					else next.rs1_rob = rob_id;
				}
				
				if (reg_prev[issue_to_rs.rs2].rob == 0) {
					next.rs2_rob = 0;
					next.rs2_data = reg_prev[issue_to_rs.rs2].data;
				}
				else {
					auto rob_id = reg_prev[issue_to_rs.rs2].rob;
					if (!stall_ex_to_rs && ex_to_rs.rob == rob_id) {
						next.rs2_rob = 0, next.rs2_data = ex_to_rs.reg;
					}
					else if (!stall_slb_to_rs_prev && slb_to_rs_prev.rob == rob_id) {
						next.rs2_rob = 0, next.rs2_data = slb_to_rs_prev.reg;
					}
					else if (rob_prev[rob_id].ready) {
						next.rs2_rob = 0, next.rs2_data = rob_prev[rob_id].res.reg;
					}
					else next.rs2_rob = rob_id;
				}
			}
			
			stall_rs_to_ex = true;
			for (int i = 0; i < rs_SIZE; ++i) {
				if (rs_prev[i].busy) {
					auto type = getType(rs_prev[i].divide);
					bool flag = false; //check if operation is ready
					switch (type) {
						case OperationType::R:
						case OperationType::B:
							if (rs_prev[i].rs1_rob == 0 && rs_prev[i].rs2_rob == 0) {
								flag = true;
							}
							break;
						case OperationType::I:
							if (rs_prev[i].rs1_rob == 0) {
								flag = true;
							}
							break;
						case OperationType::U:
						case OperationType::J:
							flag = true;
							break;
					}
					if (flag) {
						stall_rs_to_ex = false;
						rs_to_ex = (rs_to_ex_t) {rs_prev[i].divide, rs_prev[i].imm,
						                         rs_prev[i].rs1_data, rs_prev[i].rs2_data,
						                         rs_prev[i].pc, rs_prev[i].rd_rob,
						                         rs_prev[i].time};
						rs_next[i].busy = false;
						break;
					}
				}
			}
			
			if (!stall_ex_to_rs) {
				for (uint32_t i = 0; i < rs_SIZE; ++i) {
					if (rs_prev[i].busy) {
						if (rs_prev[i].rs1_rob == ex_to_rs.rob) {
							rs_next[i].rs1_rob = 0, rs_next[i].rs1_data = ex_to_rs.reg;
						}
						if (rs_prev[i].rs2_rob == ex_to_rs.rob) {
							rs_next[i].rs2_rob = 0, rs_next[i].rs2_data = ex_to_rs.reg;
						}
					}
				}
			}
			if (!stall_slb_to_rs_prev) {
				for (uint32_t i = 0; i < rs_SIZE; ++i) {
					if (rs_prev[i].busy) {
						if (rs_prev[i].rs1_rob == slb_to_rs_prev.rob) {
							rs_next[i].rs1_rob = 0, rs_next[i].rs1_data = slb_to_rs_prev.reg;
						}
						if (rs_prev[i].rs2_rob == slb_to_rs_prev.rob) {
							rs_next[i].rs2_rob = 0, rs_next[i].rs2_data = slb_to_rs_prev.reg;
						}
					}
				}
			}
		}
		
		//check 0!!!!
		void run_ex(){
			/*
			在这一部分你需要完成的工作：
			根据Reservation Station发出的信息进行相应的计算
			tips: 考虑如何处理跳转指令并存储相关信息
				  Store/Load的指令并不在这一部分进行处理
			*/
			stall_ex_to_rob = stall_ex_to_rs = stall_ex_to_slb = true;
			if (!stall_rs_to_ex) {
				--rs_to_ex.time;
				if (rs_to_ex.time == 0) { //run
					auto res = execute(rs_to_ex.divide, rs_to_ex.imm, rs_to_ex.v1, rs_to_ex.v2, rs_to_ex.pc);
					stall_ex_to_rob = stall_ex_to_rs = stall_ex_to_slb = false;
					ex_to_rob = (ex_to_rob_t) {rs_to_ex.rob_id, res};
					ex_to_rs = (ex_to_rs_t) {rs_to_ex.rob_id, res.reg};
					ex_to_slb = (ex_to_rs_t) {rs_to_ex.rob_id, res.reg};
				}
			}
		}
		
		void run_slbuffer(){
			/*
			在这一部分中，由于slbUFFER的设计较为多样，在这里给出两种助教的设计方案：
			1. 1）通过循环队列，设计一个先进先出的slbUFFER，同时存储 head1、head2、tail三个变量。
			   其中，head1是真正的队首，记录第一条未执行的内存操作的指令；tail是队尾，记录当前最后一条未执行的内存操作的指令。
			   而head2负责确定处在head1位置的指令是否可以进行内存操作，其具体实现为在rob中增加一个head_ensure的变量，每个周期head_ensure做取模意义下的加法，直到等于tail或遇到第一条跳转指令，
			   这个时候我们就可以保证位于head_ensure及之前位置的指令，因中间没有跳转指令，一定会执行。因而，只要当head_ensure当前位置的指令是Store、Load指令，我们就可以向slbuffer发信息，增加head2。
			   简单概括即对head2之前的Store/Load指令，我们根据判断出rob中该条指令之前没有jump指令尚未执行，从而确定该条指令会被执行。
	
			   2）同时slbUFFER还需根据上个周期EX和slbUFFER的计算结果遍历slbUFFER进行数据的更新。
	
			   3）此外，在我们的设计中，将slbUFFER进行内存访问时计算需要访问的地址和对应的读取/存储内存的操作在slbUFFER中一并实现，
			   也可以考虑分成两个模块，该部分的实现只需判断队首的指令是否能执行并根据指令相应执行即可。
	
			2. 1）slb每个周期会查看队头，若队头指令还未ready，则阻塞。
			   
			   2）当队头ready且是load指令时，slb会直接执行load指令，包括计算地址和读内存，
			   然后把结果通知rob，同时将队头弹出。rob commit到这条指令时通知Regfile写寄存器。
			   
			   3）当队头ready且是store指令时，slb会等待rob的commit，commit之后会slb执行这
			   条store指令，包括计算地址和写内存，写完后将队头弹出。
	
			   4）同时slbUFFER还需根据上个周期EX和slbUFFER的计算结果遍历slbUFFER进行数据的更新。
			*/

			if (!stall_issue_to_slb) {
//				if (issue_to_slb.divide.value == 0x00112623) {
//					std::cerr << "w\n";
//				}
				SaveLoadBuffer_t next = issue_to_slb;
				auto reg = reg_prev[issue_to_slb.rs1_data];
				if (reg.rob == 0) {
					next.rs1_rob = 0, next.rs1_data = reg.data;
				}
				else {
					if (!stall_ex_to_slb && ex_to_slb.rob == reg.rob) {
						next.rs1_rob = 0, next.rs1_data = ex_to_slb.reg;
					}
					else if (!stall_slb_to_slb_prev && slb_to_slb_prev.rob == reg.rob) {
						next.rs1_rob = 0, next.rs1_data = slb_to_slb_prev.reg;
					}
					else if (rob_prev[reg.rob].ready) {
						next.rs1_rob = 0, next.rs1_data = rob_prev[reg.rob].res.reg;
					}
					else {
						next.rs1_rob = reg.rob;
					}
				}
				reg = reg_prev[issue_to_slb.rs2_data];
				if (reg.rob == 0) {
					next.rs2_rob = 0, next.rs2_data = reg.data;
				}
				else {
					if (!stall_ex_to_slb && ex_to_slb.rob == reg.rob) {
						next.rs2_rob = 0, next.rs2_data = ex_to_slb.reg;
					}
					else if (!stall_slb_to_slb_prev && slb_to_slb_prev.rob == reg.rob) {
						next.rs2_rob = 0, next.rs2_data = slb_to_slb_prev.reg;
					}
					else if (rob_prev[reg.rob].ready) {
						next.rs2_rob = 0, next.rs2_data = rob_prev[reg.rob].res.reg;
					}
					else {
						next.rs2_rob = reg.rob;
					}
				}
				slb_next.push(next);
			}
			
			if (!stall_ex_to_slb) {
				for (uint32_t i = slb_prev.head; i != slb_prev.tail; ++i) {
					if (slb_prev[i].rs1_rob == ex_to_slb.rob) {
						slb_next[i].rs1_rob = 0, slb_next[i].rs1_data = ex_to_slb.reg;
					}
					if (slb_prev[i].rs2_rob == ex_to_slb.rob) {
						slb_next[i].rs2_rob = 0, slb_next[i].rs2_data = ex_to_slb.reg;
					}
				}
			}
			
			if (!stall_slb_to_slb_prev) {
				for (uint32_t i = slb_prev.head; i != slb_prev.tail; ++i) {
					if (slb_prev[i].rs1_rob == slb_to_slb_prev.rob) {
						slb_next[i].rs1_rob = 0, slb_next[i].rs1_data = slb_to_slb_prev.reg;
					}
					if (slb_prev[i].rs2_rob == slb_to_slb_prev.rob) {
						slb_next[i].rs2_rob = 0, slb_next[i].rs2_data = slb_to_slb_prev.reg;
					}
				}
			}
			
			if (!stall_commit_to_slb) {
				slb_next[commit_to_slb.slb_id].rd_rob = INF32;
				--slb_next[commit_to_slb.slb_id].time;
			}
			
			//execute slb; why not run_ex_slb()?
			stall_slb_to_rob_next = stall_slb_to_rs_next = stall_slb_to_slb_next = true;
			if (!slb_prev.empty()) {
				const auto &front = slb_prev.front();
				if (front.divide.opcode == 0x03) { //load
					if (front.rs1_rob == 0) { //ready
						if (front.time == 1) {
							auto res = execute(front.divide, front.imm, front.rs1_data, front.rs2_data, 0);
							slb_to_rob_next = (ex_to_rob_t) {front.rd_rob, res};
							slb_to_rs_next = (ex_to_rs_t) {front.rd_rob, res.reg};
							slb_to_slb_next = (ex_to_rs_t) {front.rd_rob, res.reg};
							stall_slb_to_rob_next = stall_slb_to_rs_next = stall_slb_to_slb_next = false;
							slb_next.pop();
						}
						else {
							--slb_next.front().time;
						}
					}
				}
				else { //save
					if (stall_commit_to_slb || commit_to_slb.slb_id != slb_prev.head) {
						if (front.rs1_rob == 0 && front.rs2_rob == 0) {
							if (front.rd_rob != INF32) {
								stall_slb_to_rob_next = false;
								slb_to_rob_next = (ex_to_rob_t) {front.rd_rob};
							} else {
								if (front.time == 1) { //==0?
									auto x = front.rs1_data + front.imm, y = front.rs2_data;
									switch (front.divide.funct3) { //sb sh sw
										case 0:
											mem.getPos8(x) = getBitBetween(y, 0, 7);
											break;
										case 1:
											mem.getPos16(x) = getBitBetween(y, 0, 15);
											break;
										case 2:
											mem.getPos32(x) = y;
											break;
									}
									slb_next.pop();
								} else {
									--slb_next.front().time;
								}
							}
						}
					}
				}
			}
			
			if (!stall_clear_slb) {
				if (slb_prev.front().rd_rob == INF32 && slb_prev.front().time > 1) {//time?
					slb_next.tail = slb_next.head + 1;
				}
				else {
					slb_next.clear();
				}
				stall_slb_to_rob_next = stall_slb_to_slb_next = stall_slb_to_rs_next = true;
				return;
			}
		}
		
		void run_rob(){
			/*
			在这一部分你需要完成的工作：
			1. 实现一个先进先出的rob，存储所有指令
			1. 根据issue阶段发射的指令信息分配空间进行存储。
			2. 根据EX阶段和slbUFFER的计算得到的结果，遍历rob，更新rob中的值
			3. 对于队首的指令，如果已经完成计算及更新，进行commit
			*/
			
			if (!stall_clear_rob) { //!!
				rob_next.clear();
				stall_rob_to_commit = true;
				return;
			}
			
			if (!stall_issue_to_rob) { //jump?? check if not chg rd; rd=0
				ReorderBuffer_t next = issue_to_rob;
				next.ready = false;
				rob_next.push(next);
			}
			if (!stall_ex_to_rob) {
				rob_next[ex_to_rob.rob].ready = true;
				rob_next[ex_to_rob.rob].res = ex_to_rob.res;
			}
			if (!stall_slb_to_rob_prev) {
				rob_next[slb_to_rob_prev.rob].ready = true;
				rob_next[slb_to_rob_prev.rob].res = slb_to_rob_prev.res;
			}
			
			stall_rob_to_commit = true;
			if (!rob_prev.empty() && rob_prev.front().ready) {
				const auto &front = rob_prev.front();
				rob_to_commit = (rob_to_commit_t) {front.divide, front.reg_id, rob_prev.head, front.slb_id, front.res};
				stall_rob_to_commit = false;
				rob_next.pop();
			}
			
		}
		
		void run_commit(){
			/*
			在这一部分你需要完成的工作：
			1. 根据rob发出的信息通知regfile修改相应的值，包括对应的rob和是否被占用状态（注意考虑issue和commit同一个寄存器的情况）
			2. 遇到跳转指令更新pc值，并发出信号清空所有部分的信息存储（这条对于很多部分都有影响，需要慎重考虑）
			*/
			//		struct rob_to_commit_t {
			//			DivideType divide;
			//			uint32_t reg_id, rob_id, slb_id;
			//			execute_t res;
			//		} rob_to_commit;
			stall_commit_to_regfile = stall_commit_to_slb = true;
			stall_clear_rob = stall_clear_slb = stall_clear_rs = stall_clear_fq = stall_clear_regfile = true;
			if (!stall_rob_to_commit) {
				auto type = getType(rob_to_commit.divide);
				if (type != OperationType::B && type != OperationType::S) { //write rd
					stall_commit_to_regfile = false;
					commit_to_regfile = (commit_to_regfile_t) {rob_to_commit.rob_id, rob_to_commit.reg_id, rob_to_commit.res.reg};
				}
				if ((type == OperationType::B || rob_to_commit.divide.opcode == 0x6F || rob_to_commit.divide.opcode == 0x67)
					&& rob_to_commit.res.jump) {
					//jump: jal jalr bxx
					stall_clear_rob = stall_clear_slb = stall_clear_rs = stall_clear_fq = stall_clear_regfile = false;
					pc_fetch = rob_to_commit.res.pc;
				}
				if (type == OperationType::S) {
					stall_commit_to_slb = false;
					commit_to_slb = (commit_to_slb_t) {rob_to_commit.slb_id};
				}
//				std::cerr << "code commited " << rob_to_commit.divide.value <<
//				" res = " << rob_to_commit.reg_id << "=" << rob_to_commit.res.reg << " " << rob_to_commit.res.pc << " " << rob_to_commit.res.jump << '\n';
			}
		}

		void update(){
			/*
			在这一部分你需要完成的工作：
			对于模拟中未完成同步的变量（即同时需记下两个周期的新/旧信息的变量）,进行数据更新。
			*/
			fetchQueue_prev = fetchQueue_next;
			reg_prev = reg_next;
			rs_prev = rs_next;
			slb_prev = slb_next;
			rob_prev = rob_next;
			slb_to_rob_prev = slb_to_rob_next;
			slb_to_rs_prev = slb_to_rs_next;
			slb_to_slb_prev = slb_to_slb_next;
			stall_slb_to_rob_prev = stall_slb_to_rob_next;
			stall_slb_to_rs_prev = stall_slb_to_rs_next;
			stall_slb_to_slb_prev = stall_slb_to_slb_next;
		}
		
		
		execute_t execute(DivideType divide, int32_t imm, uint32_t v1, uint32_t v2, uint32_t pc) {
			switch (divide.opcode) {
				case 0x03: { //load: lb lh lw lbu lhu
					switch (divide.funct3) {
						case 0: return (execute_t) {(uint32_t)signedExtend<8>(mem.getPos8(v1 + imm)), 0, false};
						case 1: return (execute_t) {(uint32_t)signedExtend<16>(mem.getPos16(v1 + imm)), 0, false};
						case 2: return (execute_t) {mem.getPos32(v1 + imm), 0, false};
						case 4: return (execute_t) {unsignedExtend<8>(mem.getPos8(v1 + imm)), 0, false};
						case 5: return (execute_t) {unsignedExtend<16>(mem.getPos16(v1 + imm)), 0, false};
					}
				}
				case 0x23: { //sb sh sw
					switch (divide.funct3) {
						case 0: return (execute_t) {0, 0, false};
						case 1: return (execute_t) {0, 0, false};
						case 2: return (execute_t) {0, 0, false};
					}
				}
				case 0x33: {
					switch (divide.funct3) { //add sub sll slt sltu xor srl sra
						case 0: //add sub
							if (divide.funct7 == 0) return (execute_t) {v1 + v2, 0, false};
							else return (execute_t) {v1 - v2, 0, false};
						case 1: //sll
							return (execute_t) {v1 << getBitBetween(v2, 0, 4), 0, false};
						case 2: //slt sltu
							return (execute_t) {static_cast<int32_t>(v1) < static_cast<int32_t>(v2), 0, false};
						case 3: //slt
							return (execute_t) {v1 < v2, 0, false};
						case 4: //xor
							return (execute_t) {v1 ^ v2, 0, false};
						case 5: //srl sra
							if (divide.funct7 == 0) return (execute_t) {v1 >> getBitBetween(v2, 0, 4), 0, false};
							else return (execute_t) {(uint32_t)(static_cast<int32_t>(v1) >> static_cast<int32_t>(getBitBetween(v2, 0, 4))), 0, false};
						case 6: //or and
							return (execute_t) {v1 | v2, 0, false};
						case 7:
							return (execute_t) {v1 & v2, 0, false};
					}
				}
				case 0x13: {
					switch (divide.funct3) { //addi slti sltiu xori ori andi slli srli srai
						case 0: return (execute_t) {v1 + imm, 0, false};
						case 2: return (execute_t) {static_cast<int32_t>(v1) < imm, 0, false};
						case 3: return (execute_t) {v1 < static_cast<uint32_t>(imm), 0, false};
						case 4: return (execute_t) {v1 ^ imm, 0, false};
						case 6: return (execute_t) {v1 | imm, 0, false};
						case 7: return (execute_t) {v1 & imm, 0, false};
						case 1: return (execute_t) {v1 << divide.rs2, 0, false};
						case 5:
							if (divide.funct7 == 0) return (execute_t) {v1 >> divide.rs2, 0, false};
							else return (execute_t) {(uint32_t)(static_cast<int32_t>(v1) >> static_cast<int32_t>(divide.rs2)), 0, false};
					}
				}
				case 0x37: return (execute_t) {(uint32_t)imm, 0, false}; //lui
				case 0x17: return (execute_t) {pc + imm, 0, false}; //auipc
				case 0x6F: return (execute_t) {pc + 4, pc + imm, true}; //jal; assert(rd > 0)
				case 0x67: return (execute_t) {pc + 4, ((v1 + imm) & (~1u)), true}; //jalr; assert rd>0; write after read
				case 0x63: {
					switch (divide.funct3) { //beq bne blt bge bltu bgeu
						case 0: return (execute_t) {0, pc + imm, v1 == v2};
						case 1: return (execute_t) {0, pc + imm, v1 != v2};
						case 4: return (execute_t) {0, pc + imm, static_cast<int32_t>(v1) < static_cast<int32_t>(v2)};
						case 5: return (execute_t) {0, pc + imm, static_cast<int32_t>(v1) >= static_cast<int32_t>(v2)};
						case 6: return (execute_t) {0, pc + imm, v1 < v2};
						case 7: return (execute_t) {0, pc + imm, v1 >= v2};
					}
				}
				case 0: default: { //error
					return (execute_t) {INF32, 0, false};
				}
			}
		}
		
		
		
//		//operations
//		void lb(const DivideType &divide, int32_t imm) {
////			std::cout << "lb" << divide.rs1 << ' ' << divide.rd << ' ' << imm << '\n';
//			reg[divide.rd] = signedExtend<8>(mem.getPos8(reg[divide.rs1] + imm));
//		}
//		void lh(const DivideType &divide, int32_t imm) {
//			reg[divide.rd] = signedExtend<16>(mem.getPos16(reg[divide.rs1] + imm));
//		}
//		void lw(const DivideType &divide, int32_t imm) {
//			reg[divide.rd] = mem.getPos32(reg[divide.rs1] + imm);
//		}
//
//
//		void lbu(const DivideType &divide, int32_t imm) {
////			std::cout << "lb" << divide.rs1 << ' ' << divide.rd << ' ' << imm << '\n';
//			reg[divide.rd] = unsignedExtend<8>(mem.getPos8(reg[divide.rs1] + imm));
//		}
//		void lhu(const DivideType &divide, int32_t imm) {
//			reg[divide.rd] = unsignedExtend<16>(mem.getPos16(reg[divide.rs1] + imm));
//		}
//		void lwu(const DivideType &divide, int32_t imm) {
//			reg[divide.rd] = mem.getPos32(reg[divide.rs1] + imm);
//		}
//
//		void sb(const DivideType &divide, int32_t imm) {
////			std::cout << "sb" << divide.rs1 << ' ' << divide.rs2 << ' ' << imm << '\n';
//			mem.getPos8(reg[divide.rs1] + imm) = getBitBetween(reg[divide.rs2], 0, 7);
//		}
//		void sh(const DivideType &divide, int32_t imm) {
//			mem.getPos16(reg[divide.rs1] + imm) = getBitBetween(reg[divide.rs2], 0, 15);
//		}
//		void sw(const DivideType &divide, int32_t imm) {
//			mem.getPos32(reg[divide.rs1] + imm) = reg[divide.rs2];
//		}
//		void add(const DivideType &divide) {
//			reg[divide.rd] = reg[divide.rs1] + reg[divide.rs2];//ignoring overflow
//		}
//		void sub(const DivideType &divide) {
//			reg[divide.rd] = reg[divide.rs1] - reg[divide.rs2];//ignoring overflow
//		}
//		void getxor(const DivideType &divide) {
//			reg[divide.rd] = reg[divide.rs1] ^ reg[divide.rs2];
//		}
//		void getor(const DivideType &divide) {
//			reg[divide.rd] = reg[divide.rs1] | reg[divide.rs2];
//		}
//		void getand(const DivideType &divide) {
//			reg[divide.rd] = reg[divide.rs1] & reg[divide.rs2];
//		}
//		void sll(const DivideType &divide) {
//			reg[divide.rd] = reg[divide.rs1] << getBitBetween(reg[divide.rs2], 0, 4);
//		}
//		void srl(const DivideType &divide) {
//			reg[divide.rd] = reg[divide.rs1] >> getBitBetween(reg[divide.rs2], 0, 4);
//		}
//		void sra(const DivideType &divide) {
//			reg[divide.rd] = static_cast<int32_t>(reg[divide.rs1]) >> static_cast<int32_t>(getBitBetween(reg[divide.rs2], 0, 4));
//		}
//		void slt(const DivideType &divide) {
//			reg[divide.rd] = static_cast<bool>(static_cast<int32_t>(reg[divide.rs1]) < static_cast<int32_t>(reg[divide.rs2]));
//		}
//		void sltu(const DivideType &divide) {
//			reg[divide.rd] = static_cast<bool>(reg[divide.rs1] < reg[divide.rs2]);
//		}
//		void addi(const DivideType &divide, int32_t imm) {
//			reg[divide.rd] = reg[divide.rs1] + imm;//ignoring overflow
//		}
//		void xori(const DivideType &divide, int32_t imm) {
//			reg[divide.rd] = reg[divide.rs1] ^ imm;
//		}
//		void ori(const DivideType &divide, int32_t imm) {
//			reg[divide.rd] = reg[divide.rs1] | imm;
//		}
//		void andi(const DivideType &divide, int32_t imm) {
//			reg[divide.rd] = reg[divide.rs1] & imm;
//		}
//		void slli(const DivideType &divide) { //assert(shamt[5] == 0)
//			reg[divide.rd] = reg[divide.rs1] << divide.rs2;
//		}
//		void srli(const DivideType &divide) {
//			reg[divide.rd] = reg[divide.rs1] >> divide.rs2;
//		}
//		void srai(const DivideType &divide) {
//			reg[divide.rd] = static_cast<int32_t>(reg[divide.rs1]) >> static_cast<int32_t>(divide.rs2);
//		}
//		void slti(const DivideType &divide, int32_t imm) {
//			reg[divide.rd] = static_cast<bool>(static_cast<int32_t>(reg[divide.rs1]) < imm);
//		}
//		void sltiu(const DivideType &divide, int32_t imm) {
//			reg[divide.rd] = static_cast<bool>(reg[divide.rs1] < static_cast<uint32_t>(imm));
//		}
//		void lui(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
//			reg[divide.rd] = imm;
//		}
//		void auipc(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
//			reg[divide.rd] = pc + imm;
//		}
//		void beq(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
//			if (reg[divide.rs1] == reg[divide.rs2]) {
//				pc += imm - 4;
//			}
//		}
//		void bne(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
//			if (reg[divide.rs1] != reg[divide.rs2]) {
//				pc += imm - 4;
//			}
//		}
//		void blt(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
//			if (static_cast<int32_t>(reg[divide.rs1]) < static_cast<int32_t>(reg[divide.rs2])) {
//				pc += imm - 4;
//			}
//		}
//		void bge(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
//			if (static_cast<int32_t>(reg[divide.rs1]) >= static_cast<int32_t>(reg[divide.rs2])) {
//				pc += imm - 4;
//			}
//		}
//		void bltu(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
//			if (reg[divide.rs1] < reg[divide.rs2]) {
//				pc += imm - 4;
//			}
//		}
//		void bgeu(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
//			if (reg[divide.rs1] >= reg[divide.rs2]) {
//				pc += imm - 4;
//			}
//		}
//		void jal(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
//			if (divide.rd) reg[divide.rd] = pc + 4;
//			pc += imm - 4;
//		}
//		void jalr(const DivideType &divide, int32_t imm) { //assert(imm >= 0)
//			uint32_t t = pc + 4;
//			pc = ((reg[divide.rs1] + imm) & (~1u)) - 4;
//			if (divide.rd) reg[divide.rd] = t;
//		}
	};
	
}

#endif //RISCV_CPU_HPP
