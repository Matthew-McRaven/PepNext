#include "./directive_skip.hpp"
#include "../argument/base.hpp"
pat::ast::node::Skip::Skip() : Directive() {}

pat::ast::node::Skip::Skip(QSharedPointer<argument::Base> argument,
                           FileLocation sourceLocation,
                           QWeakPointer<Base> parent)
    : Directive(sourceLocation, parent), _argument(argument) {
  if (!(_argument->isNumeric() && _argument->isFixedSize()))
    throw std::logic_error("Argument must be numeric and fixedSize");
}

pat::ast::node::Skip::Skip(const Skip &other)
    : Directive(other), _config(other._config), _argument(other._argument),
      _fill(other._fill), _emitsBytes(other._emitsBytes) {}

pat::ast::node::Skip::Skip(Skip &&other) noexcept { swap(*this, other); }

pat::ast::node::Skip &pat::ast::node::Skip::operator=(Skip other) {
  swap(*this, other);
  return *this;
}

const pat::ast::node::Skip::Config &pat::ast::node::Skip::config() const {
  return _config;
}

void pat::ast::node::Skip::setConfig(Config config) { _config = config; }

QSharedPointer<const pat::ast::argument::Base>
pat::ast::node::Skip::fill() const {
  return _fill;
}

void pat::ast::node::Skip::setFill(QSharedPointer<argument::Base> fill) {
  if (!(fill->isNumeric() && fill->isFixedSize()))
    throw std::logic_error("Fill must be numeric and fixedSize");
  _fill = fill;
}

QSharedPointer<pat::ast::Value> pat::ast::node::Skip::clone() const {
  return QSharedPointer<Skip>::create(*this);
}

pat::bits::BitOrder pat::ast::node::Skip::endian() const {
  return _config.endian;
}

quint64 pat::ast::node::Skip::size() const {
  quint64 size = 0;
  return _argument->value(reinterpret_cast<quint8 *>(size), sizeof(size));
  return size;
}

bool pat::ast::node::Skip::bits(QByteArray &out, bits::BitSelection src,
                                bits::BitSelection dest) const {
  throw std::logic_error("Unimplemented");
}

bool pat::ast::node::Skip::bytes(QByteArray &out, qsizetype start,
                                 qsizetype length) const {
  throw std::logic_error("Unimplemented");
}

QString pat::ast::node::Skip::string() const {
  throw std::logic_error("Unimplemented");
}

const pat::ast::node::AddressSpan &pat::ast::node::Skip::addressSpan() const {
  throw std::logic_error("Unimplemented");
}

void pat::ast::node::Skip::updateAddressSpan(void *update) const {
  throw std::logic_error("Unimplemented");
}

bool pat::ast::node::Skip::emitsBytes() const { return _emitsBytes; }

void pat::ast::node::Skip::setEmitsBytes(bool emitBytes) {
  _emitsBytes = emitBytes;
}
