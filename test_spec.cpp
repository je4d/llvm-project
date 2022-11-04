#ifndef CVQUAL
#define CVQUAL propconst
#endif
template <typename T>
struct X;

template <typename T>
struct X<CVQUAL T>
{
};

X<CVQUAL int> x;
