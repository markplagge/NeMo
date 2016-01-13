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
	short sigma[4];
	short S[4];
	bool b[4];
	bool epsilon = 0;
	bool sigma_l = 0;
	short lambda = 2;
	bool c = false;
	short TM = 0;
	short VR = -15;
	short sigmaVR = -1;
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
/**
 crTonicBursting - Creates a tonic bursting set of neurons. Needs to be called
 twice, as the model requires two neurons. Combine this with the function 
 crTonicBurstingAxon(). Once called twice, it will reset the internal counter,
 so multiple sets of these can be created.
 
 Table of values:
 | j | Gi         | ε | λ | c | α  | β | M | R | κ | 𝛾 |
 |---|------------|---|---|---|----|---|---|---|---|----|
 | 0 | 1,-100,0,0 | 1 | 1 | 0 | 18 | 0 | 0 | 1 | 1 | 0  |
 | 1 | 1,0,0,0    | 1 | 0 | 0 | 6  | 0 | 0 | 0 | 1 | 0  |
 

 */
void crTonicBursting(neuronState *s, tw_lp *lp){
    static int created = 0;
    s->isSelfFiring = true;
    short *G_i = calloc(sizeof(short), NEURONS_IN_CORE);
    bool *synapticConnectivity = calloc(sizeof(bool),NEURONS_IN_CORE);
    for (int i = 0; i < NEURONS_IN_CORE; i ++) {
        G_i[i] = 2;
    }
    short sigma[4] = {1,1,1,1};
    short S_j[4] = {0,0,0,0};
    //int st[4] = {0,0,0,0};
    bool B[4] = {0,0,0,0};
    // Keep track of the number of times we've run this
    // two neurons needed.
    
    
    //j is zero, first neuron:
    if(created == 0) {

        //TODO: Check to see if the synaptic mode is correct in the paper.
        G_i[0] = 0;
        G_i[1] = 1;
		G_i[2] = 0;
        G_i[3] = 0;

        S_j[0] = 1;
        S_j[1] = 100;
        
        int epsilon = 1;
        short lambda = 1;
        int c = 0;
        uint32_t posThreshold = 18;
        uint32_t negThreshold = 0;
        int thresholdPRNMask = 0;
        int resetVoltage = 1;
        int kappa = 1;
        int gamma = 0;
        synapticConnectivity[0] = 1;
        synapticConnectivity[1] = 1;

        
        sigma[0] = 1;
        sigma[1] = -1;
        sigma[2] = 1;
        sigma[3] = 1;
        tw_lpid dest = lGetAxonFromNeu(lGetCoreFromGID(lp->gid), 2); //output to axon 2
        
        //Sigma_l is the reset voltage sign. Here it is positive,
        //so I'm passing a 1 in the constructor directly.
        
        initNeuronEncodedRV(lGetCoreFromGID(lp->gid), lGetNeuNumLocal(lp->gid),
                   synapticConnectivity, G_i, sigma, S_j, B, epsilon,  1,
                   lambda, c, posThreshold, negThreshold, thresholdPRNMask,
                   resetVoltage, 1,
                   gamma, kappa, s, 0, dest, 2);
        
        s->neuronTypeDesc = "TONIC_BURSTING_0";
        created ++;
    
    
    }else if ( created == 1) {
        
        //second neuron, synapse type 0 has weight 1
        G_i[2] = 0; //axon 0 is type 0
        S_j[0] = 1; //type 0 has weight of 1
        int epsilon = 1;

        int lambda = 0;
        int c = 0;
        uint32_t posThreshold = 6;
        uint32_t negThreshold = 0;
        int thresholdPRNMask = 0;
        int resetVoltage = 0;
        int kappa = 1;
        int gamma = 0;
        
        synapticConnectivity[2] = 1;
        
        sigma[0] = 1;
        sigma[1] = 1;
        sigma[2] = 1;
        sigma[3] = 1;
        tw_lpid dest = lGetAxonFromNeu(lGetCoreFromGID(lp->gid), 1); //output to axon 1
        
        
        
        initNeuron(lGetCoreFromGID(lp->gid), lGetNeuNumLocal(lp->gid),
                   synapticConnectivity, G_i, sigma, S_j, B, epsilon,  1,
                   lambda, c, posThreshold, negThreshold, thresholdPRNMask,
                   resetVoltage, 1,
                   gamma, kappa, s, 0, dest, 1);
        s->neuronTypeDesc = "TONIC_BURSTING_1";
        created ++;

    }
    
    free (synapticConnectivity);
    free(G_i);

}
void crPhasicAxon(axonState *s, tw_lp *lp){
	static int created = 0;

		s->axtype = "ax_phasic";
		s->sendMsgCount = 0;
		s->axonID = lGetAxeNumLocal(lp->gid);
		s->destSynapse = lGetSynFromAxon(lp->gid);
	if (created == 0) {
		//Queue up events for the phasic spiker
		for (int i = 10; i < g_tw_ts_end; i += 1) {
			tw_stime evtTime = i + tw_rand_unif(lp->rng);
			tw_event *axe = tw_event_new(lp->gid, evtTime, lp);
			Msg_Data *data = (Msg_Data *) tw_event_data(axe);
			data->eventType = AXON_OUT;
			data->axonID = s->axonID;
			tw_event_send(axe);
		}
		created ++;
	}

}
void crTonBurstAxeEvent(axonState *s, tw_lp *lp, long i) {

	tw_stime evtTime = i + tw_rand_unif(lp->rng);
	tw_event *axe = tw_event_new(lp->gid, evtTime, lp);

	Msg_Data *data = (Msg_Data *)tw_event_data(axe);
	data->eventType = AXON_OUT;
	data->axonID = s->axonID;
	tw_event_send(axe);
}
void crTonicBurstingAxon(axonState *s, tw_lp *lp){
	static int num = 0;
	s->axtype = "norm";


	s->sendMsgCount  = 0;
	s->axonID = lGetAxeNumLocal(lp->gid);
	s->destSynapse = lGetSynFromAxon(lp->gid);
	if(num < 1) {
		s->axtype = "ax_tonic_bursting";

		//Queue up events for the tonic bursting axon.
		//Start at the first big tick
		for (long i = 100; i < g_tw_ts_end; i += 300) {
			crTonBurstAxeEvent(s,lp,i);

		}

		num ++;
	}

}

void crBioLoopback(neuronState *s, tw_lp *lp){


}

