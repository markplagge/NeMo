//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_IOSTACK_H
#define NEMO_IOSTACK_H
#include <stdio.h>
#include <stdlib.h>
#include "../globals.h"





void initFiles();
void closeFiles();

void saveEvent(tw_stime timestamp, char sourceType, id_type core, id_type local,
               id_type destCore, id_type destLocal);



void saveNeuronFire(tw_stime timestamp, id_type core, id_type local, tw_lpid destGID);


#endif //NEMO_IOSTACK_H
