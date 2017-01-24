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
#include <ns3/gnuplot.h>
#include "voip.h"
#include "ftp.h"
#include "http.h"
#include "Observador.h"

//#define T_STUDENT_VALUE 1.8331               //10 Simulaciones con 90% de intervalo de confianza
#define T_STUDENT_VALUE 2.1318                 //5 Simulaciones con 90% de intervalo de confianza
#define TASA 5000000

#define T_INICIO 1
#define T_FINAL 10

#define NUM_SIMU 5
#define NUM_PUNTOS 3
#define PORT_HTTP 80
#define PORT_FTP 20
#define PORT_VOIP 50

#define TOS_VOIP 184 
#define TOS_FTP 40
#define TOS_HTTP 0


using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("Trabajo");

typedef struct resultado {
        double PorcentajeCorrectosFTP;
        double PorcentajeCorrectosVOIP;
        double PorcentajeCorrectosHTTP;
} RESULTADO; 


RESULTADO simulacion (uint32_t nCsma, DataRate tasa_izquierda, DataRate tasa_derecha, DataRate tasa_cuello, double t_retardo_cuello, double t_retardo_izquierda, double t_retardo_derecha, int tamCola, uint32_t equiposVoIP, uint32_t equiposFTP, uint32_t equiposHTTP ) 
  {
  NS_LOG_INFO ("Creando Topologia");

  // Parametro de la simulación para los min y max de cada tipo de tráfico
  double minHablando=0.4;
  double maxHablando=0.2;
  double minSilencio=1.0;
  double maxSilencio=4.5;

  double minOnHTTP=0.4;
  double maxOnHTTP=0.2;
  double minOffHTTP=0.8;
  double maxOffHTTP=1.6;

  
  // Nodos que pertenecen al enlace punto a punto
  NodeContainer p2pNodes;
  p2pNodes.Create (2);
  
  // Nodos que pertenecen a la red de área local de la Derecha
  // El primer nodo el encaminador que proporciona acceso al enlace p2p.
  NodeContainer csmaNodesDerecha;
  csmaNodesDerecha.Add (p2pNodes.Get (1));
  csmaNodesDerecha.Create (nCsma);

    // Nodos que pertenecen a la red de área local de la Izquierda
  // El primer nodo el encaminador que proporciona acceso al enlace p2p.
  NodeContainer csmaNodesIzquierda;
  csmaNodesIzquierda.Add (p2pNodes.Get (0));
  csmaNodesIzquierda.Create (nCsma);

  
  //Instalamos el dispositivo en los nodos punto a punto
  PointToPointHelper pointToPoint;
  NetDeviceContainer p2pDevices;
  pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (tasa_cuello))); // Hay que editar los datos
  pointToPoint.SetChannelAttribute ("Delay", TimeValue (Time (t_retardo_cuello)));// Hay que editar los datos
  p2pDevices = pointToPoint.Install (p2pNodes);

  //Asignamos la cola a los dispositivos instalados
  pointToPoint.SetQueue("ns3::PfifoFastQueueDisc","Limit",UintegerValue(tamCola));
  
  // Instalamos el dispositivo de red en los nodos de las LAN
  CsmaHelper csmaDerecha;
  NetDeviceContainer csmaDevicesDerecha;
  csmaDerecha.SetChannelAttribute ("DataRate", DataRateValue (DataRate (tasa_derecha)));// Hay que editar los datos
  csmaDerecha.SetChannelAttribute ("Delay", TimeValue (Time (t_retardo_derecha))); // Hay que editar los datos
  csmaDevicesDerecha = csmaDerecha.Install (csmaNodesDerecha);

  CsmaHelper csmaIzquierda;
  NetDeviceContainer csmaDevicesIzquierda;
  csmaIzquierda.SetChannelAttribute ("DataRate", DataRateValue (DataRate (tasa_izquierda)));// Hay que editar los datos
  csmaIzquierda.SetChannelAttribute ("Delay", TimeValue (Time (t_retardo_izquierda)));// Hay que editar los datos
  csmaDevicesIzquierda = csmaIzquierda.Install (csmaNodesIzquierda);
  
  //Se monta la pila de protocolos
  InternetStackHelper stack;
  stack.Install (csmaNodesIzquierda);
  stack.Install (csmaNodesDerecha);
  
  //----------------------------------------------------------------------------
  // Asignamos direcciones a cada una de las interfaces
  // Utilizamos dos rangos de direcciones diferentes:
  //    - un rango para los dos nodos del enlace p2p
  //    - un rango para los nodos de la red de área local izquierda.
  //    - un rango para los nodos de la red de área local derecha.
  //----------------------------------------------------------------------------

  Ipv4AddressHelper address;
  //Network de los dos nodos del enlace punto a punto
  Ipv4InterfaceContainer p2pInterfaces;
  address.SetBase ("10.0.0.0", "255.255.255.0");
  p2pInterfaces = address.Assign (p2pDevices);

  //Network de CSMA Izquierda
  Ipv4InterfaceContainer csmaIzquierdaInterfaces;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  csmaIzquierdaInterfaces = address.Assign (csmaDevicesIzquierda);

  //Network de CSMA Derecha
  Ipv4InterfaceContainer csmaDerechaInterfaces;
  address.SetBase ("10.2.2.0", "255.255.255.0");
  csmaDerechaInterfaces = address.Assign (csmaDevicesDerecha);
  
  // Calculamos las rutas del escenario.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();  
  
  //----------------------------------------------------------------------------
  NS_LOG_INFO ("Añadiendo aplicaciones...");
  //----------------------------------------------------------------------------
  // 1) Sumideros
  NS_LOG_INFO ("Instalando sumideros");
  PacketSinkHelper sinkUdp ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), PORT_VOIP)));
  PacketSinkHelper sinkFtp ("ns3::TcpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), PORT_FTP)));
  PacketSinkHelper sinkHTTP ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny(), PORT_HTTP)));
  
  ApplicationContainer sumideroFTPIzq;
  ApplicationContainer sumideroFTPDer;

  ApplicationContainer sumideroVoipIzq;
  ApplicationContainer sumideroVoipDer;

  ApplicationContainer sumideroHTTPizq;
  ApplicationContainer sumideroHTTPder;

   //Instalación de los sumuderos FTP en los equipos

  for (unsigned index_ftp = 1; index_ftp < (equiposFTP+1); index_ftp++) 
  {
    NS_LOG_INFO ("Instalando sumidero para FTP en " << csmaIzquierdaInterfaces.GetAddress(index_ftp));
    sumideroFTPIzq = sinkFtp.Install (csmaNodesIzquierda.Get(index_ftp));
    sumideroFTPIzq.Start (Seconds (T_INICIO));
    sumideroFTPIzq.Stop (Seconds (T_FINAL));
  
    NS_LOG_INFO ("Instalando sumidero para FTP en " << csmaDerechaInterfaces.GetAddress(index_ftp));
    sumideroFTPDer = sinkFtp.Install (csmaNodesDerecha.Get(index_ftp));
    sumideroFTPDer.Start (Seconds (T_INICIO));
    sumideroFTPDer.Stop (Seconds (T_FINAL));
  }
  //Instalación de los sumuderos VOIP en los equipos


  for (unsigned index_voip = 1+equiposFTP; index_voip < (equiposFTP+1+equiposVoIP); index_voip++) 
  {
    NS_LOG_INFO ("Instalando sumidero para VoIP en " << csmaIzquierdaInterfaces.GetAddress(index_voip));
    sumideroVoipIzq  = sinkUdp.Install (csmaNodesIzquierda.Get(index_voip));
    sumideroVoipIzq.Start (Seconds (T_INICIO));
    sumideroVoipIzq.Stop (Seconds (T_FINAL));
    
    NS_LOG_INFO ("Instalando sumidero para VoIP en " << csmaDerechaInterfaces.GetAddress(index_voip));
    sumideroVoipDer  = sinkUdp.Install (csmaNodesDerecha.Get(index_voip));
    sumideroVoipDer.Start (Seconds (T_INICIO));
    sumideroVoipDer.Stop (Seconds (T_FINAL));
  }
  
  //Instalación de los sumuderos HTTP en los equipos

  for (unsigned index_http = 1+equiposFTP+equiposVoIP; index_http < (1+equiposFTP+equiposVoIP+equiposHTTP); index_http++) 
  {
    NS_LOG_INFO ("Instalando sumidero para HTTP en " << csmaIzquierdaInterfaces.GetAddress(index_http));
    sumideroHTTPizq = sinkHTTP.Install  (csmaNodesIzquierda.Get(index_http));
    sumideroHTTPizq.Start (Seconds (T_INICIO));
    sumideroHTTPizq.Stop (Seconds (T_FINAL));
  
    NS_LOG_INFO ("Instalando sumidero para HTTP en " << csmaDerechaInterfaces.GetAddress(index_http));
    sumideroHTTPder = sinkHTTP.Install  (csmaNodesDerecha.Get(index_http));
    sumideroHTTPder.Start (Seconds (T_INICIO));
    sumideroHTTPder.Stop (Seconds (T_FINAL));
  }

  NS_LOG_DEBUG("Se han instalado correctamente todos los sumideros");
  
  //----------------------------------------------------------------------------
  // 2) Servidores FTP
  ApplicationContainer appFtpDerecha;
  ApplicationContainer appFtpIzquierda;
  
  //Creamos los sockets de FTP y los asociamos a las interfaces de los nodos 

  for (unsigned index_ftp = 1; index_ftp < (equiposFTP+1); index_ftp++) 
  {   

    InetSocketAddress socketAddressFTPDer (csmaIzquierdaInterfaces.GetAddress (index_ftp), PORT_FTP);
    socketAddressFTPDer.SetTos(TOS_FTP);
    FTPHelper ftpDerecha (csmaIzquierdaInterfaces.GetAddress (index_ftp), PORT_FTP,socketAddressFTPDer);
    appFtpDerecha = ftpDerecha.Install (csmaNodesDerecha.Get(index_ftp));
    NS_LOG_INFO ("Instalando app FTP en " << csmaDerechaInterfaces.GetAddress (index_ftp));
    appFtpDerecha.Start (Seconds (T_INICIO));
    appFtpDerecha.Stop (Seconds (T_FINAL));

    InetSocketAddress socketAddressFTPIzq (csmaDerechaInterfaces.GetAddress (index_ftp), PORT_FTP);
    socketAddressFTPIzq.SetTos(TOS_FTP);
    FTPHelper ftpIzquierda (csmaDerechaInterfaces.GetAddress (index_ftp), PORT_FTP,socketAddressFTPIzq);
    appFtpIzquierda = ftpIzquierda.Install (csmaNodesIzquierda.Get(index_ftp));
    NS_LOG_INFO ("Instalando app FTP en " << csmaIzquierdaInterfaces.GetAddress (index_ftp));
    appFtpIzquierda.Start(Seconds (T_INICIO));
    appFtpIzquierda.Stop(Seconds (T_FINAL));
  }
  
  NS_LOG_DEBUG("Se han instalado correctamente las aplicaciones de FTP");



  //----------------------------------------------------------------------------
  // 3) "Teléfono" IP
  ApplicationContainer appVoipDerecha;
  ApplicationContainer appVoipIzquierda;
  
  //Creamos los sockets de VOIP y los asociamos a las interfaces de los nodos 

  for (unsigned index_voip = 1+equiposFTP; index_voip < (equiposFTP+1+equiposVoIP); index_voip++) 
  {
    InetSocketAddress socketAddresstelDer (csmaIzquierdaInterfaces.GetAddress (index_voip),PORT_VOIP);
    socketAddresstelDer.SetTos(TOS_VOIP);
    VoipHelper telefonoDerecha (csmaIzquierdaInterfaces.GetAddress (index_voip), PORT_VOIP, maxHablando, minHablando, maxSilencio, minSilencio,socketAddresstelDer);
    appVoipDerecha = telefonoDerecha.Install(csmaNodesDerecha.Get(index_voip));
    NS_LOG_INFO ("Instalando app VOIP en " << csmaDerechaInterfaces.GetAddress (index_voip));
    appVoipDerecha.Start(Seconds (T_INICIO));
    appVoipDerecha.Stop(Seconds (T_FINAL));
      
    InetSocketAddress socketAddresstelIzq (csmaDerechaInterfaces.GetAddress (index_voip),PORT_VOIP);
    socketAddresstelIzq.SetTos(TOS_VOIP);
    VoipHelper telefonoIzquierda (csmaDerechaInterfaces.GetAddress (index_voip), PORT_VOIP, maxHablando, minHablando, maxSilencio, minSilencio,socketAddresstelIzq);
    appVoipIzquierda = telefonoIzquierda.Install (csmaNodesIzquierda.Get(index_voip)); //nodo en el que se instala
    NS_LOG_INFO ("Instalando app VOIP en " << csmaDerechaInterfaces.GetAddress (index_voip));
    appVoipIzquierda.Start(Seconds (T_INICIO));
    appVoipIzquierda.Stop(Seconds (T_FINAL));
  }  

  NS_LOG_DEBUG("Se han instalado correctamente las aplicaciones de VOIP");


  //----------------------------------------------------------------------------
  // 4) Aplicación HTTP
  ApplicationContainer apphttpDerecha;
  ApplicationContainer apphttpIzquierda;


  //Creamos los sockets de IP y los asociamos a las interfaces de los nodos 


  for (unsigned index_http = 1+equiposFTP+equiposVoIP; index_http < (1+equiposFTP+equiposVoIP+equiposHTTP); index_http++) 
  {
    InetSocketAddress socketAddressHTTPDer (csmaIzquierdaInterfaces.GetAddress (index_http), PORT_HTTP);
    socketAddressHTTPDer.SetTos(TOS_HTTP);
    HTTPHelper httpDerecha (csmaIzquierdaInterfaces.GetAddress (index_http), PORT_HTTP, maxOnHTTP, minOnHTTP, maxOffHTTP, minOffHTTP,socketAddressHTTPDer);
    apphttpDerecha = httpDerecha.Install (csmaNodesDerecha.Get(index_http));
    NS_LOG_INFO ("Instalando app HTTP en " << csmaDerechaInterfaces.GetAddress (index_http));
    apphttpDerecha.Start(Seconds (T_INICIO));
    apphttpDerecha.Stop(Seconds (T_FINAL));

    InetSocketAddress socketAddressHTTPIzq (csmaDerechaInterfaces.GetAddress (index_http), PORT_HTTP);
    socketAddressHTTPDer.SetTos(TOS_HTTP);
    HTTPHelper httpIzquierda (csmaDerechaInterfaces.GetAddress (index_http), PORT_HTTP, maxOnHTTP, minOnHTTP, maxOffHTTP, minOffHTTP,socketAddressHTTPIzq );
    apphttpIzquierda = httpIzquierda.Install (csmaNodesIzquierda.Get(index_http));
    NS_LOG_INFO ("Instalando app HTTP en " << csmaDerechaInterfaces.GetAddress (index_http));
    apphttpIzquierda.Start(Seconds (T_INICIO));
    apphttpIzquierda.Stop(Seconds (T_FINAL));

  }

  NS_LOG_DEBUG("Se han instalado correctamente las aplicaciones de HTTP");

  //----------------------------------------------------------------------------
  // Observador
  Observador trafVOIP=Observador();
  Observador trafHTTP=Observador();
  Observador trafFTP=Observador();

  //Asociamos las trazas para calcular los paquetes recibidos y enviados 

  NS_LOG_INFO ("Asociando trazas de los observadores a los sumideros y a los aplicaciones  ");
  sumideroFTPIzq.Get(0)->GetObject<PacketSink>()->TraceConnectWithoutContext("Rx",MakeCallback(&Observador::PktRecibido,&trafFTP));
  sumideroFTPDer.Get(0)->GetObject<PacketSink>()->TraceConnectWithoutContext("Rx",MakeCallback(&Observador::PktRecibido,&trafFTP));
  sumideroVoipIzq.Get(0)->GetObject<PacketSink>()->TraceConnectWithoutContext("Rx",MakeCallback(&Observador::PktRecibido,&trafVOIP));
  sumideroVoipDer.Get(0)->GetObject<PacketSink>()->TraceConnectWithoutContext("Rx",MakeCallback(&Observador::PktRecibido,&trafVOIP));
  sumideroHTTPizq.Get(0)->GetObject<PacketSink>()->TraceConnectWithoutContext("Rx",MakeCallback(&Observador::PktRecibido,&trafHTTP));
  sumideroHTTPder.Get(0)->GetObject<PacketSink>()->TraceConnectWithoutContext("Rx",MakeCallback(&Observador::PktRecibido,&trafHTTP));
  

  NS_LOG_DEBUG("Se han asociado correctamente las trazas a los sumideros ");

  appFtpIzquierda.Get(0)->GetObject<BulkSendApplication>()->TraceConnectWithoutContext("Tx",MakeCallback(&Observador::PktEnviado,&trafFTP));
  appFtpDerecha.Get(0)->GetObject<BulkSendApplication>()->TraceConnectWithoutContext("Tx",MakeCallback(&Observador::PktEnviado,&trafFTP));  
  appVoipIzquierda.Get(0)->GetObject<OnOffApplication>()->TraceConnectWithoutContext("Tx",MakeCallback(&Observador::PktEnviado,&trafVOIP));
  appVoipDerecha.Get(0)->GetObject<OnOffApplication>()->TraceConnectWithoutContext("Tx",MakeCallback(&Observador::PktEnviado,&trafVOIP));
  apphttpDerecha.Get(0)->GetObject<OnOffApplication>()->TraceConnectWithoutContext("Tx",MakeCallback(&Observador::PktEnviado,&trafHTTP));
  apphttpIzquierda.Get(0)->GetObject<OnOffApplication>()->TraceConnectWithoutContext("Tx",MakeCallback(&Observador::PktEnviado,&trafHTTP));

  NS_LOG_DEBUG("Se han asociado correctamente las trazas a las aplicaciones " );

  //----------------------------------------------------------------------------
  // ACTIVAR PCAP 
  /*
  for (unsigned indice = 0; indice < nCsma; indice++) 
  {
   csmaIzquierda.EnablePcap ("CSMAizq", csmaDevicesIzquierda.Get (indice), false);
   csmaDerecha.EnablePcap ("CSMAder", csmaDevicesDerecha.Get (indice), false);
  }
 */
  //----------------------------------------------------------------------------

  NS_LOG_INFO ("Ejecutando simulacion...");
  Simulator::Stop(Seconds(T_FINAL));
  Simulator::Run();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done");

  //sacamos los porcentajes del tráfico
  double porcentajeVOIP = trafVOIP.porcentaje_Pkt_correctos();
  double porcentajeFTP = trafFTP.porcentaje_Pkt_correctos();
  double porcentajeHTTP = trafHTTP.porcentaje_Pkt_correctos();
  
  //Creamos la estructura que retornará los resultados 

  RESULTADO resultado = { porcentajeFTP, porcentajeVOIP, porcentajeHTTP };

  NS_LOG_DEBUG("Se crea la estructura para devolverla con el return, fin de la simulación" );

  return resultado;  //retornará una estructura con los resultados
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
  int grafica = 0; //0 grafica A -- 1 grafica B
  //Para gráfico A
  double t_retardo_min           = 0.002;
  double t_retardo_max           = 0.012;
  double t_retardo_actual        = 0.004; //Valor por defecto para grafica B
  double t_retardo_paso; 
  //Para gráfica B
  int tamCola_min           = 10;
  int tamCola_max           = 200;
  int tamCola_actual        = 150;        //Valor por defecto para grafica A
  int tamCola_paso; 
  
  //Parámetros Comunes  
  double t_retardo_izquierda    = 0.002;
  double t_retardo_derecha      = 0.002;
  DataRate tasa_izquierda        ("100Mbps");
  DataRate tasa_derecha          ("100Mbps");
  DataRate tasa_cuello           ("1Gbps");
  RESULTADO resultado;
  
  CommandLine cmd;
  cmd.AddValue("grafica",           "Qué gráfica va a generar. 0=A ; 1=B",      grafica);
  cmd.AddValue("nCsma",             "Número de nodos en las redes  CSMA",       nCsma);
  cmd.AddValue("equiposHTTP",       "Número de nodos  que usan app HTTP",       equiposHTTP);
  cmd.AddValue("equiposVoIP",       "Número de nodos  que usan app VoIP",       equiposVoIP);
  cmd.AddValue("equiposFTP",        "Número de nodos  que usan app FTP",        equiposFTP);
  cmd.AddValue("tasa_izquierda",    "Capacidad de la red CSMA izquierda",       tasa_izquierda);
  cmd.AddValue("tasa_derecha",      "Capacidad de la red CSMA derecha",         tasa_derecha);
  cmd.AddValue("tasa_cuello",       "Capacidad de la red p2p",                  tasa_cuello);
  cmd.AddValue("t_retardo_izquierda","Retardo de la red CSMA izquierda",        t_retardo_izquierda);
  cmd.AddValue("t_retardo_derecha", "Retardo de la red CSMA derecha",           t_retardo_derecha);
  //Para gráfico A
  cmd.AddValue("retraso_p2p_min",   "Retardo de la red p2p mínimo en seg.",     t_retardo_min);
  cmd.AddValue("retraso_p2p_max",   "Retardo de la red p2p máximo en seg.",     t_retardo_max);
  //Para gráfico B
  cmd.AddValue("tamCola_min",   "Capacidad mínima cola en paquetes. ",          t_retardo_min);
  cmd.AddValue("tamCola_max",   "Capacidad máxima cola en paquetes. ",          t_retardo_max);
  cmd.Parse (argc, argv);
  
  /*--------------------Declaración de plots para la gráfica------------------*/
  Gnuplot plot[2];

  //Gráfica con tamaño de cola constante y variación de retardoenel canal PPP
  plot[0].SetTitle ("Número medio paquetes correctos");
  plot[0].SetLegend ("retardo canal PPP (s)", "Paquetes correctos (%)");

  //Gráfica con retardo de canal constante y variación del tamaño de cola
  plot[1].SetTitle ("Número medio paquetes correctos");
  plot[1].SetLegend ("Tamaño cola", "Paquetes correctos (%)");

  Gnuplot2dDataset dataset[3];
    for (int l = 0; l < 3 ; l++) {
        dataset[l].SetStyle (Gnuplot2dDataset::LINES_POINTS);
        dataset[l].SetErrorBars(Gnuplot2dDataset::Y);
        if (l==0)
        	dataset[l].SetTitle("trafico-FTP");
        else if (l==1)
        	dataset[l].SetTitle("traficoVOiP");
       	else
       		dataset[l].SetTitle("traficoHTTP");
  }
  /*--------------------------------------------------------------------------*/
  if (equiposHTTP+equiposVoIP+equiposFTP == nCsma-1){
    /*--------------------------------------------------Gráfica A-----------------------------------------------------------*/
    if (grafica == 0)
    {
    t_retardo_paso = (t_retardo_min + t_retardo_max)/NUM_PUNTOS;
      for(double t_retardo_actual = t_retardo_min; t_retardo_actual < t_retardo_max; t_retardo_actual += t_retardo_paso){
        NS_LOG_INFO("* * * * * * *  ");
        NS_LOG_INFO("Delay en PPP = " << t_retardo_actual << "/" << t_retardo_max);
        NS_LOG_INFO("* * * * * * *  ");
	
	//Acumuladores de los porcentajes 

        Average<double> PorcentajesVOIP;
        Average<double> PorcentajesFTP;
        Average<double> PorcentajesHTTP;

	//bucle for para ejecutar varias simulaciones 

        for (int indice_simulaciones = 0; indice_simulaciones < NUM_SIMU; indice_simulaciones++)
          {
            NS_LOG_INFO("---------  ");
            NS_LOG_INFO("Simulacion = " << indice_simulaciones+1 << "/5");
            resultado = simulacion(nCsma, tasa_izquierda, tasa_derecha, tasa_cuello,t_retardo_actual, t_retardo_izquierda, t_retardo_derecha, tamCola_actual, equiposVoIP, equiposFTP, equiposHTTP);
            NS_LOG_INFO("Porcentaje paquetes FTP correctos = " << resultado.PorcentajeCorrectosFTP);
            PorcentajesFTP.Update (resultado.PorcentajeCorrectosFTP);
            NS_LOG_INFO("Porcentaje paquetes VOIP correctos = " << resultado.PorcentajeCorrectosVOIP);
            PorcentajesVOIP.Update (resultado.PorcentajeCorrectosVOIP);
            NS_LOG_INFO("Porcentaje paquetes HTTP correctos = " << resultado.PorcentajeCorrectosHTTP);
            PorcentajesHTTP.Update (resultado.PorcentajeCorrectosHTTP);            

	    NS_LOG_DEBUG("Se ha realizado una simulacion correcta para la gráfica A con indice :  " << indice_simulaciones+1  );
          }
	
	//calculo de los datoa para añadir a la gráfica

        double z[3];
        z[0] = T_STUDENT_VALUE * std::sqrt (PorcentajesFTP.Var() / NUM_SIMU);
        dataset[0].Add(t_retardo_actual, PorcentajesFTP.Mean(), z[0]);
        z[1] = T_STUDENT_VALUE * std::sqrt (PorcentajesVOIP.Var() / NUM_SIMU);
        dataset[1].Add(t_retardo_actual, PorcentajesVOIP.Mean(), z[1]);
        z[2] = T_STUDENT_VALUE * std::sqrt (PorcentajesHTTP.Var() / NUM_SIMU);
        dataset[2].Add(t_retardo_actual, PorcentajesHTTP.Mean(), z[2]);  
      }

      //añadimos los datos a la gráfica 
      
      plot[0].AddDataset (dataset[0]);
      plot[0].AddDataset (dataset[1]);
      plot[0].AddDataset (dataset[2]);
      std::ofstream plotFile1 ("trabajo_GraficaA.plt");
      plot[0].GenerateOutput (plotFile1);
      plotFile1 << "pause -1" << std::endl;
      plotFile1.close ();
    }
    /*------------------------------------------------------Gráfica B---------------------------------------------------------------*/
    else if(grafica == 1){ 
    tamCola_paso = (tamCola_min + tamCola_max)/NUM_PUNTOS;
      for(double tamCola_actual = tamCola_min; tamCola_actual < tamCola_max; tamCola_actual += tamCola_paso){
        NS_LOG_INFO("* * * * * * *  ");
        NS_LOG_INFO("Tamaño de cola actual = " << tamCola_actual << "/" << tamCola_max);
        NS_LOG_INFO("* * * * * * *  ");

	//Acumuladores de los porcentajes 
        Average<double> PorcentajesVOIP;
        Average<double> PorcentajesFTP;
        Average<double> PorcentajesHTTP;

       //bucle for para ejecutar varias simulaciones 

        for (int indice_simulaciones = 0; indice_simulaciones < NUM_SIMU; indice_simulaciones++)
          {
            NS_LOG_INFO("---------  ");
            NS_LOG_INFO("Simulacion = " << indice_simulaciones+1 << "/5");
            resultado = simulacion(nCsma, tasa_izquierda, tasa_derecha, tasa_cuello,t_retardo_actual, t_retardo_izquierda, t_retardo_derecha, tamCola_actual, equiposVoIP, equiposFTP, equiposHTTP);
            NS_LOG_INFO("Porcentaje paquetes FTP correctos = " << resultado.PorcentajeCorrectosFTP);
            PorcentajesFTP.Update (resultado.PorcentajeCorrectosFTP);
            NS_LOG_INFO("Porcentaje paquetes VOIP correctos = " << resultado.PorcentajeCorrectosVOIP);
            PorcentajesVOIP.Update (resultado.PorcentajeCorrectosVOIP);
            NS_LOG_INFO("Porcentaje paquetes HTTP correctos = " << resultado.PorcentajeCorrectosHTTP);
            PorcentajesHTTP.Update (resultado.PorcentajeCorrectosHTTP);    

	    NS_LOG_DEBUG("Se ha realizado una simulacion correcta para la gráfica B con indice :  " << indice_simulaciones+1  );        
          }

	//calculo de los datoa para añadir a la gráfica

        double z[3];
        z[0] = T_STUDENT_VALUE * std::sqrt (PorcentajesFTP.Var() / NUM_SIMU);
        dataset[0].Add(tamCola_actual, PorcentajesFTP.Mean(), z[0]);
        z[1] = T_STUDENT_VALUE * std::sqrt (PorcentajesVOIP.Var() / NUM_SIMU);
        dataset[1].Add(tamCola_actual, PorcentajesVOIP.Mean(), z[1]);
        z[2] = T_STUDENT_VALUE * std::sqrt (PorcentajesHTTP.Var() / NUM_SIMU);
        dataset[2].Add(tamCola_actual, PorcentajesHTTP.Mean(), z[2]);  
      }

      //añadimos los datos a la gráfica 

      plot[1].AddDataset (dataset[0]);
      plot[1].AddDataset (dataset[1]);
      plot[1].AddDataset (dataset[2]);
      std::ofstream plotFile1 ("trabajo_GraficaB.plt");
      plot[0].GenerateOutput (plotFile1);
      plotFile1 << "pause -1" << std::endl;
      plotFile1.close ();
    }
    /*Parámetros incorrectos--------------------------------------------------*/
    else{
    NS_LOG_ERROR("Gráfica ha de valer o bien");
    NS_LOG_ERROR("0: Genera de salida la gráfica: Gráfica A  ");
    NS_LOG_ERROR("1: Genera de salida la gráfica: Gráfica B  ");
    }
  }
  else
  NS_LOG_ERROR("Combinación de equipos imposible. Recuerde: Numero de nodos totales = (nCsma - 1) ");
}  
