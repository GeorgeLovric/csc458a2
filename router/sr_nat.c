
#include <signal.h>
#include <assert.h>
#include "sr_nat.h"
#include <unistd.h>

/*Added by student:*/
#include "sr_if.h"
#include "sr_rt.h"
#include "sr_router.h"
#include "sr_protocol.h"
#include "sr_arpcache.h"
#include "sr_utils.h"
#include "string.h"
#include "stdlib.h"

int sr_nat_init(struct sr_nat *nat) { /* Initializes the nat */

  assert(nat);

  /* Acquire mutex lock */
  pthread_mutexattr_init(&(nat->attr));
  pthread_mutexattr_settype(&(nat->attr), PTHREAD_MUTEX_RECURSIVE);
  int success = pthread_mutex_init(&(nat->lock), &(nat->attr));

  /* Initialize timeout thread */

  pthread_attr_init(&(nat->thread_attr));
  pthread_attr_setdetachstate(&(nat->thread_attr), PTHREAD_CREATE_JOINABLE);
  pthread_attr_setscope(&(nat->thread_attr), PTHREAD_SCOPE_SYSTEM);
  pthread_attr_setscope(&(nat->thread_attr), PTHREAD_SCOPE_SYSTEM);
  pthread_create(&(nat->thread), &(nat->thread_attr), sr_nat_timeout, nat);

  /* CAREFUL MODIFYING CODE ABOVE THIS LINE! */

  nat->mappings = NULL;
  /* Initialize any variables here */
  nat->next_port = 1024;
  nat->next_icmp_id = 1;

  printf("FINISHED NAT INIT \n\n");
  return success;
}


int sr_nat_destroy(struct sr_nat *nat) {  /* Destroys the nat (free memory) */

  pthread_mutex_lock(&(nat->lock));

  /* free nat memory here */

  pthread_kill(nat->thread, SIGKILL);
  return pthread_mutex_destroy(&(nat->lock)) &&
    pthread_mutexattr_destroy(&(nat->attr));

}

void *sr_nat_timeout(void *nat_ptr) {  /* Periodic Timout handling */
  struct sr_nat *nat = (struct sr_nat *)nat_ptr;
  while (1) {
    sleep(1.0);
    pthread_mutex_lock(&(nat->lock));

    time_t curtime = time(NULL);

    /* handle periodic tasks here */
    /* loop through each NAT, compare against below SR variables
    unsigned int icmp_query_timeout;
    unsigned int tcp_est_timeout;
    unsigned int tcp_trans_timeout;
    * 
    * If curtime - time_updated > timeout, remove entry
	*/
    pthread_mutex_unlock(&(nat->lock));
  }
  return NULL;
}

/* Get the mapping associated with given external port.
   You must free the returned structure if it is not NULL. */
struct sr_nat_mapping *sr_nat_lookup_external(struct sr_nat *nat,
    uint16_t aux_ext, sr_nat_mapping_type type ) {

  pthread_mutex_lock(&(nat->lock));

  /* handle lookup here, malloc and assign to copy */
  struct sr_nat_mapping *copy = NULL;
  
  struct sr_nat_mapping *lastMap = nat->mappings;
  
  while ( lastMap && ( lastMap->aux_ext != aux_ext  
  || lastMap->type != type )){
  lastMap = lastMap->next;
  }
  if (!lastMap){
    pthread_mutex_unlock(&(nat->lock));
	  return NULL;  
  }
  memcpy(copy, lastMap, sizeof(struct sr_nat_mapping));

  pthread_mutex_unlock(&(nat->lock));
  return copy;
}

/* Get the mapping associated with given internal (ip, port) pair.
   You must free the returned structure if it is not NULL. */
struct sr_nat_mapping *sr_nat_lookup_internal(struct sr_nat *nat,
  uint32_t ip_int, uint16_t aux_int, sr_nat_mapping_type type ) {

  printf("1a \n");
  pthread_mutex_lock(&(nat->lock));

  /* handle lookup here, malloc and assign to copy. */
  struct sr_nat_mapping *copy = NULL;
  
  printf("1b \n");
  struct sr_nat_mapping *lastMap = nat->mappings;
  
  printf("1c \n");
  while ( lastMap && ( lastMap->ip_int != ip_int ||
	lastMap->aux_int != aux_int 
	|| lastMap->type != type )){
    printf("1cc \n");
  lastMap = lastMap->next;
  }
  if (!lastMap){
    printf("1ccc \n");
    pthread_mutex_unlock(&(nat->lock));
	  return NULL;  
  }

  printf("1d \n");
  memcpy(copy, lastMap, sizeof(struct sr_nat_mapping));

  printf("1d \n");
  pthread_mutex_unlock(&(nat->lock));
  return copy;
}

/* Insert a new mapping into the nat's mapping table.
   Actually returns a copy to the new mapping, for thread safety.
 */
struct sr_nat_mapping *sr_nat_insert_mapping(struct sr_nat *nat,
  uint32_t ip_int, uint16_t aux_int, sr_nat_mapping_type type ) {

  printf("1??? \n");
  pthread_mutex_lock(&(nat->lock));

  printf("1.... \n");
  /* handle insert here, create a mapping, and then return a copy of it */
  struct sr_nat_mapping *mapping = malloc(sizeof(struct sr_nat_mapping));

  struct sr_nat_mapping *mapping_insert = malloc(sizeof(struct sr_nat_mapping));
  
  printf("33333 \n");
  mapping_insert->type = type;
  mapping_insert->ip_int = ip_int;
  mapping_insert->aux_int = aux_int;
  mapping_insert->last_updated = time(NULL);
	  /*hardcode external IP to ETH2 */
  printf("4444 \n");
  /*char name[] = "eth1";
  char *labelPtr = name;
  struct sr_if *sr;
  sr = sr_get_interface(nat->sr, labelPtr);
  mapping_insert->ip_ext = sr->ip;  
  
  */
  /*if (type == nat_mapping_tcp){*/

	  /* Get the next available external Port
	   * 
	   * mapping_insert->aux_ext = ; */
	  /*nat->next_port = get_next_port(nat);
	  
  } else{

	  nat->next_port = get_next_icmp_id(nat);
	  
  }*/

  /*printf("5555 \n");*/

  /* Loop through NAT until you find empty router*/
  
  /* Are we inserting to the front of the list or the back? */
  struct sr_nat_mapping *lastMap = nat->mappings;
  
  /*while (lastMap && lastMap->next) {
    printf("3aa \n");
    lastMap = lastMap->next;
  }
  
  lastMap->next = mapping_insert;*/

  nat->mappings = mapping_insert;
  mapping_insert->next = lastMap;

  memcpy(mapping, mapping_insert, sizeof(struct sr_nat_mapping));

  printf("66666 \n\n");
  
  /* Now that it is inserted, create new timeout thread for it
   * 
   * pthread_create( );
   * https://computing.llnl.gov/tutorials/pthreads/
   * */
  
  pthread_mutex_unlock(&(nat->lock));
  
  
  return mapping;
}

void sr_handle_nat(
	struct sr_instance* sr, 
	uint8_t * packet, 
	unsigned int len, 
	char *iface,
  struct sr_ip_hdr *ip_header)
{
  /* REQUIRES */
  assert(sr);
  assert(packet);
  assert(len);
  
  
  /*Must check if its ICMP vs ARP*/
  printf("Ok, we are about to send a packet, must NATify it first! Here is what is inside the packet:\n");
  sr->nat->sr = sr; /* Ensure that, given NAT, we can find SR again */
  print_hdrs(packet, len);
  
  /* 
  check if ICMP or TCP
  if packet is outgoing (internal -> external):
    lookup unique mapping, and insert if not already in mapping table
  else:
    if no mapping and not a SYN:
      drop packet

  rewrite IP src for outgoing packets, rewrite IP dst for incoming packets
  rewrite ICMP id / TCP port #
  update checksums
  route packet as normal
  */   

  uint8_t protocol = ip_protocol((packet + sizeof(sr_ethernet_hdr_t)));

  struct sr_if *dest_if = sr_get_interface_by_ip(sr, ip_header->ip_dst);

  struct sr_rt *lpm_result = NULL;

  if (dest_if == 0) {
    lpm_result = sr_find_lpm(sr->routing_table, ip_header->ip_dst);
  }

  /** UNTESTED CODE **/
  switch(protocol) {
    case ip_protocol_icmp:
      printf("THE INTERFACE IT ARRIVED ON: %s \n\n", iface);
      if (strcmp("eth1", iface) == 0) {
        printf("**---- INTERNAL INTERFACE ----** \n\n");
        /* arrived on internal interface */
        if (lpm_result != NULL && strcmp("eth1", lpm_result->interface) != 0) {
          /* going to external interface */

          sr_icmp_hdr_t *icmp_header = (sr_icmp_hdr_t *) (packet + sizeof(sr_ethernet_hdr_t) + sizeof(sr_ip_hdr_t));

          printf("1 \n");
          /*if (icmp_header->icmp_type == 8 && icmp_header->icmp_code == 0) {*/
            struct sr_nat_mapping *mapping_result = sr_nat_lookup_internal(sr->nat, ip_header->ip_src, icmp_header->icmp_id, nat_mapping_icmp);

            printf("2 \n");

            if (mapping_result == NULL) {

              printf("3a \n");

              mapping_result = sr_nat_insert_mapping(sr->nat, ip_header->ip_src, icmp_header->icmp_id, nat_mapping_icmp);
              mapping_result->ip_ext = sr_get_interface(sr, lpm_result->interface)->ip;
              mapping_result->aux_ext = get_next_icmp_id(sr->nat);

              printf("3b \n");
            }

            mapping_result->last_updated = time(NULL);

            ip_header->ip_src = mapping_result->ip_ext;
            icmp_header->icmp_id = mapping_result->aux_ext;
            ip_header->ip_sum = calc_ip_cksum(ip_header);
            icmp_header->icmp_sum = calc_icmp_cksum(icmp_header, len - sizeof(sr_ethernet_hdr_t) - sizeof(sr_ip_hdr_t));

            printf("4 \n");
          }
        /*}*/

        sr_handle_regular_IP(sr, packet, len, iface, ip_header);
      } else {
        printf("**---- EXTERNAL INTERFACE ----** \n\n");
        /* arrived on external interface */
        if (dest_if != 0) {
          /* going to internal interface */

          sr_icmp_hdr_t *icmp_header = (sr_icmp_hdr_t *) (packet + sizeof(sr_ethernet_hdr_t) + sizeof(sr_ip_hdr_t));

          if (icmp_header->icmp_type == 0 && icmp_header->icmp_code == 0) {
            struct sr_nat_mapping *mapping_result = sr_nat_lookup_external(sr->nat, icmp_header->icmp_id, nat_mapping_icmp);

            printf("AAAA \n\n");
            if (mapping_result != NULL) {

              ip_header->ip_dst = mapping_result->ip_int;
              icmp_header->icmp_id = mapping_result->aux_int;
              ip_header->ip_sum = calc_ip_cksum(ip_header);
              icmp_header->icmp_sum = calc_icmp_cksum(icmp_header, len - sizeof(sr_ethernet_hdr_t) - sizeof(sr_ip_hdr_t));

              printf("BBBB \n\n");
              sr_handle_regular_IP(sr, packet, len, iface, ip_header);
            }
          }
        } else {
          if (strcmp("eth1", lpm_result->interface) != 0) {
            printf("CCCC \n\n");
            sr_handle_regular_IP(sr, packet, len, iface, ip_header);
          }
        }

      }

      break;

    case ip_protocol_tcp:
      break;

    default:
      break;
  } 

}

uint16_t get_next_icmp_id(struct sr_nat *nat) {
  pthread_mutex_lock(&(nat->lock));

  if (nat->next_icmp_id == MAX_IDS_PORTS){
	  nat->next_icmp_id = 1; /* reset  back to 1? */
  } else {
    nat->next_icmp_id = nat->next_icmp_id + 1;
  }

  uint16_t next_icmp_id = nat->next_icmp_id;

  pthread_mutex_unlock(&(nat->lock));

  printf("NEW ICMP ID: %d \n\n", next_icmp_id);
  return next_icmp_id;
}

uint16_t get_next_port(struct sr_nat *nat) {
  pthread_mutex_lock(&(nat->lock));

  uint16_t next_port = nat->next_port;
  
  nat->next_port = nat->next_port + 1;
  if (nat->next_port == MAX_IDS_PORTS){
	  nat->next_port = 1024; /* reset  back to 1? */
  }

  pthread_mutex_unlock(&(nat->lock));

  return next_port;
}
