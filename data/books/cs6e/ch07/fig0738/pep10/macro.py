from typing import Tuple, Dict


class MacroRegistry:
    def __init__(self) -> None:
        # Map names to (argument count, body text) tuples
        self._macros: Dict[str, Tuple[int, str]] = {}

    def __contains__(self, name: str):
        return name in self._macros

    def instantiate(self, name: str, *args) -> str:
        (argc, body) = self._macros[name]
        if argc != len(args):
            raise KeyError(
                f"{name} instantiated with {argc} arguments; expected {len(args)}"
            )
        for index, arg in enumerate(args, start=1):
            body = body.replace(f"${index}", arg)
        return body

    def insert(self, name: str, argc: int, body: str):
        self._macros[name] = (argc, body)


def add_OS_macros(mr: MacroRegistry):
    mr.insert("DECI", 2, "LDWA DECI,i\nSCALL $1,$2\n")
    mr.insert("DECO", 2, "LDWA DECO,i\nSCALL $1,$2\n")
    mr.insert("HEXO", 2, "LDWA HEXO,i\nSCALL $1,$2\n")
    mr.insert("STRO", 2, "LDWA STRO,i\nSCALL $1,$2\n")
    mr.insert("SNOP", 2, "LDWA SNOP,i\nSCALL $1,$2\n")
