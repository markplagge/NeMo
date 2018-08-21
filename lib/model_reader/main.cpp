//
// Created by Mark Plagge on 8/8/18.
//

#include <cstdio>
#include "tn_parser.hh"

#include "picojson.h"



// SFINAE for safety. Sue me for putting it in a macro for brevity on the function
#define IS_INTEGRAL(T) typename std::enable_if< std::is_integral<T>::value >::type* = 0

template<class T>
std::string integral_to_binary_string(T byte, IS_INTEGRAL(T)) {
  std::bitset<sizeof(T)*CHAR_BIT> bs(byte);
  return bs.to_string();
}

int main() {
  //test the hexchar to bool:

  //string t2 =  "0123456789ABCDEF";
  string t2 = "0123456789ABCDEF";
  cout << "\n-------------------------\n";
  vector<bool> hb = fullhex2bool(t2);
  int ii = 1;
  for (auto it: hb) {

    cout << it;
    if (ii%4==0) {
      cout << "\n";
    }
    ii++;
  }
//    for (auto it : t2){
//      vector<bool> hb = fullhex2bool(it);
//      for(auto it2 : hb){
//        cout << it2;
//      }
//      cout << '\n';
//    }
  cout << "-------------------------\n";

  printf("Model Reader.\n");
  cout << "Loading templates from JSON file...";

  string test_hex = "51 40";

  string testdata = "{\n"
                    "       /* comments in the JSON file */ "
                    "        \"model\":{\n"
                    "                \"coreCount\":4042,\n"
                    "                \"neuronclass\":\"NeuronGeneral\",\n"
                    "                \"crossbarSize\":256,\n"
                    "                \"crossbarclass\":\"CrossbarBinary\",\n"
                    "                \"buildKey64\":\"20180312T181135_CPEmakeModel_37D8A05D623E7F6AC67EECBF929EDFBBE09\",\n"
                    "                \"networking\":\"INTRA\",\n"
                    "                \"useCrossbarPrototypes\":true\n"
                    "        }\n"
                    "}";
  Document document;
  char test2[] = "{\n"
                 "    \"hello\": \"world\",\n"
                 "    \"t\": true ,\n"
                 "    \"f\": false,\n"
                 "    \"n\": null,\n"
                 "    \"i\": 123,\n"
                 "    \"pi\": 3.1416,\n"
                 "    \"model\": [1, 2, 3, 4]\n"
                 "}";

  string str = "0F C0 97 D4 50 45 04 40 32 88 22 0C 85 0B A1 81 0F C0 97 D4 50 45 04 40 32 88 22 0C 85 0B A1 81";
  vector<int> array;
  stringstream ss(str);
  int i;
  while (ss >> i) {
    std::cout << i << endl;
  }

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
