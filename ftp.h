#include "ns3/applications-module.h"

using namespace ns3;

class FTPHelper : public BulkSendHelper
{
public:
  FTPHelper (Ipv4Address address, uint16_t port,InetSocketAddress socket);
  ~FTPHelper ();

private:
  uint32_t maxTamPqt;

};

