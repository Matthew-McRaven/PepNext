/*
 * Copyright (c) 2023 J. Stanley Warford, Matthew McRaven
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "./combine.hpp"
#include "asm/pas/ast/generic/attr_address.hpp"
#include "asm/pas/ast/generic/attr_argument.hpp"
#include "asm/pas/ast/generic/attr_symbol.hpp"
#include "asm/pas/ast/value/base.hpp"
#include "asm/pas/operations/generic/is.hpp"
#include "asm/symbol/entry.hpp"
#include "asm/symbol/value.hpp"
#include "bits/order.hpp"

bool pas::ops::generic::detail::isOrgSection(const ast::Node &section) {
  bool accumulator = false;
  for (const auto &child : ast::children(section)) accumulator |= pas::ops::generic::isOrg()(*child);
  return accumulator;
}

quint64 pas::ops::generic::detail::minAddress(const ast::Node &section) {
  quint64 ret = -1;
  for (auto &child : ast::children(section))
    if (child->has<ast::generic::Address>()) ret = qMin(ret, child->get<ast::generic::Address>().value.start);
  return ret;
}

pas::ops::generic::detail::Traits pas::ops::generic::detail::getTraits(const ast::Node &section) {
  quint64 start = -1, size = 0, align = 1;
  for (const auto &child : ast::children(section)) {
    if (!child->has<ast::generic::Address>()) continue;
    auto address = child->get<ast::generic::Address>().value;
    start = qMin(address.start, start);
    size += address.size;
    if (pas::ops::generic::isAlign()(*child)) {
      auto arg = child->get<pas::ast::generic::Argument>().value;
      quint64 dest = 0;
      auto destSpan = bits::span<quint8>{reinterpret_cast<quint8 *>(&dest), sizeof(dest)};
      arg->value(destSpan, bits::hostOrder());
      align = qMax(dest, align);
    }
  }
  // If start is 0xFF..F, then ther is no addressable bytes in the section.
  if (std::cmp_equal(start, -1)) return {.base = 0, .size = 0, .alignment = 1};
  else return {.base = start, .size = size, .alignment = align};
}
void pas::ops::generic::detail::addOffset(ast::Node &section, qsizetype offset) {
  for (auto &child : ast::children(section)) {
    if (child->has<ast::generic::Address>()) {
      auto address = child->get<ast::generic::Address>().value;
      address.start += offset;
      child->set(pas::ast::generic::Address{.value = address});
    }
    // Relocate symbols so that symtab will have latest address
    if (child->has<ast::generic::SymbolDeclaration>()) {
      auto sym = child->get<ast::generic::SymbolDeclaration>().value;
      if (auto asLocation = dynamic_cast<symbol::value::Location *>(&*sym->value); asLocation != nullptr)
        asLocation->addToOffset(offset);
    }
  }
}

bool pas::ops::generic::concatSectionAddresses(ast::Node &root) {

  QMap<qsizetype, QSharedPointer<pas::ast::Node>> orgSections{};
  quint64 nextBase = 0;
  auto children = ast::children(root);
  for (int it = 0; it < children.size(); it++)
    if (detail::isOrgSection(*children[it])) orgSections[it] = children[it];
  qsizetype previousOrg = -1;
  quint64 previousAddress = 0;
  for (auto [index, sec] : orgSections.asKeyValueRange()) {
    auto orgSecTraits = detail::getTraits(*sec);
    previousAddress = orgSecTraits.base;
    for (int it = index - 1; it > previousOrg; --it) {
      auto secTraits = detail::getTraits(*children[it]);
      quint64 unalignedStart = previousAddress - secTraits.size;
      quint64 alignedStart = unalignedStart - (unalignedStart % secTraits.alignment);
      quint64 paddingBytes = unalignedStart - alignedStart;
      // TODO: Insert padding block at end of current section
      if (paddingBytes != 0) {
      }
      // backwards shift
      detail::addOffset(*children[it], alignedStart);
      previousAddress = alignedStart;
    }
    previousOrg = index;
    previousAddress = orgSecTraits.base + orgSecTraits.size;
  }

  for (int it = previousOrg + 1; it < children.size(); it++) {
    // Forwards combine
    auto secTraits = detail::getTraits(*children[it]);
    quint64 paddingBytes = (secTraits.alignment - (previousAddress % secTraits.alignment)) % secTraits.alignment;
    quint64 alignedStart = previousAddress + paddingBytes;
    detail::addOffset(*children[it], alignedStart);
    // TODO: Insert padding block at start of current section
    if (previousAddress % secTraits.alignment != 0) {
    }
    previousAddress = alignedStart + secTraits.size;
  }
  // TODO: Check if any two sections have an overlapping address range.
  // If so, return false, and add an error to the start of those sections.
  return true;
}
