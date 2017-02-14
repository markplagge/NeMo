#NeMo Data File Schema

**Network Design**

Network layout & design files allow the user to design a custom neuromorphic hardware
network. These files are plaintext files.

A file must have the following parameters:


* ns_cores: The total number of neurosynaptic cores contained within the simulation. *NeMo* simulates a neuromorphic hardware processor, so this setting changes the number of cores contained within a single processor.
* neurons\_per\_core: The total number of neurons per neurosynaptic core. The IBM TrueNorth processor, for example, contains 256 neurons per core. If this value is set to 1, then NeMo will simulate a chip without a neurosynaptic core, though with some caveats. **The number of neurons in the simulation is equal to neurons\_per\_core \* ns_cores.**
* neuron_weight_count: The number of weights each neuron can have. This value, while able to be set to an arbitrary value, should generally be <= neurons\_per\_core, unless the neurons per core value is 1.

After setting these values, the neuron layout can be defined. This layout consists of multiple lines of text, one per user-defined neuron. Any neurons that exist in the simulation that do not have an explicitly configured line in this file are set to a neuron with 0 weights - a disconnected neuron.

Any lines that begin with / are ignored.

The variables for this file are:

* neuron_type: the named type of the neuron. Currently supported are:

	* "tn": the  TrueNorth neuron
	* "LIF": Standard leaky integrate and fire neuron
void tn_create_neuron_encoded_rv(
        id_type coreID, id_type nID, bool synapticConnectivity[NEURONS_IN_CORE],
        short G_i[NEURONS_IN_CORE], short sigma[4], short S[4], bool b[4],
        bool epsilon, short sigma_l, short lambda, bool c, uint32_t alpha,
        uint32_t beta, short TM, short VR, short sigmaVR, short gamma, bool kappa,
        tn_neuron_state* n, int signalDelay, uint64_t destGlobalID,
        int destAxonID);
* neuron parameters: this is a comma separated list of the parameters needed to configure the specified neuron. All parameters specified are required.
	* For the true north neuron, the list is organized as follows (Given a neuron *j*, and axons *i* ):
	 * coreID : The Neurosynaptic core the neuron is located in
	 * neuronID : The local (core based) ID of the neuron.
	 * [w<sub>i,j</sub>]	- A comma seperated list of Neuron _j_'s connectivity to synapse _i_. For example, a fully connected neuron would be: [1,1,1,1,1,1,1,1,1 ... 1,1].
	 * [G<sub>i</sub>] - A comma seperated list - the type of the _i_<sup> _th_</sup> axon.
	 * σ<sub>j</sub><sup>G<sub>i</sub></sup>,σ<sub>j</sub><sup>λ</sup>,σ<sub>j</sub><sup>VR</sup>] : Sign bits
	 * s<sub>j</sub><sup>Gi</sup> : Synaptic Weights
	 * λ<sub>j</sub> : The Leak
	 * bj<sup>Gi</sup> :Selects between deterministic and stochastic integration (can be 1 or 0)
	 * ε<sub>j</sub>: Selects between monotonic and divergent/convergent leak
	 * α<sub>j</sub>: The positive membrane potential threshold
	 * β<sub>jM</sub>: The negative membrane potential threshold
	 * M<sub>j</sub>, TM<sub>j</sub> : The (encoded) threshold psudeorandom number mask - expands to 2<sup>TM</sup> - 1
	 * Vrstj, VRj The (encoded) reset potential VR; expands to 𝜎<sup>VR</sup>(2<sup>VR</sup> -1)


For the True North neuron, an example configuration line would be:
(given 256 neurons per core):
The file must start with the initial simulation parameters first, so first the simulation parameters are given, then a single TrueNorth neuron parameter is shown.

```
/Simulation parameter example;
1024
256
4
"tn",1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,4,4,2,3,1,4,0,4,3,3,0,4,1,1,0,3,0,4,3,0,4,4,1,4,3,4,0,3,4,3,1,4,2,2,2,3,3,2,3,4,4,2,1,0,4,4,2,3,3,2,4,4,4,2,0,0,4,1,1,1,3,0,3,3,3,3,4,0,4,0,0,3,3,2,1,1,2,3,0,0,2,2,2,3,0,0,1,0,0,1,0,4,0,2,4,1,2,2,2,3,0,0,3,0,2,2,4,1,4,1,4,1,2,2,4,4,0,2,4,3,3,1,4,4,3,0,3,4,0,4,1,1,4,3,1,1,2,3,3,1,1,2,3,0,0,1,3,4,0,3,4,3,0,1,1,3,3,3,0,2,4,0,0,0,1,3,1,2,0,4,1,1,1,2,4,0,2,1,4,1,4,1,2,0,4,1,2,3,1,3,2,4,4,1,3,1,0,0,3,2,3,0,2,3,2,0,1,4,0,0,4,2,3,4,4,4,2,1,3,1,4,3,0,1,0,2,1,1,3,4,3,1,3,0,0,4,4,2,0,2,3,2,2,0,0,3,1,4,4,1,1,0,0,4,4,1,-1,1,1,2,3,4,1,0,0,-10,10001,10001,0,0
```
