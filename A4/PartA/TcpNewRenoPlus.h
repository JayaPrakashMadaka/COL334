#ifndef TCPNEWRENOPLUS_H
#define TCPNEWRENOPLUS_H

#include "ns3/tcp-socket-state.h"
#include "tcp-congestion-ops.h"

namespace ns3 {

class TcpNewRenoPlus : public TcpNewReno
{
public:

  static TypeId GetTypeId (void);

  TcpNewRenoPlus ();
  
  TcpNewRenoPlus (const TcpNewRenoPlus& sock);
     
  ~TcpNewRenoPlus();

  virtual std::string GetName () const;

  virtual void IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb,
                                uint32_t bytesInFlight);


protected:
  virtual uint32_t SlowStart (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  virtual void CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
};

} // namespace ns3

#endif
