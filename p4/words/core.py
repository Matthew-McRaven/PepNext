from ..utils import IMMEDIATE, NAMED, NEXT
from ..vm.sim import State as _State

@NAMED("DOCOL")
@NEXT
def docol(VM):
	VM.rStack.push_b16(VM.tcb.currentWord()+2, signed=False)
	VM.tcb.nextWord(VM.memory.read_b16(VM.tcb.currentWord(), signed=False) + 2)

# ( addr -- value) # Dereference a pointer
@NAMED("?")
@NEXT
def fetch(VM):
	addr = VM.pStack.pop_b16(signed=False)
	VM.pStack.push_b16(VM.memory.read_b16(addr, False), signed=False)

# ( addr value --) # Write to a pointer
@NAMED("@")
@NEXT
def question(VM):
	addr = VM.pStack.pop_b16(signed=False)
	value = VM.pStack.pop_b16(signed=False)
	VM.memory.write_b16(addr, value, False)

# Stops further instructions from executing in VM
@NAMED("HALT")
@NEXT
def halt(VM):
	VM.alive = False
	print("\nHALTING")
	
# Pop top entry of return stack and jump to it
@NAMED("EXIT")
@NEXT
def exit(VM):
	VM.tcb.nextWord(VM.rStack.pop_b16(signed=False))
	
# Enter compilation mode
@NAMED("[")
@NEXT
def lbrac(VM):
	VM.tcb.state(_State.COMPILING)
	
# Exit compilation mode
@NAMED("]")
@IMMEDIATE
@NEXT
def rbrac(VM):
	VM.tcb.state(_State.IMMEDIATE)
	
# ( -- ) Relative unconditional branch, consumes following cell for jump address
@NAMED("BRANCH")
@IMMEDIATE
@NEXT
def branch(VM):
	offset = VM.memory.read_b16(VM.tcb.nextWord(), False)
	VM.tcb.nextWord(VM.tcb.nextWord() + offset)
	
# ( n -- ) Relative conditional branch, consumes following cell for jump address
@NAMED("0BRANCH")
@IMMEDIATE
@NEXT
def branch0(VM):
	if VM.rStack.pop_b16(signed=False) == 0: branch(VM)
	else: VM.tcb.nextWord(VM.tcb.nextWord() + 2)

# ( -- n ) Push the current state of the VM onto the stack
@NAMED("STATE")
@IMMEDIATE
@NEXT
def state(VM):
	VM.pStack.push_b16(VM.tcb.state())

# ( -- n ) Push value for state "IMMEDIATE" onto the Stack
@NAMED("STATE_IMM")
@IMMEDIATE
@NEXT
def state(VM):
	VM.pStack.push_b16(_State.IMMEDIATE)

