#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include <ns3/packet.h>
#include <ns3/data-rate.h>

class Escenario
{
public:
	~Escenario();

	void AddContainer (std::string nombre, int numNode);
	ns3::NodeContainer* GetNodeContainer (std::string nombre);
	void AddNodeToContainer (std::string origen, int numNode, std::string destino);
	ns3::Ptr<ns3::Node> GetNode (std::string from, int numNode);
	void BuildInternetStack ();

	void AddCsmaNetwork (std::string to, std::string tasa, double retardo);
	void AddPPPNetwork (std::string to, std::string tasa, double retardo);
	ns3::NetDeviceContainer* GetNetDeviceContainer (std::string nombre);
	ns3::Ptr<ns3::NetDevice> GetNetDevice (std::string from, int numNode);

	void SetIpToNetwork (std::string to, std::string base, std::string mask);
	ns3::Ipv4InterfaceContainer* GetInterfaceContainer (std::string nombre);
	ns3::Ipv4Address GetIPv4Address (std::string from, int num);
	
	void EnablePCAPLogging (std::string to);

private:
	void Insert (std::string to, const ns3::NetDeviceContainer &nd);
	void Insert (std::string to, const ns3::Ipv4InterfaceContainer &nd);

  ns3::NodeContainer m_nodes;
  std::map<std::string, ns3::NodeContainer*> subnets;
  std::map<std::string, ns3::NetDeviceContainer*> netdevices;
  std::map<std::string, ns3::Ipv4InterfaceContainer*> ips;
  std::map<std::string, ns3::PcapHelperForDevice*> pcaps_helpers;
};
