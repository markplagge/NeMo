//
// Created by Neil McGlohon on 8/9/16.
//

#include "lif_neuron.h"


void LIFFire(tn_neuron_state *st, void *l);

void LIFIntegrate(id_type snyapseID, lif_neuron_state *st, void *lp);

bool LIFReceiveMessage(lif_neuron_state *st, messageData *mes, tw_lp *lp, tw_bf *bf);

void LIFReceiveReverseMessage(lif_neuron_state *st, messageData *mes, tw_lp *lp, tw_bf *bf);

void LIFShouldFire(lif_neuron_state *st, tw_lp *lp);

bool LIFFireFloorCeilingReset(lif_neuron_state *st, tw_lp *lp);

void LIFNumericLeakCalc(lif_neuron_state *st, tw_stime now);

void LIFLinearLeak(lif_neuron_state *st, tw_stime now);

void LIFSendHeartbeat(lif_neuron_state *st tw_stime time, void *lp);
