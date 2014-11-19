#include <iostream>
#include <string>
#include <locale>
#include <vector>
using namespace std;

int main() {
  setlocale(LC_CTYPE, "ja_JP.UTF-8");
  wstring s;
  wcin >> s;
  s = L"あいうえお";
  wcout << "length = " << s.size() << endl;
  for(int i = 0; i < (int)s.size(); ++i)
     wcout << "s[" << i << "] = " << s[i] << endl;
  vector<char> mbstring(s.size() * MB_CUR_MAX + 1);
  wcstombs(mbstring.data(), s.c_str(), s.length() * MB_CUR_MAX + 1);
  string mbs(mbstring.data());
  wcout << "length = " << mbs.size() << endl;
  for(int i = 0; i < (int)mbs.size(); ++i)
     wcout << "mbs[" << i << "] = " << (unsigned char)mbs[i] << endl;
}
