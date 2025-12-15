#ifndef ___RELATIONAL___ENGINE___H___
#define ___RELATIONAL___ENGINE___H___

#include "MemoryModel.h"

class RelationalEngine {
private:
    static bool ___isNum___(const std::string& s) {
        if (s.empty()) return false;

        int dotCount = 0;
        for (size_t i = 0; i < s.size(); ++i) {
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
        return !(s == "+" || s == "-" || s == ".");
    }

    static bool ___floatEquals___(const long double& a, const long double& b, long double eps = 1e-12L) {
        return std::abs(a - b) < eps;
    }

public:
    static void isEqual(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        str left = parameters[0];
        str right = parameters[1];

        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);

        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        if (!___isNum___(left) || !___isNum___(right))
            throw std::runtime_error("Use isCharsEqual()");

        bool result = ___floatEquals___(std::stold(left), std::stold(right));

        auto ra = current_memory.m_getVariable(return_address);

        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isEqual() requires boolean return address!");

        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }

    static void isGreater(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        str left = parameters[0];
        str right = parameters[1];

        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        if (!___isNum___(left) || !___isNum___(right))
            throw std::invalid_argument("All the values must be valid numbers!");

        bool result = (std::stold(left) > std::stold(right));

        auto ra = current_memory.m_getVariable(return_address);
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isGreater() requires boolean return address!");

        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }

    static void isLess(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        str left = parameters[0];
        str right = parameters[1];

        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        if (!___isNum___(left) || !___isNum___(right))
            throw std::invalid_argument("All the values must be valid numbers!");

        bool result = (std::stold(left) < std::stold(right));

        auto ra = current_memory.m_getVariable(return_address);
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isLess() requires boolean return address!");

        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }

    static void isGreaterEqual(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        str left = parameters[0];
        str right = parameters[1];

        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        if (!___isNum___(left) || !___isNum___(right))
            throw std::invalid_argument("All the values must be valid numbers!");

        bool result = (std::stold(left) >= std::stold(right));

        auto ra = current_memory.m_getVariable(return_address);
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isGreaterEqual() requires boolean return address!");

        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }

    static void isLessEqual(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        str left = parameters[0];
        str right = parameters[1];

        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        if (!___isNum___(left) || !___isNum___(right))
            throw std::invalid_argument("All the values must be valid numbers!");

        bool result = (std::stold(left) <= std::stold(right));

        auto ra = current_memory.m_getVariable(return_address);
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isLessEqual() requires boolean return address!");

        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }

    static void isNotEqual(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        str left = parameters[0];
        str right = parameters[1];

        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        if (!___isNum___(left) || !___isNum___(right))
            throw std::invalid_argument("Use isCharsNotEqual");

        bool result = !___floatEquals___(std::stold(left), std::stold(right));

        auto ra = current_memory.m_getVariable(return_address);
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isNotEqual() requires boolean return address!");

        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }

    static void isCharsEqual(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        str left = parameters[0];
        str right = parameters[1];

        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        if (___isNum___(left) || ___isNum___(right))
            throw std::runtime_error("Use isEqual()");

        bool result = (left == right);

        auto ra = current_memory.m_getVariable(return_address);
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isCharsEqual() requires boolean return address!");

        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }

    static void isCharsNotEqual(const std::vector<str>& parameters, const str& return_address, SectionMemory& current_memory) {
        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Return Address was not found in the memory!");

        if (parameters.size() != 2)
            throw std::runtime_error("Parameters exactly take two parameters!");

        str left = parameters[0];
        str right = parameters[1];

        if (left.front() == '$') left = current_memory.m_fetchStringalizedValue(left);
        if (right.front() == '$') right = current_memory.m_fetchStringalizedValue(right);

        if (___isNum___(left) || ___isNum___(right))
            throw std::runtime_error("Use isNotEqual()");

        bool result = (left != right);

        auto ra = current_memory.m_getVariable(return_address);
        if (ra.second != ENUM_TYPE_BOOL)
            throw std::runtime_error("isCharsNotEqual() requires boolean return address!");

        current_memory.m_reInsertVariable(return_address, { result, ENUM_TYPE_BOOL });
    }
};

#endif
