#include <iostream>

#include <kconfig-hello/io.hxx>

using namespace std;

void
print_out (const string& s)
{
  cout << s;
}

void
print_err (const string& s)
{
  cerr << s;
}

string
ask (const string& p)
{
  cerr << p;
  string r;
  getline (cin, r);
  return r;
}
