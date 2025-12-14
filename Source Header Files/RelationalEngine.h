#ifndef ___RELATIONAL___ENGINE___H___
#define ___RELATIONAL___ENGINE___H___

#include "MemoryModel.h"

/*
Order of member functions (for quick reference):
  private:
    - ___isNum___            : validate numeric literal strings
    - ___floatEquals___      : compare floats with epsilon
  public:
    - isEqual                : numeric equality (with epsilon)
    - isGreater              : numeric greater-than
    - isLess                 : numeric less-than
    - isGreaterEqual         : numeric greater-or-equal
    - isLessEqual            : numeric less-or-equal
    - isNotEqual             : numeric inequality (negation of isEqual)
    - isCharsEqual           : string/char equality (non-numeric)
    - isCharsNotEqual        : string/char inequality (non-numeric)

Rationale:
- Private helpers first (validation + float compare).
- Public API grouped by numeric relational ops then character/string ops.
*/

class RelationalEngine {
private:
    /*
     ___isNum___(const std::string& s)
     1) Parameters:
        - s : const std::string&  -> string to test if it's a numeric literal.
     2) Description:
        - Returns true when `s` represents a numeric literal accepted by the VM:
          optional leading '+' or '-', digits, and at most one '.' (decimal point).
     3) Visualized example:
            | "123"  | -> true
            | "-3.14"| -> true
            | "a12"  | -> false
            | "1.2.3"| -> false
     4) Per-line behavior:
        - check empty, track '.' count, allow sign only at position 0, ensure digits elsewhere.
     */
    static bool ___isNum___(const std::string& s) {
        if (s.empty()) return false;                 // empty string -> not a number

        int dotCount = 0;                            // count decimal points seen
        for (size_t i = 0; i < s.size(); ++i) {      // iterate characters in string
            char c = s[i];                           // current character

            if (c == '.') {                          // if char is decimal point
                dotCount++;                          // increment '.' counter
                if (dotCount > 1) return false;     // more than one '.' -> invalid
            }
            else if (c == '-' || c == '+') {        // if char is '-' or '+'
                if (i != 0) return false;           // sign only allowed at first position
            }
            else if (!std::isdigit(c)) return false; // any non-digit (besides '.' or sign) -> invalid
        }
        // final guards: reject lone '+' or '-' or '.'
        return !(s == "+" || s == "-" || s == ".");
    }

    /*
     ___floatEquals___(const long double& a, const long double& b, long double eps = 1e-12L)
     1) Parameters:
        - a : const long double&  -> first floating value
        - b : const long double&  -> second floating value
        - eps : long double       -> tolerance (default 1e-12)
     2) Description:
        - Returns true if the absolute difference between a and b is smaller than eps.
        - Used to consider floating-point rounding tolerance when comparing equality.
     3) Visualized example:
            | a = 1.000000000001 | b = 1.000000000002 |
            |-------------------|--------------------|
            |   |--difference--| < eps  -> equal     |
     4) Per-line:
        - compute absolute difference and compare with eps.
     */
    static bool ___floatEquals___(const long double& a, const long double& b, long double eps = 1e-12L) {
        return std::abs(a - b) < eps; // true if difference is within epsilon tolerance
    }

public:
    /*
     isEqual(parameters, return_address, current_memory)
     1) Parameters:
        - parameters : const std::vector<str>&  -> [ left, right ] operands (literals or "$var")
        - return_address : const str&           -> variable name in current_memory to store boolean result
        - current_memory : SectionMemory&       -> runtime memory to read/dereference and to store result
     2) Description:
        - Compares two numeric operands for equality under a floating-point epsilon.
        - If operands are variable references (start with '$'), they are fetched from current_memory.
        - Requires both operands to be numeric (use isCharsEqual for strings).
     3) ASCII visualization:
            | left_value |   | right_value |
            |-----------| = |------------|
            |  1.23     | ==|   1.2300001 |
            |----------------result bool----------------|
            | return_address -> true / false            |
     4) Per-line comments: explained inline below.
    */
    static void isEqual(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        // Ensure the specified return address exists in the section memory; otherwise can't store result.
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        // Validate that exactly two operands were supplied.
        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        // Copy parameter strings to local names for easier handling.
        str left = parameters[0];
        str right = parameters[1];

        // If left starts with '$', it references a variable; resolve to its stringified value.
        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);

        // If right starts with '$', resolve it similarly.
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        // Both operands must be numeric. If not numeric, instruct user to call the char/string comparator.
        if (!___isNum___(left) || !___isNum___(right))
            throw std::runtime_error("Use isCharsEqual()");

        // Convert to long double and compare with epsilon tolerance.
        bool result = ___floatEquals___(std::stold(left), std::stold(right));

        // Fetch the return address variable (to check its declared type).
        auto ra = current_memory.m_getVariable(return_address);

        // Ensure return address has boolean type; otherwise the caller provided incompatible return storage.
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isEqual() requires boolean return address!");

        // Store boolean result into the return address (reinsert variable with boolean value).
        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }

    /*
     isGreater(parameters, return_address, current_memory)
     1) Parameters:
        - parameters : [ left, right ] (literals or "$var")
        - return_address : variable to store boolean
        - current_memory : SectionMemory reference
     2) Description:
        - Evaluates (left > right) numerically. Both operands must be numeric.
        - Resolves "$" references before comparison.
     3) Visualization:
            | left_value | > | right_value |
            |-----------|   |------------|
            |   5.0     | > |    3.2     | -> true
            | return_address -> true/false |
     4) Per-line behavior annotated inline.
    */
    static void isGreater(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        // Ensure return address exists
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        // Must have exactly two parameters
        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        // Read operands
        str left = parameters[0];
        str right = parameters[1];

        // Resolve references if operands are variable references
        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        // Validate numeric inputs
        if (!___isNum___(left) || !___isNum___(right))
            throw std::invalid_argument("All the values must be valid numbers!");

        // Perform numeric greater-than comparison
        bool result = (std::stold(left) > std::stold(right));

        // Validate return address type is boolean
        auto ra = current_memory.m_getVariable(return_address);
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isGreater() requires boolean return address!");

        // Store boolean result
        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }

    /*
     isLess(parameters, return_address, current_memory)
     1) Parameters:
        - parameters : [ left, right ]
        - return_address : variable to store boolean
        - current_memory : runtime memory
     2) Description:
        - Evaluates numeric (left < right). Resolves "$" refs and validates numeric.
     3) Visualization:
            | left | < | right |
            e.g. 2 < 3 -> true
     4) Per-line: inline comments.
    */
    static void isLess(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        // Check return storage exists
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        // Validate parameter count
        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        // Local copies for operands
        str left = parameters[0];
        str right = parameters[1];

        // Resolve variable references
        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        // Ensure both are numeric
        if (!___isNum___(left) || !___isNum___(right))
            throw std::invalid_argument("All the values must be valid numbers!");

        // Compare numerically
        bool result = (std::stold(left) < std::stold(right));

        // Validate return type is bool
        auto ra = current_memory.m_getVariable(return_address);
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isLess() requires boolean return address!");

        // Write back result
        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }

    /*
     isGreaterEqual(parameters, return_address, current_memory)
     1) Parameters: same shape as other binary numeric ops
     2) Description:
        - Evaluates (left >= right) numerically.
     3) Visualization:
            left >= right -> boolean at return_address
     4) Per-line: inline comments.
    */
    static void isGreaterEqual(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        // Check return address presence
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        // Param count check
        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        // Operand setup
        str left = parameters[0];
        str right = parameters[1];

        // Resolve variable references
        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        // Numeric validation
        if (!___isNum___(left) || !___isNum___(right))
            throw std::invalid_argument("All the values must be valid numbers!");

        // Compute >=
        bool result = (std::stold(left) >= std::stold(right));

        // Validate return type
        auto ra = current_memory.m_getVariable(return_address);
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isGreaterEqual() requires boolean return address!");

        // Store result
        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }

    /*
     isLessEqual(parameters, return_address, current_memory)
     1) Parameters: [ left, right ], return storage, current memory
     2) Description:
        - Evaluates (left <= right) numerically.
     3) Visualization:
            | left | <= | right |
            Save boolean result at return_address
     4) Per-line: inline comments
    */
    static void isLessEqual(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        // Ensure return address exists
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        // Ensure two parameters are provided
        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        // Extract operands
        str left = parameters[0];
        str right = parameters[1];

        // Dereference if variable refs
        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        // Validate numericness
        if (!___isNum___(left) || !___isNum___(right))
            throw std::invalid_argument("All the values must be valid numbers!");

        // Compute <= result
        bool result = (std::stold(left) <= std::stold(right));

        // Validate return store is boolean
        auto ra = current_memory.m_getVariable(return_address);
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isLessEqual() requires boolean return address!");

        // Write result back to memory
        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }

    /*
     isNotEqual(parameters, return_address, current_memory)
     1) Parameters:
        - parameters: [ left, right ]
        - return_address: storage variable (bool)
        - current_memory: SectionMemory
     2) Description:
        - Numeric inequality: returns !(left == right). Uses float-epsilon comparison.
        - Non-numeric inputs should use isCharsNotEqual().
     3) ASCII visualization:
            left != right  <==>  !(left == right)
            | 1.0 | != | 1.0000000000001 | -> maybe false if within eps
     4) Per-line: inline comments.
    */
    static void isNotEqual(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        // ensure return address exists
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        // ensure exactly two operands
        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        // copy operands locally
        str left = parameters[0];
        str right = parameters[1];

        // resolve $-references
        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        // numeric-only check; instruct to use char comparator when not numeric
        if (!___isNum___(left) || !___isNum___(right))
            throw std::invalid_argument("Use isCharsNotEqual");

        // inequality result by negating float-equals
        bool result = !___floatEquals___(std::stold(left), std::stold(right));

        // ensure return type is boolean
        auto ra = current_memory.m_getVariable(return_address);
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isNotEqual() requires boolean return address!");

        // store boolean result
        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }

    /*
     isCharsEqual(parameters, return_address, current_memory)
     1) Parameters:
        - parameters: [ left, right ] operands (literals or $refs)
        - return_address: storage boolean variable name
        - current_memory: SectionMemory
     2) Description:
        - Performs string/character equality. Rejects numeric-looking inputs and points user to isEqual for numbers.
     3) Visualization:
            | "hello" | == | "hello" |  -> true
            |  left   |    | right   |
            store result in return_address (bool)
     4) Per-line: inline comments provided.
    */
    static void isCharsEqual(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        // ensure return address exists
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        // ensure exactly 2 parameters
        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        // local operands
        str left = parameters[0];
        str right = parameters[1];

        // resolve variable references if present
        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        // if either operand looks numeric, instruct to use numeric comparator
        if (___isNum___(left) || ___isNum___(right))
            throw std::runtime_error("Use isEqual()");

        // perform direct string comparison
        bool result = (left == right);

        // check return address is boolean
        auto ra = current_memory.m_getVariable(return_address);
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isCharsEqual() requires boolean return address!");

        // store boolean result in memory
        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }

    /*
     isCharsNotEqual(parameters, return_address, current_memory)
     1) Parameters: same as isCharsEqual
     2) Description:
        - Opposite of isCharsEqual: returns (left != right).
        - Rejects numeric-looking inputs and tells user to use isNotEqual for numeric inequality.
     3) Visualization:
            | "a" | != | "b" |  -> true
     4) Per-line: inline comments follow.
    */
    static void isCharsNotEqual(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        // ensure return address exists
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        // ensure exactly two parameters provided
        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        // copy operands locally
        str left = parameters[0];
        str right = parameters[1];

        // resolve references
        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        // if either operand looks numeric, instruct to use numeric comparator
        if (___isNum___(left) || ___isNum___(right))
            throw std::runtime_error("Use isNotEqual()");

        // perform inequality check
        bool result = (left != right);

        // confirm return address is a boolean variable
        auto ra = current_memory.m_getVariable(return_address);
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isCharsNotEqual() requires boolean return address!");

        // write result back to memory
        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }
};

#endif
