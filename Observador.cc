/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/data-rate.h>
#include "Observador.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Observador");
//Constructor
Observador::Observador()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_enviado=0;
  m_recibidos=0;
}

//Funcion que guardara el tiempo en el que se envio cada paquete haciendo uso de su Uid
void
Observador::PktEnviado(Ptr<const Packet> paquete)
{
  //NS_LOG_FUNCTION_NOARGS ();
  //Almacenamos el timpo en el que se envio el paquete
  m_enviado++;
}

//Funcion que comprueba si el paquete recibido esta en la estructura. Si es asi, calculara el retardo. Servira tambien para contabilizar el numero de paquetes recibidos
void
Observador::PktRecibido(Ptr<const Packet> paquete, const Address & dir)
{
 // NS_LOG_FUNCTION_NOARGS ();
  m_recibidos++;
}

//Funcion que calcula el porcentaje de paquetes correctos
float
Observador::porcentaje_Pkt_correctos()
{
  //NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_DEBUG("Se han enviado desde las apps: " << m_enviado);
  NS_LOG_DEBUG("Se han recibido en los sinks: " << m_recibidos );

  
  float aux = ((m_recibidos)/m_enviado)*100;
  NS_LOG_DEBUG("salida " << aux);
  return aux;
}
