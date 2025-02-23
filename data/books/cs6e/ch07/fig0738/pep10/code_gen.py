import itertools
from typing import List, Tuple

from pep10.arguments import ArgumentType, Identifier
from pep10.ir import Listable, listing, ParserIR
from pep10.symbol import SymbolEntry


def generate_code(parse_tree: List[ParserIR]) -> Tuple[List[Listable], List[str]]:
    errors: List[str] = []
    ir: List[Listable] = []
    address = 0
    for node in parse_tree:
        # TODO: recursively generate code for macros, extending our output with theirs.
        if isinstance(node, Listable):
            ir.append(node)
        else:
            continue

        line: Listable = node
        # The size of the IR line may depend on the address, e.g., .ALIGN
        line.address = address
        # Check for multiply defined symbols, and assign addresses to symbol declarations
        if maybe_symbol := getattr(node, "symbol_decl", None):
            symbol: SymbolEntry = maybe_symbol
            if symbol.is_multiply_defined():
                errors.append(f"Multiply defined symbol: {symbol}")
            elif len(line) > 0:  # Avoid re-assigning symbol values for .EQUATEs
                symbol.value = address
        # Check that symbols used as arguments are not undefined.
        if maybe_argument := getattr(line, "argument", None):
            argument: ArgumentType = maybe_argument
            if type(argument) == Identifier and argument.symbol.is_undefined():
                errors.append(f"Undefined symbol: {argument.symbol}")
        address += len(line)

    return ir, errors


def program_object_code(program: List[Listable]) -> bytes:
    return b"".join(line.object_code() for line in program)


def program_source(program: List[Listable]) -> List[str]:
    return [line.source() for line in program]


def program_listing(program: List[Listable]) -> List[str]:
    return list(itertools.chain.from_iterable(listing(line) for line in program))
