

#include <ns3/core-module.h>
#include "ns3/pfifo-fast-queue-disc.h"

using namespace ns3;

class Prioridad {
public:
	//Construcor
	Prioridad();
  void  Test ();
  void  TestTosValue (Ptr<PfifoFastQueueDisc> queue, uint8_t tos, uint32_t band);
  void  DoRun (void);
private:
	//Variable donde guardamos los bits utiles transmitidos
	uint64_t bits_utiles;
	Ptr<PfifoFastQueueDisc> queueDisc;
};
