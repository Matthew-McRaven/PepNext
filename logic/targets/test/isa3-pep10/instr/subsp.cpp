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

#include <QTest>
#include <QtCore>

#include "bits/operations/swap.hpp"
#include "sim/device/dense.hpp"
#include "targets/pep10/isa3/cpu.hpp"
#include "targets/pep10/isa3/helpers.hpp"
auto desc_mem = sim::api::device::Descriptor{
    .id = 1,
    .baseName = "ram",
    .fullName = "/ram",
};

auto desc_cpu = sim::api::device::Descriptor{
    .id = 2,
    .baseName = "cpu",
    .fullName = "/cpu",
};

auto span = sim::api::memory::Target<quint16>::AddressSpan{
    .minOffset = 0,
    .maxOffset = 0xFFFF,
};

auto make = []() {
  int i = 3;
  sim::api::device::IDGenerator gen = [&i]() { return i++; };
  auto storage =
      QSharedPointer<sim::memory::Dense<quint16>>::create(desc_mem, span);
  auto cpu = QSharedPointer<targets::pep10::isa::CPU>::create(desc_cpu, gen);
  cpu->setTarget(storage.data());
  return std::pair{storage, cpu};
};

sim::api::memory::Operation rw = {.speculative = false,
                                  .kind =
                                      sim::api::memory::Operation::Kind::data,
                                  .effectful = false};

class ISA3Pep10_SUBSP : public QObject {
  Q_OBJECT
private slots:
  void i() {
    QFETCH(quint16, target);
    auto [mem, cpu] = make();
    // Loop over a subset of possible values for the target register.
    quint16 tmp;

    // Can't capture CPU directly b/c structured bindings.
    auto _cpu = cpu;
    auto rreg = [&](isa::Pep10::Register reg) -> quint16 {
      quint16 tmp = 0;
      targets::pep10::isa::readRegister(_cpu->regs(), reg, tmp, rw);
      return tmp;
    };
    auto rcsr = [&](isa::Pep10::CSR csr) {
      bool tmp = 0;
      targets::pep10::isa::readCSR(_cpu->csrs(), csr, tmp, rw);
      return tmp;
    };

    static const auto target_reg = isa::Pep10::Register::SP;
    for (uint16_t opspec = 0; static_cast<uint32_t>(opspec) + 1 < 0x1'0000;
         opspec++) {
      auto endRegVal = static_cast<quint16>(target - opspec);

      // Object code for instruction under test.
      auto program = std::array<quint8, 3>{
          0xF8, static_cast<uint8_t>((opspec >> 8) & 0xff),
          static_cast<uint8_t>(opspec & 0xff)};

      cpu->regs()->clear(0);
      cpu->csrs()->clear(0);
      tmp = bits::hostOrder() != bits::Order::BigEndian ? bits::byteswap(target)
                                                        : target;
      cpu->regs()->write(static_cast<quint16>(target_reg) * 2,
                         {reinterpret_cast<quint8 *>(&tmp), 2}, rw);

      QVERIFY(mem->write(0, {program.data(), program.size()}, rw).completed);

      auto tick = cpu->tick(0);
      QCOMPARE(tick.error, sim::api::tick::Error::Success);

      tmp = bits::hostOrder() != bits::Order::BigEndian ? bits::byteswap(tmp)
                                                        : tmp;
      QCOMPARE(rreg(isa::Pep10::Register::A), 0);
      QCOMPARE(rreg(isa::Pep10::Register::X), 0);
      QCOMPARE(rreg(isa::Pep10::Register::PC), 0x3);
      QCOMPARE(rreg(isa::Pep10::Register::IS), 0xF8);
      // OS loaded the Mem[0x0001-0x0002].
      QCOMPARE(rreg(isa::Pep10::Register::OS), opspec);
      // Check that target register had arithmetic performed.
      QCOMPARE(rreg(isa::Pep10::Register::SP), endRegVal);
    }
  }
  void i_data() {
    QTest::addColumn<quint16>("target");
    QTest::addRow("0x0000") << quint16(0x0000);
    QTest::addRow("0x0001") << quint16(0x0001);
    QTest::addRow("0x7FFF") << quint16(0x7FFF);
    QTest::addRow("0x8000") << quint16(0x8000);
    QTest::addRow("0x8FFF") << quint16(0x8FFF);
    QTest::addRow("0xFFFF") << quint16(0xFFFF);
  }
};

#include "subsp.moc"

QTEST_MAIN(ISA3Pep10_SUBSP)
