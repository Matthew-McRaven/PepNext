from enum import Enum


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


class Mnemonic:
    def __init__(self, name: str, type: InstructionType, bit_pattern: int):
        self.name = name
        self.type = type
        self.bit_pattern = bit_pattern

    def to_byte(self, am: AddressingMode | None = None) -> int:
        if self.type == InstructionType.U or self.type == InstructionType.R:
            return self.bit_pattern
        elif self.type == InstructionType.A_ix:
            return self.bit_pattern | (0 if am is None else am.as_A())
        else:
            return self.bit_pattern | (0 if am is None else am.as_AAA())


class Mnemonics(Enum):
    RET = Mnemonic("RET", InstructionType.U, 0x01)
    # SRET=2, MOVFLGA=3, MOVAFLG=4, MOVSPA=5, MOVASP=5
    NOP = Mnemonic("NOP", InstructionType.U, 0x07)
    NOTA = Mnemonic("NOTA", InstructionType.R, 0x18)
    NOTX = Mnemonic("NOTX", InstructionType.R, 0x19)
    # NEGr, ASLr, ASRr, ROLr, RORr

    BR = Mnemonic("BR", InstructionType.A_ix, 0x24)
    # BR(LE|LT|EQ|NE|GE|GT|V|C)
    CALL = Mnemonic("CALL", InstructionType.A_ix, 0x36)
    SCALL = Mnemonic("SCALL", InstructionType.A_ix, 0x38)
    # (ADD|SUB)SP
    ADDA = Mnemonic("ADDA", InstructionType.RAAA_all, 0x50)
    ADDX = Mnemonic("ADDX", InstructionType.RAAA_all, 0x58)
    # (SUB|AND|OR|XOR)r
    CPBA = Mnemonic("CPBA", InstructionType.RAAA_all, 0xB0)
    CPBX = Mnemonic("CPBX", InstructionType.RAAA_all, 0xB8)
    # (CPW|LDW)r
    STWA = Mnemonic("ADDA", InstructionType.RAAA_noi, 0xE0)
    STWX = Mnemonic("ADDX", InstructionType.RAAA_noi, 0xE8)
    # STBr
