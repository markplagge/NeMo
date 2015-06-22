//
//  mapping.c
//  ROSS_TOP
//
//  Created by Mark Plagge on 6/18/15.
//
//

#include "mapping.h"



long getCoreFromGID(tw_lpid gid)
{
    return 0;
}


long getCoreFromPE(tw_peid gid)
{
  return 0;
}


long getCoreLocalFromGID(tw_lpid gid)
{
  return 0;
}


tw_lpid globalID(uint_fast32_t core, uint_fast32_t local)
{

   tw_lpid returnVal = 0; //(tw_lpid)calloc(sizeof(tw_lpid) ,1);
   // returnVal = (tw_lpid)core << 32 | (tw_lpid) local;
   ((int32_t*)&returnVal)[lc] = local;
   ((int32_t*)&returnVal)[co] = core;
   return returnVal;

}


tw_lpid getNeuronID(_idT core, uint_fast32_t neuron)
{
return 0;
}


