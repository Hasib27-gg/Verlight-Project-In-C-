#ifndef ___MEMORY___MODEL___H___
# define ___MEMORY___MODEL___H___

#include "includes.h"
typedef std::string str; 
enum typeEnum {
    ENUM_TYPE_I8 = 0x1A3F,
    ENUM_TYPE_I16 = 0x2B7C,
    ENUM_TYPE_I32 = 0x3D91,
    ENUM_TYPE_I64 = 0x4E2A,
    ENUM_TYPE_F32 = 0x5F88,
    ENUM_TYPE_F64 = 0x6C14,
    ENUM_TYPE_FMAX = 0x7AEE,
    ENUM_TYPE_BOOL = 0x8D03,
    ENUM_TYPE_CHAR = 0x9B6F,
    ENUM_TYPE_STRING = 0xAF42,
};

typedef std::pair<std::variant<int8_t, int16_t, int32_t, int64_t,
    float, double, long double, bool, char, str>, typeEnum> value_t;

str makeValueToString(const value_t& value) {
    str result; 
    auto writeFloat = [](auto v) {
        std::ostringstream oss;               
        oss << std::fixed << std::setprecision(6) << v; 
        return oss.str();                     
        };

    switch (value.second) {
    case ENUM_TYPE_I8:
        result = std::to_string(std::get<int8_t>(value.first));
        break;
    case ENUM_TYPE_I16:
        result = std::to_string(std::get<int16_t>(value.first));
        break;
    case ENUM_TYPE_I32:
        result = std::to_string(std::get<int32_t>(value.first));
        break;
    case ENUM_TYPE_I64:
        result = std::to_string(std::get<int64_t>(value.first));
        break;
    case ENUM_TYPE_F32:
        result = writeFloat(std::get<float>(value.first));
        break;
    case ENUM_TYPE_F64:
        result = writeFloat(std::get<double>(value.first));
        break;
    case ENUM_TYPE_FMAX:
        result = writeFloat(std::get<long double>(value.first));
        break;
    case ENUM_TYPE_CHAR:
        result = std::string(1, std::get<char>(value.first));
        break;
    case ENUM_TYPE_BOOL:
        std::get<bool>(value.first) ? result = "true" : result = "false";
        break;
    case ENUM_TYPE_STRING:
        result = std::get<str>(value.first);
        break;
    default:
        result = "<unknown>";
        break;
    }
    return result; 
}

#define UnorderedMap robin_hood::unordered_map
void printValue(const value_t& value) {
    std::cout << std::fixed << std::setprecision(6); 
    switch (value.second) {
    case ENUM_TYPE_I8:
        std::cout << +std::get<int8_t>(value.first);
        break;
    case ENUM_TYPE_I16:
        std::cout << std::get<int16_t>(value.first);
        break;
    case ENUM_TYPE_I32:
        std::cout << std::get<int32_t>(value.first);
        break;
    case ENUM_TYPE_I64:
        std::cout << std::get<int64_t>(value.first);
        break;
    case ENUM_TYPE_F32:
        std::cout << std::get<float>(value.first);
        break;
    case ENUM_TYPE_F64:
        std::cout << std::get<double>(value.first);
        break;
    case ENUM_TYPE_FMAX:
        std::cout << std::get<long double>(value.first);
        break;
    case ENUM_TYPE_CHAR:
        std::cout << std::get<char>(value.first);
        break;
    case ENUM_TYPE_BOOL:
        std::cout << std::get<bool>(value.first);
        break;
    case ENUM_TYPE_STRING:
        std::cout << std::get<str>(value.first);
        break;
    }
}
class SectionMemory {
private:
    UnorderedMap<str, int8_t> Buffer_I8;
    UnorderedMap<str, int16_t> Buffer_I16;
    UnorderedMap<str, int32_t> Buffer_I32;
    UnorderedMap<str, int64_t> Buffer_I64;
    UnorderedMap<str, float> Buffer_F32;
    UnorderedMap<str, double> Buffer_F64;
    UnorderedMap<str, long double> Buffer_Fmax;
    UnorderedMap<str, bool> Buffer_Bool;
    UnorderedMap<str, char> Buffer_Char;
    UnorderedMap<str, str> Buffer_String;
    UnorderedMap<str, typeEnum> varLocations;

public:
    void m_insertVariable(const std::string& varName, const value_t& value) {
        if (varLocations.contains(varName))
            throw std::runtime_error("Variable already exists; use reInsertVariable().");
        const auto& v = value.first; 
        const auto t = value.second; 
        switch (t) {
        case ENUM_TYPE_I8:
            Buffer_I8.emplace(varName, std::get<int8_t>(v)); 
            break;
        case ENUM_TYPE_I16:
            Buffer_I16.emplace(varName, std::get<int16_t>(v)); 
            break;
        case ENUM_TYPE_I32:
            Buffer_I32.emplace(varName, std::get<int32_t>(v)); 
            break;
        case ENUM_TYPE_I64:
            Buffer_I64.emplace(varName, std::get<int64_t>(v)); 
            break;
        case ENUM_TYPE_F32:
            Buffer_F32.emplace(varName, std::get<float>(v)); 
            break;
        case ENUM_TYPE_F64:
            Buffer_F64.emplace(varName, std::get<double>(v)); 
            break;
        case ENUM_TYPE_FMAX:
            Buffer_Fmax.emplace(varName, std::get<long double>(v)); 
            break;
        case ENUM_TYPE_BOOL:
            Buffer_Bool.emplace(varName, std::get<bool>(v));
            break;
        case ENUM_TYPE_CHAR:
            Buffer_Char.emplace(varName, std::get<char>(v)); 
            break;
        case ENUM_TYPE_STRING:
            Buffer_String.emplace(varName, std::get<str>(v)); 
            break;
        default:
            throw std::runtime_error("Unknown typeEnum in insert().");
        }
        varLocations.emplace(varName, t);
    }
    void m_removeVariable(const str& varName) {
        if (!varLocations.contains(varName))
            throw std::runtime_error("Variable does not exist.");

        typeEnum t = varLocations.at(varName);
        switch (t) {
        case ENUM_TYPE_I8:
            Buffer_I8.erase(varName);
            break;
        case ENUM_TYPE_I16:
            Buffer_I16.erase(varName);
            break;
        case ENUM_TYPE_I32:
            Buffer_I32.erase(varName);
            break;
        case ENUM_TYPE_I64:
            Buffer_I64.erase(varName);
            break;
        case ENUM_TYPE_F32:
            Buffer_F32.erase(varName);
            break;
        case ENUM_TYPE_F64:
            Buffer_F64.erase(varName);
            break;
        case ENUM_TYPE_FMAX:
            Buffer_Fmax.erase(varName);
            break;
        case ENUM_TYPE_BOOL:
            Buffer_Bool.erase(varName);
            break;
        case ENUM_TYPE_CHAR:
            Buffer_Char.erase(varName);
            break;
        case ENUM_TYPE_STRING:
            Buffer_String.erase(varName);
            break;
        default:
            throw std::runtime_error("Unknown typeEnum in removeVariable().");
        }
        varLocations.erase(varName);
    }
    void m_reInsertVariable(const str& varName, const value_t& value) {
        if (!varLocations.contains(varName))
            throw std::runtime_error("Variable doesn't exisit at all. Use insertVariable()");

        const auto& v = value.first;
        const auto t = value.second; 
        m_removeVariable(varName);
        switch (t) {
        case ENUM_TYPE_I8:
            Buffer_I8.emplace(varName, std::get<int8_t>(v));
            break;
        case ENUM_TYPE_I16:
            Buffer_I16.emplace(varName, std::get<int16_t>(v));
            break;
        case ENUM_TYPE_I32:
            Buffer_I32.emplace(varName, std::get<int32_t>(v));
            break;
        case ENUM_TYPE_I64:
            Buffer_I64.emplace(varName, std::get<int64_t>(v));
            break;
        case ENUM_TYPE_F32:
            Buffer_F32.emplace(varName, std::get<float>(v));
            break;
        case ENUM_TYPE_F64:
            Buffer_F64.emplace(varName, std::get<double>(v));
            break;
        case ENUM_TYPE_FMAX:
            Buffer_Fmax.emplace(varName, std::get<long double>(v));
            break;
        case ENUM_TYPE_BOOL:
            Buffer_Bool.emplace(varName, std::get<bool>(v));
            break;
        case ENUM_TYPE_CHAR:
            Buffer_Char.emplace(varName, std::get<char>(v));
            break;
        case ENUM_TYPE_STRING:
            Buffer_String.emplace(varName, std::get<str>(v));
            break;
        default:
            throw std::runtime_error("Unknown typeEnum in reInsertVariable().");
        }
        varLocations.emplace(varName, t);
    }

    value_t m_getVariable(const str& varName) {
        if (!varLocations.contains(varName))
            throw std::runtime_error("Variable does not exist.");

        typeEnum t = varLocations.at(varName);
        switch (t) {
        case ENUM_TYPE_I8:
            return value_t({ Buffer_I8.at(varName), ENUM_TYPE_I8 });
        case ENUM_TYPE_I16:
            return value_t({ Buffer_I16.at(varName), ENUM_TYPE_I16 });
        case ENUM_TYPE_I32:
            return value_t({ Buffer_I32.at(varName), ENUM_TYPE_I32 });
        case ENUM_TYPE_I64:
            return value_t({ Buffer_I64.at(varName), ENUM_TYPE_I64 });
        case ENUM_TYPE_F32:
            return value_t({ Buffer_F32.at(varName), ENUM_TYPE_F32 });
        case ENUM_TYPE_F64:
            return value_t({ Buffer_F64.at(varName), ENUM_TYPE_F64 });
        case ENUM_TYPE_FMAX:
            return value_t({ Buffer_Fmax.at(varName), ENUM_TYPE_FMAX });
        case ENUM_TYPE_BOOL:
            return value_t({ Buffer_Bool.at(varName), ENUM_TYPE_BOOL });
        case ENUM_TYPE_CHAR:
            return value_t({ Buffer_Char.at(varName), ENUM_TYPE_CHAR });
        case ENUM_TYPE_STRING:
            return value_t({ Buffer_String.at(varName), ENUM_TYPE_STRING });
        default:
            throw std::runtime_error("Unknown typeEnum in getVariable().");
        }
    }
    void m_passVariabe(const str& varName, SectionMemory& to) {
        if (to.varLocations.contains(varName))
            throw std::runtime_error("Error! Variable already exists at the reciver.");
        if (!varLocations.contains(varName))
            throw std::runtime_error("Variable doesn't exists.");
        to.m_insertVariable(varName, m_getVariable(varName));
        m_removeVariable(varName);
    }
    bool m_containsVariable(const str& varName) {
        return varLocations.contains(varName);
    }

    void m_monitorVariables() {
        std::cout << "|===============================================|\n";
        auto formatFloat = [](auto val) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(6) << val;
            return oss.str();
            };
        if (!Buffer_I8.empty()) {
            std::cout << "BUFFER_I8: \n";
            int idx = 0;
            for (auto& item : Buffer_I8) {
                std::cout << '[' << idx << "] " << item.first << ": " << +item.second << '\n';
                idx++;
            }
        }

        if (!Buffer_I16.empty()) {
            std::cout << "\nBUFFER_I16: \n";
            int idx = 0;
            for (auto& item : Buffer_I16) {
                std::cout << '[' << idx << "] " << item.first << ": " << item.second << '\n';
                idx++;
            }
        }

        if (!Buffer_I32.empty()) {
            std::cout << "\nBUFFER_I32: \n";
            int idx = 0;
            for (auto& item : Buffer_I32) {
                std::cout << '[' << idx << "] " << item.first << ": " << item.second << '\n';
                idx++;
            }
        }

        if (!Buffer_I64.empty()) {
            std::cout << "\nBUFFER_I64: \n";
            int idx = 0;
            for (auto& item : Buffer_I64) {
                std::cout << '[' << idx << "] " << item.first << ": " << item.second << '\n';
                idx++;
            }
        }

        if (!Buffer_F32.empty()) {
            std::cout << "\nBUFFER_F32: \n";
            int idx = 0;
            for (auto& item : Buffer_F32) {
                std::cout << '[' << idx << "] " << item.first << ": " << formatFloat(item.second) << '\n';
                idx++;
            }
        }

        if (!Buffer_F64.empty()) {
            std::cout << "\nBUFFER_F64: \n";
            int idx = 0;
            for (auto& item : Buffer_F64) {
                std::cout << '[' << idx << "] " << item.first << ": " << formatFloat(item.second) << '\n';
                idx++;
            }
        }

        if (!Buffer_Fmax.empty()) {
            std::cout << "\nBUFFER_FMAX: \n";
            int idx = 0;
            for (auto& item : Buffer_Fmax) {
                std::cout << '[' << idx << "] " << item.first << ": " << formatFloat(item.second) << '\n';
                idx++;
            }
        }

        if (!Buffer_Char.empty()) {
            std::cout << "\nBUFFER_CHAR: \n";
            int idx = 0;
            for (auto& item : Buffer_Char) {
                std::cout << '[' << idx << "] " << item.first << ": '" << item.second << '\'' << '\n';
                idx++;
            }
        }

        if (!Buffer_String.empty()) {
            std::cout << "\nBUFFER_STRING: \n";
            int idx = 0;
            for (auto& item : Buffer_String) {
                // print string wrapped in double quotes
                std::cout << '[' << idx << "] " << item.first << ": \"" << item.second << '"' << '\n';
                idx++;
            }
        }

        if (!Buffer_Bool.empty()) {
            std::cout << "\nBUFFER_BOOL: \n";
            int idx = 0;
            for (auto& item : Buffer_Bool) {
                std::cout << '[' << idx << "] " << item.first << ": " << item.second << '\n';
                idx++;
            }
        }
        if (!varLocations.empty()) {
            std::cout << "\nVAR_LOCATIONS_BLUEPRINT: \n";
            int idx = 0;
            for (auto& item : varLocations) {
                std::cout << '[' << idx << "] " << item.first << ": ";
                switch (item.second) {
                case ENUM_TYPE_I8:
                    std::cout << "ENUM_TYPE_I8" << '\n';
                    break;
                case ENUM_TYPE_I16:
                    std::cout << "ENUM_TYPE_I16" << '\n';
                    break;
                case ENUM_TYPE_I32:
                    std::cout << "ENUM_TYPE_I32" << '\n';
                    break;
                case ENUM_TYPE_I64:
                    std::cout << "ENUM_TYPE_I64" << '\n';
                    break;
                case ENUM_TYPE_F32:
                    std::cout << "ENUM_TYPE_F32" << '\n';
                    break;
                case ENUM_TYPE_F64:
                    std::cout << "ENUM_TYPE_F64" << '\n';
                    break;
                case ENUM_TYPE_FMAX:
                    std::cout << "ENUM_TYPE_FMAX" << '\n';
                    break;
                case ENUM_TYPE_BOOL:
                    std::cout << "ENUM_TYPE_BOOL" << '\n';
                    break;
                case ENUM_TYPE_STRING:
                    std::cout << "ENUM_TYPE_STRING" << '\n';
                    break;
                case ENUM_TYPE_CHAR:
                    std::cout << "ENUM_TYPE_CHAR" << '\n';
                    break;
                default:
                    throw std::runtime_error("Invalid ENUM_TYPE_* for typeEnum!");
                    break;
                }
                idx++;
            }
        }

        std::cout << "\n|===============================================|\n\n";
    }
    str m_fetchStringalizedValue(str& var_name) {
        var_name = var_name.substr(1);
        if (!m_containsVariable(var_name)) {
            std::cerr <<
                "\n[ERROR_ENGINE]  The var_name that is causing exception is: ("
                << var_name << ")\n";

            throw std::runtime_error("Can't find the variable!");
        }
        return makeValueToString(m_getVariable(var_name));
    }
};

# endif

