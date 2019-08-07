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
rqueue_t *is_working;
rqueue_t *spike_data;
pthread_t filethread;
pthread_attr_t thread_att;

int maxNeuronStruct = 512;
int maxNetworkListSize = 1024;
void openOutputFiles(char *outputFileName) {
  if (SAVE_NETWORK_STRUCTURE) {
#ifdef DEBUG
    if(g_tw_mynode == 0){
      tw_printf(TW_LOC, "Saving network structure file.\n");
    }
#endif


//    char ncfgfn[128] = {"\0"};
//    strcat(ncfgfn, "full_");
//    strcat(ncfgfn,outputFileName);
//    strcat(ncfgfn,".bin");
    char netfn[255] = {'\0'};
//    printf("LN28\n");
    sprintf(netfn, "network_config_%i.csv", g_tw_mynode);
//    printf("LN30\n");
    outNetworkCfgFile = fopen(netfn, "wb");
//    printf("LN31\n");
    char netfn2[255] = {'\0'};
    sprintf(netfn2, "neuron_config_%i.csv", g_tw_mynode);
    outNeuronCfgFile = fopen(netfn2, "wb");
//    printf("LN36\n");
    fprintf(outNetworkCfgFile, "Core,NeuronID,DestCore,DestAxon\n");
    fprintf(outNeuronCfgFile, "Core,NeuronID,DestCore,DestAxon");
    for (int i = 0; i < NEURONS_IN_CORE; i++) {
      fprintf(outNeuronCfgFile, ",input_axon_%i", i);
    }
    fprintf(outNeuronCfgFile, "\n");
    printf("LN43\n");
//    outNeuronCfgFile = fopen(ncfgfn, "wb");

  }
}
void closeOutputFiles() {
  tw_printf(TW_LOC, "Closing output files..");
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
      char *l = list_get_at(&networkStructureList, 0);
      fprintf(outNetworkCfgFile, l);
      list_delete_at(&networkStructureList, 0);

    }
  }
}

int threaded = 1;
void saveIndNeuronTh(tn_neuron_state *ns) {
  char line[NEURONS_IN_CORE] = {'\0'};
  neuronConnToSCSV(ns, line);
  fprintf(outNetworkCfgFile, line);

}
void *fileWorker() {
  printf("---------------------\nWorker thread started.\n--------------------");
  fflush(stdout);
  do {
    if (rqueue_isempty(is_working)==0) {
      char *nd = (char *) rqueue_read(is_working);
      if (nd[0]=='1') {
        threaded = false;
      }
    }
    if (rqueue_isempty(nc_data_q)==0) {
      void *nd = rqueue_read(nc_data_q);
      if (nd==NULL) {
        threaded = false;
        break;
      }
      tn_neuron_state *n = nd;
      fprintf(outNeuronCfgFile,
              "%llu, %llu, %li, %li",
              n->myCoreID,
              n->myLocalID,
              n->outputNeuronDest,
              n->outputCoreDest);
      char ncon[NEURONS_IN_CORE*3] = {'\0'};
      //saveIndNeuronTh(n);
      //ncon[0] = '\0';
      //fprintf(outNetworkCfgFile, "%s\n", ncon);
      for (int ax = 0; ax < NEURONS_IN_CORE; ax++) {
        int nwt = n->synapticWeight[n->axonTypes[ax]] && n->synapticConnectivity[ax];
        //fprintf(outNeuronCfgFile, ",%i",  n->synapticWeight[n->axonTypes[ax]] && n->synapticConnectivity[ax]);
        sprintf(ncon, "%s,%i", ncon, nwt);
      }
      //fprintf(outNeuronCfgFile, "\n");
      fprintf(outNeuronCfgFile, "%s\n", ncon);
      fflush(outNeuronCfgFile);
    }
  } while (threaded);
  printf("worker complete.\n");
}

int isSetup = 0;
void saveNeuronNetworkStructure(void *n) {
//#ifdef DEBUG
//  tw_printf(TW_LOC,"Neuron Data saving thread startup.\n");
//#endif

  //neuroCon *ndat = calloc(sizeof(ne uroCon),1);
  if (isSetup==0) {
#ifdef DEBUG
    printf("Thread starting \n");
#endif
    nc_data_q = rqueue_create(5192, RQUEUE_MODE_BLOCKING);
    is_working = rqueue_create(1, RQUEUE_MODE_BLOCKING);

    printf("\n q created.\n");

    int res = pthread_create(&filethread, NULL, fileWorker, NULL);
#ifdef DEBUG
    printf("result was %i\n", res);
#endif
    isSetup = 1;
  }
  int fullctr = 0;
  while (rqueue_write(nc_data_q, n)==-2) {
    fullctr++;
    if (fullctr%100==0) {
      printf("\n Node %i reports queue full - waiting. Full times = %i\n", g_tw_mynode, fullctr);
    }
  }
  //fileWorker();

}

void saveNeuronPreRun() {
  if (isSetup) {
    isSetup = 0;
    rqueue_write(nc_data_q, NULL);
    pthread_join(filethread, NULL);
    fclose(outNeuronCfgFile);
    fclose(outNetworkCfgFile);

  }
}

void saveNetworkStructureMPI(){
  char * network_mpi_out_filename = "network_config_mpi.csv";
  static int done = 0;   if(done == 0) {
    MPI_File net_file;
    MPI_File_open(MPI_COMM_WORLD, network_mpi_out_filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL,
                  &net_file);
    //compute the offset based on the size of the data.
    //declare the CSV header
    MPI_Offset offset = 0;
    long num_neurons_per_rank = (CORES_IN_SIM * NEURONS_IN_CORE) / g_tw_npe;
    long entry_size = sizeof(char) *
                      16; // each entry will contain 16 chars. Will include the ',' chars (so a total of 15 digits).
    //core, neuronID, DC, DA, axon_conn_list, axon_types, weights, weight_mode, newline
    long size_of_params =
            (4 + AXONS_IN_CORE + AXONS_IN_CORE + NUM_NEURON_WEIGHTS + NUM_NEURON_WEIGHTS + 1) * entry_size;
    //we need to save a line for each neuron:
    long long total_write_size = CORES_IN_SIM * NEURONS_IN_CORE * size_of_params;
    long rank_write_size = total_write_size / g_tw_npe;
    //and the offset is the total write size / num_neurons_per_ranks (since the write size is neuron-based)
    offset = rank_write_size * g_tw_mynode;
    char *neuron_data = calloc(sizeof(char), rank_write_size);
    // populate neuron data with values:
#define ld "%15d,"
    int neuron_start = num_neurons_per_rank * g_tw_mynode;
    for (int i = neuron_start; i < num_neurons_per_rank; i++) {
      tw_lpid wanted_neuron = getGIDFromLocalIDs(i / NEURONS_IN_CORE, i % NEURONS_IN_CORE);
      tn_neuron_state *n = tw_getlocal_lp(wanted_neuron)->cur_state;
      sprintf(neuron_data, "%s" ld ld ld ld, neuron_data, n->myCoreID, n->myLocalID, getCoreFromGID(n->outputGID),
              getNeuronLocalFromGID(n->outputGID));
      for (int j = 0; j < AXONS_IN_CORE; j++) {
        sprintf(neuron_data, "%s" ld, neuron_data, n->synapticConnectivity[i]);
      }
      for (int j = 0; j < AXONS_IN_CORE; j++) {
        sprintf(neuron_data, "%s" ld, neuron_data, n->axonTypes[i]);
      }
      for (int j = 0; j < NUM_NEURON_WEIGHTS; j++) {
        sprintf(neuron_data, "%s" ld, neuron_data, n->synapticWeight[i]);
      }
      for (int j = 0; j < NUM_NEURON_WEIGHTS; j++) {
        sprintf(neuron_data, "%s", ld, neuron_data, n->weightSelection[i]);
      }
      sprintf("%s\n", neuron_data);
    }
    MPI_File_write_at_all(net_file, offset, neuron_data, rank_write_size, MPI_CHAR, MPI_STATUS_IGNORE);
    done = 1;
  }
}

void debug_neuron_connections(tn_neuron_state *n,tw_lp *lp){
  static FILE *rank_output;
  static int rank_output_open = 0;
  if (! rank_output_open ) {
    if (g_tw_mynode == 0){
      tw_printf(TW_LOC,"Neuron Network Debug File init.\n");
    }
    rank_output_open = 1;
    char fn[256] = {'\0'};
    sprintf(fn,"debug_neuron_connection_rank_%li.csv",g_tw_mynode);
    rank_output = fopen(fn,"w");
    fprintf(rank_output,"gid,core,neuron,destCore,destNeuron,destGid");

    for(int i = 0; i < NUM_NEURON_WEIGHTS; i ++){
            fprintf(rank_output,"weight_%i,", i);
    }
    for (int i = 0; i < AXONS_IN_CORE; i ++){
      fprintf(rank_output, "input_%i_connection,",i);
    }
    for (int i = 0; i < AXONS_IN_CORE ; i ++){
      fprintf(rank_output, "input_%i_type",i);
      if(i < AXONS_IN_CORE - 1){
        fprintf(rank_output, ",");
      }
    }
    fprintf(rank_output, "\n");

  }

  char axon_types[65535] = {'\0'};
  char axon_con[65535] = {'\0'};
  fprintf(rank_output,"%li,%li,%li,%li,%li,%li", lp->gid, n->myCoreID,n->myLocalID,
          n->outputCoreDest,n->outputNeuronDest,n->outputGID);
  for(int i = 0; i < NUM_NEURON_WEIGHTS; i ++){
    fprintf(rank_output,",%i",n->synapticWeight[i]);
  }

  for(int i = 0; i < AXONS_IN_CORE; i ++){
    sprintf(axon_types,"%s,%i",axon_types,n->axonTypes[i]);
    sprintf(axon_con,"%s,%i",axon_con, n->synapticConnectivity[i]);
  }

  fprintf(rank_output,"%s%s\n",axon_con,axon_types);
  fflush(rank_output);

}
/**
 * non-threaded version of the saveNeuronNetworkStructure() function. Rather than taking
 * a single neuron's state and using a thread to write out the data, this function saves the neuron state through
 * one large loop.
 */
void saveNetworkStructure() {
  //openOutputFiles("network_def.csv");
  char *lntxt = calloc(sizeof(char), 65535);
  char *lxtxt = calloc(sizeof(char), 65535);
  printf("Starting network save");
  //char lntxt[NEURONS_IN_CORE * CORES_IN_SIM * 2];

  for (int core = 0; core < CORES_IN_SIM/g_tw_npe; core++) {
    for (int neuron = 0; neuron < NEURONS_IN_CORE; neuron++) {
      tw_lpid ngid = getNeuronGlobal(core, neuron);
      tn_neuron_state *n = (tn_neuron_state *) tw_getlp(ngid);
      //sprintf(lntxt, "%llu, %llu", n->myCoreID, n->myLocalID);
      fprintf(outNeuronCfgFile, "%llu, %llu", n->myCoreID, n->myLocalID);
      fflush(outNeuronCfgFile);
      neuronConnToSCSV(n, lxtxt);

      for (int ax = 0; ax < NEURONS_IN_CORE; ax++) {
        sprintf(lntxt, "%s,%llu", lntxt, n->synapticWeight[n->axonTypes[ax]] && n->synapticConnectivity[ax]);
        fprintf(outNeuronCfgFile,
                "%s,%i",
                lntxt,
                n->synapticWeight[n->axonTypes[ax]] && n->synapticConnectivity[ax]);
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

  //free(lntxt);
  //free(lxtxt);
  //free(lntxt);
  return;
  initDataStructures(g_tw_nlp);

  int neuronsStartAt = AXONS_IN_CORE + 1;
  fprintf(outNetworkCfgFile, "Core,NeuronID,DestCore,DestAxon\n");
  for (int core = 0; core < CORES_IN_SIM; core++) {
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




