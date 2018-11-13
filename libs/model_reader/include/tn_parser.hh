//
// Created by Mark Plagge on 8/8/18.
//
/* Some setup for SSE2/4 and rapidjson - Removed and migrated to CMAKE */
//#if not defined(RAPIDJSON_SSE42) || not defined(RAPIDJSON_SSE2)
//#ifdef __SSE4_2__
//#define RAPIDJSON_SSE42 1
//#elif __SSE2__
//#define RAPIDJSON_SSE2 1
//#endif
//#endif

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/error/en.h>
#include <rapidjson/error/error.h>

//#define RAPIDJSON_PARSE_DEFAULT_FLAGS KParseCommentFlags


#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <map>
#include <string>
#include <sstream>
#include <vector>
//#include "json_dto.hh"
#include <ross.h>
#include "../src/utils.hh"
#include <regex>
#include "../extern/json.hpp"
#include <stdio.h>
#include "../../../src/globals.h"
#include "../../../src/neuro/tn_neuron_struct.h"
#include "../extern/tqdm.h"
#include <iomanip>

#ifdef USE_OMP
#include <omp.h>
#endif


using json = nlohmann::json;
//#include "../../src/neuro/tn_neuron.h"
using namespace rapidjson;
using namespace std;

#define CORE_SIZE 256
#define AXONS_IN_CORE 256
#define SYNAPSES_IN_CORE 1
#define NUM_NEURON_WEIGHTS 4

typedef enum OUT_MODE_E {
  TN_OUT_CSV = 1,
  TN_OUT_BIN = 2,
  TN_OUT_LUA = 4,
  TN_OUT_PY = 8,
  TN_OUT_JSON = 10,
  TN_OUT_BIN_SPLIT = 12
} out_mode_opt;


int char2int(char input);
vector<bool> hex2bool_insert(char hex);
vector<bool> fullhex2bool(string hexstr);
/** json struct wrapper */
struct tn_neuron_state_vecs{
  tn_neuron_state *n;
  vector<int> axonTypeVec;
  vector<int> synapticWeightVec;
  vector<bool> synapticConVec;
  vector<bool> weightSelVec;
};
//void init_tn_neuron_state_vecs(*tn_neuron_state_vecs nvec,*tn_neuron_state n);

/**
 * Main Wrapper for the TN Struct used in NeMo
 * Creates a new tn_neuron_state on construction
 * Will contain to data functionality as well.
 */
class TN_State_Wrapper {
private:
  tn_neuron_state *tn;

public:
  bool isValid;
  int myCore;
  int write_bin_data(int big_end, ostream out);
  string generate_csv();
  json generate_json(json j);
  string generate_pycode();


//  ~TN_State_Wrapper(){
//    delete(tn);
//  }
  tn_neuron_state *getTn() {
    return tn;
  }
  TN_State_Wrapper() {
    tn = (tn_neuron_state *) calloc(sizeof(tn_neuron_state),1);
    isValid = true;
  }
  void init_empty() {
    //tn = (tn_neuron_state *) calloc(sizeof(tn_neuron_state), 1);
     //this->tn = tn;
    for (int i = 0; i < AXONS_IN_CORE; i++) {
      tn->axonTypes[i] = -1;
      tn->synapticConnectivity[i] = false;
    }
    for (int j = 0; j < NUM_NEURON_WEIGHTS; ++j) {
      tn->synapticWeight[j] = -1;
      tn->weightSelection[j] = false;

    }
  }

  void initialize_state(vector<int> input_axon_connectivity,
                        vector<short> input_axon_types,
                        int output_core,
                        int output_neuron,
                        int source_core,
                        int source_local,
                        int dest_delay);

  void initialize_state(int input_axon_connectivity[],
                        short input_axon_types[],
                        int output_core,
                        int output_neuron,
                        int source_core,
                        int source_local,
                        int dest_delay);

  void initialize_state(tn_neuron_state *ext_n);
  char *generate_lua();

};

class TN_Crossbar_Type {
private:
  short get_type(int loc);
  short get_type(string type);
public:
  string name;
  vector<vector<bool>> rows;
  vector<string> types;
  void add_synapse_row(string type, string synapses);
  void add_synapse_rows(vector<string> types, vector<string> synapses);
  vector<bool> get_connectivity(int neuron);
  vector<short> get_types(int neuron);
};

class TN_Neuron_Type {
  /*"name":"N0000001E","class":"NeuronGeneral","sigma0":-1,"sigma1":1,
   * "sigma2":1,"sigma3":1,"s0":1,"s1":1,"s2":0,"s3":0,
   * "b0":false,"b1":false,"b2":false,"b3":false,"sigma_lambda":1,
   * "lambda":5,"c_lambda":false,"epsilon":false,"alpha":1,"beta":0,
   * "TM":0,"gamma":0,"kappa":true ,"sigma_VR":1,"VR":0,"V":0
   * */
  tn_neuron_state *ns;
  string name;
  string n_class;
  int sigma[4];
  int s[4];
  bool b[4];
  int sigma_lambda;
  int lambda;
  bool c_lambda;
  bool epsilon;
  int alpha;
  int beta;
  int TM;
  int gamma;
  int kappa;
  int sigma_VR;
  int VR;
  int V;
  map<string, int> var_types;
  int name_id;

public:

  void set_name(string name);
  string get_name();
  int get_name_id();
  TN_State_Wrapper new_neuron_state(vector<unsigned int> input_axon_connectivity,
                                    vector<short> input_axon_types,
                                    int output_core,
                                    int output_neuron,
                                    int source_core,
                                    int source_local,
                                    int dest_delay);
  TN_State_Wrapper new_neuron_state(TN_Crossbar_Type crossbar, int output_core, int output_neuron, int source_core,
                                    int dest_delay, int neuron_id);
  void new_neuron_state_init_struct(TN_Crossbar_Type crossbar, int output_core, int output_neuron, int source_core,
                                    int dest_delay, int neuron_id,tn_neuron_state *n);
  void new_neuron_state_init_struct(vector<unsigned int> input_axon_connectivity,
                                    vector<short> input_axon_types,
                                    int output_core,
                                    int output_neuron,
                                    int source_core,
                                    int source_local,
                                    int dest_delay,
                                    tn_neuron_state *n);

  void init_neuron_from_json_doc(Document json_doc);
  void init_neuron_from_json_arr(rapidjson::GenericValue<rapidjson::UTF8<char>,
                                                         rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> > &json_array);

  /**
   * TN_Neuron_Type() - inits the map-key pair system used for finding variables in the JSON file.
   */
  TN_Neuron_Type() {

    var_types["name"] = 0;
    var_types["class"] = 1;
    var_types["sigma0"] = 2;
    var_types["sigma1"] = 3;
    var_types["sigma2"] = 4;
    var_types["sigma3"] = 5;
    var_types["s0"] = 6;
    var_types["s1"] = 7;
    var_types["s2"] = 8;
    var_types["s3"] = 9;
    var_types["b0"] = 10;
    var_types["b1"] = 11;
    var_types["b2"] = 12;
    var_types["b3"] = 13;
    var_types["sigma_lambda"] = 14;
    var_types["lambda"] = 15;
    var_types["c_lambda"] = 16;
    var_types["epsilon"] = 17;
    var_types["alpha"] = 18;
    var_types["beta"] = 19;
    var_types["TM"] = 20;
    var_types["gamma"] = 21;
    var_types["kappa"] = 22;
    var_types["sigma_VR"] = 23;
    var_types["VR"] = 24;
    var_types["V"] = 25;
  }

};
typedef struct Core_info {
  int type;
  int dendrite;
  int destCore;
  int destAxon;
  int destDelay;
} core_info;
/**
 "core":{
        "metadata":{
            "coreletClass":"th_corelet_layer_cores",
            "coreletId":2,
            "coreNumber":4,
            "parentCoreletId":[3,1,0],
            "layerNumber":2,
            "layerType":"conv"
        },
        "id":4,
        "timeScaleExponent":0,
        "rngSeed":4294967295,

"neurons":{

"dendrites":["0:251","252:255"],
"types":[25,28,23,22,24,24,12,22,27,33,23,27,39,15,23,23,44,18,31,22,30,16,31,6,36,23,18,33,14,31,26,18,24,31,32,31,17,29,23,19,36,12,33,17,40,28,30,23,27,21,23,14,12,29,23,24,32,17,31,13,22,18,27,22,25,25,10,20,28,"18:2:22",30,21,14,24,21,14,26,30,15,32,22,25,20,30,22,13,31,24,18,21,22,26,21,28,24,33,19,23,34,33,23,33,25,19,22,22,30,22,34,23,28,23,25,25,17,6,12,28,23,22,16,8,37,"22:3:28",23,22,24,24,12,22,27,33,23,27,39,15,23,23,44,18,31,22,30,16,31,6,36,23,18,33,14,31,26,18,24,31,32,31,17,29,23,19,36,12,33,17,40,28,30,23,27,21,23,14,12,29,23,24,32,17,31,13,22,18,27,22,25,25,10,20,28,"18:2:22",30,21,14,24,21,14,26,30,15,32,22,25,20,30,22,13,31,24,18,21,22,26,21,28,24,33,19,23,34,33,23,33,25,19,22,22,30,22,34,23,28,23,25,25,17,6,12,28,23,22,16,8,37,22,"0x4"],
"destCores":[878,878,"1134x4",878,"1134x5",878,1134,878,1134,"878x4",1134,1134,878,1134,"878x3",1134,1134,878,1134,878,"1134x3",878,1134,1134,878,878,1134,1134,878,1134,878,1134,878,"1134x5","878x3",1134,878,1134,878,1134,"878x3",1134,1134,878,"1134x4",878,1134,1134,878,1134,878,878,"1134x3",878,1134,"878x4",1134,878,1134,878,1134,878,"1134x3","878x3",1134,878,"1134x3","878x3",1134,878,"1134x3","878x3",1134,878,1134,"878x4","1134x3",878,1134,878,878,"1134x4",878,"1134x5",878,1134,878,1134,"878x4",1134,1134,878,1134,"878x3",1134,1134,878,1134,878,"1134x3",878,1134,1134,878,878,1134,1134,878,1134,878,1134,878,"1134x5","878x3",1134,878,1134,878,1134,"878x3",1134,1134,878,"1134x4",878,1134,1134,878,1134,878,878,"1134x3",878,1134,"878x4",1134,878,1134,878,1134,878,"1134x3","878x3",1134,878,"1134x3","878x3",1134,878,"1134x3","878x3",1134,878,1134,"878x4","1134x3",878,1134,"-1x4"],
"destAxons":[66,222,210,150,238,96,12,184,222,98,136,122,208,94,86,112,8,156,18,42,174,108,172,232,10,234,204,138,156,96,12,240,84,114,132,154,102,92,78,54,16,240,182,62,108,128,136,18,110,196,236,42,64,72,138,180,134,166,130,218,184,102,132,72,2,190,178,60,44,212,158,134,130,202,50,26,22,32,144,224,58,52,250,2,178,76,152,170,30,80,68,150,90,106,126,244,34,114,208,74,176,36,250,30,238,46,242,0,140,168,188,36,212,142,160,228,204,88,152,70,62,190,58,46,68,186,67,223,211,151,239,97,13,185,223,99,137,123,209,95,87,113,9,157,19,43,175,109,173,233,11,235,205,139,157,97,13,241,85,115,133,155,103,93,79,55,17,241,183,63,109,129,137,19,111,197,237,43,65,73,139,181,135,167,131,219,185,103,133,73,3,191,179,61,45,213,159,135,131,203,51,27,23,33,145,225,59,53,251,3,179,77,153,171,31,81,69,151,91,107,127,245,35,115,209,75,177,37,251,31,239,47,243,1,141,169,189,37,213,143,161,229,205,89,153,71,63,191,59,47,69,187,"0x4"],
"destDelays":["1x252","1x4"]
},

"crossbar":{
"name":"coreProt0000001"
}
},
 */
class TN_Core {
private:

  //void convert_tn_arr(string tn_value);
  string coreletClass;
  int coreletId;
  int coreNumber;
  vector<int> parentCorletId;
  int layerNumber;
  string layerType;
  int id;
  int timeScaleExponent;
  uint32_t rngSeed;
  // Neurons

  int dendrites[256];
  int types[256];
  int destCores[256];
  int destAxons[256];
  int destDelays[256];
  string crossbar_name;
public:

  int getLayerNumber() const;
  const string &getLayerType() const;
  int getId() const;
  const string &getCrossbar_name() const;
public:

  //void init(rapidjson::GenericObject obj);
  void init();

  void init_core_from_itr(Value::ConstMemberIterator itr);
  int get_dendrite_for_neuron(int neuronID);
  int get_neuron_type(int neuronID);
  int get_neuron_dest_core(int neuronID);
  int get_neuron_axon(int neuronID);
  int get_neuron_delay(int neuronID);

  core_info get_neuron_info(int neuronID);


};

/**
 * TN_Main - Main wrapper class for the neuron model. Contains TN_Neuron
 */
class TN_Main {
private:
  TN_Neuron_Type cur_prototype;
  TN_Crossbar_Type cur_crossbar;
  bool set_current_prot_cros(unsigned long coreID, unsigned long neuronID);
  core_info cur_neuron;
public:
  int core_count;
  string neuron_class;
  int crossbar_size;
  string crossbar_class;

  int rngSeed;
  map<string, TN_Crossbar_Type> TN_Crossbar_Type_library;
  map<int, TN_Core> TN_Neuron_Core_Library;
  map<int, TN_Neuron_Type> TN_Neuron_Library;

  TN_State_Wrapper generate_neuron_from_id(unsigned long coreID, unsigned long neuronID);
  tn_neuron_state *generate_neurons_in_core_struct(unsigned long coreID);
  void populate_neuron_from_id(unsigned long coreID, unsigned long neuronID, tn_neuron_state *n);
  vector<TN_State_Wrapper> generate_neurons_in_core_vec(int coreID);

  map<int, TN_State_Wrapper> generate_all_neurons_wrap();

  //Debug options

};



/**
 * TN_Output
 * Class that wraps output functionality for the neuron model,
 * Manages files and data.
 */
class TN_Output {
private:
  json main_json;
  int json_init;
  TN_Main main_model;
  vector<TN_State_Wrapper> neurons;
  string output_filename;
  string out_fn_csv;
  string out_fn_bin;
  string out_fn_lua;
  string out_fn_py;
  string out_fn_json;
  json generate_json(int start_core, int end_core);
  unsigned char output_mode;
  ofstream binary_out_file;
  #ifdef  USE_OMP
  int write_json_mp(int start_core, int end_core);
  json generate_json_mp(int start_core, int end_core);
  #endif


  int write_csv();
#ifdef USE_OMP
  int write_bin_mp();
#endif
  void write_bin_header();
  int write_bin_split();

  int write_bin();
  int write_lua();
  int write_py();
  int write_json();
  int write_json(int start_core, int end_core);

public:
  TN_Output(string filename, TN_Main model, unsigned char output_mode) ;

  int write_data();
  int write_data(int start_core, int end_core);
  tn_neuron_state *get_neuron_struct(int core, int neuron_id);


};

TN_Main create_tn_data(string filename);

#ifndef SUPERNEMO_TN_PARSER_HH
#define SUPERNEMO_TN_PARSER_HH

#endif //SUPERNEMO_TN_PARSER_HH
