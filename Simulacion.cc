/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//albfloper
#include <ns3/core-module.h>
#include <ns3/node.h>
#include <ns3/point-to-point-net-device.h>
#include <ns3/point-to-point-channel.h>
#include "ns3/point-to-point-module.h"
#include <ns3/drop-tail-queue.h>
//includes
#include <ns3/gnuplot.h>
#include <ns3/average.h>
#include <ns3/error-model.h>
#include <ns3/random-variable-stream.h>


using namespace ns3;
void simulacion (uint32_t equipos, DataRate capacidad, uint32_t tamPaquete,Time intervalo, Time rprop );

int main (int argc, char *argv[])
{
  // Ajustamos l resolución del reloj simulado a microsegundos
  Time::SetResolution (Time::US);
  //Parametros de la simulación

  uint32_t equipos = 2  ;
  DataRate capacidad = DataRate ("10Mb/s");
  uint32_t tamPaquete = 5000;
  Time intervalo =  Time("5ms");
  Time rprop = Time("2ms") ;

 //añadimos los valores de los parametros 
  CommandLine cmd;
  cmd.AddValue("equipos"," cantidad de equipos ",equipos);
  cmd.AddValue("capacidad","capacidad el enlace",capacidad );
  cmd.AddValue("tamPaquete"," tamaño del paquete",tamPaquete);
  cmd.AddValue("intervalo","tiempo de on ",intervalo );
  cmd.AddValue("rprop", " retardo de propagacion ",rprop);
  cmd.Parse(argc,argv);



simulacion(equipos,capacidad,tamPaquete,intervalo,rprop);




  return 0;
}


void simulacion (uint32_t equipos, DataRate capacidad, uint32_t tamPaquete,Time intervalo, Time rprop)
{   


 /*--------- Montamos el escenario-----------*/
  /*---------Nodos del escenario-------------*/
  // NodeContainer Nodoequipos;
  //Nodoequipos.Create (equipos);
  Ptr<Node> nodoVoipTx = CreateObject<Node> ();
  Ptr<Node> nodoVoipRx = CreateObject<Node> ();

  // Dos dispositivos de red punto a punto, ...
  Ptr<PointToPointNetDevice> dispTx = CreateObject<PointToPointNetDevice> ();
  Ptr<PointToPointNetDevice> dispRx = CreateObject<PointToPointNetDevice> ();

  /*-----------------canal principal del escenario--------------
  PointToPointHelper escenario;
  escenario.SetChannelAttribute ("Delay", TimeValue (rprop));
  escenario.SetDeviceAttribute ("capacidad", DataRateValue (capacidad));
  Ptr<RateErrorModel> errores = CreateObject<RateErrorModel> ();
  errores->SetUnit (RateErrorModel::ERROR_UNIT_PACKET);
  errores->SetRate (tasaError);
  escenario.SetDeviceAttribute ("ReceiveErrorModel", PointerValue (errores));
  escenario.SetQueue ("ns3::DropTailQueue", "MaxPackets", UintegerValue (50));
  NetDeviceContainer dispositivos = escenario.Install (nodos);*/
 
  /*---------------canales secundarios genericos-----------------*/
  PointToPointHelper canalSec;
  canalSec.SetChannelAttribute ("Delay", TimeValue (rprop));
  canalSec.SetDeviceAttribute ("capacidad", DataRateValue (capacidad));


  NetDeviceContainer dispositivos = escenario.Install (nodos)
    // Ptr<Enlace> transmisorVoip = CreateObject<Enlace> (dispositivos.Get (1), trtx, tamVentana, tamPaquete);
    //Ptr<Enlace> receptor = CreateObject<Enlace> (dispositivos.Get (0), trtx, tamVentana, tamPaquete);

  // Asociamos los dispositivos a los nodos
  nodoVoipTx->AddDevice (dispTx);
  nodoVoipRx->AddDevice (dispRx);
  // Asociamos las aplicaciones a los nodos
  nodoVoipTx->AddApplication (transmisorVoip);
  nodoVoipRx->AddApplication (receptor);
  // Añadimos una cola de transmisión a cada disposivo
  dispTx->SetQueue (CreateObject<DropTailQueue> ());
  dispRx->SetQueue (CreateObject<DropTailQueue> ());

  // Conectamos los dispositivos al canal
  dispTx->Attach (canalSec);
  dispRx->Attach (canalSec);

  // Modificamos los parámetos configurables;
  dispTx->SetAttribute ("DataRate",DataRateValue(Velocidad));

  // Activamos el transmisor
  transmisorVoip->SetStartTime (Seconds (1.0));
  transmisorVoip->SetStopTime (Seconds (10.0));

  NS_LOG_DEBUG ("Voy a simular");

  Simulator::Run ();
  Simulator::Destroy ();




}
