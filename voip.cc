#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <cassert>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/callback.h"
#include "ns3/internet-module.h"
#include "voip.h"
using namespace ns3;

VoipHelper::VoipHelper(Ipv4Address address, uint16_t port, double maxHablando, 
		       double minHablando, double maxSilencio, double minSilencio, InetSocketAddress socket)
  :OnOffHelper("ns3::UdpSocketFactory", Address (socket))
{
  //1) Fijar On/Off Time
  varOn=CreateObject<UniformRandomVariable>();
  varOn->SetAttribute("Max", DoubleValue(maxHablando));
  varOn->SetAttribute("Min", DoubleValue(minHablando));
  varOff=CreateObject<UniformRandomVariable>();
  varOff->SetAttribute("Max", DoubleValue(maxSilencio));
  varOff->SetAttribute("Min", DoubleValue(minSilencio));
  //2) Configurar DataRate (en principio constante)
  SetConstantRate (DataRate ("1Mbps")); // "500kbps"
  SetAttribute("OnTime", PointerValue(varOn));
  SetAttribute("OffTime", PointerValue(varOff));
  

}
VoipHelper::~VoipHelper()
{

}
