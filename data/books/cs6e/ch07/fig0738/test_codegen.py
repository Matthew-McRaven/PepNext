from pep10.arguments import Decimal
from pep10.ir import UnaryIR, listing, NonUnaryIR
from pep10.keywords import AddressingMode
from pep10.symbol import SymbolTable


def test_unary():
    st = SymbolTable()
    s = st.define("cat")
    i = UnaryIR("RET", sym=s)
    i.address = 0
    assert i.source().rstrip() == "cat:   RET"
    assert "".join(listing(i)).rstrip() == "0000        cat:   RET"
    i = UnaryIR("RET")
    i.address = 0
    assert i.source().rstrip() == "       RET"
    assert "".join(listing(i)).rstrip() == "0000               RET"
    i = UnaryIR("RET", comment="hi")
    i.address = 0
    assert i.source().rstrip() == "       RET                ;hi"
    assert "".join(listing(i)).rstrip() == "0000               RET                ;hi"


def test_nonunary():
    st = SymbolTable()
    s = st.define("cat")
    i = NonUnaryIR("ADDA", Decimal(10), AddressingMode.SX, sym=s)
    i.address = 0
    assert i.source().rstrip() == "cat:   ADDA   10,sx"
    assert "".join(listing(i)).rstrip() == "0000        cat:   ADDA   10,sx"
    i = NonUnaryIR("ADDA", Decimal(10), AddressingMode.SX)
    i.address = 0
    assert i.source().rstrip() == "       ADDA   10,sx"
    assert "".join(listing(i)).rstrip() == "0000               ADDA   10,sx"
    i = NonUnaryIR("ADDA", Decimal(10), AddressingMode.SX, comment="hi")
    i.address = 0
    assert i.source().rstrip() == "       ADDA   10,sx       ;hi"
    assert "".join(listing(i)).rstrip() == "0000               ADDA   10,sx       ;hi"
