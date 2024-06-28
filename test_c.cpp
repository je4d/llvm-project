template <typename T>
struct X;

template <typename T>
struct X<const T>
{
};

X<const int> x;
