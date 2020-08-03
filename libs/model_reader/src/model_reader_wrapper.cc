//
// Created by Mark Plagge on 9/3/18.
//


#include "../include/tn_parser.hh"

#include <rapidjson/filewritestream.h>

TN_Main c_model;
TN_Main create_tn_data(char *filename){
  return create_tn_data(string(filename));
}


extern "C" {

#include "../include/model_reader_wrapper.h"
tn_neuron_state *get_neuron_state(unsigned long my_core, unsigned long neuron_id) {
  tn_neuron_state * s = c_model.generate_neuron_from_id(my_core,neuron_id).getTn();
  return s;
}

void init_neuron_state(unsigned long my_core, unsigned long neuron_id, tn_neuron_state *n){
  c_model.populate_neuron_from_id(my_core, neuron_id,n);
}

tn_neuron_state *get_neuron_state_array(int my_core) {
  return c_model.generate_neurons_in_core_struct(my_core);
}

void load_and_init_json_model(char *filename, int node) {
  printf("\n\n\n -------- Loading neuron model file %s \n", filename);
  c_model = create_tn_data(filename);
#ifdef DEBUG
  if(node == 0) {
    printf("Node0: Debug model file enabled.\n");
    string csv_dbg = c_model.generate_debug_csv();
    FILE *of = fopen("converted_nscs_debug.csv", "w");
    fprintf(of, csv_dbg.c_str());
    fclose(of);
    printf("Maximum core found: %i \n", c_model.get_max_core());
  }
#endif
}

int serial_load_json(char *json_filename) {
  int result = 0;

}

void clean_up(TN_Main* m){
  delete(m);
}

/** @todo DEBUG CODE - REMOVE WHEN DONE
 * Does not work in MPI mode.*/
Document neuron_js;
Value main_holder(kArrayType);
Value main_obj(kObjectType);
char *filename = "neuron_structure_debug.json";
void debug_add_neuron_to_json(tn_neuron_state *s, tw_lp *lp){
  rapidjson::Document::AllocatorType & allocator = neuron_js.GetAllocator();
  Value obj(kObjectType);
  Value val(kObjectType);
  Value typeArr(kArrayType);
  Value weightArr(kArrayType);
  Value connectionArr(kArrayType);
  Value elementArray(kArrayType);

  char * single_vals[] = {"gid","core","neuronID","destCore","destNeuron","destGID"};

  obj.AddMember("gid",lp->gid,allocator);
  obj.AddMember("core",s->myCoreID,allocator);
  obj.AddMember("neuronID",s->myLocalID,allocator);
  obj.AddMember("destCore",(int64_t )s->outputCoreDest,allocator);
  obj.AddMember("destNeuron",(int64_t )s->outputNeuronDest,allocator);
  obj.AddMember("destGID",(int64_t )s->outputGID,allocator);

  for(int i = 0; i < AXONS_IN_CORE; i ++){
  //  char axname[512] = {'\0'};
//    sprintf(axname,"neuron_%i_type",i);
    typeArr.PushBack(s->axonTypes[i],allocator);
    connectionArr.PushBack(s->synapticConnectivity[i],allocator);
  }
  for(int i = 0; i < NUM_NEURON_WEIGHTS; i ++){
    weightArr.PushBack(s->synapticWeight[i],allocator);
  }
  obj.AddMember("synapse_types",typeArr,allocator);
  obj.AddMember("connectivity",connectionArr,allocator);
  obj.AddMember("weights",weightArr,allocator);

  //misc data
  obj.AddMember("dbg_dendriteLocal",s->dendriteLocal,allocator);

  //char nn[1024] = {'\0'};
  char * nn = (char*)calloc(sizeof(char),1024);

  sprintf(nn,"neuron_%i_%i_%i",s->myCoreID,s->myLocalID,lp->gid);
  string ns(nn);
  Value name(kStringType);
  name.SetString(ns.c_str(),allocator);
  Value obj_holder(kObjectType);
  obj_holder.AddMember(name,obj,allocator);
  main_holder.PushBack(obj_holder,allocator);
  free(nn);





}
void debug_init_neuron_json(){
  neuron_js.SetObject();


}

void debug_close_neuron_json(){
  rapidjson::Document::AllocatorType & allocator = neuron_js.GetAllocator();
  neuron_js.AddMember("neurons",main_holder,allocator);
FILE *of = fopen(filename,"w");
char writeBuffer[65535];
FileWriteStream os(of,writeBuffer,sizeof(writeBuffer));
Writer<FileWriteStream> writer(os);
neuron_js.Accept(writer);
fclose(of);
}

}
