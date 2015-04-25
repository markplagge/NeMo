//The C driver file for a ROSS model
//This file includes:
// - initialized command line arguments
// - an initialization function
// - a forward event function
// - a reverse event function
// - a finalization function
// - all custom mapping functions (if needed)
// - the LP type definition 

//Includes
#include <stdio.h>

#include "ross.h"
#include "model.h"

//Initialize command line arguments
unsigned int setting_1 = 0;

//Helper Functions
void SWAP (double *a, double *b) {
  double tmp = *a;
  *a = *b;
  *b = tmp;
}


//Init function
// - called once for each LP
// ! LP can only send messages to itself during init !
void model_init (state *s, tw_lp *lp) {
  int self = lp->gid;

  // init state data
  s->rcvd_count_H = 0;
  s->rcvd_count_G = 0;
  s->value = -1;

  // Init message to myself
  tw_event *e = tw_event_new(self, 1, lp);
  message *msg = tw_event_data(e);
  msg->type = HELLO;
  msg->contents = tw_rand_unif(lp->rng);
  msg->sender = self;
  tw_event_send(e);
}

//Forward event handler
void model_event (state *s, tw_bf *bf, message *in_msg, tw_lp *lp) {
  int self = lp->gid;

  // initialize the bit field
  *(int *) bf = (int) 0;

  // update the current state
  // however, save the old value in the 'reverse' message
  SWAP(&(s->value), &(in_msg->contents));

  // handle the message
  switch (in_msg->type) {
    case HELLO :
    {
      s->rcvd_count_H++;
      break;
    }
    case GOODBYE :
    {
      s->rcvd_count_G++;
      break;
    }
    default :
      printf("Unhandeled forward message type %d\n", in_msg->type);
  }

  tw_event *e = tw_event_new(self, 1, lp);
  message *msg = tw_event_data(e);
  //# randomly choose message type
  double random = tw_rand_unif(lp->rng);
  if (random < 0.5) {
    msg->type = HELLO;
  } else {
    msg->type = GOODBYE;
  }
  msg->contents = tw_rand_unif(lp->rng);
  msg->sender = self;
  tw_event_send(e);
}

//Reverse Event Handler
void model_event_reverse (state *s, tw_bf *bf, message *in_msg, tw_lp *lp) {
  int self = lp->gid;

  // undo the state update using the value stored in the 'reverse' message
  SWAP(&(s->value), &(in_msg->contents));

  // handle the message
  switch (in_msg->type) {
    case HELLO :
    {
      s->rcvd_count_H--;
      break;
    }
    case GOODBYE :
    {
      s->rcvd_count_G--;
      break;
    }
    default :
      printf("Unhandeled reverse message type %d\n", in_msg->type);
  }

  // don't forget to undo all rng calls
  tw_rand_reverse_unif(lp->rng);
  tw_rand_reverse_unif(lp->rng);
}

//report any final statistics for this LP
void model_final (state *s, tw_lp *lp){
  int self = lp->gid;
  printf("%d handled %d Hello and %d Goodbye messages\n", self, s->rcvd_count_H, s->rcvd_count_G);
}

/*
//Custom mapping fuctions are used so
// - no LPs are unused
// - event activity is balanced

extern unsigned int nkp_per_pe;
//#define VERIFY_MAPPING 1 //useful for debugging

//This function maps LPs to KPs on PEs and is called at the start
//This example is the same as Linear Mapping
void model_custom_mapping(void){
  tw_pe *pe;
  int nlp_per_kp;
  int lp_id, kp_id;
  int i, j;
  
  // nlp should be divisible by nkp (no wasted LPs)
  nlp_per_kp = ceil((double) g_tw_nlp / (double) g_tw_nkp);
  if (!nlp_per_kp) tw_error(TW_LOC, "Not enough KPs defined: %d", g_tw_nkp);
  
  //gid of first LP on this PE (aka node)
  g_tw_lp_offset = g_tw_mynode * g_tw_nlp;
  
#if VERIFY_MAPPING
  prinf("NODE %d: nlp %lld, offset %lld\n", g_tw_mynode, g_tw_nlp, g_tw_lp_offset);
#endif
  
  // Loop through each PE (node)
  for (kp_id = 0, lp_id = 0, pe = NULL; (pe = tw_pe_next(pe)); ) {
    
    // Define each KP on the PE
    for (i = 0; i < nkp_per_pe; i++, kp_id++) {
      
      tw_kp_onpe(kpid, pe);
      
      // Define each LP on the KP
      for (j = 0; j < nlp_per_kp && lp_id < g_tw_nlp; j++, lp_id++) {
	
	tw_lp_onpe(lp_id, pe, g_tw_lp_offset + lp_id);
	tw_lp_onkp(g_tw_lp[lp_id], g_tw_kp[kp_id]);
	
#if VERIFY_MAPPING
	if (0 == j % 20) { // print detailed output for only some LPs
	  printf("PE %d\tKP %d\tLP %d\n", pe->id, kp_id, (int) lp_id + g_tw_lp_offset);
	}
#endif
      }
    }
  }
  
  //Error checks for the mapping
  if (!g_tw_lp[g_tw_nlp - 1]) {
    tw_error(TW_LOC, "Not all LPs defined! (g_tw_nlp=%d)", g_tw_nlp);
  }
  
  if (g_tw_lp[g_tw_nlp - 1]->gid != g_tw_lp_offset + g_tw_nlp - 1) {
    tw_error(TW_LOC, "LPs not sequentially enumerated");
  }
}

//Given a gid, return the local LP (global id => local id mapping)
tw_lp * model_mapping_to_lp(tw_lpid){
  int local_id = lp_id - g_tw_offset;
  return g_tw_lp[id];
}
*/

//Given a gid, return the PE (or node) id
tw_peid model_map(tw_lpid gid){
  return (tw_peid) gid / g_tw_nlp;
}
//*/

//This defines the fuctions used by the LPs
//   multiple sets can be defined (for multiple LP types)
tw_lptype model_lps[] = {
  {
    (init_f) model_init,
    (pre_run_f) NULL,
    (event_f) model_event,
    (revent_f) model_event_reverse,
    (final_f) model_final,
    (map_f) model_map,
    sizeof(state)
  },
  { 0 },
};
