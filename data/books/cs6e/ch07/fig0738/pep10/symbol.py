from typing import Dict, Union


class SymbolEntry:
    def __init__(self, name: str):
        self.name: str = name
        self.definition_count: int = 0
        self._value: SymbolEntry | int | None = None

    def is_undefined(self):
        return self.definition_count == 0

    def is_singly_defined(self):
        return self.definition_count == 1

    def is_multiply_defined(self):
        return self.definition_count > 1

    @property
    def value(self):
        return self._value

    @value.setter
    def value(self, value: Union["SymbolEntry", int]):
        if type(value) is int:
            self._value = value
        # If given a value that itself is a symbol we must prevent cycles.
        else:
            # parents is a set which contains all visited SymbolEntry instances
            parents, next_value = {self}, value
            # "Recurse" on .value until we reach an integer value
            while type(next_value) is SymbolEntry:
                parents.add(next_value)
                next_value = next_value.value
                # If next_value is already in parent set, then we have a cycle which cannot be resolved.
                if next_value in parents:
                    raise RecursionError()
            self._value = value

    @value.deleter
    def value(self):
        self._value = None

    def __int__(self) -> int:
        if self.value is None:
            return 0
        elif type(self.value) is int:
            return self.value
        else:
            return int(self.value)

    def __str__(self):
        return self.name


class SymbolTable:
    def __init__(self) -> None:
        self._table: Dict[str, SymbolEntry] = {}

    def reference(self, name: str) -> SymbolEntry:
        if name not in self._table:
            self._table[name] = SymbolEntry(name)
        return self._table[name]

    def define(self, name: str) -> SymbolEntry:
        sym = self.reference(name)
        sym.definition_count += 1
        return sym

    def __contains__(self, name: str) -> bool:
        return name in self._table

    def __getitem__(self, name: str):
        return self._table[name]


def add_OS_symbols(st: SymbolTable):
    st.define("pwrOff").value = 0xFFFF
    st.define("charOut").value = 0xFFFE
    st.define("charIn").value = 0xFFFD
    st.define("DECI").value = 0
    st.define("DECO").value = 1
    st.define("HEXO").value = 2
    st.define("STRO").value = 3
    st.define("SNOP").value = 4
