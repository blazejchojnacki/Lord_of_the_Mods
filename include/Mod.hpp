/*
author: blazejchojnacki
project: Lord_of_the_Mods
AI involvment: Gemini Pro created the base file.
*/

#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>

class ModManifest {
private:
    std::string id_;
    std::string name_;
    std::filesystem::path mod_directory_;
    std::vector<std::filesystem::path> file_list_;

public:
    ModManifest(std::string id, std::string name, std::filesystem::path mod_dir)
        : id_(std::move(id)), name_(std::move(name)), mod_directory_(std::move(mod_dir)) {
    }

    const std::string& get_id() const { return id_; }
    const std::string& get_name() const { return name_; }
    const std::filesystem::path& get_mod_dir() const { return mod_directory_; }

    void add_file(const std::filesystem::path& relative_path) {
        file_list_.push_back(relative_path);
    }

    const std::vector<std::filesystem::path>& get_file_list() const {
        return file_list_;
    }
};


class ModInstaller {
private:
    std::filesystem::path game_directory_;
    std::filesystem::path backup_directory_;

public:
    ModInstaller(std::filesystem::path game_dir, std::filesystem::path backup_dir)
        : game_directory_(std::move(game_dir)), backup_directory_(std::move(backup_dir)) {
        std::filesystem::create_directories(backup_directory_);
    }

    bool activate(const ModManifest& mod) {
        std::error_code err_code;

        for (const auto& rel_path : mod.get_file_list()) {
            std::filesystem::path mod_file_path = mod.get_mod_dir() / rel_path;
            std::filesystem::path game_file_path = game_directory_ / rel_path;
            std::filesystem::path backup_file_path = backup_directory_ / rel_path;

            if (!std::filesystem::exists(mod_file_path)) {
                std::cerr << "Mod file missing: " << mod_file_path << std::endl;
                return false;
            }

            // backup
            if (std::filesystem::exists(game_file_path) && !std::filesystem::exists(backup_file_path)) {
                std::filesystem::create_directories(backup_file_path.parent_path());
                std::filesystem::copy_file(game_file_path, backup_file_path, std::filesystem::copy_options::overwrite_existing, err_code);
                if (err_code) {
                    std::cerr << "Backup failed for " << game_file_path << ": " << err_code.message() << std::endl;
                    return false;
                }
            }

            // overwrite by copy
            std::filesystem::create_directories(game_file_path.parent_path());
            std::filesystem::copy_file(mod_file_path, game_file_path, std::filesystem::copy_options::overwrite_existing, err_code);
            if (err_code) {
                std::cerr << "File copy failed for " << rel_path << ": " << err_code.message() << std::endl;
                return false;
            }
        }
        return true;
    }

    bool deactivate(const ModManifest& mod) {
        std::error_code err_code;

        for (const auto& rel_path : mod.get_file_list()) {
            std::filesystem::path game_file_path = game_directory_ / rel_path;
            std::filesystem::path backup_file_path = backup_directory_ / rel_path;

            // remove the copied mod file from game folder
            if (std::filesystem::exists(game_file_path)) {
                std::filesystem::remove(game_file_path, err_code);
            }

            // restore
            if (std::filesystem::exists(backup_file_path)) {
                std::filesystem::copy_file(backup_file_path, game_file_path, std::filesystem::copy_options::overwrite_existing, err_code);
                std::filesystem::remove(backup_file_path, err_code);
            }
        }
        return true;
    }
};
