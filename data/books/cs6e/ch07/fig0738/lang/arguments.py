import abc


class AArgument(abc.ABC):
    @abc.abstractmethod
    def generate_code(self) -> str:
        return ""

    @abc.abstractmethod
    def generate_listing(self) -> str:
        return ""


class IdentifierArgument(AArgument):
    def __init__(self, value: str):
        self.value = value

    def generate_code(self) -> str:
        return self.value

    def generate_listing(self) -> str:
        return self.value


class IntegerArgument(AArgument):
    def __init__(self, value: int):
        self.value = value

    def generate_code(self) -> str:
        return f"{self.value}"

    def generate_listing(self) -> str:
        return f"{self.value}"
