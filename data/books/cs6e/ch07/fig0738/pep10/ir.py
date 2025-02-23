import itertools
from typing import List, Protocol, runtime_checkable

from pep10.arguments import ArgumentType
from pep10.mnemonics import AddressingMode, INSTRUCTION_TYPES, BITS, as_int
from pep10.symbol import SymbolEntry


def source(
    op: str,
    args: List[str],
    symbol: SymbolEntry | None = None,
    comment: str | None = None,
) -> str:
    sym_str = f"{symbol}:" if symbol else ""
    comment_str = f";{comment}" if comment else ""
    return f"{sym_str:7}{op:7}{','.join(args):12}{comment_str}"


class ParserIR(Protocol):
    symbol_decl: SymbolEntry | None
    comment: str | None

    def source(self) -> str: ...


class ErrorNode:
    def __init__(
        self,
    ):
        self.comment: str | None = None
        self.symbol_decl: SymbolEntry | None = None
        self.address = 0

    def source(self) -> str:
        return ";Failed to parse line"


class EmptyNode:
    def __init__(self):
        self.comment: str | None = None
        self.symbol_decl: SymbolEntry | None = None
        self.address: int | None = None

    def source(self) -> str:
        return source("", [], None, None)

    def object_code(self) -> bytearray:
        return bytearray()

    def __len__(self) -> int:
        return 0


class CommentNode:
    def __init__(self, comment: str):
        self.comment: str | None = comment
        self.symbol_decl: SymbolEntry | None = None
        self.address: int | None = None

    def source(self) -> str:
        return source("", [], None, self.comment)

    def object_code(self) -> bytearray:
        return bytearray()

    def __len__(self) -> int:
        return 0


class UnaryNode:
    def __init__(
        self, mn: str, sym: SymbolEntry | None = None, comment: str | None = None
    ):
        self.symbol_decl: SymbolEntry | None = sym
        mn = mn.upper()
        assert mn in INSTRUCTION_TYPES
        self.mnemonic = mn
        self.comment: str | None = comment

    def source(self) -> str:
        return source(str(self.mnemonic.upper()), [], self.symbol_decl, self.comment)


class NonUnaryNode:
    def __init__(
        self,
        mn: str,
        argument: ArgumentType,
        am: AddressingMode,
        sym: SymbolEntry | None = None,
        comment: str | None = None,
    ):
        self.symbol_decl: SymbolEntry | None = sym
        mn = mn.upper()
        assert mn in INSTRUCTION_TYPES
        self.mnemonic = mn
        self.addressing_mode: AddressingMode = am
        self.argument: ArgumentType = argument
        self.comment: str | None = comment

    def source(self) -> str:
        args = [str(self.argument), self.addressing_mode.name.lower()]
        return source(self.mnemonic.upper(), args, self.symbol_decl, self.comment)


@runtime_checkable
class Listable(Protocol):
    address: int | None

    def source(self) -> str: ...
    def object_code(self) -> bytearray: ...
    def __len__(self) -> int: ...


def listing(to_list: Listable) -> List[str]:
    object_code = to_list.object_code()
    if len(object_code) <= 3:
        line_object_code, object_code = object_code, bytearray([])
    else:
        line_object_code, object_code = object_code[0:2], object_code[3:]
    address = f"{to_list.address:04x}" if to_list.address is not None else 4 * " "
    lines = [f"{address} {'':6} {to_list.source()}"]
    for b in itertools.batched(object_code, 3):
        lines.append(f"{'':4} {'':6}")
    return lines


class UnaryIR(UnaryNode):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.address: int | None = None

    def object_code(self) -> bytearray:
        bits = BITS[self.mnemonic]
        return bytearray(bits.to_bytes(1))

    def __len__(self) -> int:
        return 1


class NonUnaryIR(NonUnaryNode):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.address: int | None = None

    def object_code(self) -> bytearray:
        bits = as_int(self.mnemonic, am=self.addressing_mode)
        mn_bytes = bits.to_bytes(1, signed=False)
        arg_bytes = int(self.argument).to_bytes(2)
        return bytearray(mn_bytes + arg_bytes)

    def __len__(self) -> int:
        return 3
