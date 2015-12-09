//
//  bioModel.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 11/11/15.
//
//

#include "bioModel.h"
#include "mapping.h"


void crPhasic(neuronState *s, tw_lp *lp){
    bool synapticConnectivity[NEURONS_IN_CORE];
	short G_i[NEURONS_IN_CORE];
	int sigma[4];
	int S[4];
	bool b[4];
	bool epsilon = 0;
	bool sigma_l = 0;
	short lambda = 2;
	bool c = false;
	short TM = 0;
	short VR = 0;
	short sigmaVR = 1;
	short gamma = 0;
	bool kappa = true;
    
    int alpha = 2;
    int beta = 10;
    S[0] = 4;
    S[1] = 20;
    S[2] = 0;
    S[3] = 0;
    s->synapticWeight[0]= 4;
    s->synapticWeight[1]= 20;
    s->synapticWeight[2]= 0;
    s->synapticWeight[3]= 0;
    sigma[0] = 1;
    sigma[1] = 1;
    sigma[2] = 1;
    sigma[3] = 1;
    int signalDelay = 1;
    
    for (int i = 0; i < NEURONS_IN_CORE; i ++) {
        synapticConnectivity[i] = 0;
        G_i[i] = 3;
		
    }
	G_i[0] = 0;


    //! According to the paper, there is one input, of type 0.
    
    synapticConnectivity[0] = 1;
    

    
    initNeuron(lGetCoreFromGID(lp->gid), lGetNeuNumLocal(lp->gid), synapticConnectivity,
               G_i, sigma, S, b, epsilon, sigma_l, lambda, c, alpha, beta,
               TM, VR, sigmaVR, gamma, kappa, s, signalDelay, lGetCoreFromGID(lp->gid),
               lGetNeuNumLocal(lp->gid));
    s->neuronTypeDesc = "PHASIC";


}
void crPhasicAxon(axonState *s, tw_lp *lp){
	s->axtype = "ax_phasic";
	s->sendMsgCount = 0;
	s->axonID = lGetAxeNumLocal(lp->gid);
	s->destSynapse = lGetSynFromAxon(lp->gid);
		//Queue up events for the phasic spiker
	for (int i = 1; i < g_tw_ts_end; i ++) {
		tw_stime evtTime = i + tw_rand_unif(lp->rng);
		tw_event *axe = tw_event_new(lp->gid, evtTime, lp);
		Msg_Data *data = (Msg_Data *)tw_event_data(axe);
		data->eventType = AXON_OUT;
		data->axonID = s->axonID;
		tw_event_send(axe);
	}

}


void crBioLoopback(neuronState *s, tw_lp *lp){
    
    
}

