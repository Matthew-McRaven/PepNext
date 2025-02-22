from pep10.ir import UnaryIR, listing
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
