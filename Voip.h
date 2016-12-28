/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/application.h"

using namespace ns3;

class Voip: public Application
{
public:
  Voip ()
  {
  }
  
   // Constructor de la clase. Necesita como parámetros el puntero
  // al dispositivo de red con el que debe comunicarse, el temporizador
  // de retransmisiones y el tamaño de paquete.
  // Inicializa las variables privadas.
  Voip (Ptr<NetDevice> disp,
          const Time           hablando,
          const Time           silencio, 
          const Time         retardo,
          uint32_t       tamPqt);
  
  static TypeId 
  GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::Voip")
      .SetParent<Application> ()
      //      .AddConstructor<Enlace> ()
      .AddTraceSource ("NuevoPaquete", 
                       "Trace source indicating a new packet has been generated",
                       MakeTraceSourceAccessor (&Enlace::m_nuevoPaquete))
      .AddTraceSource ("ANivel3", 
                       "Trace source indicating a packet is going to be pass to Level 3",
                       MakeTraceSourceAccessor (&Enlace::m_aNivel3))
      ;
    return tid;
  } 
          
          
  // Función para el procesamiento de paquetes recibidos.
  // Comprueba si es de datos o asentimiento y actúa en consecuencia.
  bool
  NuevoPaquete (Ptr<NetDevice>        receptor,
                Ptr<const Packet>     recibido,
                uint16_t              protocolo,
                const Address &       desde);

  // Función de vencimiento del temporizador
  void
  VenceTemporizador ();

private:

  // Método que se llama en el instante de comienzo de la aplicación.
  void
  StartApplication ()
  {
   
    // Enviamos el primer paquete
   
    EnviaDatos ();
  }

  // Método que se llama en el instante de final de la aplicación.
  void
  StopApplication ()
  {
   
    Simulator::Stop ();
  }
  

  // Función para el procesamiento de paquetes de datos recibidos.
  // Comprueba el paquete recibido y envía un ACK.
  void
  DatosRecibidos (Ptr<const Packet>     recibido);
  
  // Función que envía un paquete de datos.
  void
  EnviaDatos ();

  // Dispositivo de red con el que hay que comunicarse.
  Ptr<NetDevice> m_disp;
  // Tamaño del paquete
  uint32_t       m_tamPqt;
  // Número de secuencia de los paquetes a recibir
  uint8_t        m_rx;
   // Número de secuencia de los paquetes a transmitir
  uint8_t        m_tx;
  // Evento de retransmision
  EventId        m_temporizador;
  // CallBack de nuevo paquete transmitido
  TracedCallback<Ptr<const Packet> > m_nuevoPaquete;
  // CallBack de nuevos octetos a nivel superior
  TracedValue<uint32_t> m_aNivel3;
}; 
