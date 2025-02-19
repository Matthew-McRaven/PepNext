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
