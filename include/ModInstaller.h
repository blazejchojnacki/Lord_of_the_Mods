// include/services/ModInstaller.h
#pragma once
#include "models/Mod.h"
#include <system_error>
#include <iostream>

class ModInstaller {
private:
    fs::path game_dir_;
    fs::path backup_dir_;

public:
    ModInstaller(fs::path game_dir, fs::path backup_dir)
        : game_dir_(std::move(game_dir)), backup_dir_(std::move(backup_dir)) {
        fs::create_directories(backup_dir_);
    }

    // -------------------------------------------------------------
    // ACTIVATE MOD
    // -------------------------------------------------------------
    bool activate(const Mod& mod) {
        std::error_code ec;

        for (const auto& rel_path : mod.get_relative_files()) {
            fs::path source_file = mod.get_mod_dir() / rel_path;
            fs::path target_file = game_dir_ / rel_path;
            fs::path backup_file = backup_dir_ / rel_path;

            if (!fs::exists(source_file)) {
                std::cerr << "Mod file missing: " << source_file << std::endl;
                return false;
            }

            // 1. If original game file exists and is not yet backed up, back it up
            if (fs::exists(target_file) && !fs::exists(backup_file)) {
                fs::create_directories(backup_file.parent_path());
                fs::copy_file(target_file, backup_file, fs::copy_options::overwrite_existing, ec);
                if (ec) {
                    std::cerr << "Backup failed for " << target_file << ": " << ec.message() << std::endl;
                    return false;
                }
            }

            // 2. Overwrite target game file with the mod file
            fs::create_directories(target_file.parent_path());
            fs::copy_file(source_file, target_file, fs::copy_options::overwrite_existing, ec);
            if (ec) {
                std::cerr << "File copy failed for " << rel_path << ": " << ec.message() << std::endl;
                return false;
            }
        }
        return true;
    }

    // -------------------------------------------------------------
    // DEACTIVATE MOD
    // -------------------------------------------------------------
    bool deactivate(const Mod& mod) {
        std::error_code ec;

        for (const auto& rel_path : mod.get_relative_files()) {
            fs::path target_file = game_dir_ / rel_path;
            fs::path backup_file = backup_dir_ / rel_path;

            // 1. Remove the mod file from game folder
            if (fs::exists(target_file)) {
                fs::remove(target_file, ec);
            }

            // 2. If a backup file exists, restore it to the game folder
            if (fs::exists(backup_file)) {
                fs::copy_file(backup_file, target_file, fs::copy_options::overwrite_existing, ec);
                fs::remove(backup_file, ec); // Clean up backup after restoring
            }
        }
        return true;
    }
};