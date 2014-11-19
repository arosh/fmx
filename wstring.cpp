#include <iostream>
#include <string>
#include <locale>
using namespace std;

wstring to_wstring(const string & mbs) {
  // http://linuxjm.sourceforge.jp/html/LDP_man-pages/man3/mbstowcs.3.html
  size_t ws_len = mbstowcs(NULL, mbs.c_str(), 0);
  wstring ws(ws_len, '\0');
  mbstowcs(&ws[0], mbs.c_str(), ws.size());
  return ws;
}

wstring to_wstring(const char & mbc) {
  string mbs;
  mbs += mbc;
  return to_wstring(mbs);
}

string to_mbstring(const wstring & ws) {
  // http://linuxjm.sourceforge.jp/html/LDP_man-pages/man3/wcstombs.3.html
  size_t mb_len = wcstombs(NULL, ws.c_str(), 0);
  string mbs(mb_len, '\0');
  wcstombs(&mbs[0], ws.c_str(), mbs.size());
  return mbs;
}

string to_mbstring(const wchar_t & wc) {
  wstring ws;
  ws += wc;
  return to_mbstring(ws);
}
