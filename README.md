# *NeMo2*: A Generic Neuromorphic Hardware Simulation Model built on top of ROSS

NeMo 2 is a neuromorphic hardware simulation model. This model uses ROSS (the Rensselaer Optimistic Simulation System)
as its backend. NeMo2 is a feature-enhanced version of NeMo:
* Synapses are collapsed into a single simulation element, increasing efficiency
* Models are read in using the IBM TrueNorth spiking system's JSON output
* New saturation and other synthetic benchmarks are available.


Currently, scheduler algorithms and energy-modeling are being implemented. Further work is in progress to bring 
C++ neuron models into the sytstem, and integrating these neurons with CODES to provide an end-to-end heterogenious 
HPC simulation system.


Requires:
* sqlite3
* C++ 11 compatible compiler
* MPI (Mpich gives better performance, but OpenMPI will work as well)

_Tested on PPC64-LE with IBM-XL C / Clang, X86-64 with GCC-8 & Clang 7, OSX Mojave_
