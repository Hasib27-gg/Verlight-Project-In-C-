#ifndef ___LOOP___ENGINE___H___
#define ___LOOP___ENGINE___H___
#include "MemoryModel.h"
class LoopEngine {
    static bool ___isNum___(const std::string& s)
    {
        if (s.empty()) return false;              
        int dotCount = 0;                         
        for (size_t i = 0; i < s.size(); ++i)      
        {
            char c = s[i];                       

            if (c == '.') {                    
                dotCount++;                       
                if (dotCount > 1) return false;   
            }
            else if (c == '-' || c == '+') {     
                if (i != 0) return false;        
            }
            else if (!std::isdigit(c)) return false; 
        }
        if (s == "+" || s == "-" || s == ".") return false; 
        return true;                              
    }
    static void STORE_LOOP_OPTION(const str& KEY, const value_t& VAL, SectionMemory& current_section)
    {
        if (current_section.m_containsVariable(KEY))
            throw std::runtime_error("Error! Loop option already exists: " + KEY); 
        current_section.m_insertVariable(KEY, VAL); 
    }
    static value_t makeIntValue(int64_t v) {
        if (v >= std::numeric_limits<int8_t>::min() && v <= std::numeric_limits<int8_t>::max())
            return { static_cast<int8_t>(v), ENUM_TYPE_I8 };
        if (v >= std::numeric_limits<int16_t>::min() && v <= std::numeric_limits<int16_t>::max())
            return { static_cast<int16_t>(v), ENUM_TYPE_I16 };
        if (v >= std::numeric_limits<int32_t>::min() && v <= std::numeric_limits<int32_t>::max())
            return { static_cast<int32_t>(v), ENUM_TYPE_I32 };
        return { static_cast<int64_t>(v), ENUM_TYPE_I64 };
    }

public:
    static void loop(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_section) {
        if (parameters.size() != 3)
            throw std::runtime_error("Error! loop() requires exactly 3 parameters!");
        if (!current_section.m_containsVariable(return_address))
            throw std::runtime_error("Error! return_address not found!");
        str s_start = parameters[0];
        str s_stop = parameters[1];
        str s_step = parameters[2];
        if (!s_start.empty() && s_start.front() == '$')
            s_start = current_section.m_fetchStringalizedValue(s_start);
        if (!s_stop.empty() && s_stop.front() == '$')
            s_stop = current_section.m_fetchStringalizedValue(s_stop);
        if (!s_step.empty() && s_step.front() == '$')
            s_step = current_section.m_fetchStringalizedValue(s_step);

        if (!___isNum___(s_start)) throw std::runtime_error("Error! invalid number: start");
        if (!___isNum___(s_stop))  throw std::runtime_error("Error! invalid number: stop");
        if (!___isNum___(s_step))  throw std::runtime_error("Error! invalid number: step");
     
        int64_t start, stop, step;
        try {
            start = std::stoll(s_start);
            stop = std::stoll(s_stop);
            step = std::stoll(s_step);
        }
        catch (...) {
            throw std::runtime_error("Error! number literal too big for int64_t.");
        }
        if (step == 0)
            throw std::runtime_error("Error! step must not be zero.");
        if (step > 0 && start > stop)
            throw std::runtime_error("Error! step > 0 but start > stop.");
        if (step < 0 && start < stop)
            throw std::runtime_error("Error! step < 0 but start < stop.");

        STORE_LOOP_OPTION(str("___LOOP___ENGINE___") + return_address + str("___start___"), makeIntValue(start), current_section);
        STORE_LOOP_OPTION(str("___LOOP___ENGINE___") + return_address + str("___stop___"), makeIntValue(stop), current_section);
        STORE_LOOP_OPTION(str("___LOOP___ENGINE___") + return_address + str("___step___"), makeIntValue(step), current_section);
    }
};
#endif

