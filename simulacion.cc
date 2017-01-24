
//Explicación:
// Main:
//  uint32_t nCsma = 4;
//  uint32_t equiposHTTP = 1;
//  uint32_t equiposVoIP = 1;
//  uint32_t equiposFTP  = 1;
// 
// Ahora la CSMA izquierda y derecha son SIMÉTRICAS en cuanto a nº pero pueden variar en datarate y delays
// pueden haber tantos equipos que transmitan HTTP, VoIP o FTP como se quiera, eso si
// la suma de equipos ha de tener lógica (han de sumar siempre nCsma-1)
// Haz la prueba por ejemplo con nCsma = 5 y dos VoIP y verás como se generan bien en los LOGS, asignandose bien las IPs de los demás equipos.
//
//
// Hay varios bucles, de momento estan capados para que no se tire esto 2 horas. Las lineas comentadas son los bucles correctos,
// hay comentarios puestos para indicar cuál quitar, no tiene problema.
// 
// El bucle más externo va variando el delay en el cuello, luego hay otro que varia la tasa de error en el medio y finalmente
// otro que varía el nº de simulaciones para el cálculo de la IC



// En simulación se monta toda la topología
// Ha cambiado mucho por que no se podía hacer de la forma en la que se hacía antes por que no me fue posible realizar varias simulaciones
// por que se pisaban las IPs y hacer que hubiera varios equipos con VoIP era horrible.
// todas las funciones de manejo del escenario operan en escenario.h.
// En principio esto está ya echao a andar y no hay que mirarlo más 
// (Si acaso la activación de CAPS que está en EnablePCAPLogging al final del todo)

// ¿Cuál es el problema?
// Todo va bien en el momento de montar el escenario, que se monta bien, incluso se habilitan los pcaps
// el problema creo haberlo localizado en las lineas del tipo: (se repiten para las 3 aplicaciones)
// InetSocketAddress socketAddressFTPDer (escenario.GetIPv4Address("CSMADerecha", index_ftp), PORT_FTP); 
// Y es que un socket FTPDer debería vincularse con la IP del otro extremo de la red, es decir con CSMAIzquierda (entrecomillado)
// ¡Pero no me deja el hijo de una hiena! 
// ¿Por qué? ¿Por qué? ¿Por qué? Pues asi llevo cerca de una hora. 

// Y ya está, osea, solventar ese problema de sockets y andaría todo a full. O eso creo. No sé cuanto tiempo llevo ya mirando estos códigos.

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/applications-module.h>
#include <ns3/ipv4-global-routing-helper.h>
#include "ns3/drop-tail-queue.h"
#include "ns3/ipv4-queue-disc-item.h"
#include "ns3/pfifo-fast-queue-disc.h"
#include "ns3/ipv6-queue-disc-item.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/object-factory.h"
#include "ns3/traffic-control-module.h"
#include "ns3/socket.h"
#include "voip.h"
#include "ftp.h"
#include "http.h"
#include "escenario.h"

#define TSTUDENT 1.8331               //10 Simulaciones con 90% de intervalo de confianza

#define TASA 5000000

#define T_INICIO 1
#define T_FINAL 30 // o 40
#define NUM_SIMU 1        // ¡Cambiar a más!

#define PORT_HTTP 80
#define PORT_FTP 20
#define PORT_VOIP 50

#define TOS_VOIP 184 
#define TOS_FTP 40
#define TOS_HTTP 0

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Trabajo");

int simulacion (uint32_t nCsma, std::string tasa_izquierda, std::string tasa_derecha, std::string tasa_cuello, double t_retardo_cuello, double t_retardo_izquierda, double t_retardo_derecha, uint32_t equiposVoIP, uint32_t equiposFTP, uint32_t equiposHTTP ) 
  {
  NS_LOG_INFO ("Creando Topologia");
  
  double maxHablando=0.2;
  double minHablando=0.1;
  double maxSilencio=0.8;
  double minSilencio=0.9;
  
  double maxOnHTTP=2;
  double minOnHTTP=1;
  double maxOffHTTP=0;
  double minOffHTTP=0.01;
  
  //Manejador de escenario
  Escenario escenario;
  // Nodos que pertenecen al enlace punto a punto
  escenario.AddContainer ("p2p", 2);
  // Nodos que pertenecen a la red de área local de la Izquierda
  // El primer nodo el encaminador que proporciona acceso al enlace p2p.
  escenario.AddContainer ("CSMAIzquierda", nCsma);
  escenario.AddNodeToContainer("p2p", 0, "CSMAIzquierda"); //mete el router en la red csma
  // Nodos que pertenecen a la red de área local de la Derecha
  // El primer nodo el encaminador que proporciona acceso al enlace p2p.
  escenario.AddContainer ("CSMADerecha", nCsma);
  escenario.AddNodeToContainer("p2p", 1, "CSMADerecha"); //mete el router en la red csma
  
  //Instalamos el dispositivo en los nodos punto a punto
  escenario.AddPPPNetwork("p2p", tasa_cuello, t_retardo_cuello);


  // Instalamos el dispositivo de red en los nodos de las LAN
  escenario.AddCsmaNetwork("CSMAIzquierda", tasa_izquierda, t_retardo_izquierda);
  escenario.AddCsmaNetwork("CSMADerecha", tasa_derecha, t_retardo_derecha);
  
  //Se monta la pila de protocolos
  escenario.BuildInternetStack ();
  
  //----------------------------------------------------------------------------
  // Asignamos direcciones a cada una de las interfaces
  // Utilizamos dos rangos de direcciones diferentes:
  //    - un rango para los dos nodos del enlace p2p
  //    - un rango para los nodos de la red de área local izquierda.
  //    - un rango para los nodos de la red de área local derecha.
  //----------------------------------------------------------------------------
  //Network de los dos nodos del enlace punto a punto
  escenario.SetIpToNetwork ("p2p",            "10.0.0.0", "255.255.255.0");
  escenario.SetIpToNetwork ("CSMAIzquierda",  "10.1.1.0", "255.255.255.0");
  escenario.SetIpToNetwork ("CSMADerecha",    "10.2.2.0", "255.255.255.0");
  
  // Calculamos las rutas del escenario.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();  
  
  //----------------------------------------------------------------------------
  NS_LOG_INFO ("Añadiendo aplicaciones...");
  //----------------------------------------------------------------------------
  // 1) Sumideros
  NS_LOG_INFO ("Instalando sumideros");
  PacketSinkHelper sinkUdp ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), PORT_VOIP)));
  PacketSinkHelper sinkFtp ("ns3::TcpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), PORT_FTP)));
  PacketSinkHelper sinkHTTP ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), PORT_VOIP)));

  for (unsigned index_voip = 2; index_voip < (1+equiposVoIP+1); index_voip++) 
  {
    ApplicationContainer app2izq = sinkUdp.Install (escenario.GetNode("CSMAIzquierda", index_voip)); 
    NS_LOG_INFO ("Instalando sumidero para VoIP en " << escenario.GetIPv4Address("CSMAIzquierda",index_voip-1));
    app2izq.Start (Seconds (T_INICIO));
    app2izq.Stop (Seconds (T_FINAL));
  
    ApplicationContainer app2der = sinkUdp.Install (escenario.GetNode("CSMADerecha", index_voip));
    NS_LOG_INFO ("Instalando sumidero para VoIP en " << escenario.GetIPv4Address("CSMAIzquierda",index_voip-1));
    app2der.Start (Seconds (T_INICIO));
    app2der.Stop (Seconds (T_FINAL));
  }
  
  for (unsigned index_ftp = 2+equiposVoIP; index_ftp < (1+equiposVoIP+1+equiposFTP); index_ftp++) 
  {
    ApplicationContainer app1Izq = sinkFtp.Install (escenario.GetNode("CSMAIzquierda", index_ftp));
    NS_LOG_INFO ("Instalando sumidero para FTP en " << escenario.GetIPv4Address("CSMAIzquierda",index_ftp-1));
    app1Izq.Start (Seconds (T_INICIO));
    app1Izq.Stop (Seconds (T_FINAL));
  
    ApplicationContainer app1Der = sinkFtp.Install (escenario.GetNode("CSMADerecha", index_ftp));
    NS_LOG_INFO ("Instalando sumidero para FTP en " << escenario.GetIPv4Address("CSMADerecha",index_ftp-1));
    app1Der.Start (Seconds (T_INICIO));
    app1Der.Stop (Seconds (T_FINAL));
  }
  
  for (unsigned index_http = 2+equiposVoIP+equiposFTP; index_http < (1+equiposVoIP+1+equiposFTP+equiposHTTP); index_http++) 
  {
    ApplicationContainer app3izq = sinkHTTP.Install (escenario.GetNode("CSMAIzquierda", index_http));
    NS_LOG_INFO ("Instalando sumidero para HTTP en " << escenario.GetIPv4Address("CSMAIzquierda",index_http-1));
    app3izq.Start (Seconds (T_INICIO));
    app3izq.Stop (Seconds (T_FINAL));
  
    ApplicationContainer app3der = sinkHTTP.Install (escenario.GetNode("CSMADerecha", index_http));
    NS_LOG_INFO ("Instalando sumidero para HTTP en " << escenario.GetIPv4Address("CSMADerecha",index_http-1));
    app3der.Start (Seconds (T_INICIO));
    app3der.Stop (Seconds (T_FINAL));
  }
  //----------------------------------------------------------------------------
  // 2) "Teléfono" IP
  NS_LOG_INFO ("Instalando telefonos VoIP");
  for (unsigned index_voip = 2; index_voip < (1+equiposVoIP+1); index_voip++) 
  {
    InetSocketAddress socketAddressTelDer (escenario.GetIPv4Address("CSMADerecha",index_voip), PORT_VOIP);
    socketAddressTelDer.SetTos(TOS_VOIP);
    
    InetSocketAddress socketAddressTelIzq (escenario.GetIPv4Address("CSMAIzquierda",index_voip), PORT_VOIP);
    socketAddressTelIzq.SetTos(TOS_VOIP);
    
    VoipHelper telefonoDerecha (escenario.GetIPv4Address("CSMADerecha",index_voip), PORT_VOIP, maxHablando, minHablando, maxSilencio, minSilencio,socketAddressTelDer);
    VoipHelper telefonoIzquierda (escenario.GetIPv4Address("CSMAIzquierda",index_voip), PORT_VOIP, maxHablando, minHablando, maxSilencio, minSilencio,socketAddressTelIzq);
    
    ApplicationContainer appVoipDerecha = telefonoDerecha.Install(escenario.GetNode("CSMADerecha", index_voip));
    ApplicationContainer appVoipIzquierda = telefonoIzquierda.Install(escenario.GetNode("CSMAIzquierda", index_voip));
    appVoipDerecha.Start (Seconds (T_INICIO));
    appVoipDerecha.Stop (Seconds (T_FINAL));
    appVoipIzquierda.Start (Seconds (T_INICIO));
    appVoipIzquierda.Stop (Seconds (T_FINAL));
    NS_LOG_INFO ("Instalando telefono VoIP en " << escenario.GetIPv4Address("CSMAIzquierda",index_voip-1));
    NS_LOG_INFO ("Instalando telefono VoIP en " << escenario.GetIPv4Address("CSMADerecha",index_voip-1));
  }
  
  //----------------------------------------------------------------------------
  // 3) Servidores FTP
  for (unsigned index_ftp = 2+equiposVoIP; index_ftp < (1+equiposVoIP+1+equiposFTP); index_ftp++) 
  {
    InetSocketAddress socketAddressFTPDer (escenario.GetIPv4Address("CSMADerecha", index_ftp), PORT_FTP);
    socketAddressFTPDer.SetTos(TOS_FTP);
    InetSocketAddress socketAddressFTPIzq (escenario.GetIPv4Address("CSMAIzquierda", index_ftp), PORT_FTP);
    socketAddressFTPIzq.SetTos(TOS_FTP);
    
    FTPHelper ftpDerecha(escenario.GetIPv4Address("CSMADerecha",index_ftp), PORT_FTP, socketAddressFTPDer);
    FTPHelper ftpIzquierda(escenario.GetIPv4Address("CSMAIzquierda",index_ftp), PORT_FTP, socketAddressFTPIzq);
    ApplicationContainer appFtpDerecha = ftpDerecha.Install (escenario.GetNode("CSMADerecha", index_ftp));
    ApplicationContainer appFtpIzquierda = ftpIzquierda.Install (escenario.GetNode("CSMAIzquierda", index_ftp));
    appFtpDerecha.Start (Seconds (T_INICIO));
    appFtpDerecha.Stop (Seconds (T_FINAL));
    appFtpIzquierda.Start (Seconds (T_INICIO));
    appFtpIzquierda.Stop (Seconds (T_FINAL));
    NS_LOG_INFO ("Instalando app FTP en " << escenario.GetIPv4Address("CSMAIzquierda",index_ftp-1));
    NS_LOG_INFO ("Instalando app FTP en " << escenario.GetIPv4Address("CSMADerecha",index_ftp-1));
  }
  //----------------------------------------------------------------------------
  // 4) Aplicación HTTP
  for (unsigned index_http = 2+equiposVoIP+equiposFTP; index_http < (1+equiposVoIP+1+equiposFTP+equiposHTTP); index_http++) 
  {
    InetSocketAddress socketAddressHTTPDer (escenario.GetIPv4Address("CSMADerecha", index_http), PORT_HTTP);
    socketAddressHTTPDer.SetTos(TOS_HTTP);
    InetSocketAddress socketAddressHTTPIzq (escenario.GetIPv4Address("CSMAIzquierda", index_http), PORT_HTTP);
    socketAddressHTTPIzq.SetTos(TOS_HTTP);
    
    HTTPHelper httpDerecha(escenario.GetIPv4Address("CSMADerecha",index_http), PORT_HTTP,maxOnHTTP, minOnHTTP, maxOffHTTP, minOffHTTP,socketAddressHTTPDer);
    HTTPHelper httpIzquierda(escenario.GetIPv4Address("CSMAIzquierda",index_http), PORT_HTTP,maxOnHTTP, minOnHTTP, maxOffHTTP, minOffHTTP,socketAddressHTTPIzq);
    
    ApplicationContainer apphttpDerecha = httpDerecha.Install (escenario.GetNode("CSMADerecha", index_http));
    ApplicationContainer apphttpIzquierda = httpIzquierda.Install (escenario.GetNode("CSMAIzquierda", index_http));
    
    apphttpDerecha.Start (Seconds (T_INICIO));
    apphttpDerecha.Stop (Seconds (T_FINAL));
    apphttpIzquierda.Start (Seconds (T_INICIO));
    apphttpIzquierda.Stop (Seconds (T_FINAL));
    NS_LOG_INFO ("Instalando app HTTP en " << escenario.GetIPv4Address("CSMAIzquierda",index_http-1));
    NS_LOG_INFO ("Instalando app HTTP en " << escenario.GetIPv4Address("CSMADerecha",index_http-1));
  }
  //----------------------------------------------------------------------------
  // ACTIVAR PCAP
  escenario.EnablePCAPLogging ("CSMAIzquierda");
  escenario.EnablePCAPLogging ("CSMADerecha");
  NS_LOG_INFO ("Ejecutando simulacion...");
  Simulator::Stop(Seconds(T_FINAL));
  Simulator::Run();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done");
  return 0;  //retornará el estadistico que recoja el observador
  }
  
int
main (int argc, char *argv[])
{
  LogComponentEnable("Trabajo", LOG_LEVEL_ALL); // ¡¡SÓLO PARA DEBUG, BORRAR AL FINAL!!
  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
  Time::SetResolution (Time::NS);
  
  uint32_t nCsma = 4; //numero total de equipos en las CSMA (serán siempre SIMÉTRICAS)
  uint32_t equiposHTTP = 1;
  uint32_t equiposVoIP = 1;
  uint32_t equiposFTP  = 1;
  double t_retardo_min           = 0.002;
  double t_retardo_max           = 0.012;
  double t_retardo_actual;
  double t_retardo_paso; 
  double t_retardo_izquierda    = 0.002;
  double t_retardo_derecha      = 0.002;
  std::string tasa_izquierda        ("100Mbps");
  std::string tasa_derecha          ("100Mbps");
  std::string tasa_cuello           ("1Gbps");
  double tasa_error;  
  
  CommandLine cmd;
  cmd.AddValue("nCsma",             "Número de nodos en las redes  CSMA",       nCsma);
  cmd.AddValue("equiposHTTP",       "Número de nodos  que usan app HTTP",       equiposHTTP);
  cmd.AddValue("equiposVoIP",       "Número de nodos  que usan app VoIP",       equiposVoIP);
  cmd.AddValue("equiposFTP",        "Número de nodos  que usan app FTP",        equiposFTP);
  cmd.AddValue("tasa_izquierda",    "Capacidad de la red CSMA izquierda",        tasa_izquierda);
  cmd.AddValue("tasa_derecha",      "Capacidad de la red CSMA derecha",          tasa_derecha);
  cmd.AddValue("tasa_cuello",       "Capacidad de la red p2p",                   tasa_cuello);
  cmd.AddValue("retraso_p2p_min",   "Retardo de la red p2p mínimo",              t_retardo_min );
  cmd.AddValue("retraso_p2p_max",   "Retardo de la red p2p mínimo",              t_retardo_max);
  cmd.AddValue("t_retardo_izquierda","Retardo de la red CSMA izquierda",         t_retardo_izquierda);
  cmd.AddValue("t_retardo_derecha", "Retardo de la red CSMA derecha",            t_retardo_derecha);
  cmd.Parse (argc, argv);
  
  if (equiposHTTP+equiposVoIP+equiposFTP == nCsma-1){
    t_retardo_paso = (t_retardo_min + t_retardo_max)/6;
    //for(t_retardo_actual = t_retardo_min; t_retardo_actual <= t_retardo_max; t_retardo_actual += t_retardo_paso){
      for(t_retardo_actual = 1; t_retardo_actual < 1.001; t_retardo_actual += t_retardo_paso){ // <-- ¡CAMBIAR! por la de arriba
      NS_LOG_INFO("El tiempo de retardo actual en el cuello es: " << t_retardo_actual);
      NS_LOG_INFO("El nº de nodos en la red es izquierda es : " << nCsma);
      for(tasa_error = 0; tasa_error <= 0; tasa_error += 0.1){  //Actalmente la tasa de error es 0 siempre, se podría cambiar
        NS_LOG_INFO("La tasa de error en el medio es : " << tasa_error);
        //for (int indice_simulaciones = 0; indice_simulaciones <= NUM_SIMU; indice_simulaciones++)
        for (int indice_simulaciones = 0; indice_simulaciones <= NUM_SIMU; indice_simulaciones++) // <-- ¡CAMBIAR! por la de arriba
          {
            int rendimiento = simulacion(nCsma, tasa_izquierda, tasa_derecha, tasa_cuello,t_retardo_actual, t_retardo_izquierda, t_retardo_derecha, equiposVoIP, equiposFTP, equiposHTTP);
            NS_LOG_INFO("Rendimiento = " << rendimiento);
          }
      }
    } 
  }
  else
  NS_LOG_ERROR("Combinación de equipos imposible. Recuerde: Numero de nodos totales = (nCsma - 1) ");
}  
