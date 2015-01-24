#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#ifdef __GNUC__
// https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html#Diagnostic-Pragmas
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <msgpack.hpp>
#include "cmdline/cmdline.h"
#include "sais/sais.hxx"
#include "picojson/picojson.h"

#pragma GCC diagnostic pop
#endif // __GNUC__

using namespace std;

struct Book {
  string title, author, content;
  Book(const string & title_, const string & author_, const string & content_) :
    title(title_), author(author_), content(content_) { }

  MSGPACK_DEFINE(title, author, content);
};

string read_oneline(const string & ifname) {
  ifstream ifs(ifname);

  if (!ifs) {
    perror(ifname.c_str());
    exit(EXIT_FAILURE);
  }

  string s;
  getline(ifs, s);
  return s;
}

vector<Book> read_datasets(const string & json_file) {
  ifstream ifs(json_file);

  if (!ifs) {
    perror(json_file.c_str());
    exit(EXIT_FAILURE);
  }

  picojson::value json;
  string err = picojson::parse(json, ifs);

  if (!err.empty()) {
    cerr << err << endl;
    exit(EXIT_FAILURE);
  }

  auto && array = json.get<picojson::array>();
  vector<Book> ret;
  for (const picojson::value &it : array) {
    picojson::object o = it.get<picojson::object>();
    string filename = o["filename"].get<string>();
    string title = o["title"].get<string>();
    string author = o["author"].get<string>();
    ret.emplace_back(title, author, read_oneline(filename));
  }
  return ret;
}

vector<uint8_t> join_content(const vector<Book> & books) {
  size_t size = 0;
  for(auto && it : books) {
    size += it.content.size() + 1; // '\n'
  }
  ++size; // '\0'
  vector<uint8_t> SV(size);
  size_t cur = 0;
  for(auto && it : books) {
    copy(it.content.begin(), it.content.end(), SV.begin() + cur);
    cur += it.content.size();
    SV[cur++] = '\n';
  }
  SV[cur++] = '\0';
  return SV;
}

int main(int argc, char * argv[]) {
  cmdline::parser parser;
  parser.add<string>("json", '\0', "", /* need = */ true);
  parser.add("help", 'h', "show help");

  if (!parser.parse(argc, argv) || parser.exist("help")) {
    cerr << parser.error_full() << parser.usage() << endl;
    exit(EXIT_FAILURE);
  }

  std::ofstream book_buf("msg/book.msg");
  std::ofstream content_buf("msg/content.msg");
  std::ofstream sa_buf("msg/sa.msg");
  std::ofstream bwt_buf("msg/bwt.msg");
  std::ofstream book_id_buf("msg/book_id.msg");

  string json_file = parser.get<string>("json");
  // book
  vector<Book> books = read_datasets(json_file);
  msgpack::pack(book_buf, books);

  // content
  vector<uint8_t> SV = join_content(books);
  msgpack::pack(content_buf, SV);
  const size_t seqLen = SV.size();

  // sa
  vector<int64_t> SA(seqLen);
  saisxx(SV.begin(), SA.begin(), (int64_t)seqLen);
  msgpack::pack(sa_buf, SA);

  // bwt
  vector<uint8_t> TV(seqLen);
  for(size_t i = 0; i < seqLen; ++i) {
    size_t j;
    if(SA[i] == 0) j = seqLen - 1;
    else j = SA[i] - 1;
    TV[i] = SV[j];
  }
  msgpack::pack(bwt_buf, TV);
  TV.clear();
  SV.clear();

  // book id
  vector<uint16_t> UV(seqLen);
  {
    vector<uint16_t> tmp(seqLen);
    size_t cur = 0;
    for(size_t id = 0; id < books.size(); ++id) {
      for(size_t i = 0; i < books[id].content.size(); ++i) {
        tmp[cur++] = id;
      }
      tmp[cur++] = id; // for '\n'
    }
    tmp[cur++] = books.size() - 1; // for '\0'
    for(size_t i = 0; i < seqLen; ++i) {
      UV[i] = tmp[SA[i]];
    }
  }
  msgpack::pack(book_id_buf, UV);
}
