import io
import os
import string
from enum import Enum
from typing import TypeAlias, Union, Tuple, Literal, List


class Tokens(Enum):
    EMPTY = 1
    COMMA = 2
    COMMENT = 3
    IDENTIFIER = 4
    SYMBOL = 5
    DECIMAL = 6
    HEX = 7
    DOT = 8
    CHARACTER = 9
    STRING = 10
    INVALID = -1


class Lexer:
    class States(Enum):
        START = 0
        COMMENT = 3
        IDENTIFIER = 4
        MAYBE_HEX = 6
        HEX_PREFIX = 7
        HEX = 8
        MAYBE_SIGNED = 9
        DECIMAL = 10
        MAYBE_DOT = 11
        DOT = 12
        CHAR_OPEN = 13
        CHAR_AWAITING_CLOSE = 14
        CHAR_EXPECT_ESCAPE = 15
        CHAR_EXPECT_HEX0 = 16
        CHAR_EXPECT_HEX1 = 17
        STRING_AWAITING_CLOSE = 18
        STRING_EXPECT_ESCAPE = 19
        STRING_EXPECT_HEX0 = 20
        STRING_EXPECT_HEX1 = 21
        STOP = -1

    TokenType: TypeAlias = Union[
        Tuple[Literal[Tokens.DECIMAL], int]
        | Tuple[Literal[Tokens.HEX], int]
        | Tuple[Literal[Tokens.IDENTIFIER], str]
        | Tuple[Literal[Tokens.SYMBOL], str]
        | Tuple[Literal[Tokens.COMMENT], str]
        | Tuple[Literal[Tokens.DOT], str]
        | Tuple[Literal[Tokens.CHARACTER], bytes]
        | Tuple[Literal[Tokens.STRING], bytes]
        | Tuple[Literal[Tokens.EMPTY], None]
        | Tuple[Literal[Tokens.COMMA], None]
        | Tuple[Literal[Tokens.INVALID], None]
    ]

    def __init__(self, buffer):
        self.buffer: io.StringIO = buffer

    def __iter__(self) -> "Lexer":
        return self

    def __next__(self) -> TokenType:
        prev_pos = self.buffer.tell()
        next_ch = self.buffer.read(1)
        if len(next_ch) == 0:
            raise StopIteration
        else:
            self.buffer.seek(prev_pos, os.SEEK_SET)

        as_str_list: List[str] = []
        as_int: int = 0
        sign: Literal[-1, 1] = 1
        state: Lexer.States = Lexer.States.START
        token: Lexer.TokenType = (Tokens.EMPTY, None)

        while state != Lexer.States.STOP and token[0] != Tokens.INVALID:
            prev_pos = self.buffer.tell()
            ch: str = self.buffer.read(1)
            match state:
                case _ if len(ch) == 0:
                    state = Lexer.States.STOP

                case Lexer.States.START:
                    if ch == "\n":
                        state = Lexer.States.STOP
                    elif ch == ",":
                        state = Lexer.States.STOP
                        token = (Tokens.COMMA, None)
                    elif ch.isspace():
                        pass
                    elif ch == ";":
                        state = Lexer.States.COMMENT
                    elif ch.isalpha():
                        as_str_list.append(ch)
                        state = Lexer.States.IDENTIFIER
                    elif ch == "0":
                        state = Lexer.States.MAYBE_HEX
                    elif ch.isdecimal():
                        as_int = ord(ch) - ord("0")
                        state = Lexer.States.DECIMAL
                    elif ch == ".":
                        state = Lexer.States.MAYBE_DOT
                    elif ch == "'":
                        state = Lexer.States.CHAR_OPEN
                    elif ch == '"':
                        state = Lexer.States.STRING_AWAITING_CLOSE
                    elif ch == "+" or ch == "-":
                        sign = -1 if ch == "-" else 1
                        state = Lexer.States.MAYBE_SIGNED
                    else:
                        token = (Tokens.INVALID, None)

                case Lexer.States.COMMENT:
                    if ch == "\n":
                        self.buffer.seek(prev_pos, os.SEEK_SET)
                        state = Lexer.States.STOP
                        token = (Tokens.COMMENT, "".join(as_str_list))
                    else:
                        as_str_list.append(ch)

                case Lexer.States.IDENTIFIER:
                    if ch.isalnum() or ch == "_":
                        as_str_list.append(ch)
                    elif ch == ":":
                        state = Lexer.States.STOP
                        token = (Tokens.SYMBOL, "".join(as_str_list))
                    else:
                        self.buffer.seek(prev_pos, os.SEEK_SET)
                        state = Lexer.States.STOP
                        token = (Tokens.IDENTIFIER, "".join(as_str_list))

                case Lexer.States.MAYBE_HEX:
                    if ch.isdigit():
                        as_int = ord(ch) - ord("0")
                        state = Lexer.States.DECIMAL
                    elif ch == "x" or ch == "X":
                        state = Lexer.States.HEX_PREFIX
                    else:
                        self.buffer.seek(prev_pos, os.SEEK_SET)
                        state, token = Lexer.States.STOP, (Tokens.DECIMAL, 0)

                case Lexer.States.HEX_PREFIX:
                    if ch in string.digits:
                        state = Lexer.States.HEX
                        as_int = as_int * 16 + (ord(ch) - ord("0"))
                    elif ch in string.hexdigits:
                        state = Lexer.States.HEX
                        as_int = as_int * 16 + (10 + ord(ch.lower()) - ord("a"))
                    else:
                        token = (Tokens.INVALID, None)

                case Lexer.States.HEX:
                    if ch in string.digits:
                        as_int = as_int * 16 + (ord(ch) - ord("0"))
                    elif ch in string.hexdigits:
                        as_int = as_int * 16 + (10 + ord(ch.lower()) - ord("a"))
                    else:
                        self.buffer.seek(prev_pos, os.SEEK_SET)
                        state, token = Lexer.States.STOP, (Tokens.HEX, as_int)

                case Lexer.States.MAYBE_SIGNED:
                    if ch in string.digits:
                        as_int = as_int * 10 + (ord(ch) - ord("0"))
                        state = Lexer.States.DECIMAL
                    else:
                        token = (Tokens.INVALID, None)

                case Lexer.States.DECIMAL:
                    if ch in string.digits:
                        as_int = as_int * 10 + (ord(ch) - ord("0"))
                    else:
                        self.buffer.seek(prev_pos, os.SEEK_SET)
                        state = Lexer.States.STOP
                        token = (Tokens.DECIMAL, sign * as_int)

                case Lexer.States.MAYBE_DOT:
                    if ch.isalpha():
                        as_str_list.append(ch)
                        state = Lexer.States.DOT
                    else:
                        token = (Tokens.INVALID, None)

                case Lexer.States.DOT:
                    if ch.isalnum() or ch == "_":
                        as_str_list.append(ch)
                    else:
                        self.buffer.seek(prev_pos, os.SEEK_SET)
                        state = Lexer.States.STOP
                        token = (Tokens.DOT, "".join(as_str_list))

                case _:
                    token = (Tokens.INVALID, None)

        return token

    def skip_to_next_line(self):
        while (ch := self.buffer.read(1)) != "\n" and len(ch) > 0:
            pass
