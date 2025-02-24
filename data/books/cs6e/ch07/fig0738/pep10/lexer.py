import io
import os
import string
from enum import Enum
from typing import Literal, List

from pep10.tokens import Tokens, TokenType


class Lexer:
    class States(Enum):
        START, COMMENT, IDENTIFIER, MAYBE_HEX, HEX_PREFIX, HEX = range(0, 6)
        MAYBE_SIGNED, DECIMAL, MAYBE_DOT, DOT, CHAR_OPEN = range(6, 11)
        CHAR_AWAITING_CLOSE, CHAR_EXPECT_ESCAPE, CHAR_EXPECT_HEX0 = range(11, 14)
        CHAR_EXPECT_HEX1, STRING_AWAITING_CLOSE, STRING_EXPECT_ESCAPE = range(14, 17)
        STRING_EXPECT_HEX0, STRING_EXPECT_HEX1 = range(17, 19)
        STOP = -1

    def __init__(self, buffer) -> None:
        self.buffer: io.StringIO = buffer

    def __iter__(self) -> "Lexer":
        return self

    def __next__(self) -> "TokenType":
        prev_pos = self.buffer.tell()
        next_ch = self.buffer.read(1)
        if len(next_ch) == 0:
            raise StopIteration
        else:
            self.buffer.seek(prev_pos, os.SEEK_SET)

        as_str_list: List[str] = []
        as_bytes: bytes = bytes()
        as_int: int = 0
        sign: Literal[-1, 1] = 1
        state: Lexer.States = Lexer.States.START
        token: "TokenType" = (Tokens.EMPTY, None)

        while state != Lexer.States.STOP and token[0] != Tokens.INVALID:
            prev_pos = self.buffer.tell()
            ch: str = self.buffer.read(1)
            if len(ch) == 0:
                break
            match state:
                case Lexer.States.START:
                    if ch == "\n":
                        state = Lexer.States.STOP
                    elif ch == ",":
                        state, token = Lexer.States.STOP, (Tokens.COMMA, None)
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
                        state, as_int = Lexer.States.DECIMAL, ord(ch) - ord("0")
                    elif ch == ".":
                        state = Lexer.States.MAYBE_DOT
                    elif ch == "'":
                        state = Lexer.States.CHAR_OPEN
                    elif ch == '"':
                        state = Lexer.States.STRING_AWAITING_CLOSE
                    elif ch == "+" or ch == "-":
                        state, sign = Lexer.States.MAYBE_SIGNED, -1 if ch == "-" else 1
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

                case Lexer.States.STRING_AWAITING_CLOSE:
                    if ch == '"':
                        token = (Tokens.STRING, as_bytes)
                        state = Lexer.States.STOP
                    elif ch == "\\":
                        state = Lexer.States.STRING_EXPECT_ESCAPE
                    else:
                        as_bytes = as_bytes + ch.encode("utf-8")
                case Lexer.States.STRING_EXPECT_ESCAPE:
                    escapes = dict(zip('rtbn"\\', '\r\t\b\n"\\'))
                    if ch in escapes:
                        state = Lexer.States.STRING_AWAITING_CLOSE
                        as_bytes += escapes[ch].encode("utf-8")
                    elif ch in "xX":
                        state = Lexer.States.STRING_EXPECT_HEX0
                    else:
                        token = (Tokens.INVALID, None)
                case Lexer.States.STRING_EXPECT_HEX0:
                    if ch in string.digits:
                        state = Lexer.States.STRING_EXPECT_HEX1
                        as_int = as_int * 16 + (ord(ch) - ord("0"))
                    elif ch in string.hexdigits:
                        state = Lexer.States.STRING_EXPECT_HEX1
                        as_int = as_int * 16 + (10 + ord(ch.lower()) - ord("a"))
                case Lexer.States.STRING_EXPECT_HEX1:
                    if ch in string.digits:
                        state = Lexer.States.STRING_AWAITING_CLOSE
                        as_int = as_int * 16 + (ord(ch) - ord("0"))
                        as_bytes += as_int.to_bytes(1)
                    elif ch in string.hexdigits:
                        state = Lexer.States.STRING_AWAITING_CLOSE
                        as_int = as_int * 16 + (10 + ord(ch.lower()) - ord("a"))
                        as_bytes += as_int.to_bytes(1)
                    elif ch == '"':
                        state = Lexer.States.STOP
                        token = (Tokens.STRING, as_bytes + as_int.to_bytes(1))

                case _:
                    token = (Tokens.INVALID, None)

        return token

    def skip_to_next_line(self):
        while (ch := self.buffer.read(1)) != "\n" and len(ch) > 0:
            pass
