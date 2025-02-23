import io
from typing import cast

from pep10.arguments import Decimal, Hexadecimal, Identifier, StringConstant
from pep10.ir import (
    UnaryIR,
    NonUnaryIR,
    CommentNode,
    EmptyNode,
    ErrorNode,
    CommentIR,
    EmptyIR,
)
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

    ret = parse('cat: BR "h\'",i')
    item = cast(NonUnaryIR, ret[0])
    assert type(item) == NonUnaryIR
    assert str(item.symbol_decl) == "cat"
    assert item.mnemonic == "BR"
    assert type(item.argument) == StringConstant
    arg: StringConstant = item.argument
    assert int(arg).to_bytes(2) == "h'".encode("utf-8")
    assert str(arg) == '"h\'"'

    ret = parse('cat: BR "\\r\\"",i')
    item = cast(NonUnaryIR, ret[0])
    assert type(item) == NonUnaryIR
    assert str(item.symbol_decl) == "cat"
    assert item.mnemonic == "BR"
    assert type(item.argument) == StringConstant
    arg: StringConstant = item.argument
    assert int(arg).to_bytes(2) == '\r"'.encode("utf-8")
    assert str(arg) == '"\\r\\""'


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


def test_nonunary_arg_range():
    ret = parse("BR 65535\n")
    assert type(ret[0]) != ErrorNode
    ret = parse("BR 65536\n")
    assert type(ret[0]) == ErrorNode
    ret = parse("BR -32768\n")
    assert type(ret[0]) != ErrorNode
    ret = parse("BR -32769\n")
    assert type(ret[0]) == ErrorNode
    ret = parse("BR 0xFFFF\n")
    assert type(ret[0]) != ErrorNode
    ret = parse("BR 0x10000\n")
    assert type(ret[0]) == ErrorNode


def test_comment():
    par = Parser(io.StringIO("  ;comment \n"))
    item: CommentNode = cast(CommentNode, next(par))
    assert type(item) == CommentIR
    assert item.comment == "comment "


def test_empty():
    par = Parser(io.StringIO("\n"))
    item: EmptyNode = cast(EmptyNode, next(par))
    assert type(item) == EmptyIR


def test_parser_synchronization():
    ret = parse("NOPN HELLO CRUEL: WORLD\nNOPN\nRET\n")
    assert len(ret) == 3
