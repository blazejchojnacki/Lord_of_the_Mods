// include/models/Mod.h
#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

class Mod {
private:
    std::string id_;
    std::string name_;
    fs::path mod_dir_; // Path to where the mod's source files live
    std::vector<fs::path> relative_files_; // e.g. {"data/ini/gamedata.ini", "maps/map1.map"}

public:
    Mod(std::string id, std::string name, fs::path mod_dir)
        : id_(std::move(id)), name_(std::move(name)), mod_dir_(std::move(mod_dir)) {
    }

    const std::string& get_id() const { return id_; }
    const std::string& get_name() const { return name_; }
    const fs::path& get_mod_dir() const { return mod_dir_; }

    void add_relative_file(const fs::path& rel_path) {
        relative_files_.push_back(rel_path);
    }

    const std::vector<fs::path>& get_relative_files() const {
        return relative_files_;
    }
};