#ifndef ___MEMORY___MODEL___H___
# define ___MEMORY___MODEL___H___

#include "includes.h"

/*
Order of top-level declarations in this file (quick reference):

1) typedef std::string str;
2) enum typeEnum                  - numeric type tags used across the VM
3) typedef value_t                - variant + typeEnum pair representing a runtime value
4) makeValueToString(...)         - convert value_t -> human-readable string
5) #define UnorderedMap           - shorthand for robin_hood::unordered_map
6) printValue(...)                - print a value_t to std::cout
7) class SectionMemory { ... }    - runtime per-section variable storage
   - private buffers for each primitive type + varLocations map
   - public methods:
       m_insertVariable
       m_removeVariable
       m_reInsertVariable
       m_getVariable
       m_passVariabe
       m_containsVariable
       m_monitorVariables
       m_fetchStringalizedValue

Rationale:
- Free functions operate on value_t and are utility helpers for printing/serializing.
- SectionMemory encapsulates the per-section runtime "heap"/"stack" (variables).
*/

typedef std::string str; // Simple alias for readability: str == std::string

// typeEnum:
// Parameters: (none) — this is an enum type used to tag union contents.
// Description:
//   - A unique enumerator for each supported data type within the VM's runtime.
//   - These constants are stored alongside variant values in value_t so the VM knows how to interpret them.
// Visualization:
//    +----------------------+
//    | ENUM_TYPE_I8 : 0x1A3F |
//    | ENUM_TYPE_I16: 0x2B7C |
//    | ...                  |
//    +----------------------+
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

// value_t:
// Parameters:
//   - std::variant<...> first: holds the actual value (one of the types enumerated).
//   - typeEnum second: tag identifying which variant alternative is active.
// Description:
//   - A small tagged-value pair used throughout the VM to represent typed runtime values.
// Visualization:
//   +--------------------------------------+
//   | value_t = { variant, ENUM_TYPE_I32 } |
//   +--------------------------------------+
typedef std::pair<std::variant<int8_t, int16_t, int32_t, int64_t,
    float, double, long double, bool, char, str>, typeEnum> value_t;


/*
 makeValueToString(const value_t& value)
 Parameters:
   - value: (const value_t&) the runtime value to be converted to a string.
 Description:
   - Converts any supported value_t into a standardized string representation.
   - Floats are formatted with fixed notation and 6 decimal places.
   - Integers, bools, chars, and strings are converted sensibly.
 Visualization:
    input: value_t({ 42, ENUM_TYPE_I32 })  --> output: "42"
    input: value_t({ 3.14159, ENUM_TYPE_F64 }) --> output: "3.141590"
 Per-line notes:
   - writeFloat lambda centralizes floating-point to-string formatting.
   - switch on value.second selects how to extract variant and format it.
*/
str makeValueToString(const value_t& value) {
    str result; // accumulator for the produced string

    // writeFloat: helper lambda for formatting floating-point types to strings
    auto writeFloat = [](auto v) {
        std::ostringstream oss;               // create an ostringstream to format the float
        oss << std::fixed << std::setprecision(6) << v; // use fixed notation with 6 decimals
        return oss.str();                     // return formatted string
        };

    // Decide what to do based on the stored type tag
    switch (value.second) {
    case ENUM_TYPE_I8:
        // extract int8_t and convert to decimal string
        result = std::to_string(std::get<int8_t>(value.first));
        break;
    case ENUM_TYPE_I16:
        // extract int16_t and convert
        result = std::to_string(std::get<int16_t>(value.first));
        break;
    case ENUM_TYPE_I32:
        // extract int32_t and convert
        result = std::to_string(std::get<int32_t>(value.first));
        break;
    case ENUM_TYPE_I64:
        // extract int64_t and convert
        result = std::to_string(std::get<int64_t>(value.first));
        break;
    case ENUM_TYPE_F32:
        // extract float and format with writeFloat
        result = writeFloat(std::get<float>(value.first));
        break;
    case ENUM_TYPE_F64:
        // extract double and format
        result = writeFloat(std::get<double>(value.first));
        break;
    case ENUM_TYPE_FMAX:
        // extract long double and format
        result = writeFloat(std::get<long double>(value.first));
        break;
    case ENUM_TYPE_CHAR:
        // extract char and make a 1-char std::string
        result = std::string(1, std::get<char>(value.first));
        break;
    case ENUM_TYPE_BOOL:
        // extract bool and convert to "true"/"false"
        std::get<bool>(value.first) ? result = "true" : result = "false";
        break;
    case ENUM_TYPE_STRING:
        // extract std::string directly
        result = std::get<str>(value.first);
        break;
    default:
        // unknown tag -> sentinel string
        result = "<unknown>";
        break;
    }

    return result; // return produced string
}

// Macro alias: make robin_hood::unordered_map easier to type in this file
#define UnorderedMap robin_hood::unordered_map

/*
 printValue(const value_t& value)
 Parameters:
   - value: const value_t& -- the typed value to print to stdout.
 Description:
   - Prints the contained value to std::cout using appropriate formatting.
   - Floating point numbers adhere to fixed format with 6 decimals (same rule as makeValueToString).
 Visualization:
     input: value_t({ "hello", ENUM_TYPE_STRING }) -> prints: hello
 Per-line notes:
   - Set the stream formatting, then switch on the tag and print the matching alternative.
*/
void printValue(const value_t& value) {
    std::cout << std::fixed << std::setprecision(6); // set fixed, 6-precision for float outputs

    switch (value.second) {
    case ENUM_TYPE_I8:
        // unary + makes int8_t print as number rather than character
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
        // print a single char
        std::cout << std::get<char>(value.first);
        break;
    case ENUM_TYPE_BOOL:
        // prints "0" or "1" by default; if you want "true"/"false" use logic similar to makeValueToString
        std::cout << std::get<bool>(value.first);
        break;
    case ENUM_TYPE_STRING:
        std::cout << std::get<str>(value.first);
        break;
    }
}

/*
 SectionMemory
 Description:
   - Encapsulates per-section variable storage. Each supported primitive type has its own
     robin_hood::unordered_map mapping variable name -> value for that type.
   - varLocations maps variable name -> typeEnum so we can quickly locate which buffer contains a variable.
 Visualization:
     SectionMemory:
     +-----------------------------+
     | Buffer_I32: { "x": 42 }     |
     | Buffer_F64: { "pi": 3.14 }  |
     | varLocations:               |
     |   { "x": ENUM_TYPE_I32,     |
     |  "pi": ENUM_TYPE_F64 }      |
     +-----------------------------+
 Order of member functions (within class):
   1. m_insertVariable
   2. m_removeVariable
   3. m_reInsertVariable
   4. m_getVariable
   5. m_passVariabe
   6. m_containsVariable
   7. m_monitorVariables
   8. m_fetchStringalizedValue
*/
class SectionMemory {
private:
    // Storage buffers: one unordered_map per VM-supported type.
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

    // varLocations: single map telling us which buffer to consult for a given variable name
    UnorderedMap<str, typeEnum> varLocations;

public:
    /*
     m_insertVariable(const std::string& varName, const value_t& value)
     Parameters:
       - varName: const std::string& name of variable to insert
       - value: const value_t& typed value to store
     Description:
       - Inserts the variable into the appropriate buffer based on value.second (typeEnum).
       - Throws if varName already exists or if typeEnum is unknown.
     Visualization:
         Call: m_insertVariable("x", {42, ENUM_TYPE_I32})
         Result:
           Buffer_I32: { "x" : 42 }
           varLocations: { "x" : ENUM_TYPE_I32 }
     Per-line notes:
       - test for duplicate variable using varLocations.contains(varName)
       - extract variant (v) and tag (t) for a shorter local name
       - switch on t to place the underlying variant into the correct Buffer_*
       - finally, record in varLocations
    */
    void m_insertVariable(const std::string& varName, const value_t& value) {
        // If the variable already exists, insertion is not allowed.
        if (varLocations.contains(varName))
            throw std::runtime_error("Variable already exists; use reInsertVariable().");

        const auto& v = value.first; // reference to the variant union part
        const auto t = value.second; // the type tag

        // Select the correct underlying buffer by type and insert the value
        switch (t) {
        case ENUM_TYPE_I8:
            Buffer_I8.emplace(varName, std::get<int8_t>(v)); // insert int8_t
            break;
        case ENUM_TYPE_I16:
            Buffer_I16.emplace(varName, std::get<int16_t>(v)); // insert int16_t
            break;
        case ENUM_TYPE_I32:
            Buffer_I32.emplace(varName, std::get<int32_t>(v)); // insert int32_t
            break;
        case ENUM_TYPE_I64:
            Buffer_I64.emplace(varName, std::get<int64_t>(v)); // insert int64_t
            break;
        case ENUM_TYPE_F32:
            Buffer_F32.emplace(varName, std::get<float>(v)); // insert float
            break;
        case ENUM_TYPE_F64:
            Buffer_F64.emplace(varName, std::get<double>(v)); // insert double
            break;
        case ENUM_TYPE_FMAX:
            Buffer_Fmax.emplace(varName, std::get<long double>(v)); // insert long double
            break;
        case ENUM_TYPE_BOOL:
            Buffer_Bool.emplace(varName, std::get<bool>(v)); // insert bool
            break;
        case ENUM_TYPE_CHAR:
            Buffer_Char.emplace(varName, std::get<char>(v)); // insert char
            break;
        case ENUM_TYPE_STRING:
            Buffer_String.emplace(varName, std::get<str>(v)); // insert string
            break;
        default:
            // unknown type tag -> error
            throw std::runtime_error("Unknown typeEnum in insert().");
        }

        // Record the variable's type in the varLocations map for fast lookup later
        varLocations.emplace(varName, t);
    }

    /*
     m_removeVariable(const str& varName)
     Parameters:
       - varName: const str& name of variable to remove
     Description:
       - Removes a variable from its buffer and erases its entry from varLocations.
       - Throws if the variable does not exist or if the stored type is unknown.
     Visualization:
         Before: Buffer_I32: { "x": 1 } varLocations: { "x": ENUM_TYPE_I32 }
         Call: m_removeVariable("x")
         After: Buffer_I32: {} varLocations: {}
     Per-line notes:
       - Check varLocations for existence, get its type, switch on type and erase from appropriate buffer,
         then remove the mapping from varLocations.
    */
    void m_removeVariable(const str& varName) {
        // ensure variable exists
        if (!varLocations.contains(varName))
            throw std::runtime_error("Variable does not exist.");

        // get the associated type tag
        typeEnum t = varLocations.at(varName);

        // remove from the corresponding buffer based on type
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
            // unknown tag -> error
            throw std::runtime_error("Unknown typeEnum in removeVariable().");
        }

        // finally remove the blueprint entry
        varLocations.erase(varName);
    }

    /*
     m_reInsertVariable(const str& varName, const value_t& value)
     Parameters:
       - varName: const str& – the name of the existing variable to replace
       - value: const value_t& – new typed value to assign
     Description:
       - Replace the existing variable's value and type. It requires that the variable already exists.
       - Internally removes the old value and inserts the new value and type.
     Visualization:
        var "x" exists as ENUM_TYPE_I32. After reInsert with ENUM_TYPE_F64, "x" moves into Buffer_F64.
     Per-line notes:
       - Ensure var exists, then remove it, then insert new value into appropriate buffer and update varLocations.
    */
    void m_reInsertVariable(const str& varName, const value_t& value) {
        // variable must exist to be re-inserted
        if (!varLocations.contains(varName))
            throw std::runtime_error("Variable doesn't exisit at all. Use insertVariable()");

        const auto& v = value.first; // variant payload
        const auto t = value.second; // new type tag

        // remove old value first (from previous buffer)
        m_removeVariable(varName);

        // insert new value in appropriate buffer
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

        // record the updated type
        varLocations.emplace(varName, t);
    }

    /*
     m_getVariable(const str& varName)
     Parameters:
       - varName: const str& name of the variable to fetch
     Returns:
       - value_t containing the variable value and its type.
     Description:
       - Looks up the variable's type via varLocations and then retrieves the value
         from the correct buffer, packaging both into a value_t and returning it.
       - Throws if variable doesn't exist or if the tag is unknown.
     Visualization:
       Before: Buffer_I32: { "x": 7 }, varLocations: { "x": ENUM_TYPE_I32 }
       Call: m_getVariable("x") -> returns value_t({7, ENUM_TYPE_I32})
     Per-line notes:
       - Validate presence, determine type, and return a value_t created from the correct buffer.
    */
    value_t m_getVariable(const str& varName) {
        // ensure existence of the variable
        if (!varLocations.contains(varName))
            throw std::runtime_error("Variable does not exist.");

        // fetch the recorded type
        typeEnum t = varLocations.at(varName);

        // switch to read the proper buffer and wrap it into value_t for return
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
            // unknown tag -> error
            throw std::runtime_error("Unknown typeEnum in getVariable().");
        }
    }

    /*
     m_passVariabe(const str& varName, SectionMemory& to)
     Parameters:
       - varName: const str& name of the variable to move
       - to: SectionMemory& the target SectionMemory to receive the variable
     Description:
       - Moves a variable from this SectionMemory into another SectionMemory.
       - Performs checks: the target must not already have the variable, the source must have it.
       - After insertion into the target, removes the variable from the source.
     Visualization:
       Source: has x -> move to target; source loses x; target gains x.
     Per-line notes:
       - Validate non-existence in receiver, existence in this object, call to.m_insertVariable, then remove locally.
    */
    void m_passVariabe(const str& varName, SectionMemory& to) {
        // target must not already contain the variable
        if (to.varLocations.contains(varName))
            throw std::runtime_error("Error! Variable already exists at the reciver.");
        // source must contain the variable
        if (!varLocations.contains(varName))
            throw std::runtime_error("Variable doesn't exists.");

        // insert into target using current variable's value, then remove from current
        to.m_insertVariable(varName, m_getVariable(varName));
        m_removeVariable(varName);
    }

    /*
     m_containsVariable(const str& varName)
     Parameters:
       - varName: const str& name to check
     Returns:
       - bool true if varLocations contains varName
     Description:
       - Simple helper that tells whether this SectionMemory currently contains a variable by name.
     Per-line notes:
       - Delegates to varLocations.contains which is O(1) average for robin_hood::unordered_map.
    */
    bool m_containsVariable(const str& varName) {
        return varLocations.contains(varName);
    }

    /*
     m_monitorVariables()
     Parameters:
       - none
     Description:
       - Pretty-prints the contents of each non-empty buffer and the varLocations blueprint to std::cout.
       - Useful for debugging and interactive inspection of a SectionMemory state.
     Visualization:
        |===============================================|
        BUFFER_I32:
        [0] x: 42
        BUFFER_F64:
        [0] pi: 3.141590
        VAR_LOCATIONS_BLUEPRINT:
        [0] x: ENUM_TYPE_I32
        [1] pi: ENUM_TYPE_F64
        |===============================================|
     Per-line notes:
       - prints headers, iterates each buffer only if not empty, prints elements with an index
       - formatFloat lambda is used to ensure floating types display consistently
    */
    void m_monitorVariables() {
        std::cout << "|===============================================|\n";

        // formatFloat lambda mirrors formatting used elsewhere (6 decimal fixed)
        auto formatFloat = [](auto val) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(6) << val;
            return oss.str();
            };

        // For each buffer, if not empty print header + each entry with index and value
        if (!Buffer_I8.empty()) {
            std::cout << "BUFFER_I8: \n";
            int idx = 0;
            for (auto& item : Buffer_I8) {
                // '+' to promote int8_t to integer display rather than char
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
                // use formatFloat to produce consistent fixed decimals
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
                // print char wrapped in single quotes
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

        // Print the varLocations blueprint which shows every variable and its type
        if (!varLocations.empty()) {
            std::cout << "\nVAR_LOCATIONS_BLUEPRINT: \n";
            int idx = 0;
            for (auto& item : varLocations) {
                std::cout << '[' << idx << "] " << item.first << ": ";
                // translate enum to human-readable label
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
                    // invalid enum -> throw, since varLocations must contain valid enums
                    throw std::runtime_error("Invalid ENUM_TYPE_* for typeEnum!");
                    break;
                }
                idx++;
            }
        }

        std::cout << "\n|===============================================|\n\n";
    }

    /*
     m_fetchStringalizedValue(str& var_name)
     Parameters:
       - var_name: str& A string starting with '$' to indicate fetch-of-variable by name.
         The function will remove the leading '$' and lookup the variable.
     Returns:
       - std::string stringified representation of the variable's value (using makeValueToString).
     Description:
       - Helper used by the VM when a branch condition or value is stored as "$varName".
       - Strips leading '$', checks presence, and returns the textual representation of the variable's value.
     Visualization:
       Input: "$x" -> strips to "x" -> returns makeValueToString(m_getVariable("x"))
     Per-line notes:
       - Remove first char (the '$'), validate the variable exists, call makeValueToString on m_getVariable(var_name)
    */
    str m_fetchStringalizedValue(str& var_name) {
        // remove the leading '$' (assumes caller provided a string beginning with '$')
        var_name = var_name.substr(1);

        // verify the variable exists in this SectionMemory
        if (!m_containsVariable(var_name)) {
            std::cerr <<
                "\n[ERROR_ENGINE]  The var_name that is causing exception is: ("
                << var_name << ")\n";

            throw std::runtime_error("Can't find the variable!");
        }

        // return stringized value using helper above
        return makeValueToString(m_getVariable(var_name));
    }
};

# endif
