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
#include <QtCore>
#include "pas/pas_globals.hpp"

namespace pas::ast::value {
class Base;
}
namespace pas::ast::generic {
struct PAS_EXPORT Argument {
  static const inline QString attributeName = u"generic:arg"_qs;
  QSharedPointer<value::Base> value = {};
  bool operator==(const Argument &other) const = default;
};

struct PAS_EXPORT ArgumentList {
  static const inline QString attributeName = u"generic:arg_list"_qs;
  QList<QSharedPointer<value::Base>> value = {};
  bool operator==(const ArgumentList &other) const = default;
};
} // namespace pas::ast::generic
Q_DECLARE_METATYPE(pas::ast::generic::Argument);
Q_DECLARE_METATYPE(pas::ast::generic::ArgumentList);
