#ifndef ___ARITHEMETIC___ENGINE___H____
#define ___ARITHEMETIC___ENGINE___H____

#include "MemoryModel.h"

/*
Order of member functions (quick reference)
  - private:
      ___isNum___        : validate numeric literal string
  - public:
      isNum              : store boolean indicating if a parameter string is numeric
      sum                : add multiple numeric parameters
      product            : multiply multiple numeric parameters
      subtract           : subtract two parameters (val1 - val2)
      divide             : divide two parameters (val1 / val2)
      mod                : modulus (val1 % val2) using integer casting
      floor              : floor(value)
      ceiling            : ceil(value)
      abs                : absolute value
      pow                : power (base ^ exponent)
Rationale:
  - Private helpers at the top.
  - Public arithmetic operations follow a consistent pattern:
      validate return_address -> resolve parameters ($refs) -> validate numeric -> compute -> store to return_address with type-checks.
  - No behavior changed; comments only.
*/

class ArithmeticEngine
{
private:
    /*
     ___isNum___(const std::string& s)
     1) Parameters:
        - s : const std::string&  — string to test for numeric literal
     2) Description:
        - Returns true if s represents a numeric literal supported by the VM:
          optional leading '+' or '-', digits, and at most one '.'.
     3) ASCII visualization:
            | "123"   | -> true
            | "-3.14" | -> true
            | "a12"   | -> false
            | "1.2.3" | -> false
     4) Per-line explanation:
        - Reject empty
        - Count '.' occurrences
        - Allow sign only at position 0
        - Ensure all other characters are digits
        - Reject lone sign or dot
    */
    static bool ___isNum___(const std::string& s)
    {
        if (s.empty()) return false;               // empty string is not numeric

        int dotCount = 0;                          // track decimal points
        for (size_t i = 0; i < s.size(); ++i)      // inspect each char
        {
            char c = s[i];

            if (c == '.')
            {
                dotCount++;                        // increment decimal count
                if (dotCount > 1) return false;   // more than one dot => invalid
            }
            else if (c == '-' || c == '+')
            {
                if (i != 0) return false;         // sign allowed only as first char
            }
            else if (!std::isdigit(c))
            {
                return false;                     // any non-digit char -> invalid
            }
        }

        if (s == "+" || s == "-" || s == ".")     // standalone sign/dot -> invalid
            return false;

        return true;                              // all checks passed -> numeric
    }

public:

    /*
     isNum(parameters, return_address, current_memory)
     1) Parameters:
        - parameters: const std::vector<str>&  — expects exactly 1 parameter (string to test)
        - return_address: const str&           — variable to store boolean result (must exist)
        - current_memory: SectionMemory&       — used to verify return_address
     2) Description:
        - Tests whether the provided parameter string is a numeric literal using ___isNum___.
        - Stores boolean result at return_address (ENUM_TYPE_BOOL).
     3) ASCII visualization:
            Input: isNum(["123"], "ra", mem)
            Output: mem["ra"] = true  (ENUM_TYPE_BOOL)
     4) Per-line:
        - check parameter count, ensure return address exists, evaluate, reinsert boolean result.
    */
    static void isNum(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 1)
            throw std::runtime_error("isNum() can only take 1 parameter!"); // must have exactly one param

        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Error! can't find the return address!"); // ensure return slot exists

        value_t value = { ___isNum___(parameters[0]), ENUM_TYPE_BOOL }; // evaluate numeric-ness
        current_memory.m_reInsertVariable(return_address, value);       // store boolean result
    }

    /*
     sum(parameters, return_address, current_memory)
     1) Parameters:
        - parameters: list of numeric literals or "$var" references to be added
        - return_address: variable name where result will be stored (must exist)
        - current_memory: SectionMemory& for resolving $refs and storing the result
     2) Description:
        - Accumulates all parameters (converted to long double) into accumulator.
        - After computing, checks the type of return_address and safely converts/stores result
          according to its declared type (checks for overflow where applicable).
     3) ASCII visualization:
            params: ["1", "2.5", "$x"] with $x = "3"
            sum => 6.5
            store into return_address with correct typed cast
            Visual:
            +-----------------------------+
            | accumulator <- 0 + 1 + 2.5 + 3 = 6.5 |
            | write mem[return_address] = 6.5 (typed) |
            +-----------------------------+
     4) Per-line:
        - ensure return exists
        - iterate parameters, resolve $refs, validate numeric, accumulate
        - fetch return var to check its type
        - switch on type: check range, cast, reinsert
    */
    static void sum(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        // ensure the destination return variable exists in memory
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Error! can't find the return address!");

        str current_str = "";               // temporary to hold each parameter's resolved string
        long double accumulator = 0;        // use high-precision accumulator for summing

        // resolve each parameter and add to accumulator
        for (auto item : parameters)
        {
            if (item.front() == '$')                              // if starts with '$', it's a variable reference
                current_str = current_memory.m_fetchStringalizedValue(item); // fetch stringified value
            else
                current_str = item;                               // literal token

            if (!___isNum___(current_str))                        // validate numeric string
                throw std::runtime_error("Bad value for sum(): " + current_str);

            accumulator += std::stold(current_str);              // convert to long double and add
        }

        auto ra = current_memory.m_getVariable(return_address); // get return var descriptor (value_t) to check type

        // store accumulator according to the declared type of return_address, checking bounds
        switch (ra.second)
        {
        case ENUM_TYPE_I8:
        {
            if (accumulator < -128 || accumulator > 127)
                throw std::runtime_error("Return address too small for sum");

            int8_t val = static_cast<int8_t>(accumulator);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I8 });
            break;
        }

        case ENUM_TYPE_I16:
        {
            if (accumulator < -32768 || accumulator > 32767)
                throw std::runtime_error("Return address too small for sum");

            int16_t val = static_cast<int16_t>(accumulator);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I16 });
            break;
        }

        case ENUM_TYPE_I32:
        {
            if (accumulator < INT32_MIN || accumulator > INT32_MAX)
                throw std::runtime_error("Return address too small for sum");

            int32_t val = static_cast<int32_t>(accumulator);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I32 });
            break;
        }

        case ENUM_TYPE_I64:
        {
            int64_t val = static_cast<int64_t>(accumulator);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I64 });
            break;
        }

        case ENUM_TYPE_F32:
        {
            if (accumulator < -FLT_MAX || accumulator > FLT_MAX)
                throw std::runtime_error("Return address too small for sum");

            float val = static_cast<float>(accumulator);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F32 });
            break;
        }

        case ENUM_TYPE_F64:
        {
            if (accumulator < -DBL_MAX || accumulator > DBL_MAX)
                throw std::runtime_error("Return address too small for sum");

            double val = static_cast<double>(accumulator);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F64 });
            break;
        }

        case ENUM_TYPE_FMAX:
        {
            // highest precision: reinsert as long double
            current_memory.m_reInsertVariable(return_address, { accumulator, ENUM_TYPE_FMAX });
            break;
        }

        default:
            throw std::runtime_error("Invalid type for sum(): only numeric types allowed");
        }
    }

    /*
     product(parameters, return_address, current_memory)
     1) Parameters:
        - parameters: list of numeric tokens or "$var" references
        - return_address: variable name where result will be stored (must exist)
        - current_memory: SectionMemory&
     2) Description:
        - Multiplies the provided parameters together (accumulator *= each).
        - Performs the same type checks and storing logic as sum(), but for multiplication.
     3) ASCII example:
            params: ["2", "3", "$a"] with $a = "4"
            accumulator: 1 -> 2 -> 6 -> 24
            store 24 in return_address with type-cast
     4) Per-line: similar to sum(), resolve $refs, validate numeric, multiply, then type-switch and reinsert.
    */
    static void product(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Error! can't find the return address!");

        str current_str = "";
        long double accumulator = 1.0L; // start with multiplicative identity

        // Multiply resolved parameter values into accumulator
        for (auto item : parameters)
        {
            if (item.front() == '$')
                current_str = current_memory.m_fetchStringalizedValue(item);
            else
                current_str = item;

            if (!___isNum___(current_str))
                throw std::runtime_error("Bad value for sum(): " + current_str); // message kept consistent with original

            accumulator *= std::stold(current_str);
        }

        auto ra = current_memory.m_getVariable(return_address);

        // Type-specific storage with bounds checks
        switch (ra.second)
        {
        case ENUM_TYPE_I8:
        {
            if (accumulator < -128 || accumulator > 127)
                throw std::runtime_error("Return address too small for product");

            int8_t val = static_cast<int8_t>(accumulator);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I8 });
            break;
        }

        case ENUM_TYPE_I16:
        {
            if (accumulator < -32768 || accumulator > 32767)
                throw std::runtime_error("Return address too small for product");

            int16_t val = static_cast<int16_t>(accumulator);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I16 });
            break;
        }

        case ENUM_TYPE_I32:
        {
            if (accumulator < INT32_MIN || accumulator > INT32_MAX)
                throw std::runtime_error("Return address too small for product");

            int32_t val = static_cast<int32_t>(accumulator);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I32 });
            break;
        }

        case ENUM_TYPE_I64:
        {
            int64_t val = static_cast<int64_t>(accumulator);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I64 });
            break;
        }

        case ENUM_TYPE_F32:
        {
            if (accumulator < -FLT_MAX || accumulator > FLT_MAX)
                throw std::runtime_error("Return address too small for product");

            float val = static_cast<float>(accumulator);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F32 });
            break;
        }

        case ENUM_TYPE_F64:
        {
            if (accumulator < -DBL_MAX || accumulator > DBL_MAX)
                throw std::runtime_error("Return address too small for product");

            double val = static_cast<double>(accumulator);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F64 });
            break;
        }

        case ENUM_TYPE_FMAX:
        {
            current_memory.m_reInsertVariable(return_address, { accumulator, ENUM_TYPE_FMAX });
            break;
        }

        default:
            throw std::runtime_error("Invalid type for product(): only numeric types allowed");
        }
    }


    /*
     subtract(parameters, return_address, current_memory)
     1) Parameters:
        - parameters: [ val1, val2 ] each a literal or "$var"
        - return_address: where to store val1 - val2
        - current_memory: SectionMemory&
     2) Description:
        - Subtracts second parameter from first and stores result with type-safety.
     3) ASCII:
            params: ["10", "3"] -> result 7 stored in return_address
     4) Per-line:
        - parameter count check, resolve $refs, validate numeric, compute difference, type switch & store.
    */
    static void subtract(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Error! can't find the return address!");

        if (parameters.size() != 2)
            throw std::runtime_error("Error! subtract() only takes two parameters!");

        str val_1 = parameters[0]; // first operand
        str val_2 = parameters[1]; // second operand

        if (val_1.front() == '$')
            val_1 = current_memory.m_fetchStringalizedValue(val_1); // resolve $ref for first

        if (val_2.front() == '$')
            val_2 = current_memory.m_fetchStringalizedValue(val_2); // resolve $ref for second

        if (!___isNum___(val_1))
            throw std::runtime_error(str("Bad values for function subtract(): ") + val_1);

        if (!___isNum___(val_2))
            throw std::runtime_error(str("Bad values for function subtract(): ") + val_2);

        long double result = std::stold(val_1) - std::stold(val_2); // compute result
        auto ra = current_memory.m_getVariable(return_address);   // get return var descriptor

        // store result according to return type with bounds checks
        switch (ra.second)
        {
        case ENUM_TYPE_I8:
        {
            if (result < -128 || result > 127)
                throw std::runtime_error("Return address too small for subtract!");

            int8_t val = static_cast<int8_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I8 });
            break;
        }

        case ENUM_TYPE_I16:
        {
            if (result < -32768 || result > 32767)
                throw std::runtime_error("Return address too small for subtract!");

            int16_t val = static_cast<int16_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I16 });
            break;
        }

        case ENUM_TYPE_I32:
        {
            if (result < INT32_MIN || result > INT32_MAX)
                throw std::runtime_error("Return address too small for subtract!");

            int32_t val = static_cast<int32_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I32 });
            break;
        }

        case ENUM_TYPE_I64:
        {
            int64_t val = static_cast<int64_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I64 });
            break;
        }

        case ENUM_TYPE_F32:
        {
            if (result < -FLT_MAX || result > FLT_MAX)
                throw std::runtime_error("Return address too small for subtract!");

            float val = static_cast<float>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F32 });
            break;
        }

        case ENUM_TYPE_F64:
        {
            if (result < -DBL_MAX || result > DBL_MAX)
                throw std::runtime_error("Return address too small for sum");

            double val = static_cast<double>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F64 });
            break;
        }

        case ENUM_TYPE_FMAX:
        {
            current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_FMAX });
            break;
        }

        default:
            throw std::runtime_error("Invalid type for subtract(): only numeric types allowed");
        }
    }


    /*
     divide(parameters, return_address, current_memory)
     1) Parameters:
        - parameters: [ val1, val2 ]
        - return_address: where to store val1 / val2
        - current_memory: SectionMemory&
     2) Description:
        - Divides first parameter by second. No explicit divide-by-zero guard in current code.
        - Stores result into return_address with type checks.
     3) ASCII:
            params: ["8", "2"] -> 4.0
     4) Per-line:
        - check return address, param count, resolve $refs, validate numeric, compute division, type switch & store
        - note: divide-by-zero is not explicitly checked (left as original behavior).
    */
    static void divide(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Error! can't find the return address!");

        if (parameters.size() != 2)
            throw std::runtime_error("Error! divide() only takes two parameters!");

        str val_1 = parameters[0];
        str val_2 = parameters[1];

        if (val_1.front() == '$')
            val_1 = current_memory.m_fetchStringalizedValue(val_1);

        if (val_2.front() == '$')
            val_2 = current_memory.m_fetchStringalizedValue(val_2);

        if (!___isNum___(val_1))
            throw std::runtime_error(str("Bad values for function divide(): ") + val_1);

        if (!___isNum___(val_2))
            throw std::runtime_error(str("Bad values for function divide(): ") + val_2);

        long double result = std::stold(val_1) / std::stold(val_2); // division (original code: no zero-division guard)

        auto ra = current_memory.m_getVariable(return_address);

        // type-switch to store result
        switch (ra.second)
        {
        case ENUM_TYPE_I8:
        {
            if (result < -128 || result > 127)
                throw std::runtime_error("Return address too small for divide");

            int8_t val = static_cast<int8_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I8 });
            break;
        }

        case ENUM_TYPE_I16:
        {
            if (result < -32768 || result > 32767)
                throw std::runtime_error("Return address too small for divide");

            int16_t val = static_cast<int16_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I16 });
            break;
        }

        case ENUM_TYPE_I32:
        {
            if (result < INT32_MIN || result > INT32_MAX)
                throw std::runtime_error("Return address too small for divide");

            int32_t val = static_cast<int32_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I32 });
            break;
        }

        case ENUM_TYPE_I64:
        {
            int64_t val = static_cast<int64_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I64 });
            break;
        }

        case ENUM_TYPE_F32:
        {
            if (result < -FLT_MAX || result > FLT_MAX)
                throw std::runtime_error("Return address too small for divide");

            float val = static_cast<float>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F32 });
            break;
        }

        case ENUM_TYPE_F64:
        {
            if (result < -DBL_MAX || result > DBL_MAX)
                throw std::runtime_error("Return address too small for divide");
            double val = static_cast<double>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F64 });
            break;
        }

        case ENUM_TYPE_FMAX:
        {
            current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_FMAX });
            break;
        }

        default:
            throw std::runtime_error("Invalid type for divide(): only numeric types allowed");
        }
    }


    /*
     mod(parameters, return_address, current_memory)
     1) Parameters:
        - parameters: [ val1, val2 ]
        - return_address: where to store integer remainder
        - current_memory: SectionMemory&
     2) Description:
        - Computes integer modulus by casting operands to long long then applying '%'.
        - This function implicitly truncates floating inputs via stold -> cast.
     3) ASCII:
            params: ["10", "3"] -> 10 % 3 = 1
     4) Per-line:
        - validate return, param count, resolve $refs, numeric validate, compute (cast to long long), type-switch & store.
        - note: mod by zero is not explicitly checked in original code.
    */
    static void mod(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Error! can't find the return address!");

        if (parameters.size() != 2)
            throw std::runtime_error("Error! mod() only takes two parameters!");

        str val_1 = parameters[0];
        str val_2 = parameters[1];

        if (val_1.front() == '$')
            val_1 = current_memory.m_fetchStringalizedValue(val_1);

        if (val_2.front() == '$')
            val_2 = current_memory.m_fetchStringalizedValue(val_2);

        if (!___isNum___(val_1))
            throw std::runtime_error(str("Bad values for function mod(): ") + val_1);

        if (!___isNum___(val_2))
            throw std::runtime_error(str("Bad values for function mod(): ") + val_2);

        long long result =
            (long long)std::stold(val_1) %
            (long long)std::stold(val_2); // cast to long long before modulus

        auto ra = current_memory.m_getVariable(return_address);

        // store result according to return type
        switch (ra.second)
        {
        case ENUM_TYPE_I8:
        {
            if (result < -128 || result > 127)
                throw std::runtime_error("Return address too small for mod");

            int8_t val = static_cast<int8_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I8 });
            break;
        }

        case ENUM_TYPE_I16:
        {
            if (result < -32768 || result > 32767)
                throw std::runtime_error("Return address too small for mod");

            int16_t val = static_cast<int16_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I16 });
            break;
        }

        case ENUM_TYPE_I32:
        {
            if (result < INT32_MIN || result > INT32_MAX)
                throw std::runtime_error("Return address too small for mod");

            int32_t val = static_cast<int32_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I32 });
            break;
        }

        case ENUM_TYPE_I64:
        {
            int64_t val = static_cast<int64_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I64 });
            break;
        }

        case ENUM_TYPE_F32:
        {
            if (result < -FLT_MAX || result > FLT_MAX)
                throw std::runtime_error("Return address too small for mod");

            float val = static_cast<float>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F32 });
            break;
        }

        case ENUM_TYPE_F64:
        {
            if (result < -DBL_MAX || result > DBL_MAX)
                throw std::runtime_error("Return address too small for mod");

            double val = static_cast<double>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F64 });
            break;
        }

        case ENUM_TYPE_FMAX:
        {
            current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_FMAX });
            break;
        }

        default:
            throw std::runtime_error("Invalid type for mod(): only numeric types allowed");
        }
    }


    /*
     floor(parameters, return_address, current_memory)
     1) Parameters:
        - parameters: [ value ] single numeric literal or "$var"
        - return_address: where to store floor(value)
        - current_memory: SectionMemory&
     2) Description:
        - Applies std::floor to the numeric input, then stores typed result.
     3) ASCII:
            params: ["3.7"] -> floor -> 3
     4) Per-line:
        - check return slot, param count, resolve $refs, validate numeric, compute floor, type-switch & store.
    */
    static void floor(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Error! can't find the return address!");

        if (parameters.size() != 1)
            throw std::runtime_error("Error! floor() only takes a single parameter");

        str val_1 = parameters[0];

        if (val_1.front() == '$')
            val_1 = current_memory.m_fetchStringalizedValue(val_1); // dereference variable

        if (!___isNum___(val_1))
            throw std::runtime_error(str("Bad values for function floor(): ") + val_1);

        long double result = std::floor(std::stold(val_1)); // floor operation

        auto ra = current_memory.m_getVariable(return_address);

        // switch on return type, check bounds, cast, and store
        switch (ra.second)
        {
        case ENUM_TYPE_I8:
        {
            if (result < -128 || result > 127)
                throw std::runtime_error("Return address too small for floor");

            int8_t val = static_cast<int8_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I8 });
            break;
        }

        case ENUM_TYPE_I16:
        {
            if (result < -32768 || result > 32767)
                throw std::runtime_error("Return address too small for floor");

            int16_t val = static_cast<int16_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I16 });
            break;
        }

        case ENUM_TYPE_I32:
        {
            if (result < INT32_MIN || result > INT32_MAX)
                throw std::runtime_error("Return address too small for floor");

            int32_t val = static_cast<int32_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I32 });
            break;
        }

        case ENUM_TYPE_I64:
        {
            int64_t val = static_cast<int64_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I64 });
            break;
        }

        case ENUM_TYPE_F32:
        {
            if (result < -FLT_MAX || result > FLT_MAX)
                throw std::runtime_error("Return address too small for floor");

            float val = static_cast<float>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F32 });
            break;
        }

        case ENUM_TYPE_F64:
        {
            if (result < -DBL_MAX || result > DBL_MAX)
                throw std::runtime_error("Return address too small for floor");

            if (result < -DBL_MIN || result > DBL_MAX)
                throw std::runtime_error("Return address too small for floor");

            double val = static_cast<double>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F64 });
            break;
        }

        case ENUM_TYPE_FMAX:
        {
            current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_FMAX });
            break;
        }

        default:
            throw std::runtime_error("Invalid type for floor(): only numeric types allowed");
        }
    }


    /*
     ceiling(parameters, return_address, current_memory)
     1) Parameters:
        - parameters: [ value ] (single)
        - return_address: storage variable
        - current_memory: SectionMemory&
     2) Description:
        - Applies std::ceil to the numeric input and stores typed result.
     3) ASCII:
            params: ["2.3"] -> ceil -> 3
     4) Per-line: resolve $refs, numeric validation, std::ceil, then storage with type-checks.
    */
    static void ceiling(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Error! can't find the return address!");

        if (parameters.size() != 1)
            throw std::runtime_error("Error! ceiling() only takes a single parameter");

        str val_1 = parameters[0];

        if (val_1.front() == '$')
            val_1 = current_memory.m_fetchStringalizedValue(val_1);

        if (!___isNum___(val_1))
            throw std::runtime_error(str("Bad values for function ceiling(): ") + val_1);

        long double result = std::ceil(std::stold(val_1));

        auto ra = current_memory.m_getVariable(return_address);

        switch (ra.second)
        {
        case ENUM_TYPE_I8:
        {
            if (result < -128 || result > 127)
                throw std::runtime_error("Return address too small for ceiling");

            int8_t val = static_cast<int8_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I8 });
            break;
        }

        case ENUM_TYPE_I16:
        {
            if (result < -32768 || result > 32767)
                throw std::runtime_error("Return address too small for ceiling");

            int16_t val = static_cast<int16_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I16 });
            break;
        }

        case ENUM_TYPE_I32:
        {
            if (result < INT32_MIN || result > INT32_MAX)
                throw std::runtime_error("Return address too small for ceiling");

            int32_t val = static_cast<int32_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I32 });
            break;
        }

        case ENUM_TYPE_I64:
        {
            int64_t val = static_cast<int64_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I64 });
            break;
        }

        case ENUM_TYPE_F32:
        {
            if (result < -FLT_MAX || result > FLT_MAX)
                throw std::runtime_error("Return address too small for ceiling");

            float val = static_cast<float>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F32 });
            break;
        }

        case ENUM_TYPE_F64:
        {
            if (result < -DBL_MAX || result > DBL_MAX)
                throw std::runtime_error("Return address too small for ceiling");

            double val = static_cast<double>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F64 });
            break;
        }

        case ENUM_TYPE_FMAX:
        {
            current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_FMAX });
            break;
        }

        default:
            throw std::runtime_error("Invalid type for ceiling(): only numeric types allowed");
        }
    }


    /*
     abs(parameters, return_address, current_memory)
     1) Parameters:
        - parameters: [ value ] single numeric
        - return_address: where to store absolute value
        - current_memory: SectionMemory&
     2) Description:
        - Returns absolute value of the parameter using std::abs on long double,
          then stores result into return_address with type checks.
     3) ASCII:
            params: ["-5"] -> abs -> 5
     4) Per-line: validate return slot, param count, resolve references, numeric validate, compute std::abs, store.
    */
    static void abs(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Error! can't find the return address!");

        if (parameters.size() != 1)
            throw std::runtime_error("Error! abs() only takes a single parameter");

        str val_1 = parameters[0];

        if (val_1.front() == '$')
            val_1 = current_memory.m_fetchStringalizedValue(val_1);

        if (!___isNum___(val_1))
            throw std::runtime_error(str("Bad values for function abs(): ") + val_1);

        long double result = std::abs(std::stold(val_1));

        auto ra = current_memory.m_getVariable(return_address);

        switch (ra.second)
        {
        case ENUM_TYPE_I8:
        {
            if (result < -128 || result > 127)
                throw std::runtime_error("Return address too small for absolute value!");

            int8_t val = static_cast<int8_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I8 });
            break;
        }

        case ENUM_TYPE_I16:
        {
            if (result < -32768 || result > 32767)
                throw std::runtime_error("Return address too small for absolute value!");

            int16_t val = static_cast<int16_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I16 });
            break;
        }

        case ENUM_TYPE_I32:
        {
            if (result < INT32_MIN || result > INT32_MAX)
                throw std::runtime_error("Return address too small for absolute value!");

            int32_t val = static_cast<int32_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I32 });
            break;
        }

        case ENUM_TYPE_I64:
        {
            int64_t val = static_cast<int64_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I64 });
            break;
        }

        case ENUM_TYPE_F32:
        {
            if (result < -FLT_MAX || result > FLT_MAX)
                throw std::runtime_error("Return address too small for absolute value!");

            float val = static_cast<float>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F32 });
            break;
        }

        case ENUM_TYPE_F64:
        {
            if (result < -DBL_MAX || result > DBL_MAX)
                throw std::runtime_error("Return address too small for abs");

            double val = static_cast<double>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F64 });
            break;
        }

        case ENUM_TYPE_FMAX:
        {
            current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_FMAX });
            break;
        }

        default:
            throw std::runtime_error("Invalid type for abs(): only numeric types allowed");
        }
    }

    /*
     pow(parameters, return_address, current_memory)
     1) Parameters:
        - parameters: [ base, exponent ] each numeric or "$var"
        - return_address: variable where base^exponent is stored
        - current_memory: SectionMemory&
     2) Description:
        - Computes std::pow(base, exponent) as long double, then stores value after type checks.
     3) ASCII:
            params: ["2", "3"] -> pow -> 8
     4) Per-line: validate return, param count, resolve $refs, numeric validate, compute pow, type-switch & store.
    */
    static void pow(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Error! can't find the return address!");

        if (parameters.size() != 2)
            throw std::runtime_error("Error! pow() only takes two parameters!");

        str val_1 = parameters[0];
        str val_2 = parameters[1];

        if (val_1.front() == '$')
            val_1 = current_memory.m_fetchStringalizedValue(val_1);

        if (val_2.front() == '$')
            val_2 = current_memory.m_fetchStringalizedValue(val_2);

        if (!___isNum___(val_1))
            throw std::runtime_error(str("Bad values for function divide(): ") + val_1);

        if (!___isNum___(val_2))
            throw std::runtime_error(str("Bad values for function divide(): ") + val_2);
        long double result = std::pow(std::stold(val_1), std::stold(val_2)); // compute power
        auto ra = current_memory.m_getVariable(return_address);
        switch (ra.second)
        {
        case ENUM_TYPE_I8:
        {
            if (result < -128 || result > 127)
                throw std::runtime_error("Return address too small for pow");

            int8_t val = static_cast<int8_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I8 });
            break;
        }

        case ENUM_TYPE_I16:
        {
            if (result < -32768 || result > 32767)
                throw std::runtime_error("Return address too small for pow");

            int16_t val = static_cast<int16_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I16 });
            break;
        }

        case ENUM_TYPE_I32:
        {
            if (result < INT32_MIN || result > INT32_MAX)
                throw std::runtime_error("Return address too small for pow");

            int32_t val = static_cast<int32_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I32 });
            break;
        }

        case ENUM_TYPE_I64:
        {
            int64_t val = static_cast<int64_t>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_I64 });
            break;
        }

        case ENUM_TYPE_F32:
        {
            if (result < -FLT_MAX || result > FLT_MAX)
                throw std::runtime_error("Return address too small for pow");

            float val = static_cast<float>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F32 });
            break;
        }

        case ENUM_TYPE_F64:
        {
            if (result < -DBL_MAX || result > DBL_MAX)
                throw std::runtime_error("Return address too small for pow");
            double val = static_cast<double>(result);
            current_memory.m_reInsertVariable(return_address, { val, ENUM_TYPE_F64 });
            break;
        }

        case ENUM_TYPE_FMAX:
        {
            current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_FMAX });
            break;
        }

        default:
            throw std::runtime_error("Invalid type for pow(): only numeric types allowed");
        }
    }
};

#endif
