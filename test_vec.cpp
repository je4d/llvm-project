#include <vector>

void mod(const std::vector<int>& v)
{
  //v.leak()[0] = 42;
}

int main()
{
  std::vector<int> v;
  v.push_back(123);
  mod(v);
  return v[0];
}
