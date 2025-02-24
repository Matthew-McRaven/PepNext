from typing import Protocol, runtime_checkable


class ArgumentType(Protocol):
    def __str__(self) -> str: ...
    def __int__(self) -> int: ...


class ParseTreeNode(Protocol):
    comment: str | None

    def source(self) -> str: ...


@runtime_checkable
class Listable(Protocol):
    address: int | None

    def source(self) -> str: ...
    def object_code(self) -> bytearray: ...
    def __len__(self) -> int: ...
