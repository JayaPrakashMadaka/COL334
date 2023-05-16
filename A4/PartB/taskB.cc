#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RipSimpleRouting");

void TearDownLink (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
  nodeA->GetObject<Ipv4> ()->SetDown (interfaceA);
  nodeB->GetObject<Ipv4> ()->SetDown (interfaceB);
}

void SetUpLink (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
  nodeA->GetObject<Ipv4> ()->SetUp (interfaceA);
  nodeB->GetObject<Ipv4> ()->SetUp (interfaceB);
}


int main (int argc, char **argv)
{
  
  bool printRoutingTables = true;

  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  Config::SetDefault ("ns3::Rip::SplitHorizon", EnumValue (RipNg::SPLIT_HORIZON));

  NS_LOG_INFO ("Create nodes.");
  Ptr<Node> src = CreateObject<Node> ();
  Names::Add ("SrcNode", src);
  Ptr<Node> dst = CreateObject<Node> ();
  Names::Add ("DstNode", dst);
  Ptr<Node> a = CreateObject<Node> ();
  Names::Add ("RouterA", a);
  Ptr<Node> b = CreateObject<Node> ();
  Names::Add ("RouterB", b);
  Ptr<Node> c = CreateObject<Node> ();
  Names::Add ("RouterC", c);
  NodeContainer net1 (src, a);
  NodeContainer net2 (a, b);
  NodeContainer net3 (a, c);
  NodeContainer net4 (b, c);
  NodeContainer net5 (c, dst);
  NodeContainer routers (a, b, c);
  NodeContainer nodes (src, dst);


  NS_LOG_INFO ("Create channels.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (100)));
  NetDeviceContainer ndc1 = csma.Install (net1);
  NetDeviceContainer ndc2 = csma.Install (net2);
  NetDeviceContainer ndc3 = csma.Install (net3);
  NetDeviceContainer ndc4 = csma.Install (net4);
  NetDeviceContainer ndc5 = csma.Install (net5);

  NS_LOG_INFO ("Create IPv4 and routing");
  RipHelper ripRouting;
  
  ripRouting.ExcludeInterface (a, 1);
  ripRouting.ExcludeInterface (c, 3);

  Ipv4ListRoutingHelper listRH;
  listRH.Add (ripRouting, 0);

  InternetStackHelper internet;
  internet.SetIpv6StackInstall (false);
  internet.SetRoutingHelper (listRH);
  internet.Install (routers);

  InternetStackHelper internetNodes;
  internetNodes.SetIpv6StackInstall (false);
  internetNodes.Install (nodes);

  NS_LOG_INFO ("Assign IPv4 Addresses.");
  Ipv4AddressHelper ipv4;

  ipv4.SetBase (Ipv4Address ("10.0.0.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic1 = ipv4.Assign (ndc1);

  ipv4.SetBase (Ipv4Address ("10.0.1.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic2 = ipv4.Assign (ndc2);

  ipv4.SetBase (Ipv4Address ("10.0.2.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic3 = ipv4.Assign (ndc3);

  ipv4.SetBase (Ipv4Address ("10.0.3.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic4 = ipv4.Assign (ndc4);

  ipv4.SetBase (Ipv4Address ("10.0.4.0"), Ipv4Mask ("255.255.255.0"));
  Ipv4InterfaceContainer iic5 = ipv4.Assign (ndc5);

  Ptr<Ipv4StaticRouting> staticRouting;
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (src->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute ("10.0.0.2", 1 );
  staticRouting = Ipv4RoutingHelper::GetRouting <Ipv4StaticRouting> (dst->GetObject<Ipv4> ()->GetRoutingProtocol ());
  staticRouting->SetDefaultRoute ("10.0.4.1", 1 );

  if (printRoutingTables)
    {
      RipHelper routingHelper;

      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);

      routingHelper.PrintRoutingTableAt (Seconds (0.0), a, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (0.0), b, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (0.0), c, routingStream);
      
      routingHelper.PrintRoutingTableAt (Seconds (10.0), a, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (10.0), b, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (10.0), c, routingStream);
      
      routingHelper.PrintRoutingTableAt (Seconds (10.0), a, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (10.0), b, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (10.0), c, routingStream);
      
      routingHelper.PrintRoutingTableAt (Seconds (50.0), a, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (50.0), b, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (50.0), c, routingStream);
      
      routingHelper.PrintRoutingTableAt (Seconds (500.0), a, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (500.0), b, routingStream);
      routingHelper.PrintRoutingTableAt (Seconds (500.0), c, routingStream);
      

    }

  NS_LOG_INFO ("Create Applications.");
  uint32_t packetSize = 1024;
  Time interPacketInterval = Seconds (1.0);
  V4PingHelper ping ("10.0.4.1");

  ping.SetAttribute ("Interval", TimeValue (interPacketInterval));
  ping.SetAttribute ("Size", UintegerValue (packetSize));
  
  ApplicationContainer apps = ping.Install (src);
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (510.0));

  //AsciiTraceHelper ascii;
  //csma.EnableAsciiAll (ascii.CreateFileStream ("rip-simple-routing.tr"));
  //csma.EnablePcapAll ("rip-simple-routing", true);

  Simulator::Schedule (Seconds (50), &TearDownLink, a, b, 2, 1);
  Simulator::Schedule (Seconds (120), &TearDownLink, a, c, 3, 1);
  Simulator::Schedule (Seconds (120), &SetUpLink, a, b, 2, 1);
  Simulator::Schedule (Seconds (120), &SetUpLink, a, c, 3, 1);
  
	
  /* Now, do the actual simulation. */
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (520.0));
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}

