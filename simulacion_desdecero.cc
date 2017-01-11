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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

///
// n0 n1 - - - - - - n2 n3
//  | |               | |
// =====             =====
// Lan1      p2p      Lan2


// Lan 1 :  10.1.1.0
// p2p   :  10.0.0.0
// Lan 2 :  10.2.2.0


//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Practica06");

int 
main (int argc, char *argv[])
{
  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
  Time::SetResolution (Time::NS);

  uint32_t nCsmaIzquierda = 1;
  uint32_t nCsmaDerecha = 1;
  /*
  CommandLine cmd;
  cmd.AddValue ("nCsma", "Dispositivos CSMA adicionales", nCsma);
  cmd.Parse (argc,argv);
 
  nCsma = nCsma == 0 ? 1 : nCsma;
  */
  
  // Nodos que pertenecen al enlace punto a punto
  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  // Nodos que pertenecen a la red de área local
  // Como primer nodo añadimos el encaminador que proporciona acceso
  //      al enlace punto a punto.
  NodeContainer csmaNodesDerecha;
  csmaNodesDerecha.Add (p2pNodes.Get (1));
  csmaNodesDerecha.Create (nCsmaDerecha);
  NodeContainer csmaNodesIzquierda;
  csmaNodesIzquierda.Add (p2pNodes.Get (0));
  csmaNodesIzquierda.Create (nCsmaIzquierda);
  
  // Instalamos el dispositivo en los nodos punto a punto
  PointToPointHelper pointToPoint;
  NetDeviceContainer p2pDevices;
  pointToPoint.SetDeviceAttribute ("DataRate",
                                   DataRateValue (DataRate ("2Mbps"))); // Hay que editar los datos
  pointToPoint.SetChannelAttribute ("Delay",
                                    TimeValue (Time ("2ms")));          // Hay que editar los datos
  p2pDevices = pointToPoint.Install (p2pNodes);

  // Instalamos el dispositivo de red en los nodos de la LAN
  CsmaHelper csmaDerecha;
  NetDeviceContainer csmaDevicesDerecha;
  csmaDerecha.SetChannelAttribute ("DataRate",
                            DataRateValue (DataRate ("100Mbps")));    // Hay que editar los datos
  csmaDerecha.SetChannelAttribute ("Delay",
                            TimeValue (Time ("6560ns")));    // Hay que editar los datos
  csmaDevicesDerecha = csmaDerecha.Install (csmaNodesDerecha);

  CsmaHelper csmaIzquierda;
  NetDeviceContainer csmaDevicesIzquierda;
  csmaIzquierda.SetChannelAttribute ("DataRate",
                            DataRateValue (DataRate ("100Mbps")));    // Hay que editar los datos
  csmaIzquierda.SetChannelAttribute ("Delay",
                            TimeValue (Time ("6560ns")));    // Hay que editar los datos
  csmaDevicesIzquierda = csmaIzquierda.Install (csmaNodesIzquierda);
  
  // Instalamos la pila TCP/IP en todos los nodos
  /*InternetStackHelper stackIzquierda;
  
  stackIzquierda.Install (p2pNodes.Get (1));
  stackIzquierda.Install (csmaNodesIzquierda);
  
  InternetStackHelper stackDerecha;
  stackDerecha.Install (p2pNodes.Get (0));
  stackDerecha.Install (csmaNodesDerecha);
  */
  
  InternetStackHelper stack;
  stack.Install (csmaNodesIzquierda);
  stack.Install (csmaNodesDerecha);
  
  // Asignamos direcciones a cada una de las interfaces
  // Utilizamos dos rangos de direcciones diferentes:
  //    - un rango para los dos nodos del enlace
  //      punto a punto
  //    - un rango para los nodos de la red de área local.
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer p2pInterfaces;
  address.SetBase ("10.0.0.0", "255.255.255.0");
  p2pInterfaces = address.Assign (p2pDevices);
  
  Ipv4InterfaceContainer csmaIzquierdaInterfaces;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  csmaIzquierdaInterfaces = address.Assign (csmaDevicesIzquierda);

  Ipv4InterfaceContainer csmaDerechaInterfaces;
  address.SetBase ("10.2.2.0", "255.255.255.0");
  csmaDerechaInterfaces = address.Assign (csmaDevicesDerecha);
  
  // Calculamos las rutas del escenario. Con este comando, los
  //     nodos de la red de área local definen que para acceder
  //     al nodo del otro extremo del enlace punto a punto deben
  //     utilizar el primer nodo como ruta por defecto.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  
  // Establecemos un sumidero para los paquetes en el puerto 9 del nodo
  //     aislado del enlace punto a punto
  uint16_t port = 9;
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (),
                                                     port)));
  ApplicationContainer app = sink.Install (p2pNodes.Get (0));

  // Instalamos un cliente OnOff en uno de los equipos de la red de área local
  OnOffHelper clientes ("ns3::UdpSocketFactory",
                        Address (InetSocketAddress (p2pInterfaces.GetAddress (0),
                                                    port)));
  
  ApplicationContainer clientApps = clientes.Install (csmaNodesDerecha.Get (nCsmaDerecha));
  clientApps.Start (Time ("2s"));
  clientApps.Stop (Time ("10s"));

  // Activamos las trazas pcap en las dos interfaces del nodo de enlace
  pointToPoint.EnablePcap ("practica06", p2pDevices.Get (0));
  csmaDerecha.EnablePcap ("practica06", csmaDevicesDerecha.Get (0), true);

  // Lanzamos la simulación
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
