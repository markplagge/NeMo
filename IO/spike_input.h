//
//  spike_input.h
//  ROSS_TOP
//
//  Created by Mark Plagge on 4/5/16.
//
//

#ifndef spike_input_h
#define spike_input_h

#include <stdio.h>


/** IOMapInfo - A struct that defines mapping - so that files can be split.
 	This should be populated with the running simulation's mapping info.
 */
typedef struct IOMapInfo {
    long cores_in_sim;
    long cores_per_pe;
    long neurons_in_core;
}ioMapInfo;




/** createFiles - creates a series of files based on the mapping of the simulation. Expects
 *	to find a spikes.csv file in the running directory. Given the number of cores per PE,
 *	this function splits the file into seperate spike files so that each PE can access it's own
 *	CSV file independantly. Saves the files as spikes_nnnn.csv, where nnnn is the PE ID where the
 *	spikes are located. 
    @returns Integer - zero if ok, otherwise -1.
 */
int createFiles(ioMapInfo map);


/** initSpikeInput - main entry point for the spike input system. This function takes
 * information about the current simulation and uses that to split the specified 
 * input spike file into chunks for each core. Rather than have the main PE read in the
 * whole file and schedule events at the beginning of the sim, the input system will
 * have each PE load in it's input file and process the events independently. 
 @returns Integer - zero if ok, otherwise -1.
 * @TODO Is that needed? Could one PE read the entire file in and then schedule events?
 */
int initSpikeInput(ioMapInfo map, char* spikeFN);



#endif /* spike_input_h */
