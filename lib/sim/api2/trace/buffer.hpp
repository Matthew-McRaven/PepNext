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
#pragma once
#include <QtCore>
#include <set>
#include <sim/api2/memory/target.hpp>
#include <stack>
#include <zpp_bits.h>
#include "../device.hpp"
#include "../frame.hpp"
#include "../packets.hpp"
#include "./iterator.hpp"

namespace sim::api2::trace {
class PathGuard;
class Sink;
namespace detail {
using namespace sim::api2::frame::header;
using namespace sim::api2::packet::header;
using namespace sim::api2::packet::payload;
using Fragment = std::variant<std::monostate, Trace, Extender, Clear, PureRead, ImpureRead, Write, Increment, Variable>;
} // namespace detail
using Fragment = detail::Fragment;
enum class Action { None, Record, Break, Assert };
struct Filter {
  virtual ~Filter() = default;
  virtual Action operator()(device::ID, quint32) = 0;
};
struct TraceFilter : public Filter {
  inline bool contains(device::ID dev) {
    auto it = std::lower_bound(_targets.cbegin(), _targets.cend(), dev);
    return it != _targets.cend() && *it == dev;
  }
  inline void insert(device::ID dev) {
    auto it = std::lower_bound(_targets.begin(), _targets.end(), dev);
    // Should maintain sorted order on insert.
    if (it == _targets.end() || *it != dev) _targets.insert(it, dev);
  }
  inline void remove(device::ID dev) {
    auto _ = std::remove(_targets.begin(), _targets.end(), dev);
    (void)_;
  }
  inline virtual Action operator()(device::ID dev, quint32) override {
    return (contains(dev)) ? Action::Record : Action::None;
  };

private:
  std::vector<device::ID> _targets;
};
template <typename T> struct ValueFilter : public Filter {
  explicit ValueFilter() = default;
  ValueFilter(const sim::api2::memory::Target<T> *target, quint32 address) : _target(target), _address(address) {}
  const sim::api2::memory::Target<T> *_target = nullptr;
  const quint32 _address = 0;
  static constexpr quint8 _valueLength = 2;
  std::vector<quint16> values = {};
  template <typename U> bool contains(U val) { return contains(quint16(val)); }
  inline bool contains(const quint16 val) {
    auto it = std::lower_bound(values.cbegin(), values.cend(), val);

    return it != values.cend() && *it == val;
  }
  template <typename U> void remove(quint16 val) { (void)std::remove(values.begin(), values.end(), val); }
  template <typename U> void insert(quint16 val) {
    auto it = std::lower_bound(values.begin(), values.end(), val);
    // Should maintain sorted order on insert.
    if (it == values.end() || *it != val) values.insert(it, val);
  }
  virtual Action operator()(device::ID dev, quint32 address) override {
    static constexpr auto gs = memory::Operation{.type = memory::Operation::Type::BufferInternal, .kind = {}};
    if (_address != address || _target->deviceID() != dev) return Action::None;
    auto _value = packet::VariableBytes<2>(_valueLength);
    auto success = _target->read(address, std::span(_value.bytes.data(), _valueLength), gs);
    quint16 val = _value.to_address<quint16>();
    if (contains(val)) return Action::Break;
    return Action::None;
  };
};

struct FilterEvent {
  device::ID deviceID;
  Action action;
  quint32 address;
};
template <typename Address>
  requires(std::is_signed<Address>::is_signed)
std::function<Action(device::ID, Address)> filter;
// If you inherit from this, you will likely want to inherit IteratorImpl as
// well. IteratorImpl allows polymorphic implementation of this class while
// mantaining a stable ABI for the iterator class.
// TODO: Add additional channel for command / simulation packets.
// Simulation packets are notifications such as "there's no MMIO".
// Command packets may set memory values, step forward some number of ticks.
// With these changes, the trace buffer can become single point of
// communication\ between the UI and the simulation.
class Buffer {
public:
  virtual ~Buffer() = default;

  // Must implicitly call updateFrameHeader to fix back links / lengths.
  virtual bool writeFragment(const api2::trace::Fragment &) = 0;
  template <packet::HasPath T> bool writeFragmentWithPath(T &&t) {
    t.path = currentPath();
    return writeFragment(Fragment{t});
  }

  virtual bool updateFrameHeader() = 0;

  // Remove the last frame from the buffer.
  // TODO: replace with integration for iterators / std::erase.
  virtual void dropLast() = 0;
  // Deriving classes MUST also call this implementation of clear() if overriding it.
  virtual void clear() { _paths = {paths_init()}; }

  virtual FrameIterator cbegin() const = 0;
  virtual FrameIterator cend() const = 0;
  virtual FrameIterator crbegin() const = 0;
  virtual FrameIterator crend() const = 0;
  // Paths must be stored on TB and not some other object, since the average target only has access to a TB.
  // Use a PathGuard to manipulate the current path.
  quint16 currentPath() const { return _paths.top(); }

  // Add, remove, or modify filters.
  virtual bool trace(device::ID deviceID, bool enabled = true) = 0;
  virtual quint16 addFilter(std::unique_ptr<sim::api2::trace::Filter>) = 0;
  virtual void removeFilter(quint16 id) = 0;
  virtual void replaceFilter(quint16 id, std::unique_ptr<sim::api2::trace::Filter>) = 0;

  // Process the events produced by the filters.
  virtual std::span<const sim::api2::trace::FilterEvent> events() const = 0;
  virtual void clearEvents() = 0;

  inline void emitFrameStart() { writeFragment({sim::api2::frame::header::Trace{}}); }
  template <typename Address>
  void emitWrite(sim::api2::device::ID id, Address address, bits::span<const quint8> src, bits::span<quint8> dest) {
    using vb = decltype(api2::packet::header::Write::address);
    auto address_bytes = vb::from_address<Address>(address);
    auto header = api2::packet::header::Write{.device = id, .address = address_bytes};
    // Don't write payloads if the buffer rejected the packet header.
    if (applyFilters(id, address, header) >= Action::Record) {
      writeFragmentWithPath(header);
      emit_payloads(src, dest);
    }
  }
  // Generate a Write packet. Bytes will not be XOR encoded.
  // A write to a MM port appends to the state of that port.
  // We do not need to know previous value, since the pub/sub system records it.
  template <typename Address>
  void emitMMWrite(sim::api2::device::ID id, Address address, bits::span<const quint8> src) {
    using vb = decltype(api2::packet::header::Write::address);
    auto header = api2::packet::header::Write{.device = id, .address = vb::from_address(address)};
    // Don't write payloads if the buffer rejected the packet header.
    if (applyFilters(id, address, header) >= Action::Record) {
      writeFragmentWithPath(header);
      emit_payloads(src);
    }
  }
  template <typename Address> void emitPureRead(sim::api2::device::ID id, Address address, Address len) {
    using vb = decltype(api2::packet::header::PureRead::address);
    auto header =
        api2::packet::header::PureRead{.device = id, .payload_len = len, .address = vb::from_address(address)};
    if (applyFilters(id, address, header) >= Action::Record) writeFragmentWithPath(header);
  }
  // Generate a ImpureRead packet. Bytes will not be XOR encoded.
  // We do not need to know previous value, since the pub/sub system records it.
  template <typename Address> void emitMMRead(sim::api2::device::ID id, Address address, bits::span<const quint8> src) {
    using vb = decltype(api2::packet::header::Write::address);
    auto header = api2::packet::header::ImpureRead{.device = id, .address = vb::from_address(address)};
    // Don't write payloads if the buffer rejected the packet header.
    if (applyFilters(id, address, header) >= Action::Record) {
      writeFragmentWithPath(header);
      emit_payloads(src);
    }
  }
  template <typename Address>
  void emitIncrement(sim::api2::device::ID id, Address address, bits::span<const quint8> val) {
    using vb = decltype(api2::packet::header::PureRead::address);
    auto header = api2::packet::header::Increment{.device = id, .address = vb::from_address(address)};
    if (applyFilters(id, address, header) >= Action::Record) {
      writeFragment(header);
      emit_payloads(val);
    }
  }

protected:
  virtual Action applyFilters(sim::api2::device::ID id, quint32 address, const Fragment &frag) = 0;
  // Max payload size is a compile time constant, so compute at compile time.
  using vb = sim::api2::packet::payload::Variable;
  static constexpr auto payload_max_size = vb::N;
  inline void emit_payloads(bits::span<const quint8> buf1, bits::span<const quint8> buf2) {
    auto data_len = std::min(buf1.size(), buf2.size());
    // Split the data into chunks that are `payload_max_size` bytes long.
    for (int it = 0; it < data_len;) {
      auto payload_len = std::min(data_len - it, payload_max_size);
      bool continues = data_len - it > payload_max_size;
      // Additional payloads needed if it is more than N elements away from data_len.
      auto bytes = api2::packet::VariableBytes<payload_max_size>(payload_len, continues);

      // XOR-encode data to reduce storage by 2x.
      bits::memcpy_xor(bits::span<quint8>{bytes.bytes}, buf1.subspan(it, payload_len), buf2.subspan(it, payload_len));

      api2::packet::payload::Variable payload{std::move(bytes)};
      writeFragment({payload});
      it += payload_len;
    }
  }
  inline void emit_payloads(bits::span<const quint8> buf) {
    auto data_len = buf.size();
    // Split the data into chunks that are `payload_max_size` bytes long.
    for (int it = 0; it < data_len;) {
      auto payload_len = std::min(data_len - it, payload_max_size);
      bool continues = data_len - it > payload_max_size;
      // Additional payloads needed if it is more than N elements away from data_len.
      auto bytes = api2::packet::VariableBytes<payload_max_size>(payload_len, continues);

      bits::memcpy(bits::span<quint8>{bytes.bytes}, buf.subspan(it, payload_len));
      api2::packet::payload::Variable payload{std::move(bytes)};
      writeFragment({payload});
      it += payload_len;
    }
  }
  void pushPath(packet::path_t path) { _paths.push(path); }
  void popPath() { _paths.pop(); }
  // stacks don't take initializer lists...
  static std::stack<packet::path_t> paths_init() {
    std::stack<packet::path_t> stack;
    stack.push(0);
    return stack;
  }
  friend class PathGuard;
  std::stack<packet::path_t> _paths = {paths_init()};
};

// Helper to enable RAII for pushing/popping paths on buffer.
class PathGuard {
public:
  PathGuard(Buffer *buffer, packet::path_t path) : _buffer(buffer), _path(path) {
    if (_buffer && _buffer->currentPath() != _path) _buffer->pushPath(_path);
  }
  ~PathGuard() {
    if (_buffer && _buffer->currentPath() == _path) _buffer->popPath();
  }
  PathGuard(const PathGuard &) = delete;
  PathGuard &operator=(const PathGuard &) = delete;
  PathGuard(PathGuard &&) = default;
  PathGuard &operator=(PathGuard &&) = default;

private:
  quint16 _path = 0;
  Buffer *_buffer = 0;
};
} // namespace sim::api2::trace
