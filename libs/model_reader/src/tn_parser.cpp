//
// Created by Mark Plagge on 8/8/18.
//

#include "../include/tn_parser.hh"

int char2int(char input) {

  if (input >= '0' && input <= '9')
    return input - '0';
  if (input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if (input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  throw std::invalid_argument("Invalid input string");
}

void int2bit(int input, bool v[4]) {
  char binary[16][5] =
      {"0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101",
       "1110", "1111"};

}
/** @defgroup nemo_import NeMoFunct
 * These functions are imported from NeMo. They define various neuron creation functions.
 * Rather than link this JSON lib into NeMo (making a NeMo Library), I re-defined the basic
 * creation functions here.
 */
/**
 * Extracted from NeMo
 * @param core
 * @param coreLocal
 * @return
 */


/** @defgroup state NeuronState Neuron state initialization routines. These initialize the state of a
 * neuron - implemented inside TN_State_Wrapper, and using outside functions pulled from the NeMo init
 * routines. Includes helper functions.
 */


tw_lpid getGIDFromLocalIDs(id_type core, id_type coreLocal) {

  return (core * CORE_SIZE) + coreLocal;
}

tw_lpid getNeuronGlobal(id_type core, id_type neuronID) {
  id_type coreLocal = AXONS_IN_CORE + SYNAPSES_IN_CORE + neuronID;
  return getGIDFromLocalIDs(core, coreLocal);
}

void TN_set_neuron_dest(int signalDelay, uint64_t gid, tn_neuron_state *n) {
  n->delayVal = signalDelay;
  n->outputGID = gid;

}


id_type getCoreFromGID(tw_lpid gid) {
  return (gid/CORE_SIZE);
}


tw_lpid coreOffset(tw_lpid gid) {

  return (getCoreFromGID(gid)*CORE_SIZE);
}


/**
 * Assumes linear mapping of GIDs.
 *
 */
id_type getLocalFromGID(tw_lpid gid) {
  id_type coreOff = coreOffset(gid);
  return (gid - coreOff);

}


void tn_create_neuron(id_type coreID, id_type nID,
                      bool synapticConnectivity[NEURONS_IN_CORE],
                      short G_i[NEURONS_IN_CORE], short sigma[4], short S[4],
                      bool b[4], bool epsilon, short sigma_l, short lambda,
                      bool c, uint32_t alpha, uint32_t beta, short TM, short VR,
                      short sigmaVR, short gamma, bool kappa,
                      tn_neuron_state *n, int signalDelay,
                      uint64_t destGlobalID, int destAxonID) {
  for (int i = 0; i < 4; i++) {
    n->synapticWeight[i] = sigma[i] * S[i];
    n->weightSelection[i] = b[i];
  }
  for (int i = 0; i < NEURONS_IN_CORE; i++) {
    n->synapticConnectivity[i] = synapticConnectivity[i];
    n->axonTypes[i] = G_i[i];
  }

  // set up other parameters
  n->myCoreID = coreID;
  n->myLocalID = nID;
  n->epsilon = epsilon;
  n->sigma_l = sigma_l;
  n->lambda = lambda;
  n->c = c;
  n->posThreshold = alpha;
  n->negThreshold = beta;
  // n->thresholdMaskBits = TM;
  // n->thresholdPRNMask = getBitMask(n->thresholdMaskBits);
  n->sigmaVR = SGN(VR);
  n->encodedResetVoltage = VR;
  n->resetVoltage = VR;  //* sigmaVR;

  n->resetMode = gamma;
  n->kappa = kappa;


  //! @TODO: perhaps calculate if a neuron is self firing or not.
  n->firedLast = false;
  n->heartbeatOut = false;
  // n->isSelfFiring = false;
  // n->receivedSynapseMsgs = 0;

  TN_set_neuron_dest(signalDelay, destGlobalID, n);

  // synaptic neuron setup:
  n->largestRandomValue = n->thresholdPRNMask;
  if (n->largestRandomValue > 256) {
    tw_error(TW_LOC, "Error - neuron (%i,%i) has a PRN Max greater than 256\n ",
             n->myCoreID, n->myLocalID);
  }
  // just using this rather than bit shadowing.

  n->dendriteLocal = destAxonID;
  n->outputGID = destGlobalID;

  // Check to see if we are a self-firing neuron. If so, we need to send
  // heartbeats every big tick.
  n->isSelfFiring =
      false;  //!@TODO: Add logic to support self-firing (spontanious) neurons
}

void tn_create_neuron_encoded_rv(
    id_type coreID, id_type nID, bool synapticConnectivity[NEURONS_IN_CORE],
    short G_i[NEURONS_IN_CORE], short sigma[4], short S[4], bool b[4],
    bool epsilon, short sigma_l, short lambda, bool c, uint32_t alpha,
    uint32_t beta, short TM, short VR, short sigmaVR, short gamma, bool kappa,
    tn_neuron_state *n, int signalDelay, uint64_t destGlobalID,
    int destAxonID) {
  tn_create_neuron(coreID, nID, synapticConnectivity, G_i, sigma, S, b, epsilon,
                   sigma_l, lambda, c, alpha, beta, TM, VR, sigmaVR, gamma,
                   kappa, n, signalDelay, destGlobalID, destAxonID);
  n->sigmaVR = sigmaVR;
  n->encodedResetVoltage = VR;
  n->resetVoltage = (int) (n->sigmaVR * (pow(2, n->encodedResetVoltage) - 1));
}

void tn_create_neuron_encoded_rv_non_global(
    int coreID, int nID, bool synapticConnectivity[NEURONS_IN_CORE],
    short G_i[NEURONS_IN_CORE], short sigma[4], short S[4], bool b[4],
    bool epsilon, int sigma_l, int lambda, bool c, int alpha,
    int beta, int TM, int VR, int sigmaVR, int gamma, bool kappa,
    tn_neuron_state *n, int signalDelay, int destCoreID,
    int destAxonID) {
  uint64_t dest_global = getNeuronGlobal(destCoreID, destAxonID);
  n->dendriteLocal = destAxonID;
  n->outputNeuronDest = destAxonID;
  n->outputCoreDest = destCoreID;
  tn_create_neuron_encoded_rv(coreID,
                              nID,
                              synapticConnectivity,
                              G_i,
                              sigma,
                              S,
                              b,
                              epsilon,
                              sigma_l,
                              lambda,
                              c,
                              alpha,
                              beta,
                              TM,
                              VR,
                              sigmaVR,
                              gamma,
                              kappa,
                              n,
                              signalDelay,
                              dest_global,
                              destAxonID);
}

void TN_State_Wrapperinitialize_state(vector<int> input_axon_connectivity,
                                      vector<short> input_axon_types,
                                      int output_core,
                                      int output_neuron,
                                      int source_core,
                                      int source_local,
                                      int dest_delay) {

}
void TN_State_Wrapper::initialize_state(int *input_axon_connectivity,
                                        short *input_axon_types,
                                        int output_core,
                                        int output_neuron,
                                        int source_core,
                                        int source_local,
                                        int dest_delay) {

}
///**
// * initialize_state - Given a pre-init tn_neuron_state, will set
// * the *tn pointer to the address of the passed in value.
// * @param ext_n Neuron structure
// */
//void TN_State_Wrapper::initialize_state(tn_neuron_state *ext_n) {
//  tn = ext_n;
//}

/** @} ********************************************************************************************************* */
void init_tn_neuron_state_vecs(tn_neuron_state_vecs *nvec) {
  /*
   *
   * int axonTypes[AXONS_IN_CORE];
   * int synapticWeight[NUM_NEURON_WEIGHTS];
   * bool synapticConnectivity[AXONS_IN_CORE];
   * bool weightSelection[NUM_NEURON_WEIGHTS];
   */
  for (int i = 0; i < NEURONS_IN_CORE; i++) {
    nvec->axonTypeVec.push_back(nvec->n->axonTypes[i]);
    nvec->synapticConVec.push_back(nvec->n->axonTypes[i]);
  }
  for (int i = 0; i < NUM_NEURON_WEIGHTS; i++) {
    nvec->synapticWeightVec.push_back(nvec->n->synapticWeight[i]);
    nvec->weightSelVec.push_back(nvec->n->synapticWeight[i]);
  }

}
/**
 * hex2bool_insert
 * Converts a hex char into a boolean array, representing the synapse connectivity matrix. Done
 * using hard-coded values to ensure correct ordering across multiple platforms.
 * @param hex
 * @return
 */
vector<bool> hex2bool_insert(char hex) {

  bool boolv[16][4] = {{false, false, false, false}, {false, false, false, true}, {false, false, true, false},
                       {false, false, true, true}, {false, true, false, false}, {false, true, false, true},
                       {false, true, true, false}, {false, true, true, true}, {true, false, false, false},
                       {true, false, false, true}, {true, false, true, false}, {true, false, true, true},
                       {true, true, false, false}, {true, true, false, true}, {true, true, true, false},
                       {true, true, true, true}};
  int cv = char2int(hex);
  vector<bool> return_val(boolv[cv], boolv[cv] + 4);
  //bool_vec.insert(bool_vec.end(),boolv[cv],boolv[cv]+4);

  return return_val;
  /*
case '0' :
  return_val = {false,false,false,false};
  break;
case '1' :
  return_val = {false,false,false,true};
  break;
case '2' :
  return_val = {false,false,true,false};
  break;
case '3' :
  return_val = {false,false,true,true};
  break;
case '4' :
  return_val = {false,true,false,false};
  break;
case '5' :
  return_val = {false,true,false,true};
  break;
case '6' :
  return_val = {false,true,true,false};
  break;
case '7' :
  return_val = {false,true,true,true};
  break;
case '8' :
  return_val = {true,false,false,false};
  break;
case '9' :
  return_val = {true,false,false,true};
  break;
case 'A' :
  return_val = {true,false,true,false};
  break;
case 'B' :
  return_val = {true,false,true,true};
  break;
case 'C' :
  return_val = {true,true,false,false};
  break;
case 'D' :
  return_val = {true,true,false,true};
  break;
case 'E' :
  return_val = {true,true,true,false};
  break;
case 'F' :
  return_val = {true,true,true,true};
  break;

  } */

}
/**
 * fullhex2bool -
 * given a synaptic connectivity line (from the TN JSON file), returns a vector of bools representing the
 * connectivity matrix.
 * @param hexstr
 * @return
 */
vector<bool> fullhex2bool(string hexstr) {
  vector<bool> bool_vec;
  for (auto it: hexstr) {
    if (it != ' ') {
      vector<bool> single = hex2bool_insert(it);
      bool_vec.insert(bool_vec.end(), single.begin(), single.end());
    }
  }
  return bool_vec;

}

void TN_Crossbar_Type::add_synapse_rows(vector<string> types, vector<string> synapses) {
  for (int i = 0; i < synapses.size(); i++) {
    vector<bool> synrow = fullhex2bool(synapses[i]);
    rows.push_back(synrow);
    this->types.push_back(types[i]);
  }
}

vector<bool> TN_Crossbar_Type::get_connectivity(int neuron) {
  // given a neuron position, returns the column of neurons.
  vector<bool> connectivity_row;
  for (int i = 0; i < rows.size(); i++) {
    connectivity_row.push_back(rows[i][neuron]);
  }
  return connectivity_row;
}
short TN_Crossbar_Type::get_type(int loc) {
  string type = types[loc];
  return get_type(type);
}
short TN_Crossbar_Type::get_type(string type) {
  std::string::size_type pos;
  type[0] = ' '; //Strings in the JSON file type params are of the form "S#", where # is 0-3.

  int type_i = stoi(type, &pos, 10);
  return (short) type_i;

}
vector<short> TN_Crossbar_Type::get_types(int neuron) {
  //given a neuron position, returns the column types.
  vector<short> type_num;
  for (const auto &item : types) {
    type_num.push_back(get_type(item));
  }
  return type_num;
}

void TN_Neuron_Type::set_name(std::string name) {
  int id = 0;
  this->name.append(name);
  name.erase(name.begin());
  this->name_id = std::stoul(name, nullptr, 16);

}

void TN_Neuron_Type::init_neuron_from_json_doc(rapidjson::Document json_doc) {
  // for each variable a neuron needs, parse it:


}
/**
 * generate_csv - Returns a CSV representation of this neuron. Primarally used
 * for debugging. Will be in the format:
 * core_id,neuron_id,output_core,output_axon,crossbar_con,synapse_types,
 * sigmas,S,b,var_types,lambda, and so on.
 * @return
 */
string TN_State_Wrapper::generate_csv() {

  //string out_data;
  tn_neuron_state_vecs *s = (tn_neuron_state_vecs *) tn;

  string ret_v;
  return ret_v;
}
/**
 * Generates a json version of this neuron. Used for debug.
 * JSON output here is neuron-wise rather than the interleaved format of the TN system.
 * @return
 */
json TN_State_Wrapper::generate_json(json j) {
  //string ret_v;
  auto *s = (tn_neuron_state_vecs *) calloc(sizeof(tn_neuron_state_vecs), 1);
  //tn_neuron_state_vecs *s;
  s->n = tn;
  init_tn_neuron_state_vecs(s);
  char name[50];
  sprintf(name, "TN_%i_%i", tn->myCoreID, tn->myLocalID);

  //given the structs, go through and assign the values to the json doc.
#define placet(name) j[#name] = name;
#define jp(x) j["neurons"][name][x]

//  j[name]["alpha"] = tn->posThreshold;
//  j[name]["beta"] = tn->negThreshold;
//  j[name]["destCore"] = tn->outputCoreDest;
//  j[name]["destLocal"] = tn->outputNeuronDest;
//  j[name]["lambda"] = tn->lambda;
//  j[name]["gamma"] = tn->resetMode;
  jp("myCore") = tn->myCoreID;
  jp("myLocal") = tn->myLocalID;
  jp("alpha") = tn->posThreshold;
  jp("beta") = tn->negThreshold;
  jp("destCore") = tn->outputCoreDest;
  jp("destLocal") = tn->outputNeuronDest;
  jp("destLocalC") = getLocalFromGID(tn->outputGID);
  jp("destCoreC") = getCoreFromGID(tn->outputGID);
  jp("lambda") = tn->lambda;
  jp("gamma") = tn->resetMode;
  jp("reset_voltage") = tn->resetVoltage;
  jp("sigmaVR") = tn->sigmaVR;
  jp("VR") = tn->encodedResetVoltage;
  jp("sigma_l") = tn->sigma_l;
  jp("delay") = tn->delayVal;
  jp("isOutput") = tn->isOutputNeuron;
  jp("epsilon") = tn->epsilon;
  jp("c") = tn->c;
  jp("kappa") = tn->kappa;
  jp("isActive") = tn->isActiveNeuron;
  jp("axonTypes") = s->axonTypeVec;
  jp("weight_selection") = s->weightSelVec;
  jp("connectivity") = s->synapticConVec;
  jp("b_j") = s->weightSelVec;
  jp("outputGID") = tn->outputGID;
  jp("membranePotential") = tn->membranePotential;
  jp("dend_local") = tn->dendriteLocal;
  jp("Mj_prnMask") = tn->thresholdPRNMask;







  //ret_v = j.dump();
  return j;
}

TN_State_Wrapper TN_Neuron_Type::new_neuron_state(TN_Crossbar_Type crossbar,
                                                  int output_core,
                                                  int output_neuron,
                                                  int source_core,
                                                  int dest_delay,
                                                  int neuron_id) {
  vector<bool> input_con_i = crossbar.get_connectivity(neuron_id);
  vector<short> input_axon_types = crossbar.get_types(neuron_id);
  vector<unsigned int> input_axon_con;
  for (auto i:input_con_i) {
    input_axon_con.push_back((unsigned int) i);
  }
  TN_State_Wrapper st = new_neuron_state(input_axon_con,
                                         input_axon_types,
                                         output_core,
                                         output_neuron,
                                         source_core,
                                         neuron_id,
                                         dest_delay);
  st.myCore = source_core;
  return st;
}
void TN_Neuron_Type::new_neuron_state_init_struct(TN_Crossbar_Type crossbar,
                                                  int output_core,
                                                  int output_neuron,
                                                  int source_core,
                                                  int dest_delay,
                                                  int neuron_id,
                                                  tn_neuron_state *n) {
  vector<bool> input_con_i = crossbar.get_connectivity(neuron_id);
  vector<short> input_axon_types = crossbar.get_types(neuron_id);
  vector<unsigned int> input_axon_con;
  for (auto i:input_con_i) {
    input_axon_con.push_back((unsigned int) i);
  }
  new_neuron_state_init_struct(input_axon_con,
                               input_axon_types,
                               output_core,
                               output_neuron,
                               source_core,
                               neuron_id,
                               dest_delay,
                               n);

}
TN_State_Wrapper TN_Neuron_Type::new_neuron_state(vector<unsigned int> input_axon_connectivity,
                                                  vector<short> input_axon_types,
                                                  int output_core,
                                                  int output_neuron,
                                                  int source_core,
                                                  int source_local,
                                                  int dest_delay) {
  TN_State_Wrapper tn_neuron;
  tn_neuron.init_empty();
  //tn_neuron_state *n = (tn_neuron_state *) calloc(1, sizeof(tn_neuron_state));
  tn_neuron_state *n = tn_neuron.getTn();
  //this->tn = n;
  new_neuron_state_init_struct(input_axon_connectivity,input_axon_types,output_core,output_neuron,source_core,
      source_local, dest_delay, n);
  return tn_neuron;

}

/**
 * Sets the values in a tn_neuron_state struct to the parameters stored in this TN_Neuron_Type prototype,
 * along with the extra given parameters. Assumes n is pre-allocated struct.
 * @param input_axon_connectivity
 * @param input_axon_types
 * @param output_core
 * @param output_neuron
 * @param source_core
 * @param source_local
 * @param dest_delay
 * @param n
 */
void TN_Neuron_Type::new_neuron_state_init_struct(vector<unsigned int> input_axon_connectivity,
                                                  vector<short> input_axon_types,
                                                  int output_core,
                                                  int output_neuron,
                                                  int source_core,
                                                  int source_local,
                                                  int dest_delay,
                                                  tn_neuron_state *n) {

  map<string, int> v = var_types;
  short sigma[4];
  sigma[0] = (short) v["sigma0"];
  sigma[1] = (short) v["sigma1"];
  sigma[2] = (short) v["sigma2"];
  sigma[3] = (short) v["sigma3"];
  short S[4];
  S[0] = (short) v["s0"];
  S[1] = (short) v["s1"];
  S[2] = (short) v["s2"];
  S[3] = (short) v["s3"];
  bool b[4];
  b[0] = (bool) v["b0"],
  b[1] = (bool) v["b1"],
  b[2] = (bool) v["b2"];
  b[3] = (bool) v["b3"];
  n->synapticWeight[0] = var_types["sigma0"];
  n->synapticWeight[1] = var_types["sigma1"];
  n->synapticWeight[2] = var_types["sigma2"];
  n->synapticWeight[3] = var_types["sigma3"];
  n->weightSelection[0] = var_types["b0"];
  n->weightSelection[1] = var_types["b1"];
  n->weightSelection[2] = var_types["b2"];
  n->weightSelection[3] = var_types["b3"];
  n->lambda = var_types["lambda"];
  n->sigma_l = var_types["sigma_lambda"];
  n->c = var_types["c_lambda"];
  n->epsilon = var_types["epsilon"];
  n->posThreshold = var_types["alpha"];
  n->negThreshold = var_types["beta"];
  n->sigmaVR = var_types["sigma_VR"];
  n->resetMode = var_types["gamma"];

  //bool *synapticConnectivity = (bool *) input_axon_connectivity.data();
  //short *input_axon_tps = input_axon_types.data();
  unsigned int *synapticConnectivity = &input_axon_connectivity[0];
  short *input_axon_tps = &input_axon_types[0];

  tn_create_neuron_encoded_rv_non_global(source_core,
                                         source_local,
                                         (bool *) synapticConnectivity,
                                         input_axon_tps,
                                         sigma,
                                         S,
                                         b,
                                         (bool) v["epsilon"],
                                         v["sigma_lambda"],
                                         v["lambda"],
                                         (bool) v["c_lambda"],
                                         v["alpha"],
                                         v["beta"],
                                         v["TM"],
                                         v["VR"],
                                         v["sigma_VR"],
                                         v["gamma"],
                                         (bool) v["kappa"],
                                         n,
                                         dest_delay,
                                         output_core,
                                         output_neuron);
  n->membranePotential = v["V"];
}


string TN_Neuron_Type::get_name() {
  return name;
}
int TN_Neuron_Type::get_name_id() {
  return name_id;
}
void TN_Neuron_Type::init_neuron_from_json_arr(rapidjson::GenericValue<rapidjson::UTF8<char>,
                                                                       rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> > &json_neuron) {
  for (auto &it : json_neuron.GetObject()) {
    int type = var_types[it.name.GetString()];
    string key = it.name.GetString();
    //cout << it.name.GetString() << '\n';
    //store the vaules in the map - this is the easier way to do this:
    int val = -999;
    if (type == 0) { // name
#ifdef DEBUG
      string namex = it.value.GetString();
      if (namex.compare("N00000019") == 0) {
        cout << "Neuron 19\n";
      }
#endif
      set_name(it.value.GetString());
    } else if (type == 1) { // class
      n_class = it.value.GetString();
    } else { // everything else!
      //Is this a bool?
      if (it.value.IsBool()) {
        bool v2 = it.value.GetBool();
        val = v2;
      } else {
        val = it.value.GetInt();
      }
    }
    var_types[key] = val;
//        switch (type) {
//            case 0: //name
//                //var_types[key] = it.value.GetString();
//                set_name(it.value.GetString());
//                break;
//            case 1: //
//                n_class = it.value.GetString();
//                break;
//            case 2:
//            case 3:
//            case 4:
//            case 5:
//            case 6:
//            case 7:
//            case 8:
//                break;
//
//            default:
//                break;
//        }
    // We don't need to do this - however, it is handy for debugging purposes
  }
}


/**
 * Initializes a core from a json iterator.
 * @param obj
 */
//void TN_Core::init(rapidjson::GenericObject obj){

//}

/**
 * Returns a map<string, TN_NeuronType> which represents a neuron type library.
 * Note that the key in the map points to the JSON neuron number ID
 * (which must be extracted from the string, and is retrieved from TN_Neuron_Type::get_name_id
 * @param json_doc
 * @return
 */
map<int, TN_Neuron_Type> generate_neurons_from_json(rapidjson::Document &json_doc) {
  map<int, TN_Neuron_Type> neuron_lib;
  //create a map, a new TN Neuron Type, and append it to the map for
  //each neuron prototype in the TN Json File.
  auto n_type_arr = json_doc["neuronTypes"].GetArray();
  for (auto &it : n_type_arr) {
    TN_Neuron_Type n;
    n.init_neuron_from_json_arr(it);
    neuron_lib[n.get_name_id()] = n;
  }
  return neuron_lib;
}
/*"core":{
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
},*/


typedef struct metadata {
  string coreletClass;
  int coreletID;
  int coreNumber;
  vector<int> parentCoreletID;
  int layerNumber;
  string layerType;
} core_metadata;

/**
 * converts string values to an integer array - based on TN spec.
 * @param tn_value
 */
vector<int> convert_tn_arr(const string &tn_value) {
  std::size_t found_c = tn_value.find(':');
  std::size_t found_x = tn_value.find('x');
  std::string segment;
  std::vector<std::string> seglist;

  stringstream tn_value_str(tn_value);
  vector<int> result;
  //debug:
  std::size_t found_dbg = tn_value.find("178:181");
  if (found_dbg != std::string::npos) {
    cout << "... \n";
  }
  if (found_c != std::string::npos) {
    int num_cols = 0;
    //Found a ":" - so we iterate through the values
    while (std::getline(tn_value_str, segment, ':')) {
      num_cols++;
      seglist.push_back(segment);
    }
    if (num_cols == 2) {
      //iterate from pos 0 to pos 1.
      int start = stoi(seglist[0], NULL, 10);
      int end = stoi(seglist[1], NULL, 10);
      int second_end = end + start;
      while (start <= end) {
        result.push_back(start);
        start++;
      }
//      for (int i = start; i < end; i++) {
//        result.push_back(i);
//      }
    } else if (num_cols == 3) {
      //two colons means:
      //start:increment:end
      std::string::size_type pos;
      int start = stoi(tn_value, &pos, 10);
      pos++;
      string sub1 = tn_value.substr(pos);
      int increment = stoi(sub1, &pos, 10);
      pos++;
      string sub2 = sub1.substr(pos);
      int end = stoi(sub2, &pos, 10);
      while (start <= end) {
        result.push_back(start);
        start += increment;
      }
//      for (int i = start; i < end; i ++){
//        result.push_back(start);
//        start += increment;
//      }


    } else {
      cout << "Found " << num_cols << " columns - not a valid number of columns\n";
    }

  } else if (found_x != std::string::npos) {
    std::string::size_type pos;
    int value = stoi(tn_value, &pos, 10);
    int num_itr = stoi(tn_value.substr(pos + 1), NULL, 10);
    for (int i = 0; i < num_itr; i++) {
      result.push_back(value);
    }

  } else {
    cout << "Error - found something unexpecteD: " << tn_value << "\n";
  }

  return result;

}

int convert_and_add_value(int *&vals, string val) {
  vector<int> valv = convert_tn_arr(val);
  for (auto it : valv) {
    *vals = it;
    vals++;
  }
}
int convert_and_add_value(int *&vals, int val) {
  *vals = val;
  vals++;
}
void convert_and_add_value(int *&vals, const Value &val) {
  if (val.IsString()) {
    string val_s = val.GetString();
    convert_and_add_value(vals, val_s);
  } else if (val.IsInt()) {
    convert_and_add_value(vals, val.GetInt());
  } else {
    cout << "Invalid conversion value! \n ";

  }
}



// int convert_and_add_value(int *vals, int arr_pos,rapidjson::GenericObject obj){
//
// }
/**
 * assign (v, name) - macro to reduce code size in the TN_Core::init_core_from_itr() function.
 * Simply extracts the value called name form v, and assigns it to the variabled named name.
 */
#define assign(v, name) (name) = (v)[#name]

/**
* sets up this TN_Core object from the iterator - json doc object.
* @param itr
*/
void TN_Core::init_core_from_itr(Value::ConstMemberIterator itr) {
  //itr->value.GetObject()
  auto main_obj = itr->value.GetObject();
  auto meta_obj = main_obj["metadata"].GetObject();
  auto neuron_obj = main_obj["neurons"].GetObject();
//  coreletClass = ob["coreletClass"].GetString();
//  coreletID = ob["coreletId"].GetInt();
//  coreNumber = ob["coreNumber"].GetInt();


  //base core data:
  //"id":4,
  //		"timeScaleExponent":0,
  //		"rngSeed":4294967295,
  assign(main_obj, timeScaleExponent).GetInt();
  assign(main_obj, rngSeed).GetUint();
  assign(main_obj, id).GetUint();


  // Metadata ----
  //		"metadata":{
  //			"coreletClass":"th_corelet_layer_cores",
  //			"coreletId":2,
  //			"coreNumber":4,
  //			"parentCoreletId":[3,1,0],
  //			"layerNumber":2,
  //			"layerType":"conv"

  assign(meta_obj, coreletClass).GetString();
  assign(meta_obj, coreletId).GetInt();
  assign(meta_obj, coreNumber).GetInt();
  if(meta_obj.HasMember("layerNumber")) {
    assign(meta_obj, layerNumber).GetInt();
    assign(meta_obj, layerType).GetString();
  }else{
    layerNumber = 0;
    layerType="None";
  }


  crossbar_name = main_obj["crossbar"]["name"].GetString();

  // Parse Arrays:
  for (auto &el : main_obj["metadata"]["parentCoreletId"].GetArray()) {
    parentCorletId.push_back(el.GetInt());
  }
  //Parse the dendrites, types, destCores, destAxons, destDelays arrays:
  //dendrites:
  //Debug Code:
  vector<int> testDendrite;

  int arr_pos = 0;
#ifdef DEBUG
  for (auto &el : neuron_obj["dendrites"].GetArray()) {
    if (el.IsString()) {
      vector<int> vals = convert_tn_arr(el.GetString());
      for (auto it : vals) {
        dendrites[arr_pos] = it;
        arr_pos++;

        testDendrite.push_back(it);

      }
    } else {
      dendrites[arr_pos] = el.GetInt();

      testDendrite.push_back(el.GetInt());

      arr_pos++;
    }
  }
#endif

  //Test code:
  arr_pos = 0;
  int *d_ptr = dendrites;
  for (auto &el : neuron_obj["dendrites"].GetArray()) {
    convert_and_add_value(d_ptr, el);
  }
#ifdef DEBUG
  int test_v = 0;
  for (auto x: testDendrite) {
    if (dendrites[test_v] != x) {
      cout << "Found error in dendrite array.\n";
    }
    test_v++;
  }
#endif
  // added function wrapper for the next ones:
  arr_pos = 0;
  int *t_ptr = types;
  for (auto &el : neuron_obj["types"].GetArray()) {
    convert_and_add_value(t_ptr, el);
  }
  int *dc_ptr = destCores;
  for (auto &el : neuron_obj["destCores"].GetArray()) {
    convert_and_add_value(dc_ptr, el);
  }
  int *da_ptr = destAxons;
  for (auto &el : neuron_obj["destAxons"].GetArray()) {
    convert_and_add_value(da_ptr, el);
  }
  int *dd_ptr = destDelays;
  for (auto &el : neuron_obj["destDelays"].GetArray()) {
    convert_and_add_value(dd_ptr, el);
  }



//  assign("parentCoreletId").GetArray();




}

/**
 * Given a json_doc (rapidjson), iterate through the cores and parse them.
 * Each core's object is sent to the TN_Core::generate_core_from_itr() function.
 * @param json_doc
 * @return
 */
map<int, TN_Core> generate_cores_from_json(rapidjson::Document &document) {
  map<int, TN_Core> core_lib;

  for (Value::ConstMemberIterator itr = document.FindMember("core"); itr != document.MemberEnd(); ++itr) {
    TN_Core core;
    core.init_core_from_itr(itr);
    core_lib[itr->value.GetObject()["id"].GetInt()] = core;
  }

  return core_lib;
}

//TN_Neuron_Type TN_Main::generate_neuron_from_id(int coreID, int neuronID) {
//  //first we need the core:
//  TN_Core core = TN_Cores[to_string(coreID)];
//  //next, each core has
//  TN_Crossbar_Type crossbar = TN_Crossbar_Type_library[]
//}


core_info TN_Core::get_neuron_info(int neuronID) {
  core_info neuron;
  neuron.type = types[neuronID];
  neuron.dendrite = dendrites[neuronID];
  neuron.destCore = destCores[neuronID];
  neuron.destAxon = destAxons[neuronID];
  neuron.destDelay = destDelays[neuronID];
  return neuron;
}
//core_info *TN_Core::get_neuron_info(int neuronID) {
//  core_info *neuron = (core_info *) calloc(1, sizeof(core_info));
//  neuron->type = types[neuronID];
//  neuron->dendrite = dendrites[neuronID];
//  neuron->destCore = destCores[neuronID];
//  neuron->destAxon = destAxons[neuronID];
//  neuron->destDelay = destDelays[neuronID];
//  return neuron;
//}
int TN_Core::getLayerNumber() const {
  return layerNumber;
}
const string &TN_Core::getLayerType() const {
  return layerType;
}
int TN_Core::getId() const {
  return id;
}

const string &TN_Core::getCrossbar_name() const {
  return crossbar_name;
}




bool TN_Main::set_current_prot_cros(unsigned long coreID, unsigned long neuronID) {
  TN_Core core = TN_Neuron_Core_Library[coreID];
  //core_info *neuron = core.get_neuron_info(neuronID);
  cur_neuron = core.get_neuron_info(neuronID);

  //see if neuron prototype exists in library:
  auto it = TN_Neuron_Library.find(cur_neuron.type);

  if (it != TN_Neuron_Library.end()) {
    //element found;
    TN_Neuron_Type prototype = TN_Neuron_Library[cur_neuron.type];
    TN_Crossbar_Type crossbar = TN_Crossbar_Type_library[core.getCrossbar_name()];

    //! @todo is this a memory leak in C++???? I don't know if this is on the stack or the heap!
    cur_prototype = prototype;
    cur_crossbar = crossbar;
    return true;
  }else{
    return false;
  }
}

/**
 * generate_neuron_from_id - This returns a TN_State_Wrapper object wrapping either the found neuron from the
 * JSON data, or an error neuron if the neuron is not found. Note that not finding a neuron is not an
 * error per se, as not all neurons are declared or set in a neuron model file.
 * @param coreID the requested neuron's core id
 * @param neuronID the requested neuron's local id within a core.
 * @return
 */
TN_State_Wrapper TN_Main::generate_neuron_from_id(unsigned long coreID, unsigned long neuronID) {
  if(set_current_prot_cros(coreID,neuronID)){
    // good neuron, okay to make a new prototype.
    return cur_prototype.new_neuron_state(cur_crossbar,cur_neuron.destCore,cur_neuron.destAxon,coreID,cur_neuron.destDelay,neuronID);
  }else{
    TN_State_Wrapper error_neuron;
    return error_neuron;
  }
  //moved this code to a private function and state variables.
  /*
  TN_Core core = TN_Neuron_Core_Library[coreID];
  core_info neuron = core.get_neuron_info(neuronID);

  //see if neuron prototype exists in library:
  auto it = TN_Neuron_Library.find(neuron.type);

  if (it != TN_Neuron_Library.end()) {
    //element found;
    TN_Neuron_Type prototype = TN_Neuron_Library[neuron.type];
    TN_Crossbar_Type crossbar = TN_Crossbar_Type_library[core.getCrossbar_name()];
    TN_State_Wrapper result_neuron =
        prototype.new_neuron_state(crossbar, neuron.destCore, neuron.destAxon, coreID, neuron.destDelay, neuronID);
    return result_neuron;
  } else {
    //printf("Missing neuron prototype:\n Type: %i\t|coreID: %i\t|LocalID: %i \n", neuron->type, coreID, neuronID);


    TN_State_Wrapper error_neuron;
    //error_neuron.isValid = false;
    return error_neuron;
  }*/
}

void TN_Main::populate_neuron_from_id(unsigned long coreID, unsigned long neuronID, tn_neuron_state *n){
  if(set_current_prot_cros(coreID,neuronID)){
    // initialize tn_neuron_state *n
    cur_prototype.new_neuron_state_init_struct(cur_crossbar,cur_neuron.destCore,
        cur_neuron.destAxon,coreID,cur_neuron.destDelay,neuronID,n);
  }else{
    //set tn_neuron_state to an inactive neuron (using dest_core -1024 as debug info)
    n->outputCoreDest = -1024;
    n->isActiveNeuron = 0;
  }

}

tn_neuron_state *TN_Main::generate_neurons_in_core_struct(unsigned long coreID) {
  tn_neuron_state *neuron_array = (tn_neuron_state *) calloc(NEURONS_IN_CORE, sizeof(tn_neuron_state));
  //struct line* array = malloc(number_of_elements * sizeof(struct line));
  //neuron_array = (tn_neuron_state *) calloc(NEURONS_IN_CORE,sizeof(tn_neuron_state));
  for (int neuron_id = 0; neuron_id < NEURONS_IN_CORE; neuron_id++) {
    TN_State_Wrapper n = generate_neuron_from_id(coreID, neuron_id);
    if (!n.isValid) {

      n.getTn()->isActiveNeuron = false;
    } else {
      n.getTn()->isActiveNeuron = true;
    }
    tn_neuron_state *state_ptr = n.getTn();
    neuron_array[neuron_id] = *state_ptr;
  }

  return neuron_array;
}

vector<TN_State_Wrapper> TN_Main::generate_neurons_in_core_vec(int coreID) {
  vector<TN_State_Wrapper> neuron_vec;
  for (int neuron_id = 0; neuron_id < NEURONS_IN_CORE; neuron_id++) {
    TN_State_Wrapper n = generate_neuron_from_id(coreID, neuron_id);
    if (!n.isValid) {
      //n.init_empty();
      n.getTn()->isActiveNeuron = false;
    } else {
      n.getTn()->isActiveNeuron = true;
    }
    neuron_vec.push_back(n);

  }
  return neuron_vec;
}

TN_Output::TN_Output(string filename, TN_Main model, unsigned char output_mode) {
  json_init = 0;
  main_model = model;
  output_filename = filename;
  this->output_mode = output_mode;
  int mask = 1;
  std::size_t found_ext;
  for (const auto &x : model.TN_Neuron_Core_Library) {
    for (auto n : model.generate_neurons_in_core_vec(x.first)) {
      neurons.push_back(n);

    }

  }
  out_fn_csv = string(filename);
  out_fn_csv += ".csv";
  out_fn_json = string(filename);
  out_fn_json += ".json";
  cout <<"JSON filename recorded as " << out_fn_json << " from orig." << filename << " | " << output_filename << "\n";
  out_fn_bin = string(filename);
  out_fn_bin += ".dat";
  out_fn_lua = string(filename);
  out_fn_lua += ".nfg1";
  out_fn_py = string(filename);
  out_fn_py += ".py";



}

int TN_Output::write_csv() {
  int result = 0;
  ofstream file(out_fn_csv, std::ostream::out);

}

#ifdef  USE_OMP
json TN_Output::generate_json_mp(int start_core, int end_core){
  if (json_init){
    return main_json;
  }
  cout << "Using OMP to generate JSON structure.\n";
   std::string result;
  json j_group;
  size_t steps_completed = 0;
  size_t total_steps = neurons.size();
  tqdm bar;
  bar.set_label("Generating JSON file");
//parallel code
#pragma omp parallel
  {
    //cout <<"Within omp parallel...\n";
    json private_j;

#pragma omp for nowait schedule(static)
    for(int i = 0; i < neurons.size(); i ++){
      auto neuron_wrap = neurons[i];
      if (start_core != end_core){
        if(neuron_wrap.myCore >= start_core && neuron_wrap.myCore <= end_core){
          private_j = neuron_wrap.generate_json(private_j);
        }
      }else{
          private_j = neuron_wrap.generate_json(private_j);
      }
      #pragma omp atomic
      steps_completed ++;

      if(omp_get_thread_num() == 0)
      {
          bar.progress(steps_completed, total_steps);
      };

    }
#pragma omp for schedule(static) ordered
      for(int i = 0; i < omp_get_num_threads(); i ++){
#pragma omp ordered
      j_group.merge_patch(private_j);
    }

   };
  main_json = j_group;
  json_init = 1;
  return j_group;
}

int TN_Output::write_json_mp(int start_core, int end_core){

  json j_group = generate_json_mp(start_core, end_core);
  cout << "writing to " << out_fn_json  <<" j_group.\n";

  ofstream file(out_fn_json,std::ostream::out);
  //file << j_group.dump(4) << std::endl;
  file << std::setw(4) << j_group << std::endl;
  file.close();
  return 0;
}
#endif


json TN_Output::generate_json(int start_core, int end_core){
#ifdef USE_OMP
  return generate_json_mp(start_core, end_core);
#endif
  if (json_init){
    return main_json;
  }
  json j;
  tqdm bar;
  auto num = neurons.size();
  int count = 0;
  bar.set_label("Generating JSON structure");
  for (auto neuron_wrap : neurons) {
    if (start_core != end_core) {
      if (neuron_wrap.myCore >= start_core && neuron_wrap.myCore <= end_core) {
        j = neuron_wrap.generate_json(j);
      }
    } else {
      j = neuron_wrap.generate_json(j);
    }
    count++;
    bar.progress(count, num);

  }
  json_init = 1;
  main_json = j;
  return j;
}

int TN_Output::write_json(int start_core, int end_core) {
#ifdef USE_OMP
  cout << "\n using omp writer...\n";
  return write_json_mp(start_core, end_core);
#endif
  json j = generate_json(start_core, end_core);
  ofstream file(out_fn_json, std::ostream::out);
  file << j.dump(4) << std::endl;
  file.close();
  return 0;
}
int TN_Output::write_json() {

  return write_json(0, 0);

}

bool isLittleEndian()
{
  short int number = 0x1;
  char *numPtr = (char*)&number;
  return (numPtr[0] == 1);
}

/**
 * writes the header of the binary file. The header information is:
 * NUM_NEURONS_IN_FILE (64 bit unsigned integer)\n
 * NUM_CORES (64 bit unsigned integer)\n
 * ENDIANNESS (32 bit unsigned integer)\n
 * ---
 * the last value is the endianness of the machine used to save this data. is 1 if little endian, 2 otherwise.
 */
void TN_Output::write_bin_header(){
  uint64_t num_neurons = neurons.size();
  uint64_t num_cores = main_model.core_count;
  uint32_t endian = isLittleEndian();



}
int TN_Output::write_bin_split(){
  map<int,ofstream> output_files;
  vector<tn_neuron_state> neuron_vec;
  for (auto neuron:neurons){
    if (output_files.count(neuron.getTn()->myCoreID) == 1){
 //     output_files[]
    }
  }

}
int TN_Output::write_bin() {
  //create a vector of TN_states:
  vector<tn_neuron_state> neuron_vec;
  //open up the file.
  ofstream bof(out_fn_bin,ostream::out | ostream::binary);
  //binary_out_file = bof;


  write_bin_header();
  json j = generate_json(0,0);
  vector<std::uint8_t> v_mpack = json::to_msgpack(j);


  bof.write((char*) v_mpack.data(), v_mpack.size() * sizeof(tn_neuron_state));
  bof.close();

  //testing straight C binary output.
  //will write 1 line of text - metadata, then one line containing the number of TN Neurons.
  //Then will output full neuron data.

  //first create a neuron array:
  auto **neur_array = (tn_neuron_state **) calloc(this->neurons.size(),sizeof(tn_neuron_state *));
  //now create some neurons:
  int icr = 0;
  for(auto nwrap : neurons){
      tn_neuron_state *n = nwrap.getTn();
      neur_array[icr] = n;
  }
  //now set up metadata:
  long num_neurons = neurons.size();
  //open file:
  string sec_b = "raw_bin_" + out_fn_bin;
  FILE *f = fopen(sec_b.c_str(),"wb");
  fprintf(f,"%li\n",num_neurons);
  fwrite(neur_array,sizeof(tn_neuron_state),num_neurons,f);
  fclose(f);


  return 0;
}

int TN_Output::write_data() {

  //unsigned char output_mode = this->output_mode;
  if (output_mode & TN_OUT_CSV) {
    cout << "Will output CSV file. \n";
  }
  if (output_mode & TN_OUT_BIN) {
    cout << "Will output BIN file. \n";
    this->write_bin();

  }
  if (output_mode & TN_OUT_LUA) {
    cout << "Will output LUA file \n";
  }
  if (output_mode & TN_OUT_JSON) {
    cout << "Will output JSON \n";
    this->write_json();
  }
  if (output_mode & TN_OUT_PY) {
    cout << "Will output Python. \n";
  }

}

/**
 * create_tn_data: preps the neuron model from the json file. Given a JSON file (TN FORMAT),
 * this will generate the crossbar array templates, the neuron templates, and then generate the
 * core templates. Once done, the TN_Main object is returned, which can generate NeMo TN LPs.
 * @param filename The TN Filename
 * @return a new TN_Main .
 */
TN_Main create_tn_data(string filename) {

//  fstream json_file(filename);
//  //    char x[40];
//  //    json_file.read(x,30);
//  //    cout << x;
//
//  std::string json_str((std::istreambuf_iterator<char>(json_file)),
//                       std::istreambuf_iterator<char>());
cout << "\n - filename : " << filename << "\n";
  //string json_str = load_file_into_memory(filename);
  printf("Starting...\n");
  Document json_doc;
  char * json_str;
  FILE *f = fopen(filename.c_str(), "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);  //same as rewind(f);
  json_str = (char *) calloc(fsize +1024, 1);
  fread(json_str,fsize,1,f);
  fclose(f);
  printf("Read file. Parsing \n");
  json_doc.Parse<kParseCommentsFlag>(json_str);



  //std::ifstream json_file(filename);
  //std::ifstream json_file(filename);
  //json_doc.ParseStream<kParseCommentsFlag>(json_file);
  //First create the main core object
  TN_Main model;
  //populate the model data.
  model.core_count = json_doc["model"]["coreCount"].GetInt();
  model.crossbar_size = json_doc["model"]["crossbarSize"].GetInt();
  model.crossbar_class = json_doc["model"]["crossbarclass"].GetString();

  //now create the crossbar prototypes.
  map<string, TN_Crossbar_Type> crossbar_library;
  for (auto &v : json_doc["crossbarTypes"].GetArray()) {
    string crossbarName = v["name"].GetString();
    vector<string> types;
    vector<string> synapses;
    for (auto &r : v["crossbar"]["rows"].GetArray()) {
      types.push_back(r["type"].GetString());
      string row = r["synapses"].GetString();
      synapses.push_back(row);

    }
    TN_Crossbar_Type tp;
    tp.add_synapse_rows(types, synapses);
    tp.name = crossbarName;
    crossbar_library[crossbarName] = tp;
  }
  //add the crossbar prototypes to the main TN model
  model.TN_Crossbar_Type_library = crossbar_library;
  //Now parse the neuron prototypes  -
  map<int, TN_Neuron_Type> neuron_types = generate_neurons_from_json(json_doc);
  model.TN_Neuron_Library = neuron_types;

  auto n_type_arr = json_doc["neuronTypes"].GetArray();
//  for (auto &it : n_type_arr) {
//    cout << "NTI" << it["name"].GetString() << "\n";
//  }

  map<int, TN_Core> core_lib;
  core_lib = generate_cores_from_json(json_doc);
  model.TN_Neuron_Core_Library = core_lib;
  //return the main model.
#ifdef DEBUG
  long num_cores_in_js = model.core_count;
  tw_printf(TW_LOC, "Model loaded - found %li cores defined neurons\n", num_cores_in_js);
#endif
  free(json_str);
  return model;
}

