//
// Created by Mark Plagge on 5/25/16.
//

#include "IOStack.h"
#include "../nemo_config.h"
#include <iostream>
#include <fstream>

FILE *weightsFP;
FILE *dataFP;


void prGetWeights(int *weights, long long coreID, long long neuronID){
    std::ifstream weightsFile(PR_WEIGHT_FILE);

//   static int isWOpen = 0;
//    if (!isWOpen) {
//        weightsFP = fopen(PR_WEIGHT_FILE, "r");

//        isWOpen = 1;
 //  }
    //get the data from the file


}
FILE * inputFile;
bool inputFileOpen;

















void saveEvent(tw_stime timestamp, char sourceType, id_type core, id_type local,
               id_type destCore, id_type destLocal){
	

	
}

