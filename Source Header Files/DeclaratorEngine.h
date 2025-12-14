#ifndef ___DECLARATOR___ENGINE___H___
#define ___DECLARATOR___ENGINE___H___

#include "MemoryModel.h"

/*
Order of member functions inside DeclaratorEngine (quick reference):
  - private:
      ___isNum___                 (helper: validate numeric string)
  - public (creation functions):
      new_i8
      new_i16
      new_i32
      new_i64
      new_f32
      new_f64
      new_fmax
      new_str
      new_char
      new_bool
  - public (re-assignment functions):
      reAssign_i8
      reAssign_i16
      reAssign_i32
      reAssign_i64
      reAssign_f32
      reAssign_f64
      reAssign_fmax
      reAssign_str
      reAssign_char
      reAssign_bool
  - public (deletion functions):
      delete_i8, delete_i16, delete_i32, delete_i64,
      delete_f32, delete_f64, delete_fmax,
      delete_str, delete_char, delete_bool

Rationale:
- Private helper at top for internal use.
- Grouped public API by intent: create / reassign / delete, in that order.
*/

class DeclaratorEngine {
private:

    /*
     ___isNum___(const std::string& s)
     Parameters:
       - s : const std::string& — string to validate as a numeric literal (supports optional leading +/-, and one optional decimal point)
     Description:
       - Returns true if the string represents a valid numeric token in the VM's literal format.
       - Accepts integer and floating representations like: "123", "-42", "+3.14", "0.5", "10.0"
       - Rejects strings with multiple dots, digits in wrong places, or lone signs/punctuation.
     Visualization:
              "123"   -> true
              "-3.14" -> true
              "."     -> false
              "+."    -> false
     Per-line notes:
       - empty input -> false
       - dotCount tracks number of decimal points (only one allowed)
       - allow leading '+' or '-' only at position 0
       - every other character must be a digit or a single dot
    */
    static bool ___isNum___(const std::string& s)
    {
        if (s.empty()) return false;          // empty string is not numeric

        int dotCount = 0;                     // count of '.' characters

        for (size_t i = 0; i < s.size(); ++i)
        {
            char c = s[i];

            if (c == '.')
            {
                dotCount++;
                if (dotCount > 1) return false; // more than one '.' is invalid
            }
            else if (c == '-' || c == '+')
            {
                if (i != 0) return false;       // sign only allowed at first character
            }
            else if (!std::isdigit(c))
                return false;                  // any non-digit (except handled above) is invalid
        }

        // a single sign or a single dot is not a valid number
        if (s == "+" || s == "-" || s == ".")
            return false;

        return true;
    }

public:

    /* -------------------------------------------------------------------------
       new_i8(...)
       Parameters:
         - parameters: const std::vector<str>& : [ variableName, valueLiteralOr$ref ]
         - return_address: const str&         : where a result would go; validated for existence
         - current_memory: SectionMemory&     : runtime memory to insert variable into
       Description:
         - Create a new signed 8-bit integer variable with name parameters[0] and value parsed from parameters[1].
         - Supports value as literal string (e.g. "123") or memory reference like "$x".
         - Performs numeric validation and bounds-checking for int8 range (-128..127).
       Visualization:
            call: new_i8(["a","42"], "someAddr", mem)
            -> inserts 'a' in mem.Buffer_I8 as 42 and varLocations["a"]=ENUM_TYPE_I8
       Per-line notes:
         - validate parameter count and return_address
         - resolve references that start with '$'
         - check variable does not exist yet
         - check numeric literal using ___isNum___
         - convert to integer, perform overflow check, then insert into SectionMemory
    ------------------------------------------------------------------------- */
    static void new_i8
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("@new_i8 can only take 2 parameters!"); // must have name and value

        // make sure the return_address exists (or is the literal "nullptr")
        if (!current_memory.m_containsVariable(return_address) && return_address != "nullptr")
            throw std::runtime_error("Can't find the return address '" + return_address + "' in the memory!");

        str name = parameters[0]; // variable name
        str value = parameters[1]; // value string or "$ref"

        if (value.front() == '$')
            value = current_memory.m_fetchStringalizedValue(value); // dereference memory reference

        if (current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' already exists!");

        if (!___isNum___(value))
            throw std::runtime_error("Value must be a valid number!");

        long long raw_value = std::stoll(value); // parse as long long to check bounds

        if (raw_value < -128 || raw_value > 127)
            throw std::runtime_error(std::string("Overflow: value for '") + name + std::string("' cannot fit in i8."));

        int8_t value_i8 = static_cast<int8_t>(raw_value);
        value_t valtValue = { value_i8 , ENUM_TYPE_I8 };

        current_memory.m_insertVariable(name, valtValue); // insert into memory
    }

    /* -------------------------------------------------------------------------
       new_i16(...)
       Parameters/behavior: same shape as new_i8 but for 16-bit signed integers.
       Checks range -32768..32767. Otherwise same validation/de-reference logic as new_i8.
    ------------------------------------------------------------------------- */
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

    /* -------------------------------------------------------------------------
       new_i32(...)
       Creates a new 32-bit signed integer. Uses INT32_MIN/INT32_MAX for bounds.
    ------------------------------------------------------------------------- */
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

    /* -------------------------------------------------------------------------
       new_i64(...)
       Create 64-bit integer variable. No explicit overflow check beyond stoll parsing.
    ------------------------------------------------------------------------- */
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

    /* -------------------------------------------------------------------------
       new_f32(...)
       Create 32-bit float variable. Parses via std::stof.
    ------------------------------------------------------------------------- */
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

    /* -------------------------------------------------------------------------
       new_f64(...)
       Create 64-bit double variable. Parses via std::stod.
    ------------------------------------------------------------------------- */
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

    /* -------------------------------------------------------------------------
       new_fmax(...)
       Create long double variable. Parses via std::stold.
    ------------------------------------------------------------------------- */
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

    /* -------------------------------------------------------------------------
       new_str(...)
       Parameters:
         - name, value
         - value is either "$ref" or a quoted literal like "\"hello\""
       Description:
         - Creates a new string variable. If value begins with '$', it will dereference.
         - Otherwise it strips the surrounding quotes (first and last char).
       Visualization:
           new_str(["s","\"hi\""], ...) -> inserts "hi" as ENUM_TYPE_STRING
    ------------------------------------------------------------------------- */
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
            value = current_memory.m_fetchStringalizedValue(value); // deref
        }
        else
        {
            // strip surrounding quotes: from "\"abc\"" -> "abc"
            value = value.substr(1, value.size() - 2);
        }

        if (current_memory.m_containsVariable(name))
            throw std::runtime_error(std::string("Variable '") + name + "' already exists!");

        value_t valtValue = { value , ENUM_TYPE_STRING };

        current_memory.m_insertVariable(name, valtValue);
    }

    /* -------------------------------------------------------------------------
       new_char(...)
       Create a new char variable. Supports character literals and escape sequences.
       Parameters:
         - name, value
         - value is "$ref" or quoted char literal like "'a'" or escaped sequences "'\\n'", "'\\x41'" or octal "'\\123'"
       Description:
         - If the value is a reference, dereference and validate result non-empty.
         - If literal, remove surrounding quotes and parse escapes:
             - simple escape sequences (n, t, r, b, f, v, \, ', ", 0)
             - hex escapes: \xNN (exactly two hex digits)
             - octal escapes: up to three octal digits (0-7)
       Visualization:
         new_char(["c","'a'"], ...) -> inserts 'a' as ENUM_TYPE_CHAR
         new_char(["c","'\\n'"], ...) -> inserts newline char
    ------------------------------------------------------------------------- */
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

            // remove single quotes: "'a'" -> "a" (or an escape string like "\n")
            value = value.substr(1, value.size() - 2);
        }

        char c;

        if (value.front() == '\\') // begins with escape marker
        {
            if (value.size() == 2)
            {
                // handle common single-character escapes
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
                // hex escape: expect exactly \xNN (4 chars total)
                if (value.size() != 4)
                    throw std::runtime_error("Invalid hex escape length for '" + name + "'");

                int hexVal = std::stoi(value.substr(2, 2), nullptr, 16); // parse hex pair
                c = static_cast<char>(hexVal);
            }
            else if (isdigit(value[1]) && value[1] >= '0' && value[1] <= '7')
            {
                // octal escape: up to three octal digits
                int len = std::min(3, static_cast<int>(value.size() - 1)); // at most 3 digits
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
            // simple literal: must be exactly length 1
            if (value.size() != 1)
                throw std::runtime_error("Fatal error! char length must be 1 for '" + name + "'");

            c = value[0];
        }

        if (current_memory.m_containsVariable(name))
            throw std::runtime_error("Variable '" + name + "' already exists! Consider using reAssign_char()");

        value_t valtValue = { c, ENUM_TYPE_CHAR };

        current_memory.m_insertVariable(name, valtValue);
    }

    /* -------------------------------------------------------------------------
       new_bool(...)
       Accepts "true" or "false" (or $ref that resolves to those strings).
    ------------------------------------------------------------------------- */
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

    /* ==================== reAssign (modify existing variables) ==================== */
    /* The reAssign_* family follow the same structure as new_*, with these differences:
       - they require the variable to already exist (and throw if it doesn't)
       - they call current_memory.m_reInsertVariable(...) at the end instead of m_insertVariable
    */

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
            value = value.substr(1, value.size() - 2); // strip surrounding quotes
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

    /* ==================== delete_var function ====================
      delete_var() validates a single parameter (variable name), validates return_address,
       checks the variable exists in current_memory, then removes it via m_removeVariable.
    */

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

