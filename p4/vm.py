import enum

from .memory import *
from ..dictionary import defcode as _defcode, nearest_header, name
from ..utils import as_hex as _as_hex
import binascii
import enum

class Memory:
	def __init__(self, count):
		self.memory = bytearray(count)

	# External interface
	def read_b8(self, addr, signed=False): return self.read_n(addr, 1, signed=signed)

	def read_b16(self, addr, signed=False): return self.read_n(addr, 2, signed=signed)

	def write_b8(self, addr, number, signed=False): return self.write_n(addr, 1, number, signed=signed)

	def write_b16(self, addr, number, signed=False): return self.write_n(addr, 2, number, signed=signed)

	def write_bytes(self, addr, bytes): raise NotImplementedError()

	# Meant to be used internally
	def read_n(self, addr, length, signed=False): return int.from_bytes(self[addr:addr + length], "big", signed=signed)

	def write_n(self, addr, length, number, signed=False): self[addr:addr + length] = number.to_bytes(length, "big",
																									  signed=signed)

	def dump(self, lo, hi): print(str(binascii.hexlify(self.memory[lo:hi])))

	# Used to make memory subscriptable
	def __getitem__(self, index): return self.memory[index]

	def __setitem__(self, index, value): self.memory[index] = value


class Stack:
	# SP is a function that does one of two things.
	# If called with no args, returns the current stack pointer.
	# Otherwise, the value will be added to the SP, and the new SP will be returned.
	def __init__(self, memory, sp, limit=lambda: 0):
		self.memory = memory
		self.sp = sp
		self.bsp = sp()
		self.limit = limit

	def push_b8(self, number, signed=False):
		self.push_int(number, 1, signed=signed)

	def push_b16(self, number, signed=False):
		self.push_int(number, 2, signed=signed)

	def push_int(self, number, length, signed=False):
		if self.limit() > self.sp() + length: raise Exception("Stack Overflow")
		# Swap byte order since stack is backwards
		self.sp(-length)
		self.memory[self.sp():self.sp() + length] = number.to_bytes(length, "big", signed=signed)

	def pop_b8(self, signed=False):
		return self.pop_int(1, signed=signed)

	def pop_b16(self, signed=False):
		return self.pop_int(2, signed=signed)

	def pop_int(self, length, signed=False):
		ret = int.from_bytes(self.memory[self.sp():self.sp() + length], "big", signed=signed)
		self.sp(length)
		if self.bsp < self.sp(): raise Exception("Stack Underflow")
		return ret

	def push_bytes(self, bytes):
		# print(self.limit(), self.sp(), len(bytes))
		if self.limit() > self.sp() - len(bytes): raise Exception("Stack Overflow")
		for byte in bytes[::-1]:
			self.sp(-1)
			self.memory[self.sp()] = byte

	def dump(self):
		self.memory.dump(self.sp(), self.bsp)


class TaskControlBlock:
	class Offsets(enum.IntEnum):
		HERE = 0
		LATEST = 2
		CURRENT_WORD = 4
		NEXT_WORD = 6
		PSP = 8
		RSP = 10
		P0 = 12
		R0 = 14
		STATE = 16

	def maxAddress(self):
		return self.baseAddress + max(TaskControlBlock.Offsets) + 1

	def __init__(self, baseAddress, memory):
		self.baseAddress = baseAddress
		self.memory = memory

		# Dictionary entries
		# Start entries at non=0, so that a dereferenced nullpointer can't clober a fundamental data structure
		self.here(2)
		# But we want that null to be the link pointer in our first entry
		# All we have to do is an 0= to check if we've arrived at dict head.
		self.latest(0)
		# Instruction pointers
		self.currentWord(0)
		self.nextWord(0)
		# Parameter stack pointers
		self.psp(0)
		self.p0(0)
		# Return stack pointers
		self.rsp(0)
		self.r0(0)
		# 0 if executing, >0 if compiling
		self.state(0)

	def here(self, value=None):
		return self.access(TaskControlBlock.Offsets.HERE, value)

	def latest(self, value=None):
		return self.access(TaskControlBlock.Offsets.LATEST, value)

	def currentWord(self, value=None):
		return self.access(TaskControlBlock.Offsets.CURRENT_WORD, value)

	def nextWord(self, value=None):
		return self.access(TaskControlBlock.Offsets.NEXT_WORD, value)

	def psp(self, value=None):
		return self.access(TaskControlBlock.Offsets.PSP, value)

	def rsp(self, value=None):
		return self.access(TaskControlBlock.Offsets.RSP, value)

	def p0(self, value=None):
		return self.access(TaskControlBlock.Offsets.P0, value)

	def r0(self, value=None):
		return self.access(TaskControlBlock.Offsets.R0, value)

	def state(self, value=None):
		return self.access(TaskControlBlock.Offsets.STATE, value)

	def access(self, offset, value=None):
		if value is None:
			return self.memory.read_b16(self.baseAddress + offset, signed=False)
		else:
			return self.memory.write_b16(self.baseAddress + offset, value, signed=False if value > 0 else True)

	def psp_helper(self, arg=None):
		if arg is not None: self.psp(self.psp() + arg)
		return self.psp()

	def rsp_helper(self, arg=None):
		if arg is not None: self.rsp(self.rsp() + arg)
		return self.rsp()


class State(enum.IntEnum):
	COMPILING = 1
	IMMEDIATE = 0

class vm (object):
	def __init__(self):
		self.memory = Memory(1024)
		self.tcb = TaskControlBlock(8, self.memory)
		self.tcb.here(self.tcb.maxAddress() + 1)
		self.tcb.p0(950)
		self.tcb.psp(self.tcb.p0())
		self.tcb.r0(990)
		self.tcb.rsp(self.tcb.r0())
		self.rStack = Stack(self.memory, self.tcb.rsp_helper, lambda: self.tcb.p0())
		self.pStack = Stack(self.memory, self.tcb.psp_helper, lambda: self.tcb.here())
		self.words = []
		# The actual instruction pointer of the VM
		# Usually, this value will be updated by next() from currentWord and nextWord
		# However, when you need to (temporarily) transfer control without changing control flow (i.e., DOCOL),
		# you can update this value directly.
		self.ip = 0
		self.alive = True
		
	def next(self):
		self.tcb.currentWord(self.tcb.nextWord())
		self.tcb.nextWord(self.tcb.nextWord() + 2)
		self.ip = self.tcb.currentWord()
	
	def herePP(self, incr):
		here = self.tcb.here()
		self.tcb.here(here + incr)
		return here
		
	# Negative token numbers are native, positive token numbers are FORTH
	def nativeWord(self, name, call, immediate=False):
		token = -len(self.words)-1
		self.words.append(call)
		return _defcode(self, name, [token], immediate=immediate), token
		
	def step(self):
		opcode = self.chase_opcode(self.ip)
		self.decode_opcode(opcode)(self)


	def decode_opcode(self, opcode):
		token = -opcode - 1
		return self.words[token]


	def chase_opcode(self, addr, debug=False):
		addr_chain = [addr] if debug else None
		opcode = self.memory.read_b16(addr, signed=True)
		while opcode > 0:
			# Push current address onto stack to imitate DOCOL
			addr, opcode = opcode, self.memory.read_b16(opcode, signed=True)
			if debug: addr_chain.append(addr)
		self.ip = addr
		if debug:
			addr_chain_str = " = ".join([f"(*[{_as_hex(it)}])" for it in addr_chain[::-1]])
			name_chain = " ".join(f"{name(self, nearest_header(self, it))}" for it in addr_chain)
			print(f"Executing CWA {_as_hex(self.ip):4} opcode {self.decode_opcode(opcode).FORTH['name']:10} = {addr_chain_str}")
			#self.rStack.dump()
			# print(f"The return stack is {name_chain} {{{self.decode_opcode(opcode).FORTH['name']}}}")
		return opcode

	def run(self):
		while self.alive: self.step()
