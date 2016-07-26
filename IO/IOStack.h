//
// Created by Mark Plagge on 5/25/16.
//

#ifndef NEMO_IOSTACK_H
#define NEMO_IOSTACK_H
#include <stdio.h>
#include <stdlib.h>
#include "../globals.h"

#define addData(data, csv_writer) _Generic(data),\
messageData: addMessage,\
default: addDataStr)(data, csv_writer)

void initFiles();
void closeFiles();

void saveEvent(tw_stime timestamp, char sourceType, id_type core, id_type local,
               id_type destCore, id_type destLocal);



#endif //NEMO_IOSTACK_H
