/* SWIG interface for test_swig_example.h. The regex %rename (PCRE2) drops
   the qs_ prefix in every target language: qs_gcd becomes gcd. */
%module qnxswig
%{
#include "test_swig_example.h"
%}
%rename("%(regex:/^qs_(.*)$/\\1/)s") "";
%newobject qs_reverse;
%include "test_swig_example.h"
