/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//Practica de Alberto Flores Pereira y Beatriz Carretero Parra
#include <ns3/core-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/data-rate.h>
#include <ns3/callback.h>
#include <ns3/packet.h>
#include <ns3/error-model.h>
#include "Enlace.h"
#include "Observador.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Enlace");

//Constructor

Voip::Voip (Ptr<NetDevice> disp,
                Time          hablando,
                Time          silencio,
	        Time          retardo,
                uint32_t       tamPqt){
  
  m_tamPqt = tamPqt;
  m_rx = 0;
  m_disp = disp;
  m_tx = 0;

}

bool
Voip::NuevoPaquete (Ptr<NetDevice>        receptor,
              Ptr<const Packet>     recibido,
              uint16_t              protocolo,
              const Address &       desde){
  
  //Funcion que comprueba si el paquete recibido es un paquete de datos o un ACK, y llamara a la funcion pertinente para que trate al mismo.

  NS_LOG_FUNCTION_NOARGS ();
  
  NS_LOG_FUNCTION (receptor << recibido->GetSize () <<
                   std::hex << protocolo << desde);
  
  if ( recibido->GetSize() >= 2 )
      //Si el paquete recibido es un paquete de Datos
      DatosRecibidos (recibido);
    }
  
  return true;
}

void
Voip::VenceTemporizador (){
  //Funcion para activar el envio de datos si tras x ms no se recibe paquetes del otro intermediario 
  NS_LOG_FUNCTION_NOARGS ();

  NS_LOG_DEBUG("Nodo  " <<  m_node->GetId() << ": VENCE TEMPORIZADOR" );
  
}

void
Voip::DatosRecibidos (
                Ptr<const Packet>     recibido)
{
  //Esta funcion comprobara que el paquete coincide con la ventana de recepcion y actualizara la ventana de recepcion si es asi. De cualquier modo enviara un ACK.
   NS_LOG_FUNCTION_NOARGS ();
  
// Obtengo el contenido del paquete 
   uint8_t contenido = 0;
  recibido-> CopyData (&contenido, 1);

  NS_LOG_INFO("El nodo "<< m_node->GetId()<<" ha recibido datos con contenido  = " << (unsigned int)contenido<< " y esperaba recibir " <<(unsigned int) m_rx );
  
  if(m_rx==contenido){ 
   
    NS_LOG_INFO("Soy el nodo "<< m_node->GetId()<<" y mi ventana coincide con el paquete recibido");
    // Incrementamos la m_tx RX    
    m_rx++;
    //Activamos la traza
    m_aNivel3+=recibido->GetSize();
  }
  EnviaACK();
    
}

void
Voip::EnviaDatos (){
  //Esta funcion se encargara de enviar paquetes de datos. En primer lugar genera un numero aleatorio para saber cuantos paquetes tiene que transmitir, Una vez enviados iniciamos el temporizador para esperar a transmitir cada uno de los paquetes de datos y  otro temporizador al acabar de transmitir los paquetes.

NS_LOG_FUNCTION_NOARGS ();

// modificar esta parte para simular el envio de paquetes 

   uint8_t credito = m_tx.Credito();
     //Si tenemos credito enviaremos tantos paquetes como credito tengamos
     if (credito  > 0){
       NS_LOG_INFO("Soy el nodo "<< m_node->GetId()<<" y mi credito es = " << (unsigned int ) credito  );

    for ( uint8_t aux = 0 ; aux < credito ; aux++ )
      {
        uint8_t siguiente_pqt_tx = m_tx.Siguiente();//Comprobamos siguiente paquete a transmitir y actualizamos
       NS_LOG_INFO("Soy el nodo "<< m_node->GetId()<<" y envio el paquete = " <<  (unsigned int )siguiente_pqt_tx);
        Ptr<Packet> paquete = Create<Packet> (   &siguiente_pqt_tx, m_tamPqt + 1);
        m_nuevoPaquete(paquete);//Activamos traza
        m_node->GetDevice (0)->Send (paquete, m_disp->GetAddress (), 0x0800);//Enviamos paquete
      }
    m_temporizador=Simulator::Schedule(m_esperaACK,&Enlace::VenceTemporizador,this);//Activamos temporizador
  }
  
}




