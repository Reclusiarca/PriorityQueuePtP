//Voip %pkt correctos +99% 150 ms retardo en una dirección y 30 ms jitter
//modelar silencios con codigo  (nivel 5)
//video streaming % pkt correctos +95%  ( nivel  4 )
//data con cuatro categorías
//best effort 0 , bulk 1 , interactive 4, critical depende
//Queuing guidelinesalso included not provisioning more than 33% of a link for realtime traffic and 25% for best effort class

//best effort  , HTTP WEB , NON CRITICAL
//BULK database syscs, email(smtp), large ftp, network backups  (invoke TCP congestion) average message size 64kb or grater
//Transactional  SAP, ORACLE,.... CLIENT-SERVER model   ,   throw 1kb to 50mb
// interactive  telnet,AOL instant messenger  Average message < 100b ; max message < 1kb

//scavenger class , less than best effort , usually entertainment, like p2p media, gamming, or video (youtube)

// cisco /solutions /enterprise/ Wan and MAN/ QoS_SRND/QoS_SRND-Book/QoSIntro.html


#include <ns3/core-module.h>
#include "Prioridad.h"
//#include "ns3/test.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/ipv4-queue-disc-item.h"
#include "ns3/ipv6-queue-disc-item.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/object-factory.h"
#include "ns3/socket.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Prioridad");

//Constructor
Prioridad::Prioridad ()
{
  NS_LOG_FUNCTION_NOARGS ();
  DoRun();
}

void
Prioridad::TestTosValue (Ptr<PfifoFastQueueDisc> queue, uint8_t tos, uint32_t band)
{
  NS_LOG_FUNCTION ("queue: " <<  queue << " tos: "<< tos << " band: " << band);

  Ptr<Packet> p = Create<Packet> (100);
  Ipv4Header ipHeader;
  ipHeader.SetPayloadSize (100);
  ipHeader.SetTos (tos);
  ipHeader.SetProtocol (6);
  SocketPriorityTag priorityTag;
  priorityTag.SetPriority (Socket::IpTos2Priority (tos));
  p->AddPacketTag (priorityTag);
  Address dest;
  Ptr<Ipv4QueueDiscItem> item = Create<Ipv4QueueDiscItem> (p, dest, 0, ipHeader);
  queue->Enqueue (item);

  NS_LOG_DEBUG("Encolando:   " << item << " con tos : " << (uint32_t)tos );
  //NS_LOG_DEBUG("Mostrando peek de cola: " << queue->Peek() );
  //NS_TEST_ASSERT_MSG_EQ (queue->GetInternalQueue (band)->GetNPackets (), 1, "enqueued to unexpected band");
  //queue->Dequeue ();
  //NS_TEST_ASSERT_MSG_EQ (queue->GetInternalQueue (band)->GetNPackets (), 0, "unable to dequeue");
}

void
Prioridad::DoRun (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  queueDisc = CreateObject<PfifoFastQueueDisc> ();
  for (uint16_t i = 0; i < 3; i++)
    {
      Ptr<DropTailQueue> queue = CreateObject<DropTailQueue> ();
    //  bool ok = queue->SetAttributeFailSafe ("MaxPackets", UintegerValue (1000));
    //  NS_LOG_DEBUG("Max Packets " << ok);
    //  NS_TEST_ASSERT_MSG_EQ (ok, true, "unable to set attribute");
      queueDisc->AddInternalQueue (queue);
    }
//  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetInternalQueue (0)->GetNPackets (), 0, "initialized non-zero");
//  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetInternalQueue (1)->GetNPackets (), 0, "initialized non-zero");
//  NS_TEST_ASSERT_MSG_EQ (queueDisc->GetInternalQueue (2)->GetNPackets (), 0, "initialized non-zero");

  //                                     // Service name       priority         band
  // TestTosValue (queueDisc, 0x00, 1);  // Normal service  -> Best Effort (0) -> 1
  // TestTosValue (queueDisc, 0x02, 1);  // MMC             -> Best Effort (0) -> 1
  // TestTosValue (queueDisc, 0x04, 1);  // MR              -> Best Effort (0) -> 1
  // TestTosValue (queueDisc, 0x06, 1);  // MMC+MR          -> Best Effort (0) -> 1
  // TestTosValue (queueDisc, 0x08, 2);  // Max. Throughput -> Bulk (2)        -> 2
  // TestTosValue (queueDisc, 0x0a, 2);  // MMC+MT          -> Bulk (2)        -> 2
  // TestTosValue (queueDisc, 0x0c, 2);  // MR+MT           -> Bulk (2)        -> 2
  // TestTosValue (queueDisc, 0x0e, 2);  // MMC+MR+MT       -> Bulk (2)        -> 2
  // TestTosValue (queueDisc, 0x10, 0);  // Minimize Delay  -> Interactive (6) -> 0
  // TestTosValue (queueDisc, 0x12, 0);  // MMC+MD          -> Interactive (6) -> 0
  // TestTosValue (queueDisc, 0x14, 0);  // MR+MD           -> Interactive (6) -> 0
  // TestTosValue (queueDisc, 0x16, 0);  // MMC+MR+MD       -> Interactive (6) -> 0
  // TestTosValue (queueDisc, 0x18, 1);  // MT+MD           -> Int. Bulk (4)   -> 1
  // TestTosValue (queueDisc, 0x1a, 1);  // MMC+MT+MD       -> Int. Bulk (4)   -> 1
  // TestTosValue (queueDisc, 0x1c, 1);  // MR+MT+MD        -> Int. Bulk (4)   -> 1
  // TestTosValue (queueDisc, 0x1e, 1);  // MMC+MR+MT+MD    -> Int. Bulk (4)   -> 1


//1º  TestTosValue (queueDisc, 0x16, 0);  // MMC+MR+MD       -> Interactive (6) -> 0
//2º TestTosValue (queueDisc, 0x1e, 1);  // MMC+MR+MT+MD    -> Int. Bulk (4)   -> 1
//3º  TestTosValue (queueDisc, 0x06, 0);  // MMC+MR+MT       -> Best Effort(0)      -> 1
//4º  TestTosValue (queueDisc, 0x0e, 0);  // MMC+MR+MT       -> Bulk (2)        -> 2

TestTosValue (queueDisc, 0x1e, 0);  // MMC+MR+MT+MD    -> Int. Bulk (4)   -> 1


  TestTosValue (queueDisc, 0x16, 0);  // MMC+MR+MD       -> Interactive (6) -> 0
  TestTosValue (queueDisc, 0x16, 0);  // MMC+MR+MD       -> Interactive (6) -> 0
  TestTosValue (queueDisc, 0x16, 0);  // MMC+MR+MD       -> Interactive (6) -> 0

  TestTosValue (queueDisc, 0x0e, 0);  // MMC+MR+MT       -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x0e, 0);  // MMC+MR+MT       -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x0e, 0);  // MMC+MR+MT       -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x0e, 0);  // MMC+MR+MT       -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x0e, 0);  // MMC+MR+MT       -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x0e, 0);  // MMC+MR+MT       -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x06, 0);  // MR              -> Best Effort (0) -> 1

  TestTosValue (queueDisc, 0x06, 0);  // Max. Throughput -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x06, 0);  // MMC+MT          -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x0e, 0);  // MMC+MR+MT       -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x06, 0);  // MR+MT           -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x06, 0);  // MMC+MR+MT       -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x06, 0);  // Normal service  -> Best Effort (0) -> 1
  TestTosValue (queueDisc, 0x06, 0);  // MMC             -> Best Effort (0) -> 1
  TestTosValue (queueDisc, 0x06, 0);  // MR              -> Best Effort (0) -> 1TestTosValue (queueDisc, 0x08, 0);  // Max. Throughput -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x0e, 0);  // MMC+MT          -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x0e, 0);  // MR+MT           -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x06, 0);  // MR              -> Best Effort (0) -> 1
  TestTosValue (queueDisc, 0x06, 0);  // Normal service  -> Best Effort (0) -> 1
  TestTosValue (queueDisc, 0x06, 0);  // MMC             -> Best Effort (0) -> 1
  TestTosValue (queueDisc, 0x06, 0);  // MR              -> Best Effort (0) -> 1

  NS_LOG_DEBUG("Comenzamos a sacar paquetes de la cola (tam: " << queueDisc->GetNPackets() << ")" );
  //Se sacan primero los de menor banda, y en la misma banda los de menor prioridad
  uint32_t tamCola = queueDisc->GetNPackets();
  Ptr <QueueItem> queueItem;
  for(uint32_t j = 0; j < tamCola ; j++){
    queueItem = queueDisc -> Dequeue ();
    NS_LOG_DEBUG("Desencolado: " << queueItem);
  }

  TestTosValue (queueDisc, 0x08, 0);  // Max. Throughput -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x0a, 0);  // MMC+MT          -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x0c, 0);  // MR+MT           -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x0e, 0);  // MMC+MR+MT       -> Bulk (2)        -> 2
  TestTosValue (queueDisc, 0x00, 0);  // Normal service  -> Best Effort (0) -> 1
  TestTosValue (queueDisc, 0x02, 0);  // MMC             -> Best Effort (0) -> 1
  TestTosValue (queueDisc, 0x04, 0);  // MR              -> Best Effort (0) -> 1

  NS_LOG_DEBUG("Comenzamos a sacar paquetes de la cola (tam: " << queueDisc->GetNPackets() << ")" );
  //Se sacan primero los de menor banda, y en la misma banda los de menor prioridad
  tamCola = queueDisc->GetNPackets();
  for(uint32_t j = 0; j < tamCola ; j++){
    queueItem = queueDisc -> Dequeue ();
    NS_LOG_DEBUG("Desencolado: " << queueItem);
  }


}
void
Prioridad::Test ()
{
  NS_LOG_FUNCTION_NOARGS ();
//  AddTestCase (new PfifoFastQueueDiscTosPrioritization, TestCase::QUICK);
}
