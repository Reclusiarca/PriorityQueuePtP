//Aquí hay muuchas funciones. Todas para montar la topología al completo.
// Hay un m_nodes (NodeContainer) que almacena tooodos los nodos
// Hay 3 maps
// subnets : NodeContainer de las distintas subnets {CSMAizq, CSMAder, p2p}
// netdevices : los NetDevices de las distintas subnets
// ips: guarda las IP de los NetDevices
// pcaps_helpers: para el manejo de la salida por wireshark


#include "escenario.h"
#define MaxQUEUINGPackets 10

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Escenario");

Escenario::~Escenario()
{
  
}

//---------------------------------------------------------------------------//

void Escenario::Insert (std::string to, const NetDeviceContainer &nd)
{
  NS_LOG_INFO ("Inserta un NetDeviceContainer");
  NetDeviceContainer *pnd = new NetDeviceContainer();
  pnd->Add(nd);
  netdevices.insert(std::pair<std::string, NetDeviceContainer*>(to, pnd));
}

//---------------------------------------------------------------------------//

void Escenario::Insert (std::string to, const Ipv4InterfaceContainer &nd)
{
  NS_LOG_INFO ("Inserta un Ipv4InterfaceContainer");
  Ipv4InterfaceContainer *pnd = new Ipv4InterfaceContainer();
  pnd->Add(nd);
  ips.insert(std::pair<std::string, Ipv4InterfaceContainer*>(to, pnd));
}

//---------------------------------------------------------------------------//
void Escenario::AddContainer (std::string nombre, int nNodos)
{
  NS_LOG_INFO ("Crea un NodeContainer con nombre y nº de nodos");
	NodeContainer *nuevoContainer = new NodeContainer();
  nuevoContainer->Create(nNodos);
	m_nodes.Add(*nuevoContainer); //se añaden al total de nodes
	subnets.insert(std::pair<std::string, NodeContainer*>(nombre, nuevoContainer));

}

//---------------------------------------------------------------------------//

NodeContainer* Escenario::GetNodeContainer (std::string nombre)
{
  NS_LOG_INFO ("Devuelve el NodeContainer");
	return subnets.at(nombre);
}

//---------------------------------------------------------------------------//

void Escenario::AddNodeToContainer (std::string origen, int numNode, std::string destino)
{
  NS_LOG_INFO ("Copia un Node a otro NodeContainer");
  GetNodeContainer (destino)->Add (GetNodeContainer (origen)->Get (numNode));
}

//---------------------------------------------------------------------------//

Ptr<Node> Escenario::GetNode (std::string from, int numNode)
{
  NS_LOG_INFO ("Obtiene un Node específico de un NodeContainer");
  return GetNodeContainer (from)->Get (numNode);
}

//---------------------------------------------------------------------------//

void Escenario::BuildInternetStack ()
{
  NS_LOG_INFO ("Agrega la pila");
	InternetStackHelper().Install(m_nodes);
}

//---------------------------------------------------------------------------//

void Escenario::AddCsmaNetwork(std::string to, std::string tasa, double retardo)
{
  NS_LOG_INFO ("Crea una red CSMA con todos los elementos de un NodeContainer");
	CsmaHelper *csma = new CsmaHelper();
	csma->SetChannelAttribute ("DataRate", DataRateValue (DataRate (tasa)));
  csma->SetChannelAttribute ("Delay", TimeValue (Time(retardo)));
  Insert(to, csma->Install (*GetNodeContainer(to)));
  pcaps_helpers.insert(std::pair<std::string, PcapHelperForDevice*>(to, csma));
}

//---------------------------------------------------------------------------//

void Escenario::AddPPPNetwork (std::string to, std::string tasa, double retardo)
{
  NS_LOG_INFO ("Crea una red P2P con todos los elementos de un NodeContainer");
  PointToPointHelper *point = new PointToPointHelper();
  point->SetDeviceAttribute ("DataRate", DataRateValue (DataRate (tasa)));
  point->SetChannelAttribute ("Delay", TimeValue (Time(retardo)));
  point->SetQueue("ns3::PfifoFastQueueDisc","Limit",UintegerValue(MaxQUEUINGPackets));
  Insert(to, point->Install (*GetNodeContainer(to)));
  pcaps_helpers.insert(std::pair<std::string, PcapHelperForDevice*>(to, point));
}

//---------------------------------------------------------------------------//

NetDeviceContainer* Escenario::GetNetDeviceContainer (std::string nombre)
{
  NS_LOG_INFO ("Obtiene un NetDeviceContainer especifico");
  return netdevices.at(nombre);
}

//---------------------------------------------------------------------------//
Ptr<NetDevice> Escenario::GetNetDevice (std::string from, int numNode)
{
  NS_LOG_INFO ("Obtiene un NetDevice especifico");
  return GetNetDeviceContainer(from)->Get (numNode);
}

//---------------------------------------------------------------------------//

void Escenario::SetIpToNetwork (std::string to, std::string base, std::string mask)
{
  NS_LOG_INFO ("Asigna direcciones IP a todos los elementos de la red");
  Ipv4AddressHelper ipv4Addr;
  ipv4Addr.SetBase (base.c_str(), mask.c_str());
  Insert(to, ipv4Addr.Assign (*GetNetDeviceContainer(to)));
}

//---------------------------------------------------------------------------//

Ipv4InterfaceContainer* Escenario::GetInterfaceContainer (std::string nombre)
{
  NS_LOG_INFO ("Obtiene un InterfaceContainer");
  return ips.at(nombre);
}

//---------------------------------------------------------------------------//

Ipv4Address Escenario::GetIPv4Address (std::string from, int num)
{
  NS_LOG_INFO ("Obtiene dirección IP respecifica");
  return GetInterfaceContainer(from)->GetAddress (num);
}

//---------------------------------------------------------------------------//


void Escenario::EnablePCAPLogging (std::string to)
{
  NS_LOG_INFO ("Activa la generación de PCAP");
  PcapHelperForDevice* helper = pcaps_helpers.at(to);
  helper->EnablePcapAll (to, false);
}

