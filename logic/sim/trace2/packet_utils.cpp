#include "packet_utils.hpp"

std::size_t sim::trace2::payload_length(const sim::api2::packet::Payload &payload) {
  return std::visit(sim::trace2::detail::PayloadLength{}, payload);
}
std::size_t sim::trace2::packet_payloads_length(sim::api2::trace::PacketIterator iter, bool includeRead) {
  return std::visit(sim::trace2::detail::PacketPayloadsLength{iter, includeRead}, *iter);
}
std::optional<sim::api2::packet::VariableBytes<8>>
sim::trace2::get_address_bytes(const sim::api2::packet::Header &header) {
  return std::visit(detail::GetAddressBytes{}, header);
}

sim::trace2::detail::PacketPayloadsLength::PacketPayloadsLength(sim::api2::trace::PacketIterator iter, bool includeRead)
    : _iter(iter), _includeRead(includeRead) {}

std::size_t
sim::trace2::detail::PacketPayloadsLength::operator()(const sim::api2::packet::header::Clear &header) const {
  return 0;
}

std::size_t
sim::trace2::detail::PacketPayloadsLength::operator()(const sim::api2::packet::header::PureRead &header) const {
  if (_includeRead) return header.payload_len;
  else return 0;
}

std::optional<sim::api2::packet::path_t> sim::trace2::get_path(const sim::api2::packet::Header &header) {
  return std::visit(detail::GetHeaderPath{}, header);
}

std::optional<sim::api2::device::ID> sim::trace2::get_id(const sim::api2::packet::Header &header) {

  return std::visit(detail::GetHeaderID{}, header);
}

bool sim::trace2::is_packet_header(const sim::api2::trace::Fragment &f) { return std::visit(IsPacketHeader{}, f); }

sim::api2::packet::Header sim::trace2::as_packet_header(const sim::api2::trace::Fragment &f) {
  return std::visit(AsPacketHeader{}, f);
}

bool sim::trace2::is_packet_payload(const sim::api2::trace::Fragment &f) { return std::visit(IsPacketPayload{}, f); }

sim::api2::packet::Payload sim::trace2::as_packet_payload(const sim::api2::trace::Fragment &f) {
  return std::visit(AsPacketPayload{}, f);
}
