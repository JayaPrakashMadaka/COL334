/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "bits/stdc++.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Task3");

uint32_t m_maxwindow=0;

class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId (void);
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

/* static */
TypeId MyApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyApp")
    .SetParent<Application> ()
    .SetGroupName ("Tutorial")
    .AddConstructor<MyApp> ()
    ;
  return tid;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  //NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
  //if(m_maxwindow<newCwnd){
  //      m_maxwindow=std::max(m_maxwindow,newCwnd);
  //      NS_LOG_UNCOND(Simulator::Now ().GetSeconds () << "\t"<< m_maxwindow);
  //}
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}




int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  NodeContainer nodes;
  nodes.Create (5);
  
  NodeContainer n0n1 = NodeContainer(nodes.Get(0),nodes.Get(1));
  NodeContainer n1n2 = NodeContainer(nodes.Get(1),nodes.Get(2));
  NodeContainer n2n4 = NodeContainer(nodes.Get(2),nodes.Get(4));
  NodeContainer n3n2 = NodeContainer(nodes.Get(3),nodes.Get(2));

  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVegas")); 
   
  InternetStackHelper stack;
  stack.Install (nodes);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("500Kbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  
  NetDeviceContainer d0d1;
  NetDeviceContainer d1d2;
  NetDeviceContainer d2d4;
  NetDeviceContainer d3d2;
  
  d0d1 = pointToPoint.Install (n0n1);
  d1d2 = pointToPoint.Install (n1n2);
  d2d4 = pointToPoint.Install (n2n4);
  d3d2 = pointToPoint.Install (n3n2);
  

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer i0i1 = address.Assign (d0d1);
  address.SetBase ("10.1.2.0", "255.255.255.252");
  Ipv4InterfaceContainer i1i2 = address.Assign (d1d2);
  address.SetBase ("10.1.3.0", "255.255.255.252");
  Ipv4InterfaceContainer i3i2 = address.Assign (d3d2);
  address.SetBase ("10.1.4.0", "255.255.255.252");
  Ipv4InterfaceContainer i2i4 = address.Assign (d2d4);
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  //Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  //em->SetAttribute ("ErrorRate", DoubleValue (0.00001));
  //d3d2.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
  
  
  //TCP Sink implemented here

  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (i3i2.GetAddress (0), sinkPort));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (3));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (100.));

  //UDP Sink implemented here
  uint16_t sinkPort1 = 9;
  Address sinkAddress1 (InetSocketAddress (i2i4.GetAddress (1), sinkPort1));
  PacketSinkHelper packetSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort1));
  ApplicationContainer sinkApps1 = packetSinkHelper1.Install (nodes.Get(4));
  sinkApps1.Start (Seconds (0.));
  sinkApps1.Stop (Seconds (100.)); 
  

  //TCP source implemented here

  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());

  Ptr<MyApp> app = CreateObject<MyApp> ();
  app->Setup (ns3TcpSocket, sinkAddress, 1040, 100000, DataRate ("250Kbps"));
  nodes.Get (0)->AddApplication (app);
  app->SetStartTime (Seconds (1.));
  app->SetStopTime (Seconds (100.));
  
  //UDP source implemented here
  
  Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (nodes.Get (1), UdpSocketFactory::GetTypeId ());

  Ptr<MyApp> app1 = CreateObject<MyApp> ();
  app1->Setup (ns3UdpSocket, sinkAddress1, 1040, 10000, DataRate ("250Kbps"));
  nodes.Get (1)->AddApplication (app1);
  app1->SetStartTime (Seconds (20.));
  app1->SetStopTime (Seconds (30.));
  
  Ptr<Socket> ns3UdpSocket1 = Socket::CreateSocket (nodes.Get (1), UdpSocketFactory::GetTypeId ());

  Ptr<MyApp> app2 = CreateObject<MyApp> ();
  app2->Setup (ns3UdpSocket1, sinkAddress1, 1040, 100000, DataRate ("500Kbps"));
  nodes.Get (1)->AddApplication (app2);
  app2->SetStartTime (Seconds (30.));
  app2->SetStopTime (Seconds (100.));  
  
 

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("task3.cwnd");
  ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));

  pointToPoint.EnablePcapAll ("second");

  Simulator::Stop (Seconds (100));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

