//
// Created by Mark Plagge on 4/25/17.
//

#include <pthread.h>
#include "IOStack.h"
#include "../neuro/tn_neuron.h"
#include "../lib/rqueue.h"

FILE *membraneFile;
FILE *outSpikeFile;
FILE *outNetworkCfgFile;
FILE *outNeuronCfgFile;

list_t networkStructureList;
list_t membranePotList;

rqueue_t *nc_data_q;
pthread_t filethread;
pthread_attr_t thread_att;


int maxNeuronStruct = 512;
int maxNetworkListSize = 1024;
void openOutputFiles(char *outputFileName) {
  if(SAVE_NETWORK_STRUCTURE){


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
  char netfn2[255] = {'\0'};
  sprintf(netfn, "neuron_config%i.csv", g_tw_mynode);
  outNeuronCfgFile = fopen(netfn2, "wb");
  printf("LN36\n");
  fprintf(outNetworkCfgFile, "Core,NeuronID,DestCore,DestAxon\n");
  fprintf(outNeuronCfgFile, "Core,NeuronID,DestCore,DestAxon,");
  for (int i = 0; i < NEURONS_IN_CORE; i++) {
    fprintf(outNeuronCfgFile, ",in_axon_%i", i);
  }
  fprintf(outNeuronCfgFile, "\n");
  printf("LN43\n");
//    outNeuronCfgFile = fopen(ncfgfn, "wb");

}}
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
int threaded = 1;
void saveIndNeuronTh(tn_neuron_state *ns){
  char line[NEURONS_IN_CORE]  = {'\0'};
  neuronConnToSCSV(ns,line);
  fprintf(outNetworkCfgFile,line);

}
void *fileWorker(){
  printf("\nWorker thread started.\n");
  do{

    if(rqueue_isempty(nc_data_q) == 0){
      void *nd = rqueue_read(nc_data_q);
      if(nd == NULL) {
        return NULL;
      }
      tn_neuron_state *n = nd;
      fprintf(outNeuronCfgFile, "%llu, %llu, %li, %li", n->myCoreID, n->myLocalID,n->outputNeuronDest,n->outputCoreDest);
      char ncon[NEURONS_IN_CORE * 3] = {'\0'};
      //saveIndNeuronTh(n);
      //ncon[0] = '\0';
      //fprintf(outNetworkCfgFile, "%s\n", ncon);
      for(int ax = 0; ax < NEURONS_IN_CORE; ax ++){
        int nwt =  n->synapticWeight[n->axonTypes[ax]] && n->synapticConnectivity[ax];
        //fprintf(outNeuronCfgFile, ",%i",  n->synapticWeight[n->axonTypes[ax]] && n->synapticConnectivity[ax]);
        sprintf(ncon,"%s,%i", ncon,nwt);
      }
      //fprintf(outNeuronCfgFile, "\n");
      fprintf(outNeuronCfgFile,"%s\n", ncon);
    }
  }while(threaded);
  printf("worker complete.\n");
}

int isSetup = 0;
void saveNeuronNetworkStructure(void *n){

  //neuroCon *ndat = calloc(sizeof(ne uroCon),1);
  if(isSetup == 0){
    printf("Thread starting \n");
    nc_data_q = rqueue_create(5192,RQUEUE_MODE_BLOCKING);
    printf("\n q created.\n");


    int res = pthread_create(&filethread, NULL, fileWorker, NULL);

    printf("result was %i\n", res);

    isSetup = 1;
  }
  int fullctr =0;
  while (rqueue_write(nc_data_q,n) == -2){
    fullctr ++;
    if (fullctr % 100 == 0){
      printf("\n Node %i reports queue full - waiting. Full times = %i\n", g_tw_mynode, fullctr);
    }
  }
  //fileWorker();

}

void saveNeuronPreRun(){
 if(isSetup){
   isSetup =0;
   rqueue_write(nc_data_q, NULL);
  // pthread_join(filethread,NULL);
   fclose(outNeuronCfgFile);
   fclose(outNeuronCfgFile);

 }
}

void saveNetworkStructure() {
  //openOutputFiles("network_def.csv");
  char *lntxt = calloc(sizeof(char), 65535);
  char *lxtxt = calloc(sizeof(char), 65535);
  printf("Starting network save");
  //char lntxt[NEURONS_IN_CORE * CORES_IN_SIM * 2];

  for (int core = 0; core < CORES_IN_SIM / g_tw_npe; core++) {
    for (int neuron = 0; neuron < NEURONS_IN_CORE ; neuron++) {
      tw_lpid ngid = getNeuronGlobal(core, neuron);
      tn_neuron_state *n = (tn_neuron_state *) tw_getlp(ngid);
      //sprintf(lntxt, "%llu, %llu", n->myCoreID, n->myLocalID);
      fprintf(outNeuronCfgFile, "%llu, %llu", n->myCoreID, n->myLocalID);
      fflush(outNeuronCfgFile);
      neuronConnToSCSV(n, lxtxt);

      for (int ax = 0; ax < NEURONS_IN_CORE; ax++) {
        sprintf(lntxt, "%s,%llu", lntxt, n->synapticWeight[n->axonTypes[ax]] && n->synapticConnectivity[ax]);
        fprintf(outNeuronCfgFile, "%s,%i", lntxt, n->synapticWeight[n->axonTypes[ax]] && n->synapticConnectivity[ax]);
        fflush(outNeuronCfgFile);
      }
      //strcat(lntxt, "\n");
      fprintf(outNeuronCfgFile, "\n");
      fflush(outNeuronCfgFile);

    }
    fprintf(outNetworkCfgFile, "%s", lxtxt);
    //fprintf(outNeuronCfgFile, lntxt);
    lntxt[0] = '\0';
    lxtxt[0] = '\0';

    //strcat(lntxt, "\n");

  }
  fclose(outNeuronCfgFile);
  fclose(outNetworkCfgFile);
  //save the file;

  free(lntxt);
  free(lxtxt);
  //free(lntxt);
  return;
  initDataStructures(g_tw_nlp);

  int neuronsStartAt = AXONS_IN_CORE + 1;
  fprintf(outNetworkCfgFile, "Core,NeuronID,DestCore,DestAxon\n");
  for (int core = 0; core < CORES_IN_SIM ; core++) {
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




