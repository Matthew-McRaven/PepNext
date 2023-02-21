#pragma once

#include "./types_values.hpp"
#include <QtCore>
#include <boost/spirit/home/x3.hpp>

namespace pat::pep::parse {
using boost::spirit::x3::char_;
using boost::spirit::x3::char_range;
using boost::spirit::x3::digit;
using boost::spirit::x3::eps;
using boost::spirit::x3::int_;
using boost::spirit::x3::lexeme;
using boost::spirit::x3::lit;
using boost::spirit::x3::long_long;
using boost::spirit::x3::no_skip;
using boost::spirit::x3::raw;
using boost::spirit::x3::rule;
using boost::spirit::x3::space;
using boost::spirit::x3::uint_parser;
using boost::spirit::x3::ulong_long;
// character / string components
const auto escape_codes = lit("\\b") | lit("\\f") | lit("\\n") | lit("\\r") |
                          lit("\\t") | lit("\\v") | lit("\\\"") | lit("\\'");
const auto hex_chars = char_("a", "f") | char_("A", "F") | char_("0", "9");
const auto escape_hex_code = lit("\\") >> lit("x") |
                             lit("X") >> hex_chars >> hex_chars;
const auto inner_char = (char_ - "\\") | escape_codes | escape_hex_code;

// Character Literal
rule<class character, parse::CharacterLiteral> character = "character";
const auto character_def = lexeme["'" >> raw[-(inner_char - "'")] >> "'"];
BOOST_SPIRIT_DEFINE(character);

// String Literal
rule<class strings, parse::StringLiteral> strings = "strings";
const auto strings_def = lexeme["\"" >> raw[*(inner_char - "\"")] >> "\""];
BOOST_SPIRIT_DEFINE(strings);

// Identifier
const auto ident_char = char_ - (space | "\"" | lit("'") | lit(":") | lit(";") |
                                 lit(",") | lit(".") | lit("-"));
rule<class identifier, parse::Identifier> identifier = "identifier";
const auto identifier_def = lexeme[raw[(ident_char - digit) >> *ident_char]];
BOOST_SPIRIT_DEFINE(identifier);

// UnsignedDecimal Literal
rule<class unsigned_decimal, parse::DecimalLiteral, true> unsigned_decimal =
    "unsigned_decimal";
const auto unsigned_decimal_def = ulong_long;
BOOST_SPIRIT_DEFINE(unsigned_decimal);

// SignedDecimal Literal
auto setSigned = [](auto &ctx) {
  _val(ctx).isSigned = true;
  _val(ctx).value *= -1;
};
rule<class signed_decimal, parse::DecimalLiteral, true> signed_decimal =
    "signed_decimal";
const auto signed_decimal_def = "-" >> ulong_long[setSigned];
BOOST_SPIRIT_DEFINE(signed_decimal);

// Hexadecimal Literal
rule<class hexadecimal, parse::HexadecimalLiteral> hexadecimal = "hexadecimal";
const auto hexadecimal_def = "0x" >> no_skip[uint_parser<quint64, 16>{}];
BOOST_SPIRIT_DEFINE(hexadecimal);

// Argument Non-terminal
rule<class argument, parse::Value> argument = "argument";
const auto argument_def = strings | character | identifier | hexadecimal |
                          unsigned_decimal | signed_decimal;
BOOST_SPIRIT_DEFINE(argument);
} // namespace pat::pep::parse
