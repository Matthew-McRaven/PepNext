import io
from typing import cast

from pep10.arguments import Decimal, Hexadecimal, Identifier
from pep10.ir import UnaryIR, NonUnaryIR, CommentNode, EmptyNode
from pep10.parser import Parser


def test_unary():
    par = Parser(io.StringIO("RET \n"))
    item: UnaryIR = cast(UnaryIR, next(par))
    assert type(item) == UnaryIR
    assert item.mnemonic == "RET"

    par = Parser(io.StringIO("caT:NOTA \n"))
    item = cast(UnaryIR, next(par))
    assert type(item) == UnaryIR
    assert item.mnemonic == "NOTA"
    assert str(item.symbol_decl) == "caT"


def test_nonunary():
    par = Parser(io.StringIO("BR 10,i \n"))
    item: NonUnaryIR = cast(NonUnaryIR, next(par))
    assert type(item) == NonUnaryIR
    assert item.mnemonic == "BR"
    assert type(item.argument) == Decimal

    par = Parser(io.StringIO("cat: BR 0x10,x ;comment\n"))
    item = cast(NonUnaryIR, next(par))
    assert type(item) == NonUnaryIR
    assert str(item.symbol_decl) == "cat"
    assert item.mnemonic == "BR"
    assert type(item.argument) == Hexadecimal
    assert item.comment == "comment"

    par = Parser(io.StringIO("cat: BR cat,i \n"))
    item = cast(NonUnaryIR, next(par))
    assert type(item) == NonUnaryIR
    assert str(item.symbol_decl) == "cat"
    assert item.mnemonic == "BR"
    assert type(item.argument) == Identifier
    arg: Identifier = item.argument
    assert str(arg) == "cat"
    assert arg.symbol.is_singly_defined()


def test_comment():
    par = Parser(io.StringIO(";comment \n"))
    item: CommentNode = cast(CommentNode, next(par))
    assert type(item) == CommentNode
    assert item.comment == "comment "


def test_empty():
    par = Parser(io.StringIO("\n"))
    item: EmptyNode = cast(EmptyNode, next(par))
    assert type(item) == EmptyNode
