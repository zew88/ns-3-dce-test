#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-helper.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Test1");

int 
main (int argc, char *argv[])
{
  std::string bufSize = "8388608";
  CommandLine cmd;
  cmd.AddValue ("buffer", "Buffer Size.", bufSize);
  cmd.Parse (argc, argv);

// topologies
  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (3);
  NodeContainer n0n1 = NodeContainer (c.Get (0), c.Get (1));
  NodeContainer n1n2 = NodeContainer (c.Get (1), c.Get (2));

  DceManagerHelper dceManager;

#ifdef KERNEL_STACK   
      dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));
      LinuxStackHelper stack;
      stack.Install (c);
      dceManager.Install (c);
      stack.SysctlSet (c, ".net.ipv4.tcp_rmem", "4096 "+bufSize+" 8388608");
      stack.SysctlSet (c, ".net.ipv4.tcp_wmem", "4096 "+bufSize+" 8388608");
      stack.SysctlSet (c, ".net.core.rmem_max", bufSize);
      stack.SysctlSet (c, ".net.core.wmem_max", bufSize);
      
#else
      NS_LOG_ERROR ("Linux kernel stack for DCE is not available. build with dce-linux module.");
      // silently exit
      return 0;
#endif
   
    
// channel
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("0ms"));
  NetDeviceContainer d0d1 = p2p.Install (n0n1);

  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer d1d2 = p2p.Install (n1n2);

// IP Address
NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i1 = ipv4.Assign (d0d1);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = ipv4.Assign (d1d2);

// Create router nodes, initialize routing database and set up the routing
// tables in the nodes.
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
#ifdef KERNEL_STACK
  LinuxStackHelper::PopulateRoutingTables ();
#endif


// Application
  NS_LOG_INFO ("Create Applications.");
  DceApplicationHelper dce;
  
  dce.SetStackSize (1 << 20);
    
  // Launch iperf client on node 0
  dce.SetBinary ("iperf");
  dce.ResetArguments ();
  dce.ResetEnvironment ();
  dce.AddArgument ("-c");
  dce.AddArgument ("10.1.2.2");
  dce.AddArgument ("-i");
  dce.AddArgument ("1");
  dce.AddArgument ("--time");
  dce.AddArgument ("10");
  ApplicationContainer ClientApps = dce.Install (c.Get (0));
  ClientApps.Start (Seconds (1));
  ClientApps.Stop (Seconds (10));
  
 // Launch iperf server on node 2
  dce.SetBinary ("iperf");
  dce.ResetArguments ();
  dce.ResetEnvironment ();
  dce.AddArgument ("-s");
  dce.AddArgument ("-P");
  dce.AddArgument ("1");
  ApplicationContainer SerApps = dce.Install (c.Get (2));
  SerApps.Start (Seconds (1));
  SerApps.Stop (Seconds (10));
  AsciiTraceHelper ascii;
  p2p.EnableAsciiAll (ascii.CreateFileStream ("Test1.tr"));
  p2p.EnablePcapAll ("Test1");

  NS_LOG_INFO ("Run Simulation.");

  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
} 







