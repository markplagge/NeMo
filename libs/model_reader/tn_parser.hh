//
// Created by Mark Plagge on 8/8/18.
//
/* Some setup for SSE2/4 and rapidjson */
#if not defined(RAPIDJSON_SSE42) || not defined(RAPIDJSON_SSE2)
#ifdef __SSE4_2__
#define RAPIDJSON_SSE42 1
#elif __SSE2__
#define RAPIDJSON_SSE2 1
#endif
#endif

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/istreamwrapper.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include "json_dto.hh"
#include <ross.h>
#include "../../src/globals.h"
#include "../../src/neuro/tn_neuron_struct.h"
#include "utils.hh"
#include <regex>
//#include "../../src/neuro/tn_neuron.h"
using namespace rapidjson;
using namespace std;

#define CORE_SIZE 256
#define AXONS_IN_CORE 256
#define SYNAPSES_IN_CORE 1

int char2int(char input);
vector<bool> hex2bool_insert(char hex);
vector<bool> fullhex2bool(string hexstr);

class TN_State_Wrapper {
public:
  tn_neuron_state *tn;
  TN_State_Wrapper() {
    tn = (tn_neuron_state *) calloc(sizeof(tn_neuron_state), 1);

  }
};

class TN_Neuron_Core {
  string type;
  int source;
  int dest_core;
  int dest_axon;
  int delay;
};

class TN_Crossbar_Row {
public:

  string type;
  vector<bool> crossbar_row;
  TN_Crossbar_Row(string synapse_row, string type);

};
class TN_Crossbar_Type {
public:
  string name;
  vector<vector<bool>> rows;
  vector<string> types;
  void add_synapse_row(string type, string synapses);
  void add_synapse_rows(vector<string> types, vector<string> synapses);
  vector<bool> get_connectivity(int neuron);
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
  tn_neuron_state *new_neuron_state(vector<int> input_axon_connectivity,
                                    vector<short> input_axon_types,
                                    int output_core,
                                    int output_neuron,
                                    int source_core,
                                    int source_local,
                                    int dest_delay);
  void init_neuron_from_json_doc(Document json_doc);
  void init_neuron_from_json_arr(rapidjson::GenericValue<rapidjson::UTF8<char>,
                                                         rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> > &json_array);
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
  int coreletID;
  int coreNumber;
  vector<int> parentCorletID;
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

 //void init(rapidjson::GenericObject obj);
  void init();
 void init_core_from_itr(Value::ConstMemberIterator itr);



};
class TN_Main {
public:
  int core_count;
  string neuron_class;
  int crossbar_size;
  string crossbar_class;

  int rngSeed;
  map<string, TN_Crossbar_Type> TN_Crossbar_Type_library;
  map<string, TN_Neuron_Core> TN_Neuron_Core_Library;
  map<int, TN_Neuron_Type> TN_Neuron_Library;
  map<string, TN_Core> TN_Cores;

};

class TN_State_Worker {
private:
  vector<TN_State_Wrapper> neurons;
//  map<string,TN_Core_Map> cores;
  map<string, TN_Crossbar_Type> crossbars;

public:
  void parse_tn_json();
};

class TN_Wrapper {
public:
  //Giant parameterized TN_Wrapper creation with variable numbers of sigmas and stuff?
  //Giant parameterized TN_Wrapper creation
  tn_neuron_state *create_tn_state() {

  }
  void init(string name,
            string tn_class,
            int sigma0,
            int sigma1,
            int sigma2,
            int sigma3,
            int s0,
            int s1,
            int s2,
            int s3,
            bool b0,
            bool b1,
            bool b2,
            bool b3,
            int sigma_lambda,
            int lambda,
            bool c,
            bool epsilon,
            int alpha,
            int beta,
            int TM,
            int gamma,
            bool kappa,
            int sigma_VR,
            int VR,
            int V) {
    this->name = name;
    this->tn_id = tn_class;
    this->sigmas.insert(this->sigmas.end(), {sigma0, sigma1, sigma2, sigma3});
    s.insert(s.end(), {s0, s1, s2, s3});
    b.insert(b.end(), {b0, b1, b2, b3});
    sigma_lmbda = sigma_lambda;
    lmbda = lambda;
    this->c = c;
    this->epsilon = epsilon;
    this->alpha = alpha;
    this->beta = beta;
    this->TM = TM;
    this->gamma = gamma;
    this->kappa = kappa;
    this->sigmaVR = sigma_VR;
    this->VR = VR;
    this->V = V;

  }
  /*{"name":"N0000001F","class":"NeuronGeneral",
 * "sigma0":-1,"sigma1":1,"sigma2":1,"sigma3":1,
 * "s0":1,"s1":1,"s2":0,"s3":0,"b0":false,"b1":false,
 * "b2":false,"b3":false,"sigma_lambda":1,"lambda":5,
 * "c_lambda":false,"epsilon":false,"alpha":1,"beta":0,
 * "TM":0,"gamma":0,"kappa":true ,"sigma_VR":1,"VR":0,"V":0} */
  const string &getClass() const {
    return tn_id;
  }

  void init_tn_struct();

  /**
   * creates default TN Neuron
   */
  TN_Wrapper() {
    init("default", "default", 0, 0, 0, 0, 0, 0, 0, 0,
         false, false, false, false, 0, 0,
         false, false,
         0, 0, 0, 0,
         false,
         0, 0, 0);
  }
  TN_Wrapper(Value &jsVal) {
    const char *p = "neuronTypes";
    //Value &jsVal;  = jsVa;
    init(
        jsVal["name"].GetString(),
        jsVal["class"].GetString(),
        jsVal["sigma0"].GetInt(),
        jsVal["sigma1"].GetInt(),
        jsVal["sigma2"].GetInt(),
        jsVal["sigma3"].GetInt(),
        jsVal["s0"].GetInt(),
        jsVal["s1"].GetInt(),
        jsVal["s2"].GetInt(),
        jsVal["s3"].GetInt(),
        jsVal["b0"].GetBool(),
        jsVal["b1"].GetBool(),
        jsVal["b2"].GetBool(),
        jsVal["b3"].GetBool(),
        jsVal["sigma_lambda"].GetInt(),
        jsVal["lambda"].GetInt(),
        jsVal["c_lambda"].GetBool(),
        jsVal["epsilon"].GetBool(),
        jsVal["alpha"].GetInt(),
        jsVal["beta"].GetInt(),
        jsVal["TM"].GetInt(),
        jsVal["gamma"].GetInt(),
        jsVal["kappa"].GetBool(),
        jsVal["sigma_VR"].GetInt(),
        jsVal["VR"].GetInt(),
        jsVal["V"].GetInt()
    );
  }
  TN_Wrapper(Value &jsVal, int pos) {
    const char *p = "neuronTypes";
    //Value &jsVal;  = jsVa;
    init(
        jsVal["name"].GetString(),
        jsVal["class"].GetString(),
        jsVal["sigma0"].GetInt(),
        jsVal["sigma1"].GetInt(),
        jsVal["sigma2"].GetInt(),
        jsVal["sigma3"].GetInt(),
        jsVal["s0"].GetInt(),
        jsVal["s1"].GetInt(),
        jsVal["s2"].GetInt(),
        jsVal["s3"].GetInt(),
        jsVal["b0"].GetBool(),
        jsVal["b1"].GetBool(),
        jsVal["b2"].GetBool(),
        jsVal["b3"].GetBool(),
        jsVal["sigma_lambda"].GetInt(),
        jsVal["lambda"].GetInt(),
        jsVal["c_lambda"].GetBool(),
        jsVal["epsilon"].GetBool(),
        jsVal["alpha"].GetInt(),
        jsVal["beta"].GetInt(),
        jsVal["TM"].GetInt(),
        jsVal["gamma"].GetInt(),
        jsVal["kappa"].GetBool(),
        jsVal["sigma_VR"].GetInt(),
        jsVal["VR"].GetInt(),
        jsVal["V"].GetInt()
    );
    //Document level code
//    init(
//        jsVal[p][pos]["name"].GetString(),
//        jsVal[p][pos]["class"].GetString(),
//        jsVal[p][pos]["sigma0"].GetInt(),
//        jsVal[p][pos]["sigma1"].GetInt(),
//        jsVal[p][pos]["sigma2"].GetInt(),
//        jsVal[p][pos]["sigma3"].GetInt(),
//        jsVal[p][pos]["s0"].GetInt(),
//        jsVal[p][pos]["s1"].GetInt(),
//        jsVal[p][pos]["s2"].GetInt(),
//        jsVal[p][pos]["s3"].GetInt(),
//        jsVal[p][pos]["b0"].GetBool(),
//        jsVal[p][pos]["b1"].GetBool(),
//        jsVal[p][pos]["b2"].GetBool(),
//        jsVal[p][pos]["b3"].GetBool(),
//        jsVal[p][pos]["sigma_lambda"].GetInt(),
//        jsVal[p][pos]["lambda"].GetInt(),
//        jsVal[p][pos]["c_lambda"].GetBool(),
//        jsVal[p][pos]["epsilon"].GetBool(),
//        jsVal[p][pos]["alpha"].GetInt(),
//        jsVal[p][pos]["beta"].GetInt(),
//        jsVal[p][pos]["TM"].GetInt(),
//        jsVal[p][pos]["gamma"].GetInt(),
//        jsVal[p][pos]["kappa"].GetInt(),
//        jsVal[p][pos]["sigma_VR"].GetInt(),
//        jsVal[p][pos]["VR"].GetInt(),
//        jsVal[p][pos]["V"].GetInt()
//        );

  }
  template<typename Json_Io>
  void json_io(Json_Io &io) {
    io & json_dto::mandatory("name", name) &
        json_dto::mandatory("class", tn_id) &
        json_dto::mandatory("sigma0", sigma0) &
        json_dto::mandatory("sigma1", sigma1) &
        json_dto::mandatory("sigma2", sigma2) &
        json_dto::mandatory("sigma3", sigma3) &
        json_dto::mandatory("s0", s0) &
        json_dto::mandatory("s1", s1) &
        json_dto::mandatory("s2", s2) &
        json_dto::mandatory("s3", s3) &
        json_dto::mandatory("b0", b0) &
        json_dto::mandatory("b1", b1) &
        json_dto::mandatory("b2", b2) &
        json_dto::mandatory("b3", b3) &
        json_dto::mandatory("sigma_lambda", sigma_lmbda) &
        json_dto::mandatory("lambda", lmbda) &
        json_dto::mandatory("c_lambda", c) &
        json_dto::mandatory("epsilon", epsilon) &
        json_dto::mandatory("alpha", alpha) &
        json_dto::mandatory("beta", beta) &
        json_dto::mandatory("TM", TM) &
        json_dto::mandatory("gamma", gamma) &
        json_dto::mandatory("kappa", kappa) &
        json_dto::mandatory("sigma_VR", sigmaVR) &
        json_dto::mandatory("VR", VR) &
        json_dto::mandatory("V", V);
  }
private:

  string name;
public:
  const string &getName() const;
  void setName(const string &name);
  tn_neuron_state *tn_state;
private:
  string tn_id;
  vector<int> sigmas;
  vector<int> s;
  vector<bool> b;
  bool epsilon;
  int sigma_lmbda;
  int lmbda;
  bool c;
  int alpha;
  int beta;
  int TM;
  int gamma;
  bool kappa;
  int sigmaVR;
  int VR;
  int V;
  // holders for ind. data:
  int sigma0;
  int sigma1;
  int sigma2;
  int sigma3;
  int s0;
  int s1;
  int s2;
  int s3;
  bool b0;
  bool b1;
  bool b2;
  bool b3;

};

class model_info {
  int coreCount;
  string neuronClass;
  int crossbarSize;
  string crossbarclass;

};
class CrossbarRow {
  string type;
  string synapses;
  int synapse_values[];
};
class CrossbarType {
  string name;
  vector<CrossbarRow> crossbar;

};

class NeuronTypeLib {
public:
  //neuron_type_library()
  void gen_type_map(Value &n_types);
  TN_Wrapper get_neuron(string name);
private:
  map<string, TN_Wrapper> neuron_types;

};

class TNParser {
public:

  TNParser(string filename) {
    this->filename = filename;
  }
  TNParser() {
    string fn = "./th_small_test.json";
    this->filename = fn;
  }

  void parse_tn_json();
  void create_lua();

private:
  string create_tn_string(string prototype_name);
  string create_tn_string();
  void load_file();
  void write_lua_line();
  void write_lua_header();
  string filename;
  ofstream json_file;
  NeuronTypeLib neuron_templates;
  map<string, CrossbarType> crossbar_templates;

//  map<string,TN_Wrapper> neurons;

};

TN_Main create_tn_data(string filename);

#ifndef SUPERNEMO_TN_PARSER_HH
#define SUPERNEMO_TN_PARSER_HH

#endif //SUPERNEMO_TN_PARSER_HH
