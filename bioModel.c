//
//  bioModel.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 11/11/15.
//
//

#include "bioModel.h"

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

	s->epsilon = 0;
	s->lambda = 2;
	s->c = 0;
	s->posThreshold = 2;
	s->negThreshold = 10;
	s->resetVoltage = 15;
	s->kappa = 1;
	s->resetMode=0;
	int weights[4];
	s->synapticWeight[0]= 4;
	s->synapticWeight[1]= 20;
	s->synapticWeight[2]= 0;
	s->synapticWeight[3]= 0;

	s->synapticConnectivity[0] = 1;
		//s->synapticConnectivity[1] = 1;

	s->axonTypes[0] = 0;
		//	s->axonTypes[1] = 0;

}