#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // for auto-converting std::vector
#include <pybind11/stl/filesystem.h> // for auto-converting std::filesystem::path
#include "../include/Mod.hpp"

namespace py = pybind11;

PYBIND11_MODULE(lotm, m) {
    m.doc() = "C++ Mod Manager Core Engine";

    // Bind the TransferType Enum
    py::enum_<TransferType>(m, "TransferType")
        .value("COPY", TransferType::COPY)
        .value("MOVE", TransferType::MOVE)
        .value("DELETE", TransferType::DELETE)
        .export_values();

    // Bind the FileData Struct
    py::class_<FileData>(m, "FileData")
        .def(py::init<>()) // Default constructor
        .def_readwrite("relative_path", &FileData::relative_path)
        .def_readwrite("hash", &FileData::hash)
        .def_readwrite("transfer_type", &FileData::transfer_type);

    // Bind the ModManifest Class
    py::class_<ModManifest>(m, "ModManifest")
        .def(py::init<std::string, std::string, std::filesystem::path>())
        .def("get_id", &ModManifest::get_id)
        .def("get_name", &ModManifest::get_name)
        .def("get_mod_dir", &ModManifest::get_mod_dir)
        .def("add_file", &ModManifest::add_file)
        .def("get_file_list", &ModManifest::get_file_list);

    // Bind the ModInstaller Class
    py::class_<ModInstaller>(m, "ModInstaller")
        .def(py::init<std::filesystem::path, std::filesystem::path>())
        .def("activate", &ModInstaller::activate)
        .def("deactivate", &ModInstaller::deactivate);
}
