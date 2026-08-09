/*
author: blazejchojnacki
project: Lord_of_the_Mods
AI involvement: Gemini Pro refined with atomic rollback and state manifest.
*/

#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <system_error>

enum class TransferType {
    COPY, MOVE, DELETE
};

struct FileData {
    std::filesystem::path relative_path;
    std::string hash;
    TransferType transfer_type;
};


class ModManifest {
private:
    std::string id_;
    std::string name_;
    std::filesystem::path mod_directory_;
    std::vector<FileData> file_list_;

public:
    ModManifest(std::string id, std::string name, std::filesystem::path mod_dir)
        : id_(std::move(id)), name_(std::move(name)), mod_directory_(std::move(mod_dir)) {
    }

    const std::string& get_id() const { return id_; }
    const std::string& get_name() const { return name_; }
    const std::filesystem::path& get_mod_dir() const { return mod_directory_; }

    void add_file(const FileData& file_data) {
        file_list_.push_back(file_data);
    }

    const std::vector<FileData>& get_file_list() const {
        return file_list_;
    }
};


class ModInstaller {
private:
    std::filesystem::path game_directory_;
    std::filesystem::path backup_directory_;
    std::filesystem::path state_file_path_;

    void write_state(const std::string& mod_id, const std::string& status) {
        std::ofstream out(state_file_path_, std::ios::trunc);
        if (out.is_open()) {
            out << mod_id << "\n" << status << "\n";
        }
    }

    void clear_state() {
        std::error_code err_code;
        if (std::filesystem::exists(state_file_path_)) {
            std::filesystem::remove(state_file_path_, err_code);
        }
    }

    void transfer_file(TransferType transfer_type, const std::filesystem::path& file_path) {
        std::error_code err_code;
        if (transfer_type == TransferType::DELETE) {
            if (std::filesystem::exists(file_path)) {
                std::filesystem::remove(file_path, err_code);
            }
        }
        if (err_code) {
            // raise error?
            std::cerr << "File transfer failed: " << file_path << std::endl;
        }
    }

    void transfer_file(TransferType transfer_type, const std::filesystem::path &source_file_path, std::filesystem::path &destination_file_path) {
        std::error_code err_code;
        if (transfer_type == TransferType::COPY and transfer_type == TransferType::MOVE) {
            if (std::filesystem::exists(destination_file_path)) {
                std::filesystem::remove(destination_file_path, err_code);
            }
            if (std::filesystem::exists(source_file_path)) {
                std::filesystem::create_directories(destination_file_path.parent_path());
                std::filesystem::copy_file(source_file_path, destination_file_path, std::filesystem::copy_options::overwrite_existing, err_code);
                if (transfer_type == TransferType::MOVE) {
                    std::filesystem::remove(source_file_path, err_code);
                }
            }
        }
        if (err_code) {
            // raise error?
            std::cerr << "File transfer failed: " << source_file_path << std::endl;
        }
    }

    bool process_manifest(const std::vector<FileData>& file_manifest, std::filesystem::path &source_directory, std::filesystem::path& destination_directory) {
        for (const auto& file_data : file_manifest) {
            auto rel_path = file_data.relative_path;
            std::filesystem::path source_file_path = source_directory / rel_path;
            std::filesystem::path destination_file_path = destination_directory / rel_path;

            auto transfer_type = file_data.transfer_type;
            if (transfer_type == TransferType::COPY or transfer_type == TransferType::MOVE) {
                transfer_file(transfer_type, source_file_path, destination_file_path);
            }
            else if (transfer_type == TransferType::DELETE) {
                transfer_file(transfer_type, destination_file_path);
            }
        }
        return true;
    }

    void rollback(const std::vector<FileData>& processed_files) {
        process_manifest(processed_files, backup_directory_, game_directory_);
        clear_state();
    }

public:
    ModInstaller(std::filesystem::path game_dir, std::filesystem::path backup_dir)
        : game_directory_(std::move(game_dir)), backup_directory_(std::move(backup_dir)) {
        std::filesystem::create_directories(backup_directory_);
        state_file_path_ = backup_directory_ / "active_state.dat";
    }

    bool activate(const ModManifest& mod) {
        std::error_code err_code;
        std::vector<FileData> processed_files;

        // Transaction Log: Mark installation as started
        write_state(mod.get_id(), "INSTALLING");

        if (!process_manifest(mod.get_file_list(), game_directory_, backup_directory_)) {
            std::cerr << "Error. processing rollback" << std::endl;
            rollback(processed_files);
            return false;
        }

        //for (const auto& rel_path : mod.get_file_list()) {
        //    std::filesystem::path mod_file_path = mod.get_mod_dir() / rel_path;
        //    std::filesystem::path game_file_path = game_directory_ / rel_path;
        //    std::filesystem::path backup_file_path = backup_directory_ / rel_path;

        //    if (!std::filesystem::exists(mod_file_path)) {
        //        std::cerr << "Mod file missing: " << mod_file_path << std::endl;
        //        rollback(processed_files);
        //        return false;
        //    }

        //    // move to backup
        //    transfer_file(TransferType::MOVE, game_file_path, backup_file_path);
            //if (std::filesystem::exists(game_file_path) && !std::filesystem::exists(backup_file_path)) {
            //    std::filesystem::create_directories(backup_file_path.parent_path());
            //    std::filesystem::copy_file(game_file_path, backup_file_path, std::filesystem::copy_options::overwrite_existing, err_code);
            //    if (err_code) {
            //        std::cerr << "Backup failed for " << game_file_path << ": " << err_code.message() << std::endl;
            //        rollback(processed_files);
            //        return false;
            //    }
            //}

            // copy from mod to game
            //transfer_file(TransferType::COPY, mod_file_path, game_file_path);
            //std::filesystem::create_directories(game_file_path.parent_path());
            //std::filesystem::copy_file(mod_file_path, game_file_path, std::filesystem::copy_options::overwrite_existing, err_code);
            //if (err_code) {
            //    std::cerr << "File copy failed for " << rel_path << ": " << err_code.message() << std::endl;
            //    rollback(processed_files);
            //    return false;
            //}

            // Track successful copies for potential rollback
            processed_files.push_back(rel_path);
        }

        // Transaction Log: Mark installation as successfully completed
        write_state(mod.get_id(), "COMPLETE");
        return true;
    }

    bool deactivate(const ModManifest& mod) {
        std::error_code err_code;

        for (const auto& rel_path : mod.get_file_list()) {
            std::filesystem::path game_file_path = game_directory_ / rel_path;
            std::filesystem::path backup_file_path = backup_directory_ / rel_path;

            transfer_file(TransferType::MOVE, backup_file_path, game_file_path);
        }

        clear_state();
        return true;
    }
};
