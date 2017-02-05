/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <ns3/packet.h>
#include <ns3/data-rate.h>
#include <ns3/average.h>
#include <ns3/ethernet-header.h>

using namespace ns3;

class Observador 
{
  
public:
  Observador();
  void PktEnviado(Ptr<const Packet> paquete); //Funcion que guardara el tiempo en el que se envio cada paquete haciendo uso de su Uid
  void PktRecibido(Ptr<const Packet> paquete, const Address & dir);//Funcion que comprueba si el paquete recibido esta en la estructura. Si es asi, calculara el retardo. Servira tambien para contabilizar el numero de paquetes recibidos
  float porcentaje_Pkt_correctos();//Funcion que calcula el porcentaje de paquet
  
    
private:

  // variables para contar los paquetes 
  float m_enviado; //LLevara la cuenta de paquetes enviados en la simulacion
  float m_recibidos;//Llevara la cuenta de los paquetes correctos en la simulacion

};


