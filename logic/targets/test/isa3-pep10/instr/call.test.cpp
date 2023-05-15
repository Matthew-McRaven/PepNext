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

class ISA3Pep10_CALL : public QObject {
  Q_OBJECT
private slots:
  void i() {
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

    const quint8 truth[2] = {0x11, 0x25};
    quint8 buf[2];
    auto bufSpan = bits::span<quint8>{buf};
    static const auto target_reg = isa::Pep10::Register::PC;
    for (uint16_t opspec = 0x00; static_cast<uint32_t>(opspec) + 1 < 0x01'00;
         opspec++) {
      auto endRegVal = static_cast<quint16>(opspec);

      // Object code for instruction under test.
      auto program = std::array<quint8, 3>{
          0x2E, static_cast<uint8_t>((opspec >> 8) & 0xff),
          static_cast<uint8_t>(opspec & 0xff)};

      cpu->regs()->clear(0);
      cpu->csrs()->clear(0);
      targets::pep10::isa::writeRegister(cpu->regs(), isa::Pep10::Register::SP,
                                         0xFFFF, rw);
      // Make pushed return addres non-zero.
      targets::pep10::isa::writeRegister(cpu->regs(), isa::Pep10::Register::PC,
                                         0x1122, rw);
      QVERIFY(
          mem->write(0x1122, {program.data(), program.size()}, rw).completed);

      auto tick = cpu->tick(0);
      QCOMPARE(tick.error, sim::api::tick::Error::Success);

      QCOMPARE(rreg(isa::Pep10::Register::SP), 0xFFFD);
      QCOMPARE(rreg(isa::Pep10::Register::A), 0);
      QCOMPARE(rreg(isa::Pep10::Register::X), 0);
      QCOMPARE(rreg(isa::Pep10::Register::TR), 0);
      QCOMPARE(rreg(isa::Pep10::Register::IS), 0x2E);
      QVERIFY(mem->read(0xFFFD, bufSpan, rw).completed);
      for (int it = 0; it < 2; it++)
        QCOMPARE(bufSpan[it], truth[it]);
      // OS loaded the Mem[0x0001-0x0002].
      QCOMPARE(rreg(isa::Pep10::Register::OS), opspec);
      QCOMPARE(rreg(isa::Pep10::Register::PC), opspec);
    }
  }
};

#include "call.test.moc"

QTEST_MAIN(ISA3Pep10_CALL)
