// The extension module test_pybind11 loads into the ported python3: plain
// functions with keyword and default arguments, a class with a constructor,
// a property, a method and __repr__, STL conversions (vector, map, string),
// a C++ exception translated to a Python exception, and a Python callable
// called from C++. Built against pybind11 and the python3 headers, linked
// without libpython (the interpreter that loads it provides the C API).
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <functional>
#include <map>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>
namespace py = pybind11;

struct Pet {
  std::string name; int age;
  Pet(std::string n, int a) : name(std::move(n)), age(a) {}
  std::string greet() const { return name + " is " + std::to_string(age); }
};

PYBIND11_MODULE(qnxpb11, m) {
  m.doc() = "pybind11 port test module";
  m.def("add", [](int a, int b) { return a + b; }, py::arg("a"), py::arg("b") = 10);
  m.def("total", [](const std::vector<double> &v) { return std::accumulate(v.begin(), v.end(), 0.0); });
  m.def("lengths", [](const std::vector<std::string> &v) {
    std::map<std::string, size_t> r; for (auto &s : v) r[s] = s.size(); return r; });
  m.def("fail", [](const std::string &why) -> int { throw std::invalid_argument(why); });
  m.def("apply_twice", [](const std::function<int(int)> &f, int x) { return f(f(x)); });
  py::class_<Pet>(m, "Pet")
    .def(py::init<std::string, int>())
    .def_readwrite("name", &Pet::name)
    .def_property("age", [](const Pet &p) { return p.age; },
                  [](Pet &p, int a) { if (a < 0) throw py::value_error("negative age"); p.age = a; })
    .def("greet", &Pet::greet)
    .def("__repr__", [](const Pet &p) { return "<Pet " + p.name + ">"; });
}
