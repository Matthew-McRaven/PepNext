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
#include <QSysInfo>
#include <QtCore>
namespace bits {
enum class Order {
  BigEndian = QSysInfo::Endian::BigEndian,
  LittleEndian = QSysInfo::Endian::LittleEndian,
  NotApplicable
};
constexpr Order hostOrder() {
  switch (QSysInfo::ByteOrder) {
  case QSysInfo::LittleEndian: return Order::LittleEndian;
  case QSysInfo::BigEndian: return Order::BigEndian;
  default: throw std::logic_error("Endian must be big or little");
  }
}

} // namespace bits
