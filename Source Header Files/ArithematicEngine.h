#ifndef ___ARITHEMETIC___ENGINE___H____
#define ___ARITHEMETIC___ENGINE___H____
#include "MemoryModel.h"
class ArithmeticEngine
{
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
            {
                return false;                     
            }
        }

        if (s == "+" || s == "-" || s == ".")     
            return false;

        return true;                              
    }

public:
    static void isNum(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 1)
            throw std::runtime_error("isNum() can only take 1 parameter!"); 

        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Error! can't find the return address!"); 

        value_t value = { ___isNum___(parameters[0]), ENUM_TYPE_BOOL };
        current_memory.m_reInsertVariable(return_address, value);      
    }
    static void sum(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Error! can't find the return address!");
        str current_str = "";             
        long double accumulator = 0;      
        for (auto item : parameters)
        {
            if (item.front() == '$')                             
                current_str = current_memory.m_fetchStringalizedValue(item);
            else
                current_str = item;                             
            if (!___isNum___(current_str))                      
                throw std::runtime_error("Bad value for sum(): " + current_str);
            accumulator += std::stold(current_str);             
        }
        auto ra = current_memory.m_getVariable(return_address); 
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
    static void product(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Error! can't find the return address!");

        str current_str = "";
        long double accumulator = 1.0L; 
        for (auto item : parameters)
        {
            if (item.front() == '$')
                current_str = current_memory.m_fetchStringalizedValue(item);
            else
                current_str = item;

            if (!___isNum___(current_str))
                throw std::runtime_error("Bad value for sum(): " + current_str); 

            accumulator *= std::stold(current_str);
        }
        auto ra = current_memory.m_getVariable(return_address);
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

        str val_1 = parameters[0];
        str val_2 = parameters[1]; 

        if (val_1.front() == '$')
            val_1 = current_memory.m_fetchStringalizedValue(val_1); 

        if (val_2.front() == '$')
            val_2 = current_memory.m_fetchStringalizedValue(val_2);

        if (!___isNum___(val_1))
            throw std::runtime_error(str("Bad values for function subtract(): ") + val_1);

        if (!___isNum___(val_2))
            throw std::runtime_error(str("Bad values for function subtract(): ") + val_2);

        long double result = std::stold(val_1) - std::stold(val_2); 
        auto ra = current_memory.m_getVariable(return_address);   
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

        long double result = std::stold(val_1) / std::stold(val_2); 
        auto ra = current_memory.m_getVariable(return_address);
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
            (long long)std::stold(val_2); 

        auto ra = current_memory.m_getVariable(return_address);
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

