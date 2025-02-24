from enum import Enum
from typing import TypeAlias, Union, Tuple, Literal


class Tokens(Enum):
    EMPTY, COMMA, COMMENT, IDENTIFIER, SYMBOL, DECIMAL, HEX, DOT = range(0, 8)
    CHARACTER, STRING, MACRO = range(8, 11)
    INVALID = -1


TokenType: TypeAlias = Union[
    Tuple[Literal[Tokens.DECIMAL], int]
    | Tuple[Literal[Tokens.HEX], int]
    | Tuple[Literal[Tokens.IDENTIFIER], str]
    | Tuple[Literal[Tokens.SYMBOL], str]
    | Tuple[Literal[Tokens.COMMENT], str]
    | Tuple[Literal[Tokens.DOT], str]
    | Tuple[Literal[Tokens.MACRO], str]
    | Tuple[Literal[Tokens.CHARACTER], bytes]
    | Tuple[Literal[Tokens.STRING], bytes]
    | Tuple[Literal[Tokens.EMPTY], None]
    | Tuple[Literal[Tokens.COMMA], None]
    | Tuple[Literal[Tokens.INVALID], None]
]
