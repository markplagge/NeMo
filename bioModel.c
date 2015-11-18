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
	short lambda = 0;
	bool c = false;
	short TM = 0;
	short VR = 0;
	short sigmaVR = 1;
	short gamma = 0;
	bool kappa = false;
    int weights[4];
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

    int signalDelay = 1;

	

	s->synapticConnectivity[0] = 1;
		//s->synapticConnectivity[1] = 1;

	s->axonTypes[0] = 0;
		//	s->axonTypes[1] = 0;
    initNeuron(lGetCoreFromGID(lp->gid), lGetNeuNumLocal(lp->gid), synapticConnectivity,
               G_i, sigma, S, b, epsilon, sigma_l, lambda, c, alpha, beta,
               TM, VR, sigmaVR, gamma, kappa, s, signalDelay, lGetCoreFromGID(lp->gid),
               lGetNeuNumLocal(lp->gid));


}



void crBioLoopback(neuronState *s, tw_lp *lp){
    
    
}