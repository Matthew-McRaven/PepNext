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

#include <QObject>

#include "./types.hpp"
#include "macro_globals.hpp"

namespace macro {
class MACRO_EXPORT Parsed : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ name CONSTANT);
  Q_PROPERTY(QString body READ body CONSTANT);
  Q_PROPERTY(quint8 argCount READ argCount CONSTANT);
  Q_PROPERTY(QString architecture READ architecture CONSTANT)
public:
  Parsed(QString name, quint8 argCount, QString body, QString architecture);
  QString name() const;
  QString body() const;
  quint8 argCount() const;
  QString architecture() const;

private:
  QString _name, _body, _architecture;
  quint8 _argCount;
};

} // namespace macro
