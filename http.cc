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
#include "http.h"
using namespace ns3;

HTTPHelper::HTTPHelper(Ipv4Address address, uint16_t port, double maxOn, double minOn, double maxOff, double minOFF, InetSocketAddress socket)
  :OnOffHelper("ns3::TcpSocketFactory", Address (socket))
{ 
  //1) Fijar On/Off Time
  varOn=CreateObject<UniformRandomVariable>();
  varOn->SetAttribute("Max", DoubleValue(maxOn));
  varOn->SetAttribute("Min", DoubleValue(minOn));
  varOff=CreateObject<UniformRandomVariable>();
  varOff->SetAttribute("Max", DoubleValue(maxOff));
  varOff->SetAttribute("Min", DoubleValue(minOFF));
  //2) Configurar DataRate (en principio constante)
  SetConstantRate (DataRate ("100Mbps")); //si se queda constante definirla arriba en mayus
  SetAttribute("OnTime", PointerValue(varOn));
  SetAttribute("OffTime", PointerValue(varOff));
}
HTTPHelper::~HTTPHelper()
{

}
