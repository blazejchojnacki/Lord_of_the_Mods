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

    std::vector<FileData> backup_manifest;
    std::vector<FileData> forward_manifest;

    std::vector<FileData> retrieve_manifest;
    std::vector<FileData> restore_manifest;

    std::vector<FileData> rollback_manifest;

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

    void enlist_activation_manifest(const ModManifest& mod) {
        backup_manifest.clear();
        forward_manifest.clear();
        
        for (const auto& file_data : mod.get_file_list()) {

            auto backup_data = file_data;
            backup_data.transfer_type = TransferType::MOVE;
            backup_manifest.push_back(backup_data);

            if (file_data.transfer_type == TransferType::COPY or file_data.transfer_type == TransferType::MOVE) {
                forward_manifest.push_back(file_data);
            }
        }
    }

    void enlist_deactivation_manifest(const ModManifest& mod) {
        retrieve_manifest.clear();
        restore_manifest.clear();

        for (const auto& file_data : mod.get_file_list()) {
            if (file_data.transfer_type == TransferType::COPY or file_data.transfer_type == TransferType::MOVE) {
                auto retrieve_data = file_data;
                retrieve_data.transfer_type = TransferType::MOVE;
                retrieve_manifest.push_back(retrieve_data);
            }

            auto restore_data = file_data;
            restore_data.transfer_type = TransferType::MOVE;
            restore_manifest.push_back(restore_data);
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

    void transfer_file(TransferType transfer_type, const std::filesystem::path &source_file_path, const std::filesystem::path &destination_file_path) {
        std::error_code err_code;
        if (transfer_type == TransferType::COPY or transfer_type == TransferType::MOVE) {
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

    bool process_manifest(const std::vector<FileData>& file_manifest, const std::filesystem::path &source_directory,
        const std::filesystem::path& destination_directory) {

        rollback_manifest.clear();

        for (const auto& file_data : file_manifest) {
            auto rollback_data = file_data;

            auto rel_path = file_data.relative_path;
            std::filesystem::path source_file_path = source_directory / rel_path;
            std::filesystem::path destination_file_path = destination_directory / rel_path;

            auto transfer_type = file_data.transfer_type;
            if (transfer_type == TransferType::COPY or transfer_type == TransferType::MOVE) {
                transfer_file(transfer_type, source_file_path, destination_file_path);

                rollback_data.transfer_type = TransferType::MOVE;
                rollback_manifest.push_back(rollback_data);
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

        // Transaction Log: Mark installation as started
        write_state(mod.get_id(), "INSTALLING");

        if (backup_manifest.empty() or forward_manifest.empty()) {
            enlist_activation_manifest(mod);
        }

        if (process_manifest(backup_manifest, game_directory_, backup_directory_)) {
            restore_manifest = rollback_manifest;
        } else {
            std::cerr << "Error. processing rollback" << std::endl;
            rollback(rollback_manifest);
            return false;
        }
        
        if (process_manifest(forward_manifest, mod.get_mod_dir(), game_directory_)) {
            retrieve_manifest = rollback_manifest;
        } else {
            std::cerr << "Error. processing rollback" << std::endl;
            rollback(rollback_manifest);
            return false;
        }

        // Transaction Log: Mark installation as successfully completed
        write_state(mod.get_id(), "COMPLETE");
        backup_manifest.clear();
        forward_manifest.clear();
        return true;
    }

    bool deactivate(const ModManifest& mod) {
        std::error_code err_code;

        if (retrieve_manifest.empty() or restore_manifest.empty()) {
            enlist_deactivation_manifest(mod);
        }

        if (process_manifest(retrieve_manifest, game_directory_, mod.get_mod_dir())) {
            forward_manifest = rollback_manifest;
        } else {
            std::cerr << "Error. processing rollback" << std::endl;
            rollback(rollback_manifest);
            return false;
        }

        if (process_manifest(restore_manifest, backup_directory_, game_directory_)) {
            backup_manifest = rollback_manifest;
        } else {
            std::cerr << "Error. processing rollback" << std::endl;
            rollback(rollback_manifest);
            return false;
        }

        clear_state();
        retrieve_manifest.clear();
        restore_manifest.clear();
        return true;
    }
};
