#ifndef ___LOOP___ENGINE___H___
#define ___LOOP___ENGINE___H___
#include "MemoryModel.h"

/*
 Order of member functions (class LoopEngine):
  - private:
      1) ___isNum___(const std::string& s)
      2) STORE_LOOP_OPTION(const str& KEY, const value_t& VAL, SectionMemory& current_section)
      3) makeIntValue(int64_t v)
  - public:
      4) loop(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_section)

 Rationale:
  - Private helpers for validation and small utilities are grouped first.
  - Public API exposes loop(...) which uses the helpers above.
*/

class LoopEngine {

    /*
     ___isNum___(const std::string& s)
     1) Parameters:
        - s : const std::string&  -> input string to test for numeric literal.
     2) Description:
        - Returns true if s is a valid numeric literal for the VM:
          optional leading +/-, digits, and at most one '.'.
     3) ASCII visualization:
            | "123"   | -> true
            | "-10"   | -> true
            | "3.14"  | -> true
            | "a1"    | -> false
            | "1.2.3" | -> false
     4) Per-line explanation:
        - empty check, '.' counting, sign-position check, digit verification, final special-case rejection.
    */
    static bool ___isNum___(const std::string& s)
    {
        if (s.empty()) return false;               // if input empty -> not a number

        int dotCount = 0;                          // counter for decimal points
        for (size_t i = 0; i < s.size(); ++i)      // iterate over all characters
        {
            char c = s[i];                         // current character

            if (c == '.') {                        // if character is a dot
                dotCount++;                        // increment dot count
                if (dotCount > 1) return false;   // more than one dot -> invalid
            }
            else if (c == '-' || c == '+') {      // if character is sign
                if (i != 0) return false;         // sign only allowed at position 0
            }
            else if (!std::isdigit(c)) return false; // non-digit (and not . or sign) -> invalid
        }
        if (s == "+" || s == "-" || s == ".") return false; // lone sign/dot -> invalid

        return true;                              // all checks passed -> valid numeric literal
    }

    /*
     STORE_LOOP_OPTION(KEY, VAL, current_section)
     1) Parameters:
        - KEY : const str&         -> name under which the loop metadata will be stored
        - VAL : const value_t&     -> typed value to insert
        - current_section : SectionMemory& -> memory section to insert into
     2) Description:
        - Helper to store loop control variables (start/stop/step) in current_section.
        - Throws if KEY already exists in current_section.
     3) ASCII visualization:
            current_section memory (before):
            |-------------------------------|
            | (no ___LOOP___ENGINE___...)   |
            |-------------------------------|
            After STORE_LOOP_OPTION("...start...", VAL):
            |-------------------------------|
            | "___LOOP...start..." -> VAL   |
            |-------------------------------|
     4) Per-line explanation:
        - Check for key existence, throw if present, otherwise insert variable into SectionMemory.
    */
    static void STORE_LOOP_OPTION(const str& KEY, const value_t& VAL, SectionMemory& current_section)
    {
        if (current_section.m_containsVariable(KEY))
            throw std::runtime_error("Error! Loop option already exists: " + KEY); // prevent overwrite
        current_section.m_insertVariable(KEY, VAL); // insert the typed value into the section memory
    }


    /*
     makeIntValue(v)
     1) Parameters:
        - v : int64_t  -> integer value to convert into an appropriately-typed value_t
     2) Description:
        - Produces a value_t that chooses the smallest signed integer type that can hold v.
          Order checked: i8 -> i16 -> i32 -> i64.
     3) ASCII visualization:
            v = 42  -> { int8_t(42), ENUM_TYPE_I8 }
            v = 70000 -> { int32_t(70000), ENUM_TYPE_I32 }
     4) Per-line explanation:
        - Compare v against type limits, return the first matching smaller typed value_t.
    */
    static value_t makeIntValue(int64_t v) {
        // if fits in int8_t, return as ENUM_TYPE_I8
        if (v >= std::numeric_limits<int8_t>::min() && v <= std::numeric_limits<int8_t>::max())
            return { static_cast<int8_t>(v), ENUM_TYPE_I8 };
        // else if fits in int16_t, return as ENUM_TYPE_I16
        if (v >= std::numeric_limits<int16_t>::min() && v <= std::numeric_limits<int16_t>::max())
            return { static_cast<int16_t>(v), ENUM_TYPE_I16 };
        // else if fits in int32_t, return as ENUM_TYPE_I32
        if (v >= std::numeric_limits<int32_t>::min() && v <= std::numeric_limits<int32_t>::max())
            return { static_cast<int32_t>(v), ENUM_TYPE_I32 };
        // otherwise return as int64_t (ENUM_TYPE_I64)
        return { static_cast<int64_t>(v), ENUM_TYPE_I64 };
    }

public:

    /*
     loop(parameters, return_address, current_section)
     1) Parameters:
        - parameters : const std::vector<str>&  -> [ start, stop, step ] each as literal or "$var"
        - return_address : const str&           -> the loop identifier (used as suffix when storing options)
        - current_section : SectionMemory&      -> section memory where loop variables will be stored
     2) Description:
        - Validates the three loop parameters (start, stop, step), resolves any "$" references,
          converts to int64_t, enforces non-zero step and direction compatibility, and stores three
          specially-named variables in current_section for the VM's loop machinery to consume:
            ___LOOP___ENGINE___{return_address}___start___
            ___LOOP___ENGINE___{return_address}___stop___
            ___LOOP___ENGINE___{return_address}___step___
        - These stored values are typed via makeIntValue so they occupy the smallest integer type that fits.
     3) ASCII visualization (high-level):
            Input: loop( ["1", "5", "1"], "i", current_section )
            Behavior:
              validate strings -> parse to ints -> store:
                "___LOOP___ENGINE___i___start___" -> 1
                "___LOOP___ENGINE___i___stop___"  -> 5
                "___LOOP___ENGINE___i___step___"  -> 1
            The VM later reads these keys to iterate the loop body.
            Visual box:
              +------------------------------------------+
              | current_section memory                   |
              |  "___LOOP...i...start..." -> 1           |
              |  "___LOOP...i...stop..."  -> 5           |
              |  "___LOOP...i...step..."  -> 1           |
              +------------------------------------------+
     4) Per-line explanation:
        - Validate parameter count and return address presence.
        - Resolve '$' references for each parameter if present.
        - Validate numeric form using ___isNum___.
        - Convert to int64_t with safe catch for overflow.
        - Validate step != 0 and that direction of step matches start/stop relation.
        - Store each control value in current_section via STORE_LOOP_OPTION and makeIntValue.
    */
    static void loop(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_section) {
        // require exactly 3 parameters (start, stop, step)
        if (parameters.size() != 3)
            throw std::runtime_error("Error! loop() requires exactly 3 parameters!");

        // ensure the return_address (loop identifier) exists in the current section memory,
        // because VM expects a valid "return address" variable for the loop construct
        if (!current_section.m_containsVariable(return_address))
            throw std::runtime_error("Error! return_address not found!");

        // copy textual parameters locally
        str s_start = parameters[0];
        str s_stop = parameters[1];
        str s_step = parameters[2];

        // if parameter starts with '$', treat as variable reference and fetch its stringified value
        if (!s_start.empty() && s_start.front() == '$')
            s_start = current_section.m_fetchStringalizedValue(s_start);
        if (!s_stop.empty() && s_stop.front() == '$')
            s_stop = current_section.m_fetchStringalizedValue(s_stop);
        if (!s_step.empty() && s_step.front() == '$')
            s_step = current_section.m_fetchStringalizedValue(s_step);

        // validate that each resolved string is a valid numeric literal
        if (!___isNum___(s_start)) throw std::runtime_error("Error! invalid number: start");
        if (!___isNum___(s_stop))  throw std::runtime_error("Error! invalid number: stop");
        if (!___isNum___(s_step))  throw std::runtime_error("Error! invalid number: step");

        // convert strings to int64_t with try/catch to handle overflow or bad conversion
        int64_t start, stop, step;
        try {
            start = std::stoll(s_start);
            stop = std::stoll(s_stop);
            step = std::stoll(s_step);
        }
        catch (...) {
            // any exception while converting => number too large or otherwise invalid for int64_t
            throw std::runtime_error("Error! number literal too big for int64_t.");
        }

        // step must not be zero (would create infinite loop or undefined iteration)
        if (step == 0)
            throw std::runtime_error("Error! step must not be zero.");

        // if step > 0 then start must be <= stop
        if (step > 0 && start > stop)
            throw std::runtime_error("Error! step > 0 but start > stop.");

        // if step < 0 then start must be >= stop
        if (step < 0 && start < stop)
            throw std::runtime_error("Error! step < 0 but start < stop.");

        // store loop control variables into current_section using a deterministic key naming scheme
        // the values are converted to the smallest integer type that can represent them
        STORE_LOOP_OPTION(str("___LOOP___ENGINE___") + return_address + str("___start___"), makeIntValue(start), current_section);
        STORE_LOOP_OPTION(str("___LOOP___ENGINE___") + return_address + str("___stop___"), makeIntValue(stop), current_section);
        STORE_LOOP_OPTION(str("___LOOP___ENGINE___") + return_address + str("___step___"), makeIntValue(step), current_section);
    }
};
#endif
