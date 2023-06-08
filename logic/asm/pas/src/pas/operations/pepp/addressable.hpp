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

#pragma once
#include "pas/ast/generic/attr_directive.hpp"
#include "pas/ast/node.hpp"
#include "pas/operations/generic/is.hpp"
#include "pas/operations/pepp/is.hpp"

namespace pas::ops::pepp {
template <typename ISA> bool isAddressable(const ast::Node &node);
}

template <typename ISA>
bool pas::ops::pepp::isAddressable(const ast::Node &node) {
  static const auto addressableDirectives =
      QSet<QString>{"ALIGN", "ASCII", "BLOCK", "BYTE", "WORD"};
  if (generic::isDirective()(node) &&
      addressableDirectives.contains(
          node.get<ast::generic::Directive>().value.toUpper()))
    return true;
  return isUnary<ISA>()(node) || isNonUnary<ISA>()(node);
}
