//
// Created by Mark Plagge on 4/25/17.
//

#include "IOStack.h"
#include "../neuro/tn_neuron.h"

FILE *membraneFile;
FILE *outSpikeFile;
FILE *outNetworkCfgFile;
FILE *outNeuronCfgFile;

list_t networkStructureList;
list_t membranePotList;

int maxNeuronStruct = 512;
int maxNetworkListSize = 1024;
void openOutputFiles(char *outputFileName) {
//    char ncfgfn[128] = {"\0"};
//    strcat(ncfgfn, "full_");
//    strcat(ncfgfn,outputFileName);
//    strcat(ncfgfn,".bin");
  char netfn[255] = {'\0'};
  printf("LN28\n");
  sprintf(netfn, "network_config_%i.csv", g_tw_mynode);
  printf("LN30\n");
  outNetworkCfgFile = fopen(netfn, "wb");
  printf("LN31\n");
  netfn[0] = '\0';
  sprintf(netfn, "neuron_config%i.csv", g_tw_mynode);
  outNeuronCfgFile = fopen(netfn, "wb");
  printf("LN36\n");
  //fprintf(outNetworkCfgFile, "Core,NeuronID,DestCore,DestAxon\n");
  fprintf(outNeuronCfgFile, "Core,NeuronID");
  for (int i = 0; i < NEURONS_IN_CORE; i++) {
    fprintf(outNeuronCfgFile, ",axon_%i", i);
  }
  fprintf(outNeuronCfgFile, "\n");
  printf("LN43\n");
//    outNeuronCfgFile = fopen(ncfgfn, "wb");

}
void closeOutputFiles() {
  while (list_size(&networkStructureList) > 0) {
    char *line = list_get_at(&networkStructureList, 0);
    fprintf(outNetworkCfgFile, line);
    list_delete_at(&networkStructureList, 0);
  }

  fclose(outNetworkCfgFile);
  fclose(outNeuronCfgFile);
}
void initDataStructures(int simSize) {
  list_init(&networkStructureList);
  list_attributes_copy(&networkStructureList, list_meter_string, 1);

}
/**
 * Writes neuron connections to a char array. Useful for making CSV files.
 * @param n
 * @param state
 */
int neuronConnToSCSV(tn_neuron_state *n, char *state) {
  id_type destCore = getCoreFromGID(n->outputGID);
  id_type destAxon = getAxonLocal(n->outputGID);
  id_type core = n->myCoreID;
  id_type local = n->myLocalID;
  return sprintf(state, "%li,%li,%i,%i\n", core, local, destCore, destAxon);
}
void saveIndNeuron(void *ns) {
  tn_neuron_state *n = (tn_neuron_state *) ns;
  char *line = calloc(sizeof(char), 128);
  neuronConnToSCSV(n, line);
  list_append(&networkStructureList, line);
  if (list_size(&networkStructureList) > maxNetworkListSize) {
    while (list_size(&networkStructureList) > 0) {
      char *line = list_get_at(&networkStructureList, 0);
      fprintf(outNetworkCfgFile, line);
      list_delete_at(&networkStructureList, 0);

    }
  }
}

void saveNetworkStructure() {
  //openOutputFiles("network_def.csv");
  char *lntxt = calloc(sizeof(char), 65535);
  char *lxtxt = calloc(sizeof(char), 65535);
  printf("Starting network save");
  //char lntxt[NEURONS_IN_CORE * CORES_IN_SIM * 2];

  for (int core = 0; core < CORES_IN_SIM; core++) {
    for (int neuron = 0; neuron < NEURONS_IN_CORE / CORES_IN_SIM; neuron++) {
      tw_lpid ngid = getNeuronGlobal(core, neuron);
      tn_neuron_state *n = (tn_neuron_state *) tw_getlocal_lp(ngid);
      sprintf(lntxt, "%llu, %llu", n->myCoreID, n->myLocalID);
      neuronConnToSCSV(n, lxtxt);

      for (int ax = 0; ax < NEURONS_IN_CORE; ax++) {
        sprintf(lntxt, "%s,%llu", lntxt, n->synapticWeight[n->axonTypes[ax]] && n->synapticConnectivity[ax]);
      }
      strcat(lntxt, "\n");

    }
    fprintf(outNetworkCfgFile, "%s", lxtxt);
    fprintf(outNeuronCfgFile, lntxt);
    lntxt[0] = '\0';
    lxtxt[0] = '\0';

    strcat(lntxt, "\n");

  }
  //save the file;

  free(lntxt);
  free(lxtxt);
  //free(lntxt);
  return;
  initDataStructures(g_tw_nlp);

  int neuronsStartAt = AXONS_IN_CORE + 1;
  fprintf(outNetworkCfgFile, "Core,NeuronID,DestCore,DestAxon\n");
  for (int core = 0; core < CORES_IN_SIM /; core++) {
    for (int neuron = 0; neuron < NEURONS_IN_CORE; neuron++) {
      //save neuron connections to CSV file:
      tw_lpid ngid = getNeuronGlobal(core, neuron);
      tn_neuron_state *n = (tn_neuron_state *) tw_getlocal_lp(ngid);

      char *line = tw_calloc(TW_LOC, "SAVE_STRUCTURE", sizeof(char), 512);
      neuronConnToSCSV(n, line);
      list_append(&networkStructureList, line);

    }
    if (list_size(&networkStructureList) > maxNetworkListSize) {
      while (list_size(&networkStructureList) > 0) {
        char *line = list_get_at(&networkStructureList, 0);
        fprintf(outNetworkCfgFile, line);
        list_delete_at(&networkStructureList, 0);

      }
    }
  }
  closeOutputFiles();

}




