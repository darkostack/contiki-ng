#include "contiki.h"
#include "sys/node-id.h"
#include "net/linkaddr.h"

uint16_t node_id = 0;

void node_id_init(void)
{
  node_id = linkaddr_node_addr.u8[LINKADDR_SIZE - 1] + (linkaddr_node_addr.u8[LINKADDR_SIZE - 2] << 8);
}
