#include <iostream>
#include <vector>
#include <string>

#include "cmdline/cmdline.h"

using namespace std;

extern void init(const string &config);
extern vector<string> search_(const string &query, const int num);

int main(int argc, char *argv[]) {
  cmdline::parser parser;
  parser.add<string>("config", 'c', "config json file", /* need = */ true);
  parser.add<string>("query", 'q', "query string", /* need = */ false);
  parser.add<int>("number", 'n', "display first `number` lines",
                  /* need = */ false, /* default = */ 20);
  parser.add("interact", 'i', "launch interactive shell");
  parser.add("help", 'h', "show help");

  if (!parser.parse(argc, argv) || parser.exist("help")) {
    cerr << parser.error_full() << parser.usage() << endl;
    exit(EXIT_FAILURE);
  }

  if (parser.exist("query") == false && parser.exist("interact") == false) {
    cerr << "you should enable --query or --interact" << endl;
    exit(EXIT_FAILURE);
  }

  string cfname = parser.get<string>("config");
  init(cfname);

  if (parser.exist("query")) {
    string q = parser.get<string>("query");
    auto v = search_(q, parser.get<int>("number"));
    for(const string &item : v) {
      cout << item << endl;
    }
  }

  if (parser.exist("interact")) {
    while (true) {
      cout << "query> ";
      string q;
      if (cin >> q) {
        auto v = search_(q, parser.get<int>("number"));
        for(const string &item : v) {
          cout << item << endl;
        }
      } else {
        break;
      }
    }
  }
}
