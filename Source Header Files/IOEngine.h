#ifndef ___IOENGINE___H___
#define ___IOENGINE___H___

#include "MemoryModel.h"
class IOEngine {
public:
    static void print
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (!current_memory.m_containsVariable(return_address) &&
            return_address != "nullptr")
            throw std::runtime_error(
                "Return Address '" + return_address + "' was not found in the memory!"
            );
        for (auto item : parameters)
        {
            if (item.front() == '$')
            {
                std::cout << current_memory.m_fetchStringalizedValue(item);
            }
            else if (item.size() >= 2 && item.front() == '"' && item.back() == '"')
            {
                std::cout << item.substr(1, item.size() - 2);
            }
            else
            {
                std::cout << item;
            }
        }
    }
    static void println
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (!current_memory.m_containsVariable(return_address) &&
            return_address != "nullptr")
            throw std::runtime_error(
                "Return Address '" + return_address + "' was not found in the memory!"
            );
        for (auto item : parameters)
        {
            if (item.front() == '$')
            {
                std::cout << current_memory.m_fetchStringalizedValue(item) << '\n';
            }
            else if (item.size() >= 2 && item.front() == '"' && item.back() == '"')
            {
                std::cout << item.substr(1, item.size() - 2) << '\n';
            }
            else
            {
                std::cout << item << '\n';
            }
        }
    }
    static void flush
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (!current_memory.m_containsVariable(return_address) &&
            return_address != "nullptr")
        {
            throw std::runtime_error(
                "Return Address '" + return_address + "' was not found in the memory!"
            );
        }
        if (!parameters.empty())
            throw std::runtime_error("Error! flush() doesn't take any parameters.");
        std::cout << std::flush;
    }
    static void input
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (!current_memory.m_containsVariable(return_address) &&
            return_address != "nullptr")
            throw std::runtime_error(
                "Return Address '" + return_address + "' was not found in the memory!"
            );
        if (parameters.size() > 1)
            throw std::runtime_error("Error! input() takes at most one display string.");
        str display = parameters.empty() ? "" : parameters[0];
        if (display.front() == '$')
            display = current_memory.m_fetchStringalizedValue(display);
        if (display.front() == '"' && display.back() == '"')
            display = display.substr(1, display.size() - 2);
        std::cout << display << std::flush;
        str inpStr;
        std::getline(std::cin, inpStr);
        if (return_address != "nullptr")
        {
            value_t value = { inpStr, ENUM_TYPE_STRING };
            current_memory.m_reInsertVariable(return_address, value);
        }
    }
};

#endif

