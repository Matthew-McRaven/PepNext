import io
from collections import deque
from typing import Union, cast, List

from pep10.arguments import (
    ArgumentType,
    Hexadecimal,
    Decimal,
    Identifier,
    StringConstant,
)
from pep10.ir import (
    UnaryNode,
    NonUnaryIR,
    UnaryIR,
    ErrorNode,
    NonUnaryNode,
    ParserTreeNode,
    CommentIR,
    EmptyIR,
)
from pep10.lexer import Lexer, Tokens
from pep10.mnemonics import (
    INSTRUCTION_TYPES,
    InstructionType,
    AddressingMode,
    DEFAULT_ADDRESSING_MODES,
)
from pep10.symbol import SymbolTable


class Parser:
    def __init__(self, buffer: io.StringIO, symbol_table: SymbolTable | None = None):
        self.lexer = Lexer(buffer)
        self._buffer: deque[Lexer.TokenType] = deque()
        self.symbol_table = symbol_table if symbol_table else SymbolTable()

    def __iter__(self):
        return self

    def may_match(self, expected: Tokens) -> Union[Lexer.TokenType, None]:
        if (token := self.peek()) and token[0] == expected:
            return self._buffer.popleft()
        return None

    def must_match(self, expected: Tokens) -> Lexer.TokenType:
        if ret := self.may_match(expected):
            return ret
        else:
            raise SyntaxError()

    def peek(self) -> Lexer.TokenType | None:
        if len(self._buffer) > 0:
            return self._buffer[0]
        try:
            self._buffer.append(next(self.lexer))
        except StopIteration:
            return None
        return self._buffer[0]

    def skip_to_next_line(self):
        invalid, empty = (Tokens.INVALID, None), (Tokens.EMPTY, None)
        while self.peek() not in {invalid, empty, None}:
            self._buffer.popleft()
        # Consume trailing newline so we can begin parsing on the next line
        if len(self._buffer) and self._buffer[0] == empty:
            self._buffer.popleft()

    def __next__(self) -> ParserTreeNode:
        if self.peek() is None:
            raise StopIteration()
        try:
            return self.statement()
        except SyntaxError:
            self.skip_to_next_line()
            return ErrorNode()

    def argument(self) -> ArgumentType | None:
        if _hex := self.may_match(Tokens.HEX):
            return Hexadecimal(cast(int, _hex[1]))
        elif dec := self.may_match(Tokens.DECIMAL):
            return Decimal(cast(int, dec[1]))
        elif ident := self.may_match(Tokens.IDENTIFIER):
            sym = self.symbol_table.reference(cast(str, ident[1]))
            return Identifier(sym)
        elif str_const := self.may_match(Tokens.STRING):
            return StringConstant(cast(bytes, str_const[1]))
        return None

    def unary_instruction(self) -> UnaryNode | None:
        if not (mn := self.may_match(Tokens.IDENTIFIER)):
            return None
        mn_str = cast(str, mn[1]).upper()
        if mn_str not in INSTRUCTION_TYPES:
            return None
        match INSTRUCTION_TYPES[mn_str]:
            case InstructionType.R | InstructionType.U:
                return UnaryIR(mn_str)
        return None

    def nonunary_instruction(self) -> NonUnaryNode | None:
        if not (mn := self.may_match(Tokens.IDENTIFIER)):
            return None
        mn_str = cast(str, mn[1]).upper()
        if mn_str not in INSTRUCTION_TYPES:
            return None
        match INSTRUCTION_TYPES[mn_str]:
            case InstructionType.R | InstructionType.U:
                self._buffer.appendleft(mn)
                return None
        if not (argument := self.argument()):
            self._buffer.appendleft(mn)
            return None

        if self.may_match(Tokens.COMMA):
            # Check that addressing mode is a valid string and is allowed for the current mnemonic
            addr_str = cast(str, self.must_match(Tokens.IDENTIFIER)[1]).upper()
            try:
                addr = cast(AddressingMode, AddressingMode[addr_str])
                if not INSTRUCTION_TYPES[mn_str].allows_addressing_mode(addr):
                    raise SyntaxError()
            except KeyError:
                raise SyntaxError()
            return NonUnaryIR(mn_str, argument, addr)
        elif mn_str in DEFAULT_ADDRESSING_MODES:
            return NonUnaryIR(mn_str, argument, DEFAULT_ADDRESSING_MODES[mn_str])
        raise SyntaxError()

    def directive(self):
        raise NotImplementedError()

    def code_line(self) -> UnaryNode | NonUnaryNode | None:
        line: ParserTreeNode | None = None
        if nonunary := self.nonunary_instruction():
            line = nonunary
        elif unary := self.unary_instruction():
            line = unary
        else:
            return None

        if comment := self.may_match(Tokens.COMMENT):
            line.comment = cast(str, comment[1])
        return line

    def statement(self) -> ParserTreeNode:
        line: ParserTreeNode | None = None
        if self.may_match(Tokens.EMPTY):
            return EmptyIR()
        elif comment := self.may_match(Tokens.COMMENT):
            line = CommentIR(cast(str, comment[1]))
        elif (symbol := self.may_match(Tokens.SYMBOL)) and (code := self.code_line()):
            code.symbol_decl = self.symbol_table.define(cast(str, symbol[1]))
            line = code
        elif code := self.code_line():
            line = code
        else:
            raise SyntaxError()

        self.must_match(Tokens.EMPTY)

        return line


def parse(text: str, symbol_table: SymbolTable | None = None) -> List[ParserTreeNode]:
    # Remove trailing whitespace while insuring input is \n terminated.
    parser = Parser(io.StringIO(text.rstrip() + "\n"), symbol_table=symbol_table)
    return [line for line in parser]
