// RUN: %clang_cc1 -std=c++2c -Wno-vla-cxx-extension -verify -fsyntax-only %s

// Examples extracted from P1974R1 "Persistent constexpr allocation".
//
// The persistent-constexpr-allocation feature is gated on C++26, hence -std=c++2c.
// -Wno-vla-cxx-extension silences the VLA extension warning so that the only
// diagnostics left are the ones we care about: an array with a non-constant
// bound at file scope is rejected as a VLA, which is how the paper's
// "*...->value is not a constant expression" cases are surfaced.

template <typename T>
struct ptr {
    T* value;
    constexpr ptr(T* value) : value{value} {}
    ptr(ptr&&) = delete;
    constexpr ~ptr() { delete value; } // expected-note 2 {{read of uninitialized object is not allowed in a constant expression}} expected-note {{in call to '{*new ptr<int>#0}.~ptr()'}} expected-note {{in call to '{*new ptr<const int>#0}.~ptr()'}}
};

// P0784R5 prior-work example (a constexpr unique_ptr<unique_ptr<int>>); ill-formed
// for the same reason as `ppi` below. Modelled by ptr<ptr<int>>, see `ppi`.

// ptr<int>
constexpr ptr<int> pi = new int{1};         // OK
char str1[*pi.value]{};                      // expected-error {{variable length array declaration not allowed at file scope}}
                                             // (*pi.value is not a constant expression)
int& pi_value = *pi.value;                   // OK

// ptr<const int>
constexpr ptr<const int> pci = new int{42};  // OK
char str2[*pci.value]{};                     // OK: *pci.value is a constant expression
int& pci_value = *pci.value;                 // expected-error {{binding reference of type 'int' to value of type 'const int' drops 'const' qualifier}}

// ptr<ptr<int>> / ptr<ptr<const int>>: ill-formed -- the inner allocation's
// pointee is reachable as mutable, so in ~ptr<ptr<...>> reading it in order to
// delete it during constant destruction is not a constant expression.
constexpr ptr<ptr<int>> ppi                  // expected-error {{constexpr variable 'ppi' must have constant destruction}} expected-note {{in call to 'ppi.~ptr()'}}
    = new ptr<int>{new int{1}};              // expected-note {{heap allocation performed here}}
constexpr ptr<ptr<const int>> ppci           // expected-error {{constexpr variable 'ppci' must have constant destruction}} expected-note {{in call to 'ppci.~ptr()'}}
    = new ptr<const int>{new int{42}};       // expected-note {{heap allocation performed here}}

// ptr<const ptr<int>>
constexpr ptr<const ptr<int>> pcpi           // OK: in ~ptr<const ptr<int>>, pcpi.value->value
    = new const ptr<int>{new int{1}};        //     is a constant expression
char str3[*pcpi.value->value]{};             // expected-error {{variable length array declaration not allowed at file scope}}
                                             // (*pcpi.value->value is not a constant expression)
int& pcpi_value = *pcpi.value->value;        // OK

// ptr<const ptr<const int>>
constexpr ptr<const ptr<const int>> pcpci    // OK: in ~ptr<const ptr<const int>>, pcpci.value->value
    = new const ptr<const int>{new int{42}}; //     is a constant expression
char str4[*pcpci.value->value]{};            // OK: *pcpci.value->value is a constant expression
int& pcpci_value = *pcpci.value->value;      // expected-error {{binding reference of type 'int' to value of type 'const int' drops 'const' qualifier}}
