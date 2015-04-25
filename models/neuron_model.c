//
// Created by plaggm on 3/18/15.
//

#include <assist.h>
#include <stdbool.h>
#include "neuron_model.h"

void noLeak(void *neuronState, tw_stime end) {
   //do nothing!!
}
void revNoLeak(void *neuronState, tw_stime now){
		//do nothing!!
}
void linearLeak(void *neuronState, tw_stime end){
		//linear leak function. The rate is determined by the leak rate in the neuron.
	struct NeuronModel *s = (struct NueronModel *) neuronState;
	tw_stime delta = end - s->lastLeakTime;
	s->cVoltage -= s->leakRate * delta;
	s->lastLeakTime = end;
	
}
void synapticLeak(void *neuronState, tw_stime end) {
//	struct NeuronModel *s = (struct NeuronModel *) neuronState;
//	tw_stime delta = end - s->lastLeakTime;
//	s->cVoltage += 
}
void revLinearLeak(void *neuronState, tw_stime now){

	struct NeuronModel *s =(struct NeuronModel *) neuronState;
	tw_stime delta = s->lastLeakTime - now;
	s->cVoltage += s->leakRate * delta;
	s->lastLeakTime = now;
}

/**
* Neuron Post-Fire reset functions:
*/
void resetZero(void *neuronState) {
   //State change happens here:

   struct NeuronModel *s = (struct NeuronModel *) neuronState;
   //ALL neuron functions called AFTER neuron msg rcvd and state saved.
   //s->prVoltage = s->cVoltage; // store current voltage in previous voltage holder.
	s->prVoltage = s->cVoltage;
   s->cVoltage = 0; // set current voltage to 0.

}

void resetLinear(void *neuronState) {
	struct NeuronModel *s = (struct NeuronModel *) neuronState;
		//reduce the value of the neuron based on the linear reduction function
		//in the paper
		//s->cVoltage = s->cVoltage - s->resetVoltParam;

}

	/**
	 Neuron Post-Fire Reverse functions
	 */
void reverseLinear(void *neuronState) {
	struct NeuronModel *s = (struct NeuronModel *) neuronState;

	s->cVoltage = s->cVoltage + s->resetVoltParam;
}

void reverseZero(void *neuronState) {
	struct NeuronModel *s = (struct NeuronModel *) neuronState;
	s->cVoltage = s->prVoltage;
}


/**
* neuronReceiveMessage - a function that is the primary neuron message recipt handler.
* @params time The current timestamp (event timestamp)
* @params st The state of the neuron
* @params Msg_Data current message. Internal values will store this neuron's previous state.
* @returns bool  A bool, true if the neuron has fired.
*/
bool neuronReceiveMessage(neuronState *st, tw_stime time, Msg_Data *m,tw_lp *lp) {
   bool didFire = 0;
   //prep the rotors ( reverse functions )
   //TODO: Move previous voltage to message.
   st->prVoltage = st->cVoltage;
   m->prevVoltage = st->cVoltage;
   //prep complete. Now leaking based on time lapse:

   st->leak(st, time); //leak function call - do leak based on time passed since last communication.


   //apply weights & adjust our voltage:


   _neVoltType adjustedWeight;
   if (st->perSynapseDet[m->senderLocalID] == true) {
	  adjustedWeight = st->perSynapseWeight[m->senderLocalID];
	  st->cVoltage += adjustedWeight;

		   //next check for fire operations:
	   switch (st->fireMode) {
		   case NFM:
		   default:
			   if (st->cVoltage >= st->threshold){
				   didFire = true;
				   neuronFire(st, time, m);
			   }
	   }
   }
   else {
		   //Stochastic fire mode:
		   //from paper:
	   /*For each neuron, each synaptic weight and leak has a configuration bit, bGij and cλ j respectively, where setting the bit to 0 selects deterministic mode, and 1 selects stochastic mode. For stochastic synaptic and leak integration, operation is as follows. 
		Every time a valid synaptic or leak event occurs, the neuron draws a uniformly distributed random number ρj. If the synaptic weight sGij or leak weight λj is greater than or equal to the drawn random number ρj, then the neuron integrates{−1,+1} otherwise, it does not integrate. */
		   //This implementation uses
	   //if(st->perSynapseWeight[m->senderLocalID] >=  tw_rand_unif(lp->rng)){
			   //fire here:
		   didFire=true;
		   neuronFire(st, time, m);
	  	}
//   neuronFire(st, time, m); // call to fire function, however, not needed ATM.

   if(didFire) {
      neuronPostFire(st, time, m);
      
   }

   return didFire;
}

/** neuronPostFire - Function that cleans up the neuron state after firing. */

void neuronPostFire(neuronState *st, tw_stime time, Msg_Data *m) {
   st->lastActiveTime = time;
	
   st->doReset(st); // that may be a little fugly, but it does allow swapping of behaviors at runtime.

}

/** neuronFire Function called after firing status is determined to be true.
* Actual message is managed through model_main.c. This function adjusts parameters for tracking neuron behaviors.
*
* */
void neuronFire(neuronState *st, tw_stime time, Msg_Data *m) {
   st->lastActiveTime=time;
   st->fireCount ++;
}
