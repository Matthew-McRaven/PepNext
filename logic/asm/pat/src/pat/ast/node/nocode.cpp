#include "./nocode.hpp"

pat::ast::node::Blank::Blank() : Base() {}

pat::ast::node::Blank::Blank(FileLocation sourceLocation,
                             QWeakPointer<Base> parent)
    : node::Base(sourceLocation, parent) {}

pat::ast::node::Blank::Blank(const Blank &other) : Base(other) {}

pat::ast::node::Blank::Blank(Blank &&other) noexcept { swap(*this, other); }

pat::ast::node::Blank &pat::ast::node::Blank::operator=(Blank other) {
  swap(*this, other);
  return *this;
}

QSharedPointer<pat::ast::Value> pat::ast::node::Blank::clone() const {
  return QSharedPointer<node::Blank>::create(*this);
}

pat::bits::BitOrder pat::ast::node::Blank::endian() const {
  return pat::bits::BitOrder::NotApplicable;
}

quint64 pat::ast::node::Blank::size() const { return 0; }

bool pat::ast::node::Blank::bits(QByteArray &out, bits::BitSelection src,
                                 bits::BitSelection dest) const {
  return true;
}

bool pat::ast::node::Blank::bytes(QByteArray &out, qsizetype start,
                                  qsizetype length) const {
  return true;
}

QString pat::ast::node::Blank::string() const { return u"\n"_qs; }

const pat::ast::node::AddressSpan &pat::ast::node::Blank::addressSpan() const {
  throw std::logic_error("Unimplemented");
}

void pat::ast::node::Blank::updateAddressSpan(void *update) const {
  throw std::logic_error("Unimplemented");
}

bool pat::ast::node::Blank::emitsBytes() const { return false; }

void pat::ast::node::Blank::setEmitsBytes(bool emitBytes) {}

pat::ast::node::Comment::Comment() : Base() {}

pat::ast::node::Comment::Comment(QString comment, FileLocation sourceLocation,
                                 QWeakPointer<Base> parent)
    : node::Base(sourceLocation, parent), _comment(comment) {}

pat::ast::node::Comment::Comment(const Comment &other)
    : Base(other), _config(other._config), _indent(other._indent),
      _comment(other._comment) {}

pat::ast::node::Comment::Comment(Comment &&other) noexcept {
  swap(*this, other);
}

pat::ast::node::Comment &pat::ast::node::Comment::operator=(Comment other) {
  swap(*this, other);
  return *this;
}

const pat::ast::node::Comment::Config &pat::ast::node::Comment::config() const {
  return _config;
}

void pat::ast::node::Comment::setConfig(Config config) { _config = config; }

pat::ast::node::Comment::IndentLevel pat::ast::node::Comment::indent() const {
  return _indent;
}

void pat::ast::node::Comment::setIndent(IndentLevel indent) {
  _indent = indent;
}

QSharedPointer<pat::ast::Value> pat::ast::node::Comment::clone() const {
  auto ret = QSharedPointer<node::Comment>::create(_comment, sourceLocation(),
                                                   QWeakPointer<Comment>{});
  ret->setConfig(_config);
  ret->setIndent(_indent);
  return ret;
}

pat::bits::BitOrder pat::ast::node::Comment::endian() const {
  return bits::BitOrder::NotApplicable;
}

quint64 pat::ast::node::Comment::size() const { return 0; }

bool pat::ast::node::Comment::bits(QByteArray &out, bits::BitSelection src,
                                   bits::BitSelection dest) const {
  return true;
}

bool pat::ast::node::Comment::bytes(QByteArray &out, qsizetype start,
                                    qsizetype length) const {
  return true;
}

QString pat::ast::node::Comment::string() const {
  throw std::logic_error("Unimplemented");
}

const pat::ast::node::AddressSpan &
pat::ast::node::Comment::addressSpan() const {
  throw std::logic_error("Unimplemented");
}

void pat::ast::node::Comment::updateAddressSpan(void *update) const {
  throw std::logic_error("Unimplemented");
}

bool pat::ast::node::Comment::emitsBytes() const { return false; }

void pat::ast::node::Comment::setEmitsBytes(bool emitBytes) {}
