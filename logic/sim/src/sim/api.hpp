#pragma once
#include "bits/span.hpp"
#include <QtCore>
#include <type_traits>
namespace sim::api {

namespace device {
using ID = quint16; // Only use 9 bits (max of 512)!
struct Descriptor {
  device::ID id;
  void *compatible;
  QString baseName, fullName;
};
using IDGenerator = std::function<ID()>;
} // namespace device

namespace packet {
// clang-format off
/*
 * Flags allow a device to cast a memory location to the correct kind of Packet
 * by encoding type information.
 * kind:
 *   When scope==0, all packets with the same kind bits must have identical
 *   storage layouts. When scope==1, only packets belonging to the same device
 *   must have the same storage layout.
 * scope:
 *   0: The meaning of the flags do not depend on the ID field of the Packet
 *   1: One must delegate to the device that created the packet to
 *      return type information
 * dyn:
 *   0: The containing packet's payload contains no fields who point to
 *      heap-allocated object. No destructor is necessary
 *   1: The containg packet's payload contains at least one field who points to
 *      a heap-allocated  object. If scope==0, then the Buffer is responsible
 *      for finding and calling a suitable destructor for the packet. If
 *      scope==1, the Buffer may delegate to the device which created the
 *      packet.
 * u16:
 *   0: treat the flags as a 1-bytes value.
 *   1: treat the flags as a 2-byte value.
 * Special values for bits :
 *   0b0000'0000 indicates and empty Packet that is size 1.
 *   0bxxxx'xxxx'xxxx'xxx1 indicates the flags should be treated as a u16.
 *
 * Bits are orgnized so that all common addresses traces can fit in 0bxxx'xxx'0'0,
 * where at least one x is 1. This does mean that only 2-byte flags can hold
 * dynamic data.
 */
// clang-format on
struct Flags {
  Flags() = default;
  quint16 dyn : 1 =
      0; // 1=some data allocated with malloc/new. Must be destroyed.
  quint16 kind : 13 = 0;
  quint16 scope : 1 = 0; // 0=global, 1=specific to device;
  quint16 u16 : 1 = 0;   // Must always be 1 if flags is non-zero. Indicate that
                         // flags take 2 bytes, not 1.
  inline operator quint16() const {
    union Type {
      Type() : flags() {}
      Flags flags;
      quint16 bits;
    } type;
    type.flags = *this;
    return type.bits;
  }
};
static_assert(sizeof(Flags) == sizeof(quint16));

#pragma pack(push, 1)
struct EmptyPacket {
  union {
    quint8 size = 0;
    quint8 type;
  } const field;
};

/*
 * Assume there is a Packet<T>* ptr;
 * if *(quint8*)ptr ==0, then Packet<T> is really an EmptyPacket.
 *
 * That is, in the empty case, no fields are present, and the size and type
 * fields collapse to 0. While technically a EmptyPacket is size 1, this rule
 * gives us the nice property that when we start at a 0 in a array of Packets,
 * we can safely skip to the next non-zero byte.
 *
 * However, this means that type != 0 for all real Packets, otherwise the packet
 * will be misinterpreted.
 */

template <typename Payload> struct Packet {
  quint8 length = sizeof(Packet);
  Payload payload = {};
  // Device must be after payload, so it is at a fixed location relative to
  // type.
  //  [typdevice, type] are used to compute length if a pointer to the end of
  //  the packet is acquired.
  device::ID device = 0;
  // Flags are always stored as u16. If u16 bit is not set, then the upper 8
  // bits are unspecified.
  union FlagBits {
    FlagBits() {}
    Flags flags;
    quint16 bits =
        0b0000'0000'0000'0001; // Mark the type as 2 bytes, uninitialized.
  } type;

  Packet() {}
  Packet(device::ID device, Flags flags) : device(device), payload(), type() {
    type.flags = flags;
  }
  // Must be declared inline, otherwise fails to compile.
  template <typename Bytes>
  Packet(device::ID device, Bytes bytes, Flags flags)
      : device(device), payload(), type() {
    type.flags = flags;
    void *dst, *src;
    if constexpr (std::is_pointer_v<std::decay_t<Payload>>)
      dst = this->payload;
    else
      dst = &this->payload;
    if constexpr (std::is_pointer_v<std::decay_t<Bytes>>)
      src = bytes;
    else
      src = &bytes;
    // Should use bits::memcpy, but don't want to make bits library a
    // requirement for API.
    memcpy(dst, src, qMin(sizeof(Bytes), sizeof(Payload)));
  };
};
#pragma pack(pop)

struct Registry {
  virtual ~Registry() = default;
  typedef void (*packet_dtor)(void *);
  virtual void registerDTOR(Flags flags, packet_dtor dtor) = 0;
  virtual packet_dtor getDTOR(Flags flags) = 0; // nullptr indicates no DTOR.
};
} // namespace packet

namespace trace {

// Forward declare, will be needed inside analyzer.
struct Buffer;

struct Analyzer {
  enum class Mode {
    Streaming, // Will run on each commit for a trace where filters match.
    Batch,     // Will run at some (delayed) point after a trace is committed.
  };
  struct FilterArgs {
    Mode mode = Mode::Batch;
    enum Lifetime : quint8 {
      kExpired = 1 << 0,   // Permanent, cannnot be undone.
      kPermanent = 1 << 1, // Permanent, *can* be undone.
      kEphemeral = 1 << 2  // Ephemeral, cannot be undone.
    };
    // Permanent can be backward. Permanent/Ephemeral/Expired can be forward.
    enum Direction : quint8 {
      kForward = 1 << 0,  // Simulator is stepping forward.
      kBackward = 1 << 1, // Simulator is stepping backward.
    };
    quint8 lifetime = kExpired | kPermanent | kEphemeral,
           direction = kForward | kBackward;
    QList<device::ID> trackedDevices = {};
    std::function<bool(packet::Flags)> flags;
  };

  // Only called in Buffer decides evaluate packet, and it matched filters.
  virtual bool analyze(bits::span<const quint8> payload, packet::Flags flags);
  virtual bool unanalyze(bits::span<const quint8> payload, packet::Flags flags);
  // Called on registration with Buffer to determine when to invoke analyzer.
  // At some point, I may allow one analyzer to produce multiple filters.
  virtual FilterArgs filter() const = 0;
  virtual ~Analyzer() = 0;
};

struct Buffer {
  virtual ~Buffer() = default;

  virtual void trace(quint16 deviceID, bool enabled = true) = 0;
  virtual void setPacketRegistry(api::packet::Registry *registry) = 0;

  // If true, the analyzer will be eligible for analyzing packets. If false,
  // the buffer rejected the analyzer. This may occur in the future when an
  // untrusted analyzer attempts to analyze a trusted device.
  virtual bool registerAnalyzer(Analyzer *analyzer) = 0;
  virtual void unregisterAnalyzer(Analyzer *analyzer) = 0;
  // If  ephemeral, then the trace is allowed to expire as soon as commit
  // returns.
  // If !ephemeral, then the trace *should* persist after commit. However, the
  // implementation is not required to offer this property.
  // If you have a bus
  // system A->B->C: C must be alloc'ed+committed before B can alloc, and B
  // before A. No two alloc's may overlap in lifetime. This applies even when
  // swapping between temp/perm traces.
  // See palloc notes on design.
  template <bool ephemeral> struct Guard {
    Guard(Buffer *parent, quint8 length, device::ID id, packet::Flags flags)
        : _parent(parent),
          _data(ephemeral ? parent->ealloc(length, id, flags)
                          : parent->palloc(length, id, flags)) {}
    ~Guard() {
      if (!_data)
        ; // Don't commit an empty ptr.
      else if (ephemeral)
        _parent->ecommit();
      else
        _parent->pcommit();
    }
    operator bool() { return _data != nullptr; }
    void *data() { return _data; }

  private:
    Buffer *_parent;
    void *_data;
  };

protected:
  // To avoid double-buffering, request that the buffer provide a sufficient
  // number of bytes. Callers can then use placement new to construct their
  // Packet in-place. If return is nullptr, do not attempt to perform
  // placement.
  // Throws a bad alloc exception if there is no space in buffer.

  // Allocate and commit permant(ish) traces.
  [[nodiscard]] virtual void *palloc(quint8 length, device::ID id,
                                     packet::Flags flags) = 0;
  virtual void pcommit() = 0;
  // Allocate and commit ephemeral traces.
  [[nodiscard]] virtual void *ealloc(quint8 length, device::ID id,
                                     packet::Flags flags) = 0;
  virtual void ecommit() = 0;
};

struct Producer {
  virtual ~Producer() = default;
  virtual void setTraceBuffer(Buffer *tb) = 0;
  // Have the produce communicate with the Buffer that it would like to traced.
  // In the case of a CPU with register banks, calling trace() should cause the
  // CPU to make its register banks trace()'ed too.
  virtual void trace(bool enabled) = 0;
  virtual quint8 packetSize(packet::Flags flags) const = 0;
  // Give pointer to payload rather than trace.
  // Any changes to bit format of trace now only impacts the Buffer doing the
  // analysis.
  virtual bool
  applyTrace(bits::span<const quint8> payload,
             packet::Flags flags) = 0; // trace is a unknown payload struct.
  virtual bool
  unapplyTrace(bits::span<const quint8> payload,
               packet::Flags flags) = 0; // trace is a unknown payload struct.
};
} // namespace trace

namespace tick {
using Type = quint32; // System will crash at 2^32 ticks.
enum class Error : quint8 {
  Success = 0, // Scheduler should re-schedule this device at the next available
               // clock interval.
  NoMMInput, // Scheduler should suspend execution of all devices until more MM
             // input is provided.
  Terminate, // Scheduler should terminate execution of all devices, as the
             // device has entered an invalid state.
};

struct Result {
  bool pause;      // After this tick, should control be returned to execution
                   // environment? Yes (1) or no (0);
  bool tick_delay; // Should the delay be interpreted in ticks (1) or clock
                   // intervals (0).
  Error error;
  Type delay;
};

// AKA Clock
struct Source {
  virtual ~Source() = default;
  virtual Type interval() = 0;
};

// AKA something clocked
struct Listener {
  virtual ~Listener() = default;
  virtual const Source *getSource() = 0;
  virtual void setSource(Source *) = 0;
  virtual Result tick(tick::Type currentTick) = 0;
};
} // namespace tick

namespace memory {

// If select memory operations fail (e.g., lack of MMI, unmapped address in
// bus), specify the behavior of the target.
enum class FailPolicy {
  YieldDefaultValue, // The target picks some arbitrary default value, and
                     // returns it successfully.
  RaiseError         // The target returns an appropriate error message.
};

struct Operation {
  bool speculative;
  enum class Kind : bool { instruction = false, data = true } kind;
  bool effectful;
};

enum class Error : quint8 {
  Success = 0,
  Unmapped,   // Attempted to read a physical address with no device present.
  OOBAccess,  // Attempted out-of-bound access on a storage device.
  NeedsMMI,   // Attempted to read MMI that had no buffered input.
  Breakpoint, // Memory access triggered a breakpoint.
  Terminate,  // Generic failure condition.
  writeToRO,  // Attempt to write to read-only memory.
};

struct Result {
  bool completed; // Did the operation complete? Yes (1), or No (0).
  bool pause;     // Should a logic FSM be interrupted at the end of the current
                  // tick? yes (1) or no (0).
  Error error;    // Additional error information.
};

template <typename Address> struct Target {
  struct AddressSpan {
    Address minOffset, maxOffset;
  };
  virtual ~Target() = default;
  virtual AddressSpan span() const = 0;
  virtual Result read(Address address, bits::span<quint8> dest,
                      Operation op) const = 0;
  virtual Result write(Address address, bits::span<const quint8> src,
                       Operation op) = 0;
  virtual void clear(quint8 fill) = 0;

  // Return a QList of length maxOffset-minOffset+1, containing all the bytes of
  // the target.
  virtual void dump(bits::span<quint8> dest) const = 0;
};

template <typename Address> struct Initiator {
  virtual ~Initiator() = default;
  virtual void
  setTarget(Target<Address>
                *target) = 0; // Sets all targets. e.g., both I and D paths.
  virtual void
  setTarget(void *port,
            Target<Address> *target) = 0; // Sets one target within the
                                          // initiator. Datatype undetermined.
};
} // namespace memory

struct Scheduler {
  enum class Mode {
    Increment, // Execute only the next tick, even if no clocked device is
               // ticked.
    Jump, // Execute up to and including the next tick with a clocked device.
  };
  virtual ~Scheduler() = default;
  virtual tick::Listener *next(tick::Type current, Mode mode) = 0;
  virtual void schedule(tick::Listener *listener, tick::Type startingOn) = 0;
  virtual void reschedule(device::ID device, tick::Type startingOn) = 0;
};

template <typename Address> struct System {
  virtual ~System() = default;
  // Returns (current tick, result of ticking that clocked device).
  virtual std::pair<tick::Type, tick::Result> tick(Scheduler::Mode mode) = 0;
  virtual tick::Type currentTick() const = 0;
  virtual device::ID nextID() = 0;
  virtual device::IDGenerator nextIDGenerator() = 0;

  virtual void setTraceBuffer(trace::Buffer *buffer) = 0;
};

} // namespace sim::api
