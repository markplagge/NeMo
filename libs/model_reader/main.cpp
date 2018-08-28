//
// Created by Mark Plagge on 8/8/18.
//

#include <cstdio>
#include "tn_parser.hh"
#include "json_dto.hh"
#include "tests/test_data.hh"




///////////////// DTO TEST ////////////////////////////
struct neurons{
  vector<string> dendrites;
};
typedef struct core {
  struct metadata {
    string coreletClass;
    int coreletID;
    int coreNumber;
  };
  int id;
  int timeScaleExponent;
  uint32_t rngSeed;
  struct neurons;
}Core;
struct core_holder{
  Core core;
  template< typename Json_Io >
  void json_io( Json_Io & io )
  {
    io & json_dto::mandatory( "core", core);
  }
};

struct multi_core{
  vector<Core> cores;
  template <typename Json_Io>
  void json_io(Json_Io & io){
    io & json_dto::mandatory("cores",cores);
  }
};
namespace json_dto {
template<typename Json_Io>
void json_io(Json_Io &io, core &m) {
  io & json_dto::optional("id", m.id,0);
  io & json_dto::optional("rngSeed",m.rngSeed,0);
  io & json_dto::optional("timeScaleExponent",m.timeScaleExponent,0);
}
}

string create_new_array(string json_string){
  Document json_doc;
  json_doc.Parse<kParseCommentsFlag>(json_string);

  cout <<"type is " << json_doc["core"].GetType() ;
  cout << " " << json_doc["core"].IsArray() << "\n";

  string return_val;
  return return_val;
}
string extract_cores(string json_string){

}
#define tm(x) (x) = #x ;
void testDTO() {
#ifdef DEBUG
  string test;
  tm(test);
  cout << "ROOT: " << SRC_ROOT;
  string fn = SRC_ROOT"/tests/tn_test_cf100.json";
  string data = load_file_into_memory(fn);
  auto msg = json_dto::from_json<core,kParseCommentsFlag>(data);
  auto msg2 = json_dto::from_json<core_holder,kParseCommentsFlag>(test_json_1core);
  auto msg3 = json_dto::from_json<core_holder,kParseCommentsFlag>(test_json_1core);

  auto msg4 = json_dto::from_json<multi_core,kParseCommentsFlag>(test_multi_core_processed);
  create_new_array(data);
#endif
}

void testCoreGen(){
#ifdef DEBUG
  string fn = SRC_ROOT"/tests/tn_test_cf100.json";
  string data = load_file_into_memory(fn);
  Document jdoc;
  jdoc.Parse<kParseCommentsFlag>(data);
  cout << jdoc["core"].IsArray() << "\n";
  cout << jdoc["core"].IsObject() << "\n";
  cout << jdoc["core"].IsArray() << "\n";
  cout << jdoc["core"].GetObject().HasMember("metadata");

  Document document;
  document.Parse<kParseCommentsFlag>(data);
  static const char* kTypeNames[] =
      { "Null", "False", "True", "Object", "Array", "String", "Number" };

  for (Value::ConstMemberIterator itr = document.MemberBegin();
       itr != document.MemberEnd(); ++itr)
  {
    printf("Type of member %s is %s\n",
           itr->name.GetString(), kTypeNames[itr->value.GetType()]);
    //auto x = itr->value.GetObject();
//    cout << "\n" << x["id"].GetInt() << "\n";
//    cout << itr->value.GetObject()["id"].GetInt() << "\n";
  }
  for (auto& m : document.GetObject())
    printf("Type of member %s is %s\n",
           m.name.GetString(), kTypeNames[m.value.GetType()]);
//  for ( : document.FindMember()){
//    cout << it->value.GetObject()["id"].GetInt() << "\n";
//  }
cout << "\n\n ------------------------------------ \n \n";
for (Value::ConstMemberIterator itr = document.FindMember("core"); itr != document.MemberEnd(); ++ itr){
  cout << "Member core's ID using find member: " <<
                                                 itr->value.GetObject()["id"].GetInt() <<"\n";
  auto arr = itr->value.GetObject()["neurons"]["destCores"].GetArray();
  for (auto &x : arr){
    if (x.IsString()){
      cout << x.GetString();
    }else{
      cout <<"int " << x.GetInt64();
    }

  }
}
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void tests() {

  //  //test the hexchar to bool:
  //
  //  //string t2 =  "0123456789ABCDEF";
  //  string t2 = "0123456789ABCDEF";
  //  cout << "\n-------------------------\n";
  //  vector<bool> hb = fullhex2bool(t2);
  //  int ii = 1;
  //  for (auto it: hb) {
  //
  //    cout << it;
  //    if (ii%4==0) {
  //      cout << "\n";
  //    }
  //    ii++;
  //  }
  ////    for (auto it : t2){
  ////      vector<bool> hb = fullhex2bool(it);
  ////      for(auto it2 : hb){
  ////        cout << it2;
  ////      }
  ////      cout << '\n';
  ////    }
  //  cout << "-------------------------\n";
  //
  //  printf("Model Reader.\n");
  //  cout << "Loading templates from JSON file...";
  //
  //  string test_hex = "51 40";
  //
  //  string testdata = "{\n"
  //                    "       /* comments in the JSON file */ "
  //                    "        \"model\":{\n"
  //                    "                \"coreCount\":4042,\n"
  //                    "                \"neuronclass\":\"NeuronGeneral\",\n"
  //                    "                \"crossbarSize\":256,\n"
  //                    "                \"crossbarclass\":\"CrossbarBinary\",\n"
  //                    "                \"buildKey64\":\"20180312T181135_CPEmakeModel_37D8A05D623E7F6AC67EECBF929EDFBBE09\",\n"
  //                    "                \"networking\":\"INTRA\",\n"
  //                    "                \"useCrossbarPrototypes\":true\n"
  //                    "        }\n"
  //                    "}";
  //  Document document;
  //  char test2[] = "{\n"
  //                 "    \"hello\": \"world\",\n"
  //                 "    \"t\": true ,\n"
  //                 "    \"f\": false,\n"
  //                 "    \"n\": null,\n"
  //                 "    \"i\": 123,\n"
  //                 "    \"pi\": 3.1416,\n"
  //                 "    \"model\": [1, 2, 3, 4]\n"
  //                 "}";
  //
  //  string str = "0F C0 97 D4 50 45 04 40 32 88 22 0C 85 0B A1 81 0F C0 97 D4 50 45 04 40 32 88 22 0C 85 0B A1 81";
  //  vector<int> array;
  //  stringstream ss(str);
  //  int i;
  //  while (ss >> i) {
  //    std::cout << i << endl;
  //  }
  //  //FILE *json_file;
  //  //json_file = fopen(filename,"rb");
  //    ifstream json_file(filename);
  ////    char x[40];
  ////    json_file.read(x,30);
  ////    cout << x;
  //
  //    std::string json_str((std::istreambuf_iterator<char>(json_file)),
  //                    std::istreambuf_iterator<char>());
  //    Document json_doc;
  //    json_doc.Parse<kParseCommentsFlag>(json_str);
  //    cout << json_doc["crossbarTypes"][0]["crossbar"]["rows"][0]["synapses"].GetString();
  //

  //  document.Parse<kParseCommentsFlag>(testdata.data());
  //  //Document doc2;
  //  //doc2.Parse<kParseCommentsFlag>(testdata.data());
  // //document.Parse<kParseCommentsFlag>(testdata.data());
  //
  //  static const char* kTypeNames[] =
  //      { "Null", "False", "True", "Object", "Array", "String", "Number" };
  //  for (Value::ConstMemberIterator itr = document.MemberBegin();
  //       itr != document.MemberEnd(); ++itr)
  //  {
  //    printf("Type of member %s is %s\n",
  //           itr->name.GetString(), kTypeNames[itr->value.GetType()]);
  //    cout << document["model"]["coreCount"].GetInt();
  //  }
  //  const char *fn = "/Users/markplagge/dev/NeMo/src/model_reader/th_small_test.json";
  //  /* file pointer version */
  //  Document th_test;
  //  FILE *json_file;
  //  json_file = fopen(fn,"rb");
  //  cout << "File status: " << errno << "\n\n";
  //  /* stream file pointer version */
  //  char readBuffer[65536];
  //  FileReadStream is(json_file,readBuffer,sizeof(readBuffer));
  //  //th_test.ParseStream<kParseCommentsFlag,FileReadStream> (is);
  //  th_test.ParseStream<kParseCommentsFlag>(is);;
  //
  //
  //
  //  /* ifstream version *
  //  ifstream json_file ("th_small_test.json");
  //  IStreamWrapper tnf(json_file);
  //  //test file load
  //  //Test rapidjson stream load with the crappy json spec:
  //  Document th_test;
  //  th_test.ParseStream(tnf);
  //   */
  //
  //  ifstream js_i (fn);
  //
  //
  //
  //  printf("---------------------------\n \t model info \n ");
  //  cout <<"Num Cores Defined: " << th_test["model"]["coreCount"].GetInt() << "\n";
  //  cout << "Core 0 metadata - wild test: " << th_test["core"]["id"].GetInt() << "\n";
  //  cout <<"Crossbar type data: " << th_test["crossbarTypes"][0]["name"].GetString() << " " << "\n";
  //  cout << "Core l: " << th_test["core"].IsObject() << " " << th_test.IsArray();
  //  cout << "\nNeuron Type Lib: " << th_test["neuronTypes"][0]["name"].GetString() << "\n";
  //
  //  Value& x = th_test["neuronTypes"];
  //
  //  for (auto& m : th_test.GetObject())
  //    printf("Type of member %s is %s\n",
  //           m.name.GetString(), kTypeNames[m.value.GetType()]);
  //
  //
  //  // test TN_Wrapper creation:
  //  Value& tv = th_test["neuronTypes"][0];
  //  cout << tv["sigma0"].GetInt() << "\n";
  //  TN_Wrapper neuro_test(tv);
  //  //TN_Wrapper tn_obj(th_test["neuronTypes"][0]);
  //  vector<int> testV;
  //  int a = 1;
  //  int b = 2;
  //  testV.insert(testV.end(),{a,b});
  //  cout << testV.front();
  //
  //  StringBuffer buffer;
  //  Writer<StringBuffer> writer(buffer);
  //
  //  th_test["neuronTypes"][0].Accept(writer);
  //  const char* output= buffer.GetString();
  //  Document ntd;
  //  ntd.Parse(output);
  //  cout <<"\n" <<  output;
  //  cout <<"\n" << ntd.IsArray() << "\n";
  //  //cout <<"\n" << ntd[0]["name"].GetString() << "\n";


}
void test_itr(int *&vals){
  for (int i = 0; i < 5; i ++){
    *vals = i;
    vals ++;
  }
}
int main(int argc, char *argv[]) {
  int testVals[16] = {-1};
  int *p = testVals;
  int *ptr = testVals;
  test_itr(ptr);
  for(auto x : testVals){
    cout << x << " " ;
  }
  cout << "\n" << "test ptr: " << ptr << "\n";
  *ptr = 100;
  cout << "\n" << "change ptr 100: " << ptr << "\n";
  for(auto x : testVals){
    cout << x << " " ;
  }

  cout << "\n";
//#ifdef DEBUG
//  string test1 = "54x10";
//  string test2 = "5:10";
//  std::string::size_type pos;
//  int v1 = stoi(test1,&pos,10);
//  string t2 = test1.substr(pos + 1);
//  int v2 = stoi(t2,NULL,10);
//  cout << "v1 " << v1 << " -- v2  " << "\n";
//  return 0;
//  testDTO();
//  testCoreGen();
//#else
    string filename;
    if (argc != 3) {
        cout << "TN -> NeMo Parser \n Usage: parser PATH_TO_TN_JSON_FILE PATH_TO_TN_SPIKE_FILE\n";
        cout << " Got " << argc << " arguments";
        //Testing code:
        filename = "/Users/markplagge/dev/NeMo/scripts/model/th_corelet_net.json";
    }else{
        filename = argv[1];
    }

  cout <<"----Parsing----\n";
  //Test crossbar logic
  //char *filename = "/Users/markplagge/dev/NeMo/scripts/model/th_corelet_net.json";
    
    TN_Main model = create_tn_data(filename);
    cout << "\n Created Model. \n";
    
//#endif
}
