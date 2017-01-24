#include "ns3/applications-module.h"

using namespace ns3;

class VoipHelper : public OnOffHelper
{
public:
  VoipHelper (Ipv4Address address, uint16_t port, double maxHablando, 
	      double minHablando, double maxSilencio, double minSilencio, InetSocketAddress socket);
  ~VoipHelper ();

private:
  Ptr<UniformRandomVariable> varOn;
  Ptr<UniformRandomVariable> varOff;

};

