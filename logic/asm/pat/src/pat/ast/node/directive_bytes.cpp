#include "./directive_bytes.hpp"

pat::ast::node::Byte1::Byte1() : Directive() {}

pat::ast::node::Byte1::Byte1(
    const QList<QSharedPointer<const argument::Base>> argument,
    FileLocation sourceLocation, QWeakPointer<node::Base> parent)
    : Directive(sourceLocation, parent), _argument(argument) {}

pat::ast::node::Byte1::Byte1(const Byte1 &other)
    : Directive(other), _config(other._config), _argument(other._argument),
      _emitsBytes(other._emitsBytes) {}

pat::ast::node::Byte1::Byte1(Byte1 &&other) noexcept : Directive() {
  swap(*this, other);
}

pat::ast::node::Byte1 &pat::ast::node::Byte1::operator=(Byte1 other) {
  swap(*this, other);
  return *this;
}

const pat::ast::node::Byte1::Config &pat::ast::node::Byte1::config() const {
  return _config;
}

void pat::ast::node::Byte1::setConfig(Config config) { _config = config; }

QSharedPointer<pat::ast::Value> pat::ast::node::Byte1::clone() const {
  return QSharedPointer<Byte1>::create(*this);
}

pat::bits::BitOrder pat::ast::node::Byte1::endian() const {
  return _config.endian;
}

quint64 pat::ast::node::Byte1::size() const { return _argument.size(); }

bool pat::ast::node::Byte1::bits(QByteArray &out, bits::BitSelection src,
                                 bits::BitSelection dest) const {
  throw std::logic_error("Unimplemented");
}

bool pat::ast::node::Byte1::bytes(QByteArray &out, qsizetype start,
                                  qsizetype length) const {
  throw std::logic_error("Unimplemented");
}

QString pat::ast::node::Byte1::string() const {
  throw std::logic_error("Unimplemented");
}

const pat::ast::node::AddressSpan &pat::ast::node::Byte1::addressSpan() const {
  throw std::logic_error("Unimplemented");
}

void pat::ast::node::Byte1::updateAddressSpan(void *update) const {
  throw std::logic_error("Unimplemented");
}

bool pat::ast::node::Byte1::emitsBytes() const { return _emitsBytes; }

void pat::ast::node::Byte1::setEmitsBytes(bool emitBytes) {
  _emitsBytes = emitBytes;
}

pat::ast::node::Byte2::Byte2() : Directive() {}

pat::ast::node::Byte2::Byte2(
    QList<QSharedPointer<const argument::Base>> argument,
    FileLocation sourceLocation, QWeakPointer<Base> parent)
    : Directive(sourceLocation, parent), _argument(argument) {}

pat::ast::node::Byte2::Byte2(const Byte2 &other)
    : Directive(other), _config(other._config), _argument(other._argument),
      _emitsBytes(other._emitsBytes) {}

pat::ast::node::Byte2::Byte2(Byte2 &&other) noexcept { swap(*this, other); }

pat::ast::node::Byte2 &pat::ast::node::Byte2::operator=(Byte2 other) {
  swap(*this, other);
  return *this;
}

const pat::ast::node::Byte2::Config &pat::ast::node::Byte2::config() const {
  return _config;
}

void pat::ast::node::Byte2::setConfig(Config config) { _config = config; }

QSharedPointer<pat::ast::Value> pat::ast::node::Byte2::clone() const {
  return QSharedPointer<Byte2>::create(*this);
}

pat::bits::BitOrder pat::ast::node::Byte2::endian() const {
  return _config.endian;
}

quint64 pat::ast::node::Byte2::size() const { return 2 * _argument.size(); }

bool pat::ast::node::Byte2::bits(QByteArray &out, bits::BitSelection src,
                                 bits::BitSelection dest) const {
  throw std::logic_error("Unimplemented");
}

bool pat::ast::node::Byte2::bytes(QByteArray &out, qsizetype start,
                                  qsizetype length) const {
  throw std::logic_error("Unimplemented");
}

QString pat::ast::node::Byte2::string() const {
  throw std::logic_error("Unimplemented");
}

const pat::ast::node::AddressSpan &pat::ast::node::Byte2::addressSpan() const {
  throw std::logic_error("Unimplemented");
}

void pat::ast::node::Byte2::updateAddressSpan(void *update) const {
  throw std::logic_error("Unimplemented");
}

bool pat::ast::node::Byte2::emitsBytes() const { return _emitsBytes; }

void pat::ast::node::Byte2::setEmitsBytes(bool emitBytes) {
  _emitsBytes = emitBytes;
}
