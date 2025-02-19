from pep10.arguments import Identifier, Hexadecimal, Decimal
from pep10.symbol import SymbolTable


def test_arg_identifier():
    st = SymbolTable()
    s = st.define("cat")
    s.value = -12
    arg = Identifier(s)
    assert str(arg) == "cat"
    assert int(arg) == -12


def test_arg_hex():
    arg = Hexadecimal(17)
    assert str(arg) == "0x0011"
    assert int(arg) == 17


def test_arg_positive_dec():
    arg = Decimal(0)
    assert str(arg) == "0"
    assert int(arg) == 0
    arg = Decimal(65537)
    assert str(arg) == "65537"
    assert int(arg) == 65537


def test_arg_negative_dec():
    arg = Decimal(-0)
    assert str(arg) == "0"
    assert int(arg) == 0
    arg = Decimal(-45678)
    assert str(arg) == "-45678"
    assert int(arg) == -45678
