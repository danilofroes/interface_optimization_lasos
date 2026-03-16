#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // converter std::vector e std::string em dict e string do Python
#include "include/ILS.hpp"

namespace py = pybind11;

PYBIND11_MODULE(meta_engine, m) {
    py::class_<ILS>(m, "ILS")
        .def(py::init<>())
        .def("solve", &ILS::solve)
        .def("setParametros", [](ILS& self, const py::object& obj) {
            // Converte dict do Python para string, depois converte para nlohmann::json e passa pro C++
            std::string s = py::module_::import("json").attr("dumps")(obj).cast<std::string>();
            self.setParametros(json::parse(s));
        })
        .def("getResultados", [](const ILS& self) {
            // Pega o json do C++, converte pra string, e o Python lê como Dict
            return py::module_::import("json").attr("loads")(self.getResultados().dump());
        })
        .def("getNome", &ILS::getNome);
}