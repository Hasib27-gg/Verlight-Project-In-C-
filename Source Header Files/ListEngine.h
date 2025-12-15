#ifndef LIST___ENGINE___H
#define LIST___ENGINE___H 0xca8763

#include "MemoryModel.h"
#include "DeclaratorEngine.h"
class ListEngine {
    static std::vector<str> splitContents(const str& bracket_bounded_contents) {
        std::vector<str> contents;          
        str temp_str;                        
        bool in_double_quote = false;        
        bool in_single_quote = false;        
        int bracket_depth = 0;               
        for (size_t i = 0; i < bracket_bounded_contents.size(); ++i) {
            char ch = bracket_bounded_contents[i]; 
            if (ch == '[' && !in_double_quote && !in_single_quote) {
                bracket_depth++; 
                continue;        
            }
            if (ch == ']' && !in_double_quote && !in_single_quote) {
                bracket_depth = std::max(0, bracket_depth - 1); 
                continue; 
            }
            if (ch == '"' && !in_single_quote) {
                bool escaped = (i > 0 && bracket_bounded_contents[i - 1] == '\\'); 
                if (!escaped) {
                    in_double_quote = !in_double_quote; 
                    temp_str.push_back(ch);             
                    continue;                         
                }
            }
            if (ch == '\'' && !in_double_quote) {
                bool escaped = (i > 0 && bracket_bounded_contents[i - 1] == '\\');
                if (!escaped) {
                    in_single_quote = !in_single_quote; 
                    temp_str.push_back(ch);             
                    continue;                         
                }
            }
            if (ch == ',' && !in_double_quote && !in_single_quote && bracket_depth >= 0) {
                size_t start = 0;
                while (start < temp_str.size() && std::isspace(static_cast<unsigned char>(temp_str[start]))) ++start;
                size_t end = temp_str.size();
                while (end > start && std::isspace(static_cast<unsigned char>(temp_str[end - 1]))) --end;
                contents.push_back(temp_str.substr(start, end - start));
                temp_str.clear(); 
                continue; 
            }
            temp_str.push_back(ch);
        }
        if (!temp_str.empty()) {
            size_t start = 0;
            while (start < temp_str.size() && std::isspace(static_cast<unsigned char>(temp_str[start]))) ++start;
            size_t end = temp_str.size();
            while (end > start && std::isspace(static_cast<unsigned char>(temp_str[end - 1]))) --end;
            contents.push_back(temp_str.substr(start, end - start));
        }

        return contents; 
    }
    static bool isNum(const std::string& s, bool& containsDot) {
        containsDot = false;                  
        if (s.empty()) return false;         
        size_t start = 0;
        if (s[0] == '+' || s[0] == '-') start = 1; 
        int dotCount = 0;
        bool hasDigit = false;
        for (size_t i = start; i < s.size(); ++i) {
            char c = s[i];
            if (c == '.') {
                dotCount++;
                containsDot = true;           
                if (dotCount > 1) return false; 
            }
            else if (std::isdigit(static_cast<unsigned char>(c))) {
                hasDigit = true;             
            }
            else {
                return false;               
            }
        }
        if (!hasDigit) return false;         
        return !(s == "+" || s == "-" || s == ".");
    }
    static bool isHexDigit(char c) {
        return std::isxdigit(static_cast<unsigned char>(c)) != 0; 
    }
    static bool isOctDigit(char c) {
        return c >= '0' && c <= '7';
    }
    static bool ___isChar___(const std::string& value) {
        if (value.size() < 2) return false;           
        if (value.front() != '\'' || value.back() != '\'') return false; 

        std::string inner = value.substr(1, value.size() - 2); 
        if (inner.empty()) return false;               

        if (inner.front() == '\\') {
            if (inner.size() == 2) {
                char esc = inner[1];
                switch (esc) {
                case 'n': case 't': case 'r': case 'b': case 'f': case 'v':
                case '\\': case '\'': case '"': case '0':
                    return true;
                default:
                    return false; 
                }
            }
            if (inner.size() >= 4 && inner[1] == 'x') {
                if (isHexDigit(inner[2]) && isHexDigit(inner[3]) && inner.size() == 4) return true;
            }
            if (inner.size() >= 2 && isOctDigit(inner[1])) {
                int len = std::min<int>(3, static_cast<int>(inner.size() - 1)); 
                for (int i = 0; i < len; ++i) {
                    if (!isOctDigit(inner[1 + i])) return false; 
                }
                return true;
            }
            return false;
        }
        return inner.size() == 1;
    }

public:
    static void new_list(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    ) {
        if (parameters.size() != 3)
            throw std::runtime_error(
                "Parameters take three parameters, syntax: (name, type: [anyBuiltIn, 'dynamic'], values: [iterable])"
            );
        std::vector<str> contents = splitContents(parameters[2]);
        str listName = parameters[0];
        str size_name = str("___LIST___ENGINE___") + listName + str("___SIZE___");
        if (current_memory.m_containsVariable(size_name)) {
            std::cout << "\n[ERROR_ENGINE] The listName that is causing the error: (" << listName << ")\n";
            throw std::runtime_error("List already exsists!");
        }
        current_memory.m_insertVariable(size_name, { static_cast<int64_t>(contents.size()) , ENUM_TYPE_I64 });
        if (parameters[1] == "dynamic") {
            int it_count = 0; 
            for (auto item : contents) {
                bool containsDot = false;
                str name = str("___LIST___ENGINE___") + listName + str("___") + std::to_string(it_count) + str("___");
                if (item == "true" || item == "false") {
                    current_memory.m_insertVariable(name, { item == "true", ENUM_TYPE_BOOL });
                }
                else if (isNum(item, containsDot)) {
                    long double num = std::stold(item);
                    if (!containsDot) {
                        if (num >= -128 && num <= 127)
                            current_memory.m_insertVariable(name, { static_cast<int8_t>(num), ENUM_TYPE_I8 });
                        else if (num >= SHRT_MIN && num <= SHRT_MAX)
                            current_memory.m_insertVariable(name, { static_cast<int16_t>(num), ENUM_TYPE_I16 });
                        else if (num >= INT32_MIN && num <= INT32_MAX)
                            current_memory.m_insertVariable(name, { static_cast<int32_t>(num), ENUM_TYPE_I32 });
                        else if (num >= INT64_MIN && num <= INT64_MAX)
                            current_memory.m_insertVariable(name, { static_cast<int64_t>(num), ENUM_TYPE_I64 });
                        else
                            throw std::runtime_error("Number is too big");
                    }
                    else {
                        if (num >= FLT_MIN && num <= FLT_MAX)
                            current_memory.m_insertVariable(name, { static_cast<float>(num), ENUM_TYPE_F32 });
                        else if (num >= DBL_MIN && num <= DBL_MAX)
                            current_memory.m_insertVariable(name, { static_cast<double>(num), ENUM_TYPE_F64 });
                        else if (num >= LDBL_MIN && num <= LDBL_MAX)
                            current_memory.m_insertVariable(name, { static_cast<long double>(num), ENUM_TYPE_FMAX });
                        else
                            throw std::runtime_error("Number is too big");
                    }
                }
                else if (___isChar___(item)) {
                    DeclaratorEngine::new_char({ name, item }, "nullptr", current_memory);
                }
                else if (item.front() == '"' && item.back() == '"') {
                    item = item.substr(1, item.length() - 2); 
                    current_memory.m_insertVariable(name, { item, ENUM_TYPE_STRING });
                }
                else {
                    std::cerr << "\n[ERROR_ENGINE] The item causing exception: " << item << '\n';
                    throw std::runtime_error("Bad value for reAssign_list");
                }

                it_count++; 
            }
        }
    }
    static void reAssign_list(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    ) {
        if (parameters.size() != 3)
            throw std::runtime_error(
                "Parameters take three parameters, syntax: (name, type: [anyBuiltIn, 'dynamic'], values: [iterable])"
            );

        std::vector<str> contents = splitContents(parameters[2]);
        str listName = parameters[0];
        str size_name = str("___LIST___ENGINE___") + listName + str("___SIZE___");
        if (!current_memory.m_containsVariable(size_name)) {
        
            std::cout << "\n[ERROR_ENGINE] The listName that is causing the error: (" << listName << ")\n";
            throw std::runtime_error("Couldn't find the list!");
        }
        current_memory.m_reInsertVariable(size_name, { static_cast<int64_t>(contents.size()) , ENUM_TYPE_I64 });

        if (parameters[1] == "dynamic") {
            int it_count = 0;

            for (auto item : contents) {
                bool containsDot = false;
                str name = str("___LIST___ENGINE___") + listName + str("___") + std::to_string(it_count) + str("___");

                if (item == "true" || item == "false") {
                    current_memory.m_reInsertVariable(name, { item == "true", ENUM_TYPE_BOOL });
                }
                else if (isNum(item, containsDot)) {
                    long double num = std::stold(item);

                    if (!containsDot) {
                        if (num >= -128 && num <= 127)
                            current_memory.m_reInsertVariable(name, { static_cast<int8_t>(num), ENUM_TYPE_I8 });
                        else if (num >= SHRT_MIN && num <= SHRT_MAX)
                            current_memory.m_reInsertVariable(name, { static_cast<int16_t>(num), ENUM_TYPE_I16 });
                        else if (num >= INT32_MIN && num <= INT32_MAX)
                            current_memory.m_reInsertVariable(name, { static_cast<int32_t>(num), ENUM_TYPE_I32 });
                        else if (num >= INT64_MIN && num <= INT64_MAX)
                            current_memory.m_reInsertVariable(name, { static_cast<int64_t>(num), ENUM_TYPE_I64 });
                        else
                            throw std::runtime_error("Number is too big");
                    }
                    else {
                        if (num >= FLT_MIN && num <= FLT_MAX)
                            current_memory.m_reInsertVariable(name, { static_cast<float>(num), ENUM_TYPE_F32 });
                        else if (num >= DBL_MIN && num <= DBL_MAX)
                            current_memory.m_reInsertVariable(name, { static_cast<double>(num), ENUM_TYPE_F64 });
                        else if (num >= LDBL_MIN && num <= LDBL_MAX)
                            current_memory.m_reInsertVariable(name, { static_cast<long double>(num), ENUM_TYPE_FMAX });
                        else
                            throw std::runtime_error("Number is too big");
                    }
                }
                else if (___isChar___(item)) {
                    DeclaratorEngine::reAssign_char({ name, item }, "nullptr", current_memory);
                }
                else if (item.front() == '"' && item.back() == '"') {
                    item = item.substr(1, item.length() - 2);
                    current_memory.m_reInsertVariable(name, { item, ENUM_TYPE_STRING });
                }
                else {
                    std::cerr << "\n[ERROR_ENGINE] The item causing exception: " << item << '\n';
                    throw std::runtime_error("Bad value for reAssign_list");
                }

                it_count++;
            }
        }
    }
    static void delete_list(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    ) {

        for (const auto& listName : parameters) {
            str size_name = str("___LIST___ENGINE___") + listName + str("___SIZE___");
            if (!current_memory.m_containsVariable(size_name)) {
                std::cout << "\n[ERROR_ENGINE] The listName that is causing the error: (" << listName << ")\n";
                throw std::runtime_error("Couldn't find the list!");
            }
            int64_t list_size = std::get<int64_t>(current_memory.m_getVariable(size_name).first);
            for (int64_t idx = 0; idx < list_size; ++idx) {
                str at_name = str("___LIST___ENGINE___") +
                    listName +
                    str("___") +
                    std::to_string(idx) +
                    str("___");
                current_memory.m_removeVariable(at_name); 
            }
            current_memory.m_removeVariable(size_name);
        }
    }
    static void get(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    ) {
        if (parameters.size() != 2)
            throw std::runtime_error("get() requires two positional parameters! The proper syntax is (...list_name... , ...0_based_idx...)");

        if (!current_memory.m_containsVariable(return_address))
            throw std::runtime_error("Couldn't find the return_address, in the function get()!");
        str listName = parameters[0];
        str indexStr = parameters[1];
        if (!indexStr.empty() && (indexStr.front() == '\'' || indexStr.front() == '"')) {
            indexStr = current_memory.m_fetchStringalizedValue(indexStr);
        }

        long long idx = 0;
        try {
            idx = std::stoll(indexStr); 
        }
        catch (...) {
            throw std::runtime_error("Index for get() is not a valid integer");
        }
        str size_name = str("___LIST___ENGINE___") + listName + str("___SIZE___");
        if (!current_memory.m_containsVariable(size_name))
            throw std::runtime_error("Couldn't find the list in get()!");

        int64_t list_size = std::get<int64_t>(current_memory.m_getVariable(size_name).first);
        if (idx < 0 || idx >= list_size)
            throw std::runtime_error("Index out of range in get()");
        str at_name = str("___LIST___ENGINE___") + listName + str("___") + std::to_string(idx) + str("___");
        auto var_pair = current_memory.m_getVariable(at_name);
        current_memory.m_reInsertVariable(return_address, var_pair);
    }
    static void push(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        if (parameters.size() != 2)
            throw std::runtime_error("push() requires exactly two parameters!");

        str listName = parameters[0];
        str size_name = str("___LIST___ENGINE___") + listName + str("___SIZE___");
        if (!current_memory.m_containsVariable(size_name))
        {
            std::cout << "\n[ERROR_ENGINE] The listName that is casuing the error: (" << listName << ")\n";
            throw std::runtime_error("Couldn't find the list!");
        }
        int64_t list_size = std::get<int64_t>(current_memory.m_getVariable(size_name).first);
        str item = parameters[1];
        if (item.front() == '$')
            item = current_memory.m_fetchStringalizedValue(item);
        str new_item_name = str("___LIST___ENGINE___") + listName + str("___")
            + std::to_string(list_size) + str("___");
        bool containsDot = false;
        if (item == "true" || item == "false") {
            current_memory.m_insertVariable(new_item_name, {
                item == "true" ? true : false, ENUM_TYPE_BOOL
                });
        }
        else if (isNum(item, containsDot))
        {
            long double num = std::stold(item);
            if (!containsDot) {
                if (num >= -128 && num <= 127)
                {
                    current_memory.m_insertVariable(new_item_name,
                        { static_cast<int8_t>(num), ENUM_TYPE_I8 });

                }
                else if (num >= SHRT_MIN && num <= SHRT_MAX)
                {
                    current_memory.m_insertVariable(new_item_name,
                        { static_cast<int16_t>(num), ENUM_TYPE_I16 });
                }
                else if (num >= INT32_MIN && num <= INT32_MAX)
                {
                    current_memory.m_insertVariable(new_item_name,
                        { static_cast<int32_t>(num), ENUM_TYPE_I32 });
                }
                else if (num >= INT64_MIN && num <= INT64_MAX)
                {
                    current_memory.m_insertVariable(new_item_name,
                        { static_cast<int64_t>(num), ENUM_TYPE_I64 });
                }
                else throw std::runtime_error("Number is to big");
            }
            else {
                if (num >= FLT_MIN && num <= FLT_MAX)
                {
                    current_memory.m_insertVariable(new_item_name,
                        { static_cast<float>(num), ENUM_TYPE_F32 });
                }
                else if (num >= DBL_MIN && num <= DBL_MAX)
                {
                    current_memory.m_insertVariable(new_item_name,
                        { static_cast<double>(num), ENUM_TYPE_F64 });
                }
                else if (num >= LDBL_MIN && num <= LDBL_MAX)
                {
                    current_memory.m_insertVariable(new_item_name,
                        { static_cast<long double>(num), ENUM_TYPE_FMAX });
                }
                else throw std::runtime_error("Number is too big");
            }
        }
        else if (___isChar___(item))
        {
            DeclaratorEngine::new_char({ new_item_name , item }, "nullptr", current_memory);
        }
        else if (!item.empty() && item.front() == '"' && item.back() == '"') {
            str inner = item.substr(1, item.length() - 2);
            current_memory.m_insertVariable(new_item_name, { inner , ENUM_TYPE_STRING });
        }
        else {
            std::cerr << "\n[ERROR_ENGINE] The item which is causing exception is: " << item << '\n';
            throw std::runtime_error("Bad value for push()");
        }
        current_memory.m_reInsertVariable(size_name, { static_cast<int64_t>(list_size + 1), ENUM_TYPE_I64 });
    }
    static void pop(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    ) {
        if (parameters.size() != 1)
            throw std::runtime_error("pop() requires exactly one parameter!");

        str listName = parameters[0];
        str size_name = str("___LIST___ENGINE___") + listName + str("___SIZE___");
        if (!current_memory.m_containsVariable(size_name)) {
            std::cout << "\n[ERROR_ENGINE] The listName that is causing the error: (" << listName << ")\n";
            throw std::runtime_error("Couldn't find the list!");
        }
        int64_t list_size = std::get<int64_t>(current_memory.m_getVariable(size_name).first);
        if (list_size <= 0)
            throw std::runtime_error("Cannot pop from an empty list!");
        str last_element_name = str("___LIST___ENGINE___") +
            listName + str("___") + std::to_string(list_size - 1) + str("___");
        current_memory.m_removeVariable(last_element_name); 
        current_memory.m_reInsertVariable(size_name, { static_cast<int64_t>(list_size - 1), ENUM_TYPE_I64 }); 
    }
    static void printList(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    ) {
        if (parameters.size() != 3)
            throw std::runtime_error("printList() requires 3 parameter!");

        str listName = parameters[0];
        str head = parameters[1];
        str tail = parameters[2];
        if (head.front() == '$')
            head = current_memory.m_fetchStringalizedValue(head);
        if (tail.front() == '$')
            tail = current_memory.m_fetchStringalizedValue(tail);
        if (head.front() == '"' && head.front() == '"')
            head = head.substr(1, head.length() - 2);

        if (tail.front() == '"' && tail.front() == '"')
            tail = tail.substr(1, tail.length() - 2);
        str size_name = str("___LIST___ENGINE___") + listName + str("___SIZE___");
        if (!current_memory.m_containsVariable(size_name)) {
            std::cout << "\n[ERROR_ENGINE] The listName that is causing the error: (" << listName << ")\n";
            throw std::runtime_error("Couldn't find the list!");
        }
        int64_t list_size = std::get<int64_t>(current_memory.m_getVariable(size_name).first);
        bool mask = false;
        std::cout << head << "[";
        for (int64_t index = 0; index < list_size; ++index) {
            str curr_var_name = str("___LIST___ENGINE___") + listName + str("___") +
                std::to_string(index) + str("___");
            if (mask)
                std::cout << ", "; 
            mask = true;
            auto val = current_memory.m_getVariable(curr_var_name);
            if (val.second == ENUM_TYPE_STRING || val.second == ENUM_TYPE_CHAR)
                std::cout << '"' << makeValueToString(val) << '"'; 
            else std::cout << makeValueToString(val); 
        }
        std::cout << "]" << tail; 
    }
};
#endif

