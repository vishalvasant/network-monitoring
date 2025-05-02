#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <optional>
#include <variant>

class ConfigManager {
public:
    static ConfigManager& getInstance();

    void load(const std::string& config_file);
    void save(const std::string& config_file = "");
    void setValue(const std::string& section, const std::string& key, const std::string& value);
    void setValue(const std::string& section, const std::string& key, int value);
    void setValue(const std::string& section, const std::string& key, bool value);
    void setValue(const std::string& section, const std::string& key, double value);

    std::optional<std::string> getString(const std::string& section, const std::string& key) const;
    std::optional<int> getInt(const std::string& section, const std::string& key) const;
    std::optional<bool> getBool(const std::string& section, const std::string& key) const;
    std::optional<double> getDouble(const std::string& section, const std::string& key) const;

    bool hasSection(const std::string& section) const;
    bool hasKey(const std::string& section, const std::string& key) const;
    std::vector<std::string> getSections() const;
    std::vector<std::string> getKeys(const std::string& section) const;

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    using ConfigValue = std::variant<std::string, int, bool, double>;
    using ConfigSection = std::unordered_map<std::string, ConfigValue>;
    using ConfigData = std::unordered_map<std::string, ConfigSection>;

    void parseConfigFile(const std::string& config_file);
    void writeConfigFile(const std::string& config_file) const;
    std::string getValueAsString(const ConfigValue& value) const;
    ConfigValue parseValue(const std::string& value) const;

    ConfigData config_data_;
    std::string current_config_file_;
    mutable std::mutex mutex_;
}; 