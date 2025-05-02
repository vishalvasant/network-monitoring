#include "config/ConfigManager.hpp"
#include "utils/Logger.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

void ConfigManager::load(const std::string& config_file) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_config_file_ = config_file;
    parseConfigFile(config_file);
}

void ConfigManager::save(const std::string& config_file) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string file = config_file.empty() ? current_config_file_ : config_file;
    if (file.empty()) {
        throw std::runtime_error("No configuration file specified");
    }
    writeConfigFile(file);
}

void ConfigManager::setValue(const std::string& section, const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_data_[section][key] = value;
}

void ConfigManager::setValue(const std::string& section, const std::string& key, int value) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_data_[section][key] = value;
}

void ConfigManager::setValue(const std::string& section, const std::string& key, bool value) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_data_[section][key] = value;
}

void ConfigManager::setValue(const std::string& section, const std::string& key, double value) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_data_[section][key] = value;
}

std::optional<std::string> ConfigManager::getString(const std::string& section, const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto section_it = config_data_.find(section);
    if (section_it == config_data_.end()) {
        return std::nullopt;
    }

    auto key_it = section_it->second.find(key);
    if (key_it == section_it->second.end()) {
        return std::nullopt;
    }

    if (auto* value = std::get_if<std::string>(&key_it->second)) {
        return *value;
    }
    return std::nullopt;
}

std::optional<int> ConfigManager::getInt(const std::string& section, const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto section_it = config_data_.find(section);
    if (section_it == config_data_.end()) {
        return std::nullopt;
    }

    auto key_it = section_it->second.find(key);
    if (key_it == section_it->second.end()) {
        return std::nullopt;
    }

    if (auto* value = std::get_if<int>(&key_it->second)) {
        return *value;
    }
    return std::nullopt;
}

std::optional<bool> ConfigManager::getBool(const std::string& section, const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto section_it = config_data_.find(section);
    if (section_it == config_data_.end()) {
        return std::nullopt;
    }

    auto key_it = section_it->second.find(key);
    if (key_it == section_it->second.end()) {
        return std::nullopt;
    }

    if (auto* value = std::get_if<bool>(&key_it->second)) {
        return *value;
    }
    return std::nullopt;
}

std::optional<double> ConfigManager::getDouble(const std::string& section, const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto section_it = config_data_.find(section);
    if (section_it == config_data_.end()) {
        return std::nullopt;
    }

    auto key_it = section_it->second.find(key);
    if (key_it == section_it->second.end()) {
        return std::nullopt;
    }

    if (auto* value = std::get_if<double>(&key_it->second)) {
        return *value;
    }
    return std::nullopt;
}

bool ConfigManager::hasSection(const std::string& section) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_data_.find(section) != config_data_.end();
}

bool ConfigManager::hasKey(const std::string& section, const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto section_it = config_data_.find(section);
    if (section_it == config_data_.end()) {
        return false;
    }
    return section_it->second.find(key) != section_it->second.end();
}

std::vector<std::string> ConfigManager::getSections() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> sections;
    sections.reserve(config_data_.size());
    for (const auto& [section, _] : config_data_) {
        sections.push_back(section);
    }
    return sections;
}

std::vector<std::string> ConfigManager::getKeys(const std::string& section) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto section_it = config_data_.find(section);
    if (section_it == config_data_.end()) {
        return {};
    }

    std::vector<std::string> keys;
    keys.reserve(section_it->second.size());
    for (const auto& [key, _] : section_it->second) {
        keys.push_back(key);
    }
    return keys;
}

void ConfigManager::parseConfigFile(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open configuration file: " + config_file);
    }

    std::string current_section;
    std::string line;
    std::regex section_pattern(R"(\[([^\]]+)\])");
    std::regex key_value_pattern(R"(([^=]+)=(.*))");

    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        // Remove whitespace
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

        std::smatch matches;
        if (std::regex_match(line, matches, section_pattern)) {
            current_section = matches[1].str();
        } else if (std::regex_match(line, matches, key_value_pattern)) {
            if (current_section.empty()) {
                throw std::runtime_error("Key-value pair found outside of section");
            }
            std::string key = matches[1].str();
            std::string value = matches[2].str();
            config_data_[current_section][key] = parseValue(value);
        }
    }
}

void ConfigManager::writeConfigFile(const std::string& config_file) const {
    std::ofstream file(config_file);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open configuration file for writing: " + config_file);
    }

    for (const auto& [section, keys] : config_data_) {
        file << "[" << section << "]\n";
        for (const auto& [key, value] : keys) {
            file << key << "=" << getValueAsString(value) << "\n";
        }
        file << "\n";
    }
}

std::string ConfigManager::getValueAsString(const ConfigValue& value) const {
    return std::visit([](const auto& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::string>) {
            return v;
        } else if constexpr (std::is_same_v<T, bool>) {
            return v ? "true" : "false";
        } else {
            return std::to_string(v);
        }
    }, value);
}

ConfigManager::ConfigValue ConfigManager::parseValue(const std::string& value) const {
    // Try to parse as boolean
    if (value == "true" || value == "false") {
        return value == "true";
    }

    // Try to parse as integer
    try {
        return std::stoi(value);
    } catch (...) {
        // Not an integer
    }

    // Try to parse as double
    try {
        return std::stod(value);
    } catch (...) {
        // Not a double
    }

    // Return as string
    return value;
} 