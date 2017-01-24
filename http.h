#include "ns3/applications-module.h"

using namespace ns3;

class HTTPHelper : public OnOffHelper
{
public:
	
  HTTPHelper (Ipv4Address address, uint16_t port, double maxOn, double minOn, double maxOff, double minOFF,InetSocketAddress socket);
  ~HTTPHelper ();
private:
  Ptr<UniformRandomVariable> varOn;
  Ptr<UniformRandomVariable> varOff;
};


