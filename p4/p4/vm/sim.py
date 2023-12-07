from .memory import *
from ..dictionary import defcode as _defcode

class vm (object):
	def __init__(self):
		self.memory = Memory(256)
		self.tcb = TaskControlBlock(8, self.memory)
		self.tcb.here(self.tcb.maxAddress() + 1)
		self.tcb.psp(240)
		self.tcb.s0(240)
		self.tcb.rsp(200)
		self.rStack = Stack(self.memory, self.tcb.rsp_helper, lambda: self.tcb.here())
		self.pStack = Stack(self.memory, self.tcb.psp_helper, lambda: self.tcb.rsp())
		self.words = []
		self.alive = True
		
	def next(self):
		self.tcb.currentWord(self.tcb.nextWord())
		self.tcb.nextWord(self.tcb.nextWord() + 2)	
	
	def herePP(self, incr):
		here = self.tcb.here()
		self.tcb.here(here + incr)
		return here
		
	# Negative token numbers are native, positive token numbers are FORTH
	def nativeWord(self, name, call):
		token = -len(self.words)-1
		self.words.append(call)
		return _defcode(self, name, [token]), token
		
	def intWord(self, name, tokens):
		return _defcode(self, name, tokens)
		
	def step(self):
		cwa_exec = self.memory.read_b16(self.tcb.currentWord(), signed=False)
		token_exec = self.memory.read_b16(cwa_exec, signed = True)
		#print(f"CWA is 0x{as_hex(self.currentWord)}, word to execute is [{as_hex(cwa_exec)}]={token_exec}")
		word = self.words[-token_exec-1]
		word(self)
		
	def run(self):
		while self.alive: self.step()
