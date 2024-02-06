/*
 * Copyright (c) 2024 J. Stanley Warford, Matthew McRaven
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

#include <catch.hpp>

#include "./api.hpp"
#include "bits/operations/swap.hpp"
#include "sim/device/dense.hpp"
#include "targets/pep10/isa3/cpu.hpp"
#include "targets/pep10/isa3/helpers.hpp"

void inner(isa::Pep10::Mnemonic op) {
  auto [mem, cpu] = make();
  quint8 buf[2];
  auto bufSpan = bits::span<quint8>{buf};
  // Object code for instruction under test.
  auto program = std::array<quint8, 1>{(quint8)op};

  cpu->regs()->clear(0);
  cpu->csrs()->clear(0);
  constexpr quint8 truth[2] = {0x11, 0x25};
  targets::pep10::isa::writeRegister(cpu->regs(), isa::Pep10::Register::SP, 0xFFFD, rw);
  REQUIRE_NOTHROW(mem->write(0xFFFD, truth, rw));

  REQUIRE_NOTHROW(mem->write(0x0000, {program.data(), program.size()}, rw));
  REQUIRE_NOTHROW(cpu->clock(0));

  CHECK(reg(cpu, isa::Pep10::Register::SP) == 0xFFFF);
  CHECK(reg(cpu, isa::Pep10::Register::PC) == 0x1125);
  CHECK(reg(cpu, isa::Pep10::Register::IS) == (quint8)op);
}
TEST_CASE("ret", "[pep10][isa]") { inner(isa::Pep10::Mnemonic::RET); }

int main(int argc, char *argv[]) { return Catch::Session().run(argc, argv); }
