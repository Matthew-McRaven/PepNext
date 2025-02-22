import io
from typing import cast

from pep10.arguments import Decimal, Hexadecimal, Identifier
from pep10.ir import UnaryIR, NonUnaryIR, CommentNode, EmptyNode, ErrorNode
from pep10.mnemonics import AddressingMode
from pep10.parser import Parser, parse


def test_unary_pass():
    par = Parser(io.StringIO("RET \n"))
    item: UnaryIR = cast(UnaryIR, next(par))
    assert type(item) == UnaryIR
    assert item.mnemonic == "RET"

    res = parse("caT:NOTA \n")
    item = cast(UnaryIR, res[0])
    assert type(item) == UnaryIR
    assert item.mnemonic == "NOTA"
    assert str(item.symbol_decl) == "caT"


def test_unary_fail():
    res = parse("RETS \n")
    assert type(res[0]) == ErrorNode

    res = parse("RETS \n")
    assert type(res[0]) == ErrorNode


def test_nonunary():
    par = Parser(io.StringIO("BR 10,i \n"))
    item: NonUnaryIR = cast(NonUnaryIR, next(par))
    assert type(item) == NonUnaryIR
    assert item.mnemonic == "BR"
    assert type(item.argument) == Decimal

    ret = parse("cat: BR 0x10,x ;comment\n")
    item = cast(NonUnaryIR, ret[0])
    assert type(item) == NonUnaryIR
    assert str(item.symbol_decl) == "cat"
    assert item.mnemonic == "BR"
    assert type(item.argument) == Hexadecimal
    assert item.comment == "comment"

    ret = parse("cat: BR cat,i")
    item = cast(NonUnaryIR, ret[0])
    assert type(item) == NonUnaryIR
    assert str(item.symbol_decl) == "cat"
    assert item.mnemonic == "BR"
    assert type(item.argument) == Identifier
    arg: Identifier = item.argument
    assert str(arg) == "cat"
    assert arg.symbol.is_singly_defined()


def test_nonunary_fail():
    par = Parser(io.StringIO("ADDA 10\n"))
    assert type(next(par)) == ErrorNode

    ret = parse("ADDA 10 ,\n")
    assert type(ret[0]) == ErrorNode

    ret = parse("ADDA 10,cat\n")
    assert type(ret[0]) == ErrorNode

    ret = parse("ADDA cat:,sfx\n")
    assert type(ret[0]) == ErrorNode


# @pytest.mark.skip(reason="Exercise for students")
def test_nonunary_addr_optional():
    ret = parse("BR 10\n")
    item: NonUnaryIR = cast(NonUnaryIR, ret[0])
    assert type(item) == NonUnaryIR
    assert item.mnemonic == "BR"
    assert type(item.argument) == Decimal
    assert int(item.argument) == 10
    assert item.addressing_mode == AddressingMode.I


def test_comment():
    par = Parser(io.StringIO("  ;comment \n"))
    item: CommentNode = cast(CommentNode, next(par))
    assert type(item) == CommentNode
    assert item.comment == "comment "


def test_empty():
    par = Parser(io.StringIO("\n"))
    item: EmptyNode = cast(EmptyNode, next(par))
    assert type(item) == EmptyNode
