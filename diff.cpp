#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
using namespace std;

int main(int argc, char * argv[]) {
  if (argc < 3) {
    cerr << "usage: ./diff a b" << endl;
    return EXIT_FAILURE;
  }

  string S, T;

  {
    ifstream ifs;
    ifs.open(argv[1]);
    ifs >> S;
    cerr << "read " << argv[1] << endl;
  }

  {
    ifstream ifs;
    ifs.open(argv[2]);
    ifs >> T;
    cerr << "read " << argv[2] << endl;
  }

  if(S.size() != T.size()) {
    cerr << "not same size" << endl;
    cerr << S.size() << " " << T.size() << endl;
    return EXIT_SUCCESS;
  }
  for(size_t i = 0; i < min(S.size(), T.size()); ++i) {
    if(S[i] != T[i]) {
      cout << i << endl;
      return EXIT_SUCCESS;
    }
  }
}
