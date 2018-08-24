//
// Created by Mark Plagge on 8/8/18.
//

#include "tn_parser.hh"

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
tw_lpid getGIDFromLocalIDs(id_type core, id_type coreLocal) {

  return (core*CORE_SIZE) + coreLocal;
}

tw_lpid getNeuronGlobal(id_type core, id_type neuronID) {
  id_type coreLocal = AXONS_IN_CORE + SYNAPSES_IN_CORE + neuronID;
  return getGIDFromLocalIDs(core, coreLocal);
}

void TN_set_neuron_dest(int signalDelay, uint64_t gid, tn_neuron_state *n) {
  n->delayVal = signalDelay;
  n->outputGID = gid;

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
    n->synapticWeight[i] = sigma[i]*S[i];
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
  n->resetVoltage = (int) (n->sigmaVR*(pow(2, n->encodedResetVoltage) - 1));
}

void tn_create_neuron_encoded_rv_non_global(
    int coreID, int nID, bool synapticConnectivity[NEURONS_IN_CORE],
    short G_i[NEURONS_IN_CORE], short sigma[4], short S[4], bool b[4],
    bool epsilon, int sigma_l, int lambda, bool c, int alpha,
    int beta, int TM, int VR, int sigmaVR, int gamma, bool kappa,
    tn_neuron_state *n, int signalDelay, int destCoreID,
    int destAxonID) {
  uint64_t dest_global = getNeuronGlobal(destCoreID, destAxonID);
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
    if (it!=' ') {
      vector<bool> single = hex2bool_insert(it);
      bool_vec.insert(bool_vec.end(), single.begin(), single.end());
    }
  }
  return bool_vec;

}




//def createNeuronTemplateFromEntry(line, nSynapses=256, weights=4):
//"""
//Function that creates a new neuron from a dictionary. The dictionary is parsed from a line in the JSON file.
//:param line: The JSON line in a dictionary format
//:param nSynapses: number of synapses in the core
//:param weights: number of weights in the core
//:return: A new neuron shiny and chrome
//"""
//neuron = TN_Wrapper(nSynapses, weights)
//sigmas = []
//s = []
//b = []
//for i in range(0, weights):
//sigmas.append(line[f'sigma{i}'])
//s.append(line[f's{i}'])
//bs = line[f'b{i}']
//bs = 0 if bs == False else 1
//
//b.append(bs)
//
//neuron.S = s
//neuron.sigmaG = sigmas
//neuron.epsilon = line['epsilon']
//neuron.sigma_lmbda = line['sigma_lambda']
//neuron.lmbda = line['lambda']
//neuron.c = line['c_lambda']
//neuron.epsilon = line['epsilon']
//neuron.alpha = line['alpha']
//neuron.beta = line['beta']
//neuron.TM = line['TM']
//neuron.gamma = line['gamma']
//neuron.kappa = line['kappa']
//neuron.sigmaVR = line['sigma_VR']
//neuron.VR = line['VR']
//neuron.membrane_potential = line["V"]
//
//return neuron


const string &TN_Wrapper::getName() const {
  return name;
}
void TN_Wrapper::setName(const string &name) {
  TN_Wrapper::name = name;
}

void NeuronTypeLib::gen_type_map(Value &n_types) {
  for (auto &v : n_types.GetArray()) {
    TN_Wrapper n(v);
    //this->neuron_types.insert(std::pair<string,TN_Wrapper>(n.getName(),n));
    this->neuron_types.insert({n.getName(), n});
  }
}
TN_Wrapper NeuronTypeLib::get_neuron(string name) {
  return this->neuron_types.at(name);
}

void TN_Crossbar_Type::add_synapse_rows(vector<string> types, vector<string> synapses) {
  for (int i = 0; i < synapses.size(); i++) {
    vector<bool> synrow = fullhex2bool(synapses[i]);
    rows.push_back(synrow);
    types.push_back(types[i]);
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

void TN_Neuron_Type::set_name(std::string name) {
  int id = 0;
  this->name.append(name);
  name.erase(name.begin());
  this->name_id = std::stoul(name, nullptr, 16);

}
void TN_Neuron_Type::init_neuron_from_json_doc(rapidjson::Document json_doc) {
  // for each variable a neuron needs, parse it:

}

tn_neuron_state *TN_Neuron_Type::new_neuron_state(vector<int> input_axon_connectivity,
                                                  vector<short> input_axon_types,
                                                  int output_core,
                                                  int output_neuron,
                                                  int source_core,
                                                  int source_local,
                                                  int dest_delay) {

  tn_neuron_state *n = (tn_neuron_state *) calloc(1, sizeof(tn_neuron_state));
  //set up the variables // arrays
  map<string, int> v = var_types;
  //vector<short> sigma ({(short)v["sigma0"],(short)v["sigma1"],(short)v["sigma2"],(short)v["sigma3"]});
  short sigma[4];
  sigma[0] = (short)v["sigma0"];
  sigma[1] = (short)v["sigma1"];
  sigma[2] = (short)v["sigma2"];
  sigma[3] = (short)v["sigma3"];
  short S[4];
  S[0] =(short) v["s0"];
  S[1] =(short) v["s1"];
  S[2] =(short) v["s2"];
  S[3] =(short) v["s3"];
  bool b[4];
  b[0] =   (bool)v["b0"],
  b[1] =   (bool)v["b1"],
  b[2] =      (bool)v["b2"];
  b[3] =(bool)v["b3"];





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
  bool *synapticConnectivity = (bool *) input_axon_connectivity.data();
  short *input_axon_tps = input_axon_types.data();

  tn_create_neuron_encoded_rv_non_global(source_core,
                                         source_local,
                                         synapticConnectivity,
                                         input_axon_tps,
                                         sigma,
                                         S,
                                         b,
                                         (bool)v["epsilon"],
                                         v["sigma_lambda"],
                                         v["lambda"],
                                         (bool)v["c_lambda"],
                                         v["alpha"],
                                         v["beta"],
                                         v["TM"],
                                         v["VR"],
                                         v["sigma_VR"],
                                         v["gamma"],
                                         (bool)v["kappa"],
                                         n,
                                         dest_delay,
                                         output_core,
                                         output_neuron);
  n->membranePotential = v["V"];
//    tn_create_neuron_encoded_rv_non_global(source_core, <#id_type nID#>, <#bool *synapticConnectivity#>, <#short *G_i#>, <#short *sigma#>, <#short *S#>, <#bool *b#>, <#bool epsilon#>, <#short sigma_l#>, <#short lambda#>, <#bool c#>, <#uint32_t alpha#>, <#uint32_t beta#>, <#short TM#>, <#short VR#>, <#short sigmaVR#>, <#short gamma#>, <#bool kappa#>, <#tn_neuron_state *n#>, <#int signalDelay#>, <#uint64_t destCoreID#>, <#int destAxonID#>)
//
//
//    tn_create_neuron_encoded_rv_non_global((id_type) source_core, (id_type) source_local, input_axon_connectivity.data(), G_i, sigma, S, b, v["epsilon"], v["sigma_lambda"], v["lambda"], v["c"], v["alpha"], v["beta"], v["TM"], v["VR"], v["sigma_VR"], v["gamma"], v["kappa"], n, dest_delay, output_core, output_neuron);
  return n;

}

string TN_Neuron_Type::get_name(){
  return name;
}
int TN_Neuron_Type::get_name_id(){
  return name_id;
}
void TN_Neuron_Type::init_neuron_from_json_arr(rapidjson::GenericValue<rapidjson::UTF8<char>,
                                                                       rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> > &json_neuron) {
  //cout << "Neuron Name: " << json_array["name"].GetString();


  for (auto &it : json_neuron.GetObject()) {
    int type = var_types[it.name.GetString()];
    string key = it.name.GetString();
    cout << it.name.GetString() << '\n';
    //store the vaules in the map - this is the easier way to do this:
    int val = -999;
    if (type==0) { // name
      set_name(it.value.GetString());
    } else if (type==1) { // class
      n_class = it.value.GetString();
    } else { // everything else!
      //Is this a string?


      if (it.value.IsBool()) {
//                string f = "false";
//                string valn = it.value.GetString();
//                val = 0;
//                if (valn == f) {
//                    val = 1;
//                }
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
  string json_str = load_file_into_memory(filename);

  Document json_doc;
  json_doc.Parse<kParseCommentsFlag>(json_str);

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
  for (auto &it : n_type_arr) {
    cout << "NTI" << it["name"].GetString() << "\n";
  }


  //return the main model.

  return model;
}

