import pytest

from pep10.symbol import SymbolTable


# Referring to the same symbol in different places accesses the same underlying object.
def test_same_object():
    tb = SymbolTable()
    s0 = tb.reference("test")
    s1 = tb.reference("test")
    assert s0 == s1
    s2 = tb.define("test")
    assert s0 == s2
    assert s2 == tb.define("test")


def test_undefined():
    tb = SymbolTable()
    s = tb.reference("test")
    assert s.is_undefined()
    assert not s.is_singly_defined()
    assert not s.is_multiply_defined()


def test_definition_transitions():
    tb = SymbolTable()
    s0 = tb.reference("test")
    assert s0.is_undefined()
    assert not s0.is_singly_defined()
    assert not s0.is_multiply_defined()
    tb.define("test")
    assert not s0.is_undefined()
    assert s0.is_singly_defined()
    assert not s0.is_multiply_defined()
    # Ensure that repeated definitions keeps us in the multiply defined state.
    for i in range(3):
        tb.define("test")
        assert not s0.is_undefined()
        assert not s0.is_singly_defined()
        assert s0.is_multiply_defined()


def test_value_assignment():
    tb = SymbolTable()
    s0 = tb.reference("test")
    assert s0.value is None
    assert int(s0) == 0
    s0.value = 5
    assert s0.value == 5
    assert int(s0) == 5
    del s0.value
    assert s0.value is None
    assert int(s0) == 0


def test_value_pointers():
    tb = SymbolTable()
    s0, s1 = tb.reference("pointed"), tb.reference("pointer")
    assert s0.value is None and s1.value is None
    s0.value = 5
    assert s0.value == 5 and s1.value is None
    s1.value = s0
    assert s0.value == 5 and s1.value == s0
    assert int(s0) == int(s1)


def test_value_cycles():
    tb = SymbolTable()
    s0, s1, s2 = tb.reference("p0"), tb.reference("p1"), tb.reference("p2")
    s2.value = s1
    # Create a cycle containing exactly 2 symbols
    with pytest.raises(RecursionError):
        s1.value = s2
    # Create a cycle with 3 symbols
    s1.value = s0
    with pytest.raises(RecursionError):
        s0.value = s1
