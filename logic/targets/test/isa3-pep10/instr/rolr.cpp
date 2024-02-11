/*
 * Copyright (c) 2023-2024 J. Stanley Warford, Matthew McRaven
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

namespace {
template <isa::Pep10::Register target_reg> void inner(isa::Pep10::Mnemonic op) {
  auto [mem, cpu] = make();
  quint16 tmp;
  auto [init_reg] = GENERATE(table<quint16>({0, 1, 0x7fff, 0x8000, 0x8FFF, 0xFFFF}));
  auto [carry] = GENERATE(table<quint8>({0, 1}));
  DYNAMIC_SECTION("with initial value==" << init_reg << " and carry==" << carry) {
    auto endRegVal = static_cast<quint16>((init_reg << 1) | (carry & 1));

    // Object code for instruction under test.
    auto program = std::array<quint8, 1>{(quint8)op};

    cpu->regs()->clear(0);
    cpu->csrs()->clear(0);
    tmp = bits::hostOrder() != bits::Order::BigEndian ? bits::byteswap(init_reg) : init_reg;
    cpu->regs()->write(static_cast<quint16>(target_reg) * 2, {reinterpret_cast<quint8 *>(&tmp), 2}, rw);
    cpu->csrs()->write(static_cast<quint8>(::isa::Pep10::CSR::C), {&carry, 1}, rw);

    REQUIRE_NOTHROW(mem->write(0, {program.data(), program.size()}, rw));
    REQUIRE_NOTHROW(cpu->clock(0));

    CHECK(reg(cpu, isa::Pep10::Register::SP) == 0);
    CHECK(reg(cpu, isa::Pep10::Register::PC) == 0x1);
    CHECK(reg(cpu, isa::Pep10::Register::IS) == (quint8)op);
    CHECK(reg(cpu, isa::Pep10::Register::OS) == 0);
    CHECK(!!csr(cpu, isa::Pep10::CSR::N) == 0);
    CHECK(!!csr(cpu, isa::Pep10::CSR::Z) == 0);
    auto new_reg = reg(cpu, target_reg);
    CHECK(csr(cpu, isa::Pep10::CSR::V) == 0);
    // Carry out if high order bit was non-zero
    CHECK(csr(cpu, isa::Pep10::CSR::C) == ((init_reg & 0x8000) ? 1 : 0));
  }
}
} // namespace

TEST_CASE("ROLA", "[pep10][isa]") {
  using Register = isa::Pep10::Register;
  inner<Register::A>(isa::Pep10::Mnemonic::ROLA);
}
TEST_CASE("ROLX", "[pep10][isa]") {
  using Register = isa::Pep10::Register;
  inner<Register::X>(isa::Pep10::Mnemonic::ROLX);
}
