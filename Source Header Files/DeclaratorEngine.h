#ifndef ___DECLARATOR___ENGINE___H___
#define ___DECLARATOR___ENGINE___H___

#include "MemoryModel.h"
class DeclaratorEngine {
private:
    static bool ___isNum___(const std::string& s)
    {
        if (s.empty()) return false;         

        int dotCount = 0;                   

        for (size_t i = 0; i < s.size(); ++i)
        {
            char c = s[i];

            if (c == '.')
            {
                dotCount++;
                if (dotCount > 1) return false; 
            }
            else if (c == '-' || c == '+')
            {
                if (i != 0) return false;      
            }
            else if (!std::isdigit(c))
                return false;                
        }
        if (s == "+" || s == "-" || s == ".")
            return false;

        return true;
    }

public:
    static void new_i8
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@new_i8 can only take 2 parameters!"); 
        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");
        str name = parameters[0];
        str value = parameters[1]; 
        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);
        if (current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' already exists!");
        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");
        long long raw_value = std::stoll(value); 
        if (raw_value < -128 || raw_value > 127)
            throw std::runtime_error(std::string("Overflow: value for '") + name + std::string("' cannot fit in i8."));

        int8_t value_i8 = static_cast<int8_t>(raw_value);
        value_t valtValue = { value_i8 , ENUM_TYPE_I8 };

        current_memory.m_insertVariable(name, valtValue); 
    }
    static void new_i16
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@new_i16 can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' already exists!");

        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");

        long long raw_value = std::stoll(value);

        if (raw_value < -32768 || raw_value > 32767)
            throw std::runtime_error(std::string("Overflow: value for '") + name + "' cannot fit in i16.");

        int16_t value_i16 = static_cast<int16_t>(raw_value);
        value_t valtValue = { value_i16, ENUM_TYPE_I16 };

        current_memory.m_insertVariable(name, valtValue);
    }
    static void new_i32
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@new_i32 can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' already exists!");

        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");

        long long raw_value = std::stoll(value);

        if (raw_value < INT32_MIN || raw_value > INT32_MAX)
            throw std::runtime_error(std::string("Overflow: value for '") + name + "' cannot fit in i32.");

        int32_t value_i32 = static_cast<int32_t>(raw_value);
        value_t valtValue = { value_i32, ENUM_TYPE_I32 };

        current_memory.m_insertVariable(name, valtValue);
    }
    static void new_i64
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@new_i64 can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' already exists!");

        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");

        long long raw_value = std::stoll(value);

        int64_t value_i64 = static_cast<int64_t>(raw_value);
        value_t valtValue = { value_i64, ENUM_TYPE_I64 };

        current_memory.m_insertVariable(name, valtValue);
    }
    static void new_f32
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@new_f32 can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' already exists!");

        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");

        float raw_value = std::stof(value);
        value_t valtValue = { raw_value, ENUM_TYPE_F32 };

        current_memory.m_insertVariable(name, valtValue);
    }
    static void new_f64
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@new_f64 can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' already exists!");

        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");

        double raw_value = std::stod(value);
        value_t valtValue = { raw_value, ENUM_TYPE_F64 };

        current_memory.m_insertVariable(name, valtValue);
    }
    static void new_fmax
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@new_fmax can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' already exists!");

        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");

        long double raw_value = std::stold(value);
        value_t valtValue = { raw_value, ENUM_TYPE_FMAX };

        current_memory.m_insertVariable(name, valtValue);
    }
    static void new_str
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@new_str can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
        {
            value = current_memory.m_fetchStringalizedValue(value);
        }
        else
        {
            value = value.substr(1, value.size() - 2);
        }

        if (current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' already exists!");

        value_t valtValue = { value , ENUM_TYPE_STRING };

        current_memory.m_insertVariable(name, valtValue);
    }
    static void new_char
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@new_char requires exactly 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
        {
            value = current_memory.m_fetchStringalizedValue(value);
            if (value.empty())
                throw std::runtime_error("Reference resolved to empty string for '" + name + "'");
        }
        else
        {
            if (value.size() < 2)
                throw std::runtime_error("Invalid literal for char: '" + value + "'");
            value = value.substr(1, value.size() - 2);
        }

        char c;

        if (value.front() == '\\')
        {
            if (value.size() == 2)
            {
                switch (value[1])
                {
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case 'r': c = '\r'; break;
                case 'b': c = '\b'; break;
                case 'f': c = '\f'; break;
                case 'v': c = '\v'; break;
                case '\\': c = '\\'; break;
                case '\'': c = '\''; break;
                case '"': c = '"'; break;
                case '0': c = '\0'; break;
                default:
                    throw std::runtime_error("Unknown escape sequence '\\" + std::string(1, value[1]) + "' for '" + name + "'");
                }
            }
            else if (value[1] == 'x')
            {
                if (value.size() != 4)
                    throw std::runtime_error("Invalid hex escape length for '" + name + "'");

                int hexVal = std::stoi(value.substr(2, 2), nullptr, 16);
                c = static_cast<char>(hexVal);
            }
            else if (isdigit(value[1]) && value[1] >= '0' && value[1] <= '7')
            {
                int len = std::min(3, static_cast<int>(value.size() - 1));
                int octVal = std::stoi(value.substr(1, len), nullptr, 8);
                c = static_cast<char>(octVal);
            }
            else
            {
                throw std::runtime_error("Unknown escape format for '" + name + "'");
            }
        }
        else
        {
            if (value.size() != 1)
                throw std::runtime_error("Fatal error! char length must be 1 for '" + name + "'");

            c = value[0];
        }

        if (current_memory.m_containsVariable(name))
            throw std::runtime_error("Variable '" + name + "' already exists! Consider using reAssign_char()");

        value_t valtValue = { c, ENUM_TYPE_CHAR };

        current_memory.m_insertVariable(name, valtValue);
    }
    static void new_bool
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@new_bool can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' already exists!");

        bool boolValue = true;

        if (value == "false")
            boolValue = false;

        if (value != "false" && value != "true")
            throw std::runtime_error("Boolean value must be true or false!");

        value_t valtValue = { boolValue , ENUM_TYPE_BOOL };

        current_memory.m_insertVariable(name, valtValue);
    }
    static void reAssign_i8
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@reAssign_i8 can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (!current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' doesn't exist!");

        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");

        long long raw_value = std::stoll(value);

        if (raw_value < -128 || raw_value > 127)
            throw std::runtime_error(std::string("Overflow: value for '") + name + "' cannot fit in i8.");

        int8_t value_i8 = static_cast<int8_t>(raw_value);
        value_t valtValue = { value_i8 , ENUM_TYPE_I8 };

        current_memory.m_reInsertVariable(name, valtValue);
    }

    static void reAssign_i16
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@reAssign_i16 can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (!current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' doesn't exist!");

        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");

        long long raw_value = std::stoll(value);

        if (raw_value < -32768 || raw_value > 32767)
            throw std::runtime_error(std::string("Overflow: value for '") + name + "' cannot fit in i16.");

        int16_t value_i16 = static_cast<int16_t>(raw_value);
        value_t valtValue = { value_i16, ENUM_TYPE_I16 };

        current_memory.m_reInsertVariable(name, valtValue);
    }

    static void reAssign_i32
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@reAssign_i32 can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (!current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' doesn't exist!");

        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");

        long long raw_value = std::stoll(value);

        if (raw_value < INT32_MIN || raw_value > INT32_MAX)
            throw std::runtime_error(std::string("Overflow: value for '") + name + "' cannot fit in i32.");

        int32_t value_i32 = static_cast<int32_t>(raw_value);
        value_t valtValue = { value_i32, ENUM_TYPE_I32 };

        current_memory.m_reInsertVariable(name, valtValue);
    }

    static void reAssign_i64
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@reAssign_i64 can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (!current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' doesn't exist!");

        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");

        long long raw_value = std::stoll(value);

        int64_t value_i64 = static_cast<int64_t>(raw_value);
        value_t valtValue = { value_i64, ENUM_TYPE_I64 };

        current_memory.m_reInsertVariable(name, valtValue);
    }

    static void reAssign_f32
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@reAssign_f32 can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (!current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' doesn't exist!");

        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");

        float raw_value = std::stof(value);
        value_t valtValue = { raw_value, ENUM_TYPE_F32 };

        current_memory.m_reInsertVariable(name, valtValue);
    }

    static void reAssign_f64
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@reAssign_f64 can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (!current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' doesn't exist!");

        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");

        double raw_value = std::stod(value);
        value_t valtValue = { raw_value, ENUM_TYPE_F64 };

        current_memory.m_reInsertVariable(name, valtValue);
    }

    static void reAssign_fmax
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@reAssign_fmax can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (!current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' doesn't exist!");

        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");

        long double raw_value = std::stold(value);
        value_t valtValue = { raw_value, ENUM_TYPE_FMAX };

        current_memory.m_reInsertVariable(name, valtValue);
    }

    static void reAssign_str
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@reAssign_str can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
        {
            value = current_memory.m_fetchStringalizedValue(value);
        }
        else
        {
            value = value.substr(1, value.size() - 2); 
        }

        if (!current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' doesn't exist!");

        value_t valtValue = { value , ENUM_TYPE_STRING };

        current_memory.m_reInsertVariable(name, valtValue);
    }

    static void reAssign_char
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@reAssign_char requires exactly 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
        {
            value = current_memory.m_fetchStringalizedValue(value);
            if (value.empty())
                throw std::runtime_error("Reference resolved to empty string for '" + name + "'");
        }
        else
        {
            if (value.size() < 2)
                throw std::runtime_error("Invalid literal for char: '" + value + "'");

            value = value.substr(1, value.size() - 2);
        }

        char c;

        if (value.front() == '\\')
        {
            if (value.size() == 2)
            {
                switch (value[1])
                {
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case 'r': c = '\r'; break;
                case 'b': c = '\b'; break;
                case 'f': c = '\f'; break;
                case 'v': c = '\v'; break;
                case '\\': c = '\\'; break;
                case '\'': c = '\''; break;
                case '"': c = '"'; break;
                case '0': c = '\0'; break;
                default:
                    throw std::runtime_error("Unknown escape sequence '\\" + std::string(1, value[1]) + "' for '" + name + "'");
                }
            }
            else if (value[1] == 'x')
            {
                if (value.size() != 4)
                    throw std::runtime_error("Invalid hex escape length for '" + name + "'");

                int hexVal = std::stoi(value.substr(2, 2), nullptr, 16);
                c = static_cast<char>(hexVal);
            }
            else if (isdigit(value[1]) && value[1] >= '0' && value[1] <= '7')
            {
                int len = std::min(3, static_cast<int>(value.size() - 1));
                int octVal = std::stoi(value.substr(1, len), nullptr, 8);
                c = static_cast<char>(octVal);
            }
            else
            {
                throw std::runtime_error("Unknown escape format for '" + name + "'");
            }
        }
        else
        {
            if (value.size() != 1)
                throw std::runtime_error("Fatal error! char length must be 1 for '" + name + "'");

            c = value[0];
        }

        if (!current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' doesn't exist!");

        value_t valtValue = { c, ENUM_TYPE_CHAR };

        current_memory.m_reInsertVariable(name, valtValue);
    }

    static void reAssign_bool
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@reAssign_bool can only take 2 parameters!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        str value = parameters[1];

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value);

        if (!current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' doesn't exist!");

        bool boolValue = true;

        if (value == "false")
            boolValue = false;

        if (value != "false" && value != "true")
            throw std::runtime_error("Boolean value must be true or false!");

        value_t valtValue = { boolValue , ENUM_TYPE_BOOL };

        current_memory.m_reInsertVariable(name, valtValue);
    }
    static void delete_var
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 1)
            throw std::runtime_error("@delete_var requires exactly 1 parameter!");

        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0];
        if (name.empty())
            throw std::runtime_error("Variable name for @delete_var is empty!");

        if (!current_memory.m_containsVariable(name))
            throw std::runtime_error("Can't find the variable '" + name + "' in the memory!");

        current_memory.m_removeVariable(name);
    }

};

#endif


