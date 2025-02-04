import enum
import io
from typing import List, cast

from lang.arguments import IdentifierArgument, IntegerArgument, AArgument
from lang.code import ACode, Error, UnaryInstr
from lang.code import EmptyInstr, OneArgInstr, TwoArgInstr
from lang.mnemonics import Mnemonics
from lang.tokenizer import Tokenizer
from lang.tokens import Tokens


class Translator:
    class States(enum.Enum):
        START = 1
        UNARY = 2
        FUNCTION = 3
        OPEN = 4
        OPERAND_1ST = 5
        OPERAND_2ND = 6
        NONUNARY1 = 7
        NONUNARY2 = 8
        COMMA = 9
        FINISH = 10

    def __init__(self, buffer: io.StringIO):
        self.buffer = buffer
        self.tokenizer = Tokenizer(self.buffer)

    def parse_line(self) -> ACode | None:
        state = Translator.States.START
        code: ACode | None = None
        arg1: AArgument | None = None
        arg2: AArgument | None = None
        mnemonic: Mnemonics = Mnemonics.END
        while state != Translator.States.FINISH and type(code) is not Error:
            token_type, token_val = self.tokenizer.next_token()
            match state:
                case Translator.States.START:
                    if token_type == Tokens.IDENTIFIER and type(token_val) is str:
                        as_upper = token_val.upper()
                        try:
                            as_mnemonic: Mnemonics = Mnemonics[as_upper]
                            if as_mnemonic.is_unary():
                                code = UnaryInstr(as_mnemonic)
                                state = Translator.States.UNARY
                            else:
                                mnemonic = as_mnemonic
                                state = Translator.States.FUNCTION
                        except KeyError:
                            code = Error("Line must begin with function identifier")
                    elif token_type == Tokens.EMPTY:
                        code = EmptyInstr()
                        state = Translator.States.FINISH
                    else:
                        code = Error("Line must begin with function identifier")

                case Translator.States.FUNCTION:
                    if token_type == Tokens.LEFT_PAREN:
                        state = Translator.States.OPEN
                    else:
                        code = Error("Left parenthesis expected after function.")

                case Translator.States.OPEN:
                    if token_type == Tokens.IDENTIFIER and type(token_val) is str:
                        arg1 = IdentifierArgument(token_val)
                        state = Translator.States.OPERAND_1ST
                    else:
                        code = Error("First argument is not an identifier.")

                case Translator.States.OPERAND_1ST:
                    if mnemonic in {Mnemonics.NEG, Mnemonics.ABS}:
                        if token_type == Tokens.RIGHT_PAREN:
                            code = OneArgInstr(mnemonic, cast(AArgument, arg1))
                            state = Translator.States.NONUNARY1
                        else:
                            code = Error("Right parenthesis expected after argument.")
                    elif token_type == Tokens.COMMA:
                        state = Translator.States.COMMA
                    else:
                        code = Error("Comma expected after first argument.")

                case Translator.States.COMMA:
                    if token_type == Tokens.IDENTIFIER and type(token_val) is str:
                        arg2 = IdentifierArgument(token_val)
                        state = Translator.States.OPERAND_2ND
                    elif token_type == Tokens.INTEGER and type(token_val) is int:
                        arg2 = IntegerArgument(token_val)
                        state = Translator.States.OPERAND_2ND
                    else:
                        code = Error("Second argument not an identifier or integer.")

                case Translator.States.OPERAND_2ND:
                    if token_type == Tokens.RIGHT_PAREN:
                        code = TwoArgInstr(
                            mnemonic, cast(AArgument, arg1), cast(AArgument, arg2)
                        )
                        state = Translator.States.NONUNARY2
                    else:
                        code = Error("Right parenthesis expected after argument.")

                # Unary or NonUnary1 or NonUnary2
                case _:
                    if token_type == Tokens.EMPTY:
                        state = Translator.States.FINISH
                    else:
                        code = Error("Illegal trailing character.")
        # TODO: advance tokenizer to start of next line if we are in an error state
        return code

    def translate(self):
        program: List[ACode] = []
        error_count = 0

        while (code := self.parse_line()) is not None:
            program.append(code)
            if type(code) is Error:
                error_count += 1
            # TODO: ugly hack that should be a state variable instead. Will revisit when this becomes iterable.
            elif (
                type(code) is UnaryInstr
                and cast(UnaryInstr, code).mnemonic == Mnemonics.END
            ):
                break
        else:  # No break
            program.append(Error('Missing "end" sentinel'))
            error_count += 1

        if error_count == 0:
            print("Object code:")
            lines: List[str] = [line.generate_code() for line in program]
            print(*filter(lambda l: l != "", lines), sep="")
        elif error_count == 1:
            print("One error was detected.")
        else:
            print(f"{error_count} errors were detected.")

        print("Program listing:")
        print(*(line.generate_listing() for line in program), sep="")
