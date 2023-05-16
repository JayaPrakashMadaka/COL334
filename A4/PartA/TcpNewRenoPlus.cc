#include "TcpNewRenoPlus.h"
#include "tcp-socket-base.h"
#include "ns3/log.h"
#include "bits/stdc++.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("TcpNewRenoPlus");

NS_OBJECT_ENSURE_REGISTERED (TcpNewRenoPlus);

TypeId
TcpNewRenoPlus::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpNewRenoPlus")
    .SetParent<TcpNewReno> ()
    .SetGroupName ("Internet")
    .AddConstructor<TcpNewRenoPlus> ()
  ;
  return tid;
}

TcpNewRenoPlus::TcpNewRenoPlus (void) : TcpNewReno()
{
}

TcpNewRenoPlus::TcpNewRenoPlus (const TcpNewRenoPlus& sock)
  : TcpNewReno (sock)
{
}

TcpNewRenoPlus::~TcpNewRenoPlus (void)
{
}

uint32_t
TcpNewRenoPlus::SlowStart (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (segmentsAcked >= 1)
    {
      tcb->m_cWnd = tcb->m_cWnd + static_cast<uint32_t>((std::pow(tcb->m_segmentSize,1.91))/(tcb->m_cWnd));
      NS_LOG_INFO ("In SlowStart, updated to cwnd " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
      return segmentsAcked - 1;
    }

  return 0;
}

void
TcpNewRenoPlus::CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (segmentsAcked > 0)
    {
    
      tcb->m_cWnd += 0.51*tcb->m_segmentSize;
      NS_LOG_INFO ("In CongAvoid, updated to cwnd " << tcb->m_cWnd <<
                   " ssthresh " << tcb->m_ssThresh);
    }
}

void
TcpNewRenoPlus::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (tcb->m_cWnd < tcb->m_ssThresh)
    {
      segmentsAcked = SlowStart (tcb, segmentsAcked);
    }

  if (tcb->m_cWnd >= tcb->m_ssThresh)
    {
      CongestionAvoidance (tcb, segmentsAcked);
    }

}

std::string
TcpNewRenoPlus::GetName () const
{
  return "TcpNewRenoPlus";
}

uint32_t
TcpNewRenoPlus::GetSsThresh (Ptr<const TcpSocketState> state,
                         uint32_t bytesInFlight)
{
  NS_LOG_FUNCTION (this << state << bytesInFlight);

  return std::max (2 * state->m_segmentSize, bytesInFlight / 2);
}

}

