// Copyright (c) 2012 Zeex
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef AMXJIT_H
#define AMXJIT_H

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <utility>

#include <amx/amx.h>

#include <asmjit/core.h>
#include <asmjit/x86.h>

#if !defined _M_IX86 && !defined __i386__
	#error Unsupported architecture
#endif

#if defined _MSC_VER
	#define AMXJIT_CDECL __cdecl
	#define AMXJIT_STDCALL __stdcall
#elif defined __GNUC__
	#define AMXJIT_CDECL __attribute__((cdecl))
	#define AMXJIT_STDCALL __attribute__((stdcall))
#else
	#error Unsupported compiler
#endif

namespace amxjit {

// REG_NONE is defined in WinNT.h.
#ifdef REG_NONE
	#undef REG_NONE
#endif

enum AMXRegister {
	REG_NONE = 0,
	REG_PRI = (2 << 0),
	REG_ALT = (2 << 1),
	REG_COD = (2 << 2),
	REG_DAT = (2 << 3),
	REG_HEA = (2 << 4),
	REG_STP = (2 << 5),
	REG_STK = (2 << 6),
	REG_FRM = (2 << 7),
	REG_CIP = (2 << 8)
};

enum AMXOpcode {
	OP_NONE,         OP_LOAD_PRI,     OP_LOAD_ALT,     OP_LOAD_S_PRI,
	OP_LOAD_S_ALT,   OP_LREF_PRI,     OP_LREF_ALT,     OP_LREF_S_PRI,
	OP_LREF_S_ALT,   OP_LOAD_I,       OP_LODB_I,       OP_CONST_PRI,
	OP_CONST_ALT,    OP_ADDR_PRI,     OP_ADDR_ALT,     OP_STOR_PRI,
	OP_STOR_ALT,     OP_STOR_S_PRI,   OP_STOR_S_ALT,   OP_SREF_PRI,
	OP_SREF_ALT,     OP_SREF_S_PRI,   OP_SREF_S_ALT,   OP_STOR_I,
	OP_STRB_I,       OP_LIDX,         OP_LIDX_B,       OP_IDXADDR,
	OP_IDXADDR_B,    OP_ALIGN_PRI,    OP_ALIGN_ALT,    OP_LCTRL,
	OP_SCTRL,        OP_MOVE_PRI,     OP_MOVE_ALT,     OP_XCHG,
	OP_PUSH_PRI,     OP_PUSH_ALT,     OP_PUSH_R,       OP_PUSH_C,
	OP_PUSH,         OP_PUSH_S,       OP_POP_PRI,      OP_POP_ALT,
	OP_STACK,        OP_HEAP,         OP_PROC,         OP_RET,
	OP_RETN,         OP_CALL,         OP_CALL_PRI,     OP_JUMP,
	OP_JREL,         OP_JZER,         OP_JNZ,          OP_JEQ,
	OP_JNEQ,         OP_JLESS,        OP_JLEQ,         OP_JGRTR,
	OP_JGEQ,         OP_JSLESS,       OP_JSLEQ,        OP_JSGRTR,
	OP_JSGEQ,        OP_SHL,          OP_SHR,          OP_SSHR,
	OP_SHL_C_PRI,    OP_SHL_C_ALT,    OP_SHR_C_PRI,    OP_SHR_C_ALT,
	OP_SMUL,         OP_SDIV,         OP_SDIV_ALT,     OP_UMUL,
	OP_UDIV,         OP_UDIV_ALT,     OP_ADD,          OP_SUB,
	OP_SUB_ALT,      OP_AND,          OP_OR,           OP_XOR,
	OP_NOT,          OP_NEG,          OP_INVERT,       OP_ADD_C,
	OP_SMUL_C,       OP_ZERO_PRI,     OP_ZERO_ALT,     OP_ZERO,
	OP_ZERO_S,       OP_SIGN_PRI,     OP_SIGN_ALT,     OP_EQ,
	OP_NEQ,          OP_LESS,         OP_LEQ,          OP_GRTR,
	OP_GEQ,          OP_SLESS,        OP_SLEQ,         OP_SGRTR,
	OP_SGEQ,         OP_EQ_C_PRI,     OP_EQ_C_ALT,     OP_INC_PRI,
	OP_INC_ALT,      OP_INC,          OP_INC_S,        OP_INC_I,
	OP_DEC_PRI,      OP_DEC_ALT,      OP_DEC,          OP_DEC_S,
	OP_DEC_I,        OP_MOVS,         OP_CMPS,         OP_FILL,
	OP_HALT,         OP_BOUNDS,       OP_SYSREQ_PRI,   OP_SYSREQ_C,
	OP_FILE,         OP_LINE,         OP_SYMBOL,       OP_SRANGE,
	OP_JUMP_PRI,     OP_SWITCH,       OP_CASETBL,      OP_SWAP_PRI,
	OP_SWAP_ALT,     OP_PUSH_ADR,     OP_NOP,          OP_SYSREQ_D,
	OP_SYMTAG,       OP_BREAK
};

// The total number of opcodes.
const int NUM_AMX_OPCODES = OP_BREAK + 1;

class AMXInstruction {
public:
	AMXInstruction();

public:
	int size() const {
		return sizeof(cell) * (1 + operands_.size());
	}

	const cell address() const {
		return address_;
	}
	void setAddress(cell address) {
		address_ = address;
	}

	AMXOpcode opcode() const {
		return opcode_;
	}
	void setOpcode(AMXOpcode opcode) {
		opcode_ = opcode;
	}

	cell operand(unsigned int index = 0u) {
		assert(index < operands_.size());
		return operands_[index];
	}
	std::vector<cell> &operands() {
		return operands_;
	}
	const std::vector<cell> &operands() const {
		return operands_;
	}
	void setOperands(std::vector<cell> operands) {
		operands_ = operands;
	}
	void addOperand(cell value) {
		operands_.push_back(value);
	}
	int numOperands() const {
		return operands_.size();
	}

public:
	const char *name() const;

	int getSrcRegs() const;
	int getDestRegs() const;

	std::string string() const;

private:
	cell address_;
	AMXOpcode opcode_;
	std::vector<cell> operands_;

private:
	struct StaticInfoTableEntry {
		const char *name;
		int srcRegs;
		int destRegs;
	};
	static const StaticInfoTableEntry info[NUM_AMX_OPCODES];
};

class AMXScript {
public:
	AMXScript(AMX *amx);

public:
	operator AMX*() { return amx(); }
	AMX *operator->() { return amx(); }

public:
	AMX *amx() const {
		return amx_;
	}
	AMX_HEADER *hdr() const {
		return reinterpret_cast<AMX_HEADER*>(amx()->base);
	}

	unsigned char *code() const {
		return amx()->base + hdr()->cod;
	}
	std::size_t codeSize() const {
		return hdr()->dat - hdr()->cod;
	}

	unsigned char *data() const {
		return amx()->data != 0 ? amx()->data : amx()->base + hdr()->dat;
	}
	std::size_t dataSize() const {
		return hdr()->hea - hdr()->dat;
	}

	int numPublics() const {
		return (hdr()->natives - hdr()->publics) / hdr()->defsize;
	}
	int numNatives() const {
		return (hdr()->libraries - hdr()->natives) / hdr()->defsize;
	}

	AMX_FUNCSTUBNT *publics() const {
		return reinterpret_cast<AMX_FUNCSTUBNT*>(hdr()->publics + amx()->base);
	}
	AMX_FUNCSTUBNT *natives() const {
		return reinterpret_cast<AMX_FUNCSTUBNT*>(hdr()->natives + amx()->base);
	}

	cell getPublicAddr(int index) const;
	cell getNativeAddr(int index) const;

	const char *getPublicName(int index) const;
	const char *getNativeName(int index) const;

	int findPublic(cell address) const;
	int findNative(cell address) const;

public:
	cell *stack() const {
		return reinterpret_cast<cell*>(data() + amx()->stk);
	}
	cell stackSize() const {
		return hdr()->stp - hdr()->hea;
	}

	cell *push(cell value);
	cell pop();
	void pop(int ncells);

private:
	AMX *amx_;
};

class AMXDisassembler {
public:
	AMXDisassembler(const AMXScript &amx);

public:
	// See JIT::setOpcodeMap() for details.
	void setOpcodeMap(cell *opcodeMap) { opcodeMap_ = opcodeMap; }

	// Returns address of currently executing instruction (instruction pointer).
	cell ip() const { return ip_; }

	// Sets instruction pointer.
	void setIp(cell ip) { ip_ = ip; }

public:
	// Decodes current instruction and returns true until the end of code gets
	// reached or an error occurs. The optional error argument is set to true
	// on error.
	bool decode(AMXInstruction &instr, bool *error = 0);

private:
	AMXScript amx_;
	cell *opcodeMap_;
	cell ip_;
};

class JITCompileErrorHandler;

class JIT {
public:
	JIT(AMXScript amx);
	~JIT();

private:
	// Disallow copying and assignment.
	JIT(const JIT &);
	JIT &operator=(const JIT &);

public:
	AMXScript &amx() { return amx_; }
	const AMXScript &amx() const { return amx_; }

	// These return pointer to JIT code buffer and its size.
	void *code() const { return code_; }
	std::size_t codeSize() const { return codeSize_; }

	// Returns a pointer to a machine instruction mapped to a given AMX
	// instruction. The address is realtive to start of the AMX code.
	void *getInstrPtr(cell address) const;

	// Same as above but returns an offset to code() instead of a pointer.
	int getInstrOffset(cell address) const;

	// Sets an opcode relocation table to be used. This table only exists in the
	// GCC version of AMX where they use a GCC extension to C's goto statement
	// instead of a simple switch to minimize opcode dispatching overhead.
	void setOpcodeMap(cell *opcodeMap) { opcodeMap_ = opcodeMap; }

	// Returns a pointer to the opcode relocation table if it was previously set
	// with setOpcodeMap() or NULL if not.
	cell *opcodeMap() const { return opcodeMap_; }

	// Sets a custom assembler to be used to compile() the code. If no assembler
	// is set or setAssembler() is called with NULL argument the default
	// assembler is used.
	void setAssembler(AsmJit::X86Assembler *as) { as_ = as; }

	// Returns currently set assembler or NULL if no assembler set.
	AsmJit::X86Assembler *assembler() const { return as_; }

public:
	// compile() is used to JIT-compile the AMX script to be able to calls its
	// function with call() and exec(). If an error occurs during compilation,
	// such as invalid instruction, an optional error handler is executed.
	bool compile(JITCompileErrorHandler *errorHandler = 0);

public:
	// Calls a function at the specified address and returns one of the
	// AMX_ERROR_* codes. Function arguments should be pushed onto the AMX stack
	// prior invoking this method, for example using amx_Push*() routines.
	int call(cell address, cell *retval);

	// This is basically the same as call() but for public functions. Can be
	// used as a drop-in replacemenet for amx_Exec().
	int exec(cell index, cell *retval);

private:
	// Get addresses of all functions and jump destinations.
	bool getJumpRefs(std::set<cell> &refs) const;

	// Sets a label at the specified AMX address. The address should be relative
	// to the start of the code section.
	AsmJit::Label &L(AsmJit::X86Assembler *as, cell address);
	AsmJit::Label &L(AsmJit::X86Assembler *as, cell address, const std::string &name);

	// Sets the EBP and ESP registers to stackBase and stackPtr repectively and
	// jumps to JIT code mapped to the specified AMX address. The address must
	// be relative to the start of the of code section.
	void jump(cell address, void *stackBase, void *stackPtr);

	// A static wrapper around jump() called from within JIT code.
	static void AMXJIT_CDECL doJump(JIT *amxjit, cell address, void *stackBase, void *stackPtr);

	// Resets EBP and ESP and jumps to the place of the previous call()
	// invocation setting amx->error to error.
	void halt(int error);

	// A wrapper around halt() that is called from JIT code.
	static void AMXJIT_CDECL doHalt(JIT *amxjit, int error);

	// Sets the EBP and ESP registers to stackBase and stackPtr repectively and
	// executes a native functions at the specified index. If there is no
	// function at that index or the index is out of native table bounds the
	// halt() method is called.
	void sysreqC(cell index, void *stackBase, void *stackPtr);

	// A wrapper around sysreqC() that is called from JIT code.
	static void AMXJIT_CDECL doSysreqC(JIT *amxjit, cell index, void *stackBase, void *stackPtr);

	// Same as sysreqC() but takes an address instead of an index.
	void sysreqD(cell address, void *stackBase, void *stackPtr);

	// A wrapper around sysreqD() that is called from JIT code.
	static void AMXJIT_CDECL doSysreqD(JIT *amxjit, cell address, void *stackBase, void *stackPtr);

private:
	// An "intrinsic" (couldn't find a better term for this) is a portion of
	// machine code that substitutes calls to certain native functions and is
	// generally is a highly optimized version of such.
	typedef void (JIT::*IntrinsicImpl)(AsmJit::X86Assembler *as);

	struct Intrinsic {
		std::string   name;
		IntrinsicImpl impl;
	};

	static Intrinsic intrinsics_[];

	// AMX floating-point natives.
	void native_float(AsmJit::X86Assembler *as);
	void native_floatabs(AsmJit::X86Assembler *as);
	void native_floatadd(AsmJit::X86Assembler *as);
	void native_floatsub(AsmJit::X86Assembler *as);
	void native_floatmul(AsmJit::X86Assembler *as);
	void native_floatdiv(AsmJit::X86Assembler *as);
	void native_floatsqroot(AsmJit::X86Assembler *as);
	void native_floatlog(AsmJit::X86Assembler *as);

private:
	AMXScript amx_;

	cell *opcodeMap_;
	AsmJit::X86Assembler *as_;

	void *code_;
	std::size_t codeSize_;

	void *ebp_;
	void *esp_;
	void *resetEbp_;
	void *resetEsp_;

private:
	typedef void (AMXJIT_CDECL *HaltHelper)(int error);
	HaltHelper haltHelper_;

	typedef void (AMXJIT_CDECL *JumpHelper)(void *dest, void *stackBase, void *stackPtr);
	JumpHelper jumpHelper_;

	typedef cell (AMXJIT_CDECL *CallHelper)(void *start);
	CallHelper callHelper_;

	typedef cell (AMXJIT_CDECL *SysreqHelper)(cell address, void *stackBase, void *stackPtr);
	SysreqHelper sysreqHelper_;

private:
	typedef std::map<cell, int> PcodeToNativeMap;
	PcodeToNativeMap pcodeToNative_;

	typedef std::map<std::pair<cell, std::string>, AsmJit::Label> AddressNameToLabel;
	AddressNameToLabel addressNameToLabel_;
};

// Inherit from this class to pass your custom error handler to JIT::compile().
class JITCompileErrorHandler {
public:
	virtual ~JITCompileErrorHandler() {}
	virtual void execute(const AMXInstruction &instr) = 0;
};

} // namespace amxjit

#endif // !AMXJIT_H
