//
// Created by Mark Plagge on 5/25/16.
//

#include "IOStack.h"

FILE *inputFile;
bool inputFileOpen;

uint64_t interleave(uint32_t time, uint32_t axid) {
  return combine(time, axid);
}

void saveEvent(tw_stime timestamp, char sourceType, id_type core, id_type local,
               id_type destCore, id_type destLocal) {
  printf("\n save event from IOStack.c - Called. \n ");

}

