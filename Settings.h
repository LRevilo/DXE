#pragma once
#include <string>
#include <unordered_map>
#include <variant>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <array>
#include <sstream>

#include <filesystem>

namespace fs = std::filesystem;

using uint = unsigned int;
using string = std::string;
using float2 = std::array<float, 2>;
using float3 = std::array<float, 3>;
using float4 = std::array<float, 4>;
using VariableValue = std::variant<
    string,
    bool,
    uint,
    int,
    float,
    float2,
    float3,
    float4
>;

struct Settings
{
    Settings() {};

    std::unordered_map<std::string, VariableValue> Variables;
    std::unordered_map<std::string, bool> States;

    template <typename T>
    T& Get(const std::string& key) {
        auto it = Variables.find(key);
        if (it != Variables.end()) { return *std::get_if<T>(&(it->second)); }  // Safe retrieval

        auto& value = Variables[key] = T{};  // Initialize with default value (e.g., 0 for int, 0.0f for float)
        return *std::get_if<T>(&value);
    }

    bool& IsChanging(const std::string& key) {
        auto it = States.find(key);
        if (it != States.end()) { return it->second; }
        return  States[key] = false;
    }
private:
    void WriteValue(std::ofstream& outFile, std::string paramName, VariableValue value) {
    static constexpr int precision = 8;

    // Use std::visit with if constexpr to handle different types
    std::visit([&outFile, &paramName](auto&& val) {

        outFile << std::scientific << std::setprecision(precision);

        // If the value is a string, add quotes
        if constexpr (std::is_same_v<std::decay_t<decltype(val)>, std::string>) {
            outFile << "string:";
            outFile << paramName << "=";
            outFile << '"' << val << '"';
        }

        // If the value is an bool
        else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, bool>) {
            outFile << "bool:";
            outFile << paramName << "=";
            outFile << val;
        }

        // If the value is an integer
        else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, int>) {
            outFile << "int:";
            outFile << paramName << "=";
            outFile << val;
        }

        // If the value is an unsigned integer
        else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, uint>) {
            outFile << "uint:";
            outFile << paramName << "=";
            outFile << val;
        }

        // If the value is a float
        else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, float>) {
            outFile << "float:";
            outFile << paramName << "=";
            outFile << val;
        }

        // If the value is an array of 2 floats
        else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, std::array<float, 2>>) {
            outFile << "float2:";
            outFile << paramName << "=";
            outFile << val[0] << "," << val[1];
        }

        // If the value is an array of 3 floats
        else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, std::array<float, 3>>) {
            outFile << "float3:";
            outFile << paramName << "=";
            outFile << val[0] << "," << val[1] << "," << val[2];
        }

        // If the value is an array of 4 floats
        else if constexpr (std::is_same_v<std::decay_t<decltype(val)>, std::array<float, 4>>) {
            outFile << "float4:";
            outFile << paramName << "=";
            outFile << val[0] << "," << val[1] << "," << val[2] << "," << val[3];
        }

        outFile << "\n";
        }, value);


}
    bool ParseFloats(const std::string& str, float* out, int count) {
        std::stringstream ss(str);
        std::string item;
        int i = 0;

        while (std::getline(ss, item, ',') && i < count) {
            try {
                out[i++] = std::stof(item);
            }
            catch (...) {
                return false;
            }
        }

        return i == count;
    }
public:
    bool SerializeSettings(const std::string& path) {

        std::ofstream outFile(path);

        if (!outFile) {
            std::cerr << "Failed to open settings file for writing!" << std::endl;
            return false;
        }

        for (const auto& pair : Variables) {
            const std::string& key = pair.first;
            const VariableValue& value = pair.second;
            WriteValue(outFile, key, value);
        }  
        outFile.close();
        return true;
    }
    bool SerializeSettings(fs::path path) {

        std::ofstream outFile(path);

        if (!outFile) {
            std::cerr << "Failed to open settings file for writing!" << std::endl;
            return false;
        }

        for (const auto& pair : Variables) {
            const std::string& key = pair.first;
            const VariableValue& value = pair.second;
            WriteValue(outFile, key, value);
        }
        outFile.close();
        return true;
    }
    bool DeserializeSettings(std::string path) {

        std::ifstream inFile(path);
        std::unordered_map<std::string, VariableValue> variables;

        if (!inFile) {
            std::cerr << "Failed to open file.\n";
            return false;
        }

        std::string line;
        while (std::getline(inFile, line)) {
            // Ignore empty lines
            if (line.empty()) continue;

            // Find the type before the colon
            size_t typeEnd = line.find(':');
            std::string type = line.substr(0, typeEnd);

            // Find the name between ":" and "="
            size_t nameStart = typeEnd + 1;
            size_t nameEnd = line.find('=', nameStart);
            std::string name = line.substr(nameStart, nameEnd - nameStart);

            // Get the values after the "="
            std::string values = line.substr(nameEnd + 1);

            // Parse based on type
            if (type == "string") {
                variables[name] = values.substr(1, values.size() - 2);
            }
            else if (type == "bool") {
                variables[name] = (bool)std::stoi(values);
            }
            else if (type == "int") {
                variables[name] = std::stoi(values);
            }
            else if (type == "uint") {
                variables[name] = static_cast<unsigned int>(std::stoul(values));
            }
            else if (type == "float") {
                variables[name] = std::stof(values);
            }
            else if (type == "float2") {
                std::array<float, 2> arr;
                ParseFloats(values, arr.data(), 2);
                variables[name] = arr;
            }
            else if (type == "float3") {
                std::array<float, 3> arr;
                ParseFloats(values, arr.data(), 3);
                variables[name] = arr;
            }
            else if (type == "float4") {
                std::array<float, 4> arr;
                ParseFloats(values, arr.data(), 4);
                variables[name] = arr;
            }
        }
        inFile.close();
        Variables = variables;
        return true;
    }
    bool DeserializeSettings(fs::path path) {

        std::ifstream inFile(path);
        std::unordered_map<std::string, VariableValue> variables;

        if (!inFile) {
            std::cerr << "Failed to open file.\n";
            return false;
        }

        std::string line;
        while (std::getline(inFile, line)) {
            // Ignore empty lines
            if (line.empty()) continue;

            // Find the type before the colon
            size_t typeEnd = line.find(':');
            std::string type = line.substr(0, typeEnd);

            // Find the name between ":" and "="
            size_t nameStart = typeEnd + 1;
            size_t nameEnd = line.find('=', nameStart);
            std::string name = line.substr(nameStart, nameEnd - nameStart);

            // Get the values after the "="
            std::string values = line.substr(nameEnd + 1);

            // Parse based on type
            if (type == "string") {
                variables[name] = values.substr(1, values.size() - 2);
            }
            else if (type == "bool") {
                variables[name] = (bool)std::stoi(values);
            }
            else if (type == "int") {
                variables[name] = std::stoi(values);
            }
            else if (type == "uint") {
                variables[name] = static_cast<unsigned int>(std::stoul(values));
            }
            else if (type == "float") {
                variables[name] = std::stof(values);
            }
            else if (type == "float2") {
                std::array<float, 2> arr;
                ParseFloats(values, arr.data(), 2);
                variables[name] = arr;
            }
            else if (type == "float3") {
                std::array<float, 3> arr;
                ParseFloats(values, arr.data(), 3);
                variables[name] = arr;
            }
            else if (type == "float4") {
                std::array<float, 4> arr;
                ParseFloats(values, arr.data(), 4);
                variables[name] = arr;
            }
        }
        inFile.close();
        Variables = variables;
        return true;
    }

};
