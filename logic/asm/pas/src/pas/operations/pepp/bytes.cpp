#include "./bytes.hpp"
#include "pas/ast/generic/attr_address.hpp"
#include "pas/ast/generic/attr_argument.hpp"
#include "pas/ast/generic/attr_directive.hpp"
#include "pas/ast/value/base.hpp"
quint16 alignToBytes(const pas::ast::Node &node, quint8 *dest, quint64 length) {
  auto address = node.get<pas::ast::generic::Address>().value;
  if (length < address.size)
    return 0;
  memset(dest, 0, address.size);
  return address.size;
}

quint16 asciiToBytes(const pas::ast::Node &node, quint8 *dest, quint64 length) {
  auto argument = node.get<pas::ast::generic::Argument>().value;
  auto size = argument->size();
  if (length < size)
    return 0;
  // Strings are always host order, since they are single bytes, not multi-byte.
  // Single bytes have no order.
  argument->value(dest, size, pas::bits::hostOrder());
  return size;
}

quint16 blockToBytes(const pas::ast::Node &node, quint8 *dest, quint64 length) {
  auto argument = node.get<pas::ast::generic::Argument>().value;
  quint16 size = 0;
  argument->value(reinterpret_cast<quint8 *>(&size), 2, pas::bits::hostOrder());
  if (length < size)
    return 0;
  memset(dest, 0, size);
  return size;
}

quint16 byteToBytes(const pas::ast::Node &node, quint8 *dest, quint64 length) {
  auto argument = node.get<pas::ast::generic::Argument>().value;
  auto size = argument->size();
  if (length < size)
    return 0;
  argument->value(dest, size);
  return size;
}

quint16 wordToBytes(const pas::ast::Node &node, quint8 *dest, quint64 length) {
  auto argument = node.get<pas::ast::generic::Argument>().value;
  auto size = argument->size();
  if (length < size)
    return 0;
  argument->value(dest, size);
  return size;
}

using convertFn =
    std::function<quint16(const pas::ast::Node &, quint8 *, quint64)>;
quint16 pas::ops::pepp::detail::directiveToBytes(const ast::Node &node,
                                                 quint8 *dest, quint64 length) {
  static QMap<QString, convertFn> directives = {{u"ALIGN"_qs, alignToBytes},
                                                {u"ASCII"_qs, asciiToBytes},
                                                {u"BLOCK"_qs, blockToBytes},
                                                {u"BYTE"_qs, byteToBytes},
                                                {u"WORD"_qs, wordToBytes}};
  QString directive = node.get<ast::generic::Directive>().value;
  if (auto it = directives.find(directive); it != directives.end())
    return it.value()(node, dest, length);
  else
    return 0;
}
