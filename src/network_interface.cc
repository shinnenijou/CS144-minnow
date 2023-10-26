#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;
  
// TIMEOUT CONFIG
uint32_t NetworkInterface::ADDRESS_CACHE_TIMEOUT_MS = 30000;
uint32_t NetworkInterface::ARP_REQUEST_TIMEOUT_MS = 5000;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  (void)dgram;
  (void)next_hop;
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  (void)frame;
  return {};
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  timer += ms_since_last_tick;
  
  // expire address map cache
  while (timer >= address_expire_timers.begin()->first)
  {
    auto iter = address_cache.find(address_expire_timers.begin()->second);

    if (iter != address_cache.end())
      address_cache.erase(iter);

    address_expire_timers.erase(address_expire_timers.begin());
  }

  // expire ARP requests
  for (auto iter = arp_request_expire_timers.begin(); iter != arp_request_expire_timers.end();)
  {
    if (timer >= iter->second)
      iter = arp_request_expire_timers.erase(iter);
    else
      ++iter;
  }
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if (send_queue.empty())
    return {};

  EthernetFrame frame(std::move(send_queue.front()));
  send_queue.pop();

  return frame;
}
