from enum import Enum
from typing import Dict


class AddressingMode(Enum):
    I = 1 << 0
    D = 1 << 1
    N = 1 << 2
    S = 1 << 3
    SF = 1 << 4
    X = 1 << 5
    SX = 1 << 6
    SFX = 1 << 7

    def as_AAA(self) -> int:
        return self.value.bit_length() - 1

    def as_A(self) -> int:
        match self.value:
            case AddressingMode.I.value:
                return 0
            case AddressingMode.X.value:
                return 1
            case _:
                raise TypeError(f"Invalid addressing mode for A type: {self.name}")

    def __or__(self, other) -> int:
        return self.value | other.value


class InstructionType(Enum):
    U = "U"
    R = "R"
    A_ix = "A_ix"
    AAA_all = "AAA_all"
    AAA_i = "AAA_i"
    RAAA_all = "RAAA_all"
    RAAA_noi = "RAAA_noi"

    def address_mask(self) -> int:
        masks = {
            "U": 0,
            "R": 0,
            "A_ix": AddressingMode.I | AddressingMode.X,
            "AAA_all": 255,
            "AAA_i": AddressingMode.I.value,
            "RAAA_all": 255,
            "RAAA_noi": 254,
        }
        return masks[self.name]

    def allows_addressing_mode(self, am: AddressingMode):
        mask = self.address_mask()
        return bool(mask & am.value)


INSTRUCTION_TYPES: Dict[str, InstructionType] = {
    "RET": InstructionType.U,
    # SRET, MOVFLGA, MOVAFLG, MOVSPA, MOVASP
    "NOP": InstructionType.U,
    "NOTA": InstructionType.R,
    "NOTX": InstructionType.R,
    # NEGr, ASLr, ASRr, ROLr, RORr
    "BR": InstructionType.A_ix,
    # BR(LE|LT|EQ|NE|GE|GT|V|C)
    "CALL": InstructionType.A_ix,
    "SCALL": InstructionType.AAA_all,
    # (ADD|SUB)SP
    "ADDA": InstructionType.RAAA_all,
    "ADDX": InstructionType.RAAA_all,
    # (SUB|AND|OR|XOR)r
    "CPWA": InstructionType.RAAA_all,
    "CPWX": InstructionType.RAAA_all,
    # (CPW|LDW)r
    "STWA": InstructionType.RAAA_noi,
    "STWX": InstructionType.RAAA_noi,
    # STBr
}

DEFAULT_ADDRESSING_MODES: Dict[str, AddressingMode] = {
    "BR": AddressingMode.I,
    # BR(LE|LT|EQ|NE|GE|GT|V|C)
    "CALL": AddressingMode.I,
}


BITS: Dict[str, int] = {
    "RET": 0x01,
    # SRET, MOVFLGA, MOVAFLG, MOVSPA, MOVASP
    "NOP": 0x07,
    "NOTA": 0x18,
    "NOTX": 0x19,
    # NEGr, ASLr, ASRr, ROLr, RORr
    "BR": 0x24,
    # BR(LE|LT|EQ|NE|GE|GT|V|C)
    "CALL": 0x36,
    "SCALL": 0x38,
    # (ADD|SUB)SP
    "ADDA": 0x50,
    "ADDX": 0x58,
    # (SUB|AND|OR|XOR)r
    "CPWA": 0xB0,
    "CPWX": 0xB8,
    # (CPW|LDW)r
    "STWA": 0xE0,
    "STWX": 0xE8,
    # STBr
}


def as_int(mnemonic: str, am: AddressingMode | None = None) -> int:
    mnemonic = mnemonic.upper()
    bit_pattern, mn_type = BITS[mnemonic], INSTRUCTION_TYPES[mnemonic]

    if mn_type == InstructionType.U or mn_type == InstructionType.R:
        return bit_pattern
    elif mn_type == InstructionType.A_ix:
        return bit_pattern | (0 if am is None else am.as_A())
    else:
        return bit_pattern | (0 if am is None else am.as_AAA())
