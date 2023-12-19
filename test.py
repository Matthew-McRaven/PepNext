from p4.words import extract, core, io, math, stack, var, dict as _dict, interpret
import p4.bootstrap, p4.vm, p4.dictionary

words = [fn for item in [core, stack, io, var, math, _dict, interpret] for fn in extract(item)]
VM = p4.vm.vm()
p4.bootstrap.bootstrap(VM, words)

# Helper to look up a WORD and get its CWA, used to implement interpretted words for now.
e = lambda s: p4.dictionary.addr_from_name(VM, s)
f = lambda s: e(s)["cwa"]

p4.dictionary.defforth(VM, ("doAll", 0, ["'", ".", "CR", "EXIT"]))
p4.dictionary.defforth(VM, ("testReword", 0, "LIT 1 COREINT BRANCH -4".split()))
#p4.dictionary.defforth(VM, ("testReword", 0, " BRANCH -2 HALT".split()))
VM.tcb.nextWord(f("testReword")); VM.next()
p4.dictionary.dump(VM)
VM.run()
