#ifndef LIST___ENGINE___H
#define LIST___ENGINE___H 0xca8763

#include "MemoryModel.h"
#include "DeclaratorEngine.h"

/*
 Order of member functions (private -> public):
   Private helpers:
     1) splitContents
     2) isNum
     3) isHexDigit
     4) isOctDigit
     5) ___isChar___
   Public operations:
     6) new_list
     7) reAssign_list
     8) delete_list
     9) get
    10) push
    11) pop
    12) printList

 Rationale:
   - Helpers for parsing / testing are declared first (used by public ops).
   - Public list operations follow and implement create, update, delete and access patterns.
*/

class ListEngine {

    /*
     splitContents
     1) Parameters:
        - bracket_bounded_contents : const str&
          A string that represents the list literal including bracket characters, e.g. "[1, 'a', \"str\"]"
     2) Description:
        - Splits a bracket-bounded list literal into its top-level comma-separated elements,
          while respecting quoted strings, character literals and nested bracket sections.
        - Trims whitespace around each element.
     3) Visualization:
            Input:
                "[  1, 'a', \"hello, world\", [1,2], \"escaped \\\"comma\\\"\" ]"
            Output vector:
                { "1", "'a'", "\"hello, world\"", "[1,2]", "\"escaped \\\"comma\\\"\"" }
            Visual:
            +-----------------------------------------------+
            | [ 1 , 'a' , "hello, world" , [1,2] , "x" ]    |
            +-----------------------------------------------+
            becomes
            +----------------+----------------+------------+
            | "1"            | "'a'"         | "\"x\""    |
            +----------------+----------------+------------+
     4) Per-line / block comments:
        - Walk characters, track quote and bracket state, accumulate characters into temp_str.
        - On top-level (not inside quotes/brackets) comma -> finalize a token (trim it).
        - After loop, push final token if non-empty.
    */
    static std::vector<str> splitContents(const str& bracket_bounded_contents) {
        std::vector<str> contents;           // result vector of split elements
        str temp_str;                        // accumulator for current element
        bool in_double_quote = false;        // are we inside double quotes "..."
        bool in_single_quote = false;        // are we inside single quotes '...'
        int bracket_depth = 0;               // nested bracket depth (counts '[' and ']')

        // iterate over each character by index (we need previous char for escape checks)
        for (size_t i = 0; i < bracket_bounded_contents.size(); ++i) {
            char ch = bracket_bounded_contents[i]; // current character

            // treat opening bracket: increase depth (only when not inside quotes)
            if (ch == '[' && !in_double_quote && !in_single_quote) {
                bracket_depth++; // entering nested bracket
                continue;        // don't append the outer '[' to tokens
            }
            // treat closing bracket: reduce depth (only when not inside quotes)
            if (ch == ']' && !in_double_quote && !in_single_quote) {
                bracket_depth = std::max(0, bracket_depth - 1); // guard negative depth
                continue; // don't append the outer ']' to tokens
            }

            // double-quote handling (respect escapes and don't toggle if escaped)
            if (ch == '"' && !in_single_quote) {
                bool escaped = (i > 0 && bracket_bounded_contents[i - 1] == '\\'); // check previous char
                if (!escaped) {
                    in_double_quote = !in_double_quote; // toggle double-quote state
                    temp_str.push_back(ch);             // include the quote char in the token
                    continue;                           // continue to next char
                }
            }

            // single-quote handling (respect escapes)
            if (ch == '\'' && !in_double_quote) {
                bool escaped = (i > 0 && bracket_bounded_contents[i - 1] == '\\'); // check previous char
                if (!escaped) {
                    in_single_quote = !in_single_quote; // toggle single-quote state
                    temp_str.push_back(ch);             // include the quote char in the token
                    continue;                           // continue to next char
                }
            }

            // comma outside quotes and at non-negative bracket depth splits elements
            if (ch == ',' && !in_double_quote && !in_single_quote && bracket_depth >= 0) {
                // trim leading whitespace
                size_t start = 0;
                while (start < temp_str.size() && std::isspace(static_cast<unsigned char>(temp_str[start]))) ++start;
                // trim trailing whitespace
                size_t end = temp_str.size();
                while (end > start && std::isspace(static_cast<unsigned char>(temp_str[end - 1]))) --end;
                // push trimmed substring
                contents.push_back(temp_str.substr(start, end - start));
                temp_str.clear(); // reset accumulator for next element
                continue; // move to next character after comma
            }

            // default: accumulate character into current token
            temp_str.push_back(ch);
        }

        // after loop: if any accumulated token remains, trim and push it
        if (!temp_str.empty()) {
            size_t start = 0;
            while (start < temp_str.size() && std::isspace(static_cast<unsigned char>(temp_str[start]))) ++start;
            size_t end = temp_str.size();
            while (end > start && std::isspace(static_cast<unsigned char>(temp_str[end - 1]))) --end;
            contents.push_back(temp_str.substr(start, end - start));
        }

        return contents; // return vector of split contents
    }

    /*
     isNum
     1) Parameters:
        - s : const std::string&  -> string to test if numeric literal
        - containsDot : bool&     -> output flag set to true if a '.' is present (floating literal)
     2) Description:
        - Returns true if the string represents a valid numeric literal (integer or floating).
        - Accepts optional leading sign '+' or '-'.
        - Ensures at least one digit present and at most one decimal dot.
     3) Visualization:
            Input: "123"  -> containsDot=false, returns true
            Input: "-12.34" -> containsDot=true, returns true
            Input: "a12" -> returns false
            Visual:
            +---------------+
            |  "-12.34"     |
            +---------------+
            => parse characters, detect digits and dot
     4) Per-line / block comments:
        - Set containsDot initially false.
        - Skip optional sign.
        - Iterate characters: count dots and mark any digit found.
        - Reject if no digit or invalid chars found.
    */
    static bool isNum(const std::string& s, bool& containsDot) {
        containsDot = false;                   // default assume no dot
        if (s.empty()) return false;           // empty string -> not a number
        size_t start = 0;
        if (s[0] == '+' || s[0] == '-') start = 1; // allow leading sign (skip it)
        int dotCount = 0;
        bool hasDigit = false;

        // iterate from start (skip sign if present)
        for (size_t i = start; i < s.size(); ++i) {
            char c = s[i];
            if (c == '.') {
                dotCount++;
                containsDot = true;           // mark that we have a dot
                if (dotCount > 1) return false; // more than one dot -> invalid number
            }
            else if (std::isdigit(static_cast<unsigned char>(c))) {
                hasDigit = true;             // found at least one digit
            }
            else {
                return false;                // any non-digit and non-dot -> not a number
            }
        }
        if (!hasDigit) return false;          // must have at least one digit
        return !(s == "+" || s == "-" || s == "."); // disallow lone sign or lone dot
    }

    /*
     isHexDigit
     1) Parameters:
        - c : char -> character to test
     2) Description:
        - Returns true if the character is a valid hexadecimal digit (0-9, a-f, A-F).
     3) Visualization:
            'A' -> true
            'g' -> false
     4) Per-line:
        - Uses std::isxdigit on unsigned-casted char to be locale-safe.
    */
    static bool isHexDigit(char c) {
        return std::isxdigit(static_cast<unsigned char>(c)) != 0; // true for hex digits
    }

    /*
     isOctDigit
     1) Parameters:
        - c : char -> character to test
     2) Description:
        - Returns true if the character is an octal digit (0..7).
     3) Visualization:
            '5' -> true
            '8' -> false
     4) Per-line:
        - Simple range check for ASCII chars '0'..'7'
    */
    static bool isOctDigit(char c) {
        return c >= '0' && c <= '7';
    }

    /*
     ___isChar___
     1) Parameters:
        - value : const std::string&  -> token to test whether it's a character literal (like 'a' or '\n' or '\x41' or '\123')
     2) Description:
        - Validates C-style character literals wrapped in single quotes:
            - simple char: 'a'
            - escape sequences: '\n', '\t', '\'', '\\', '\"', '\0'
            - hex escape: '\xNN' (two hex digits expected)
            - octal escape: '\123' (1-3 octal digits)
        - Returns true if value is a valid character literal; false otherwise.
     3) Visualization:
            Input:
              "'a'"   -> true
              "'\\n'" -> true
              "'\\x41' -> true
              "'abc'" -> false
            Visual:
            +-----------+
            |  'a'      |
            +-----------+
     4) Per-line comments:
        - Ensure string begins and ends with single quotes and inner content obeys expected escape rules.
        - NOTE: this function intentionally inspects inner characters to determine escape type.
    */
    static bool ___isChar___(const std::string& value) {
        if (value.size() < 2) return false;            // must at least be two chars: quotes
        if (value.front() != '\'' || value.back() != '\'') return false; // must be single-quoted

        std::string inner = value.substr(1, value.size() - 2); // content inside quotes
        if (inner.empty()) return false;               // empty inner -> invalid

        // escaped form: starts with backslash
        if (inner.front() == '\\') {
            if (inner.size() == 2) {
                // single-character escape sequences like '\n', '\t', '\r', etc.
                char esc = inner[1];
                switch (esc) {
                case 'n': case 't': case 'r': case 'b': case 'f': case 'v':
                case '\\': case '\'': case '"': case '0':
                    return true; // recognized 2-char escape sequence
                default:
                    return false; // unrecognized 2-char escape
                }
            }
            // NOTE: the following line attempts to support hex escapes like \xNN
            // but is written to return a bool directly for two hex digits after prefix.
            // It checks inner[2] and inner[3] — ensure indexes are valid before use.
            if (inner.size() >= 4 && inner[1] == 'x') {
                // hex form '\xNN' requires exactly two hex digits after 'x'
                if (isHexDigit(inner[2]) && isHexDigit(inner[3]) && inner.size() == 4) return true;
            }
            // octal escape detection: '\nnn' where digits are octal (1..3 digits)
            if (inner.size() >= 2 && isOctDigit(inner[1])) {
                int len = std::min<int>(3, static_cast<int>(inner.size() - 1)); // up to 3 octal digits
                for (int i = 0; i < len; ++i) {
                    if (!isOctDigit(inner[1 + i])) return false; // ensure each is octal
                }
                return true;
            }
            return false;
        }
        // non-escaped single character: inner must be exactly one char long
        return inner.size() == 1;
    }

public:

    /*
     new_list
     1) Parameters:
        - parameters : const std::vector<str>&
              expected pattern: (name, type, values)
              - name: string name of list
              - type: either "dynamic" or a built-in type (but current implementation handles "dynamic")
              - values: bracketed iterable as a string, e.g. "[1, 2, 'a']"
        - return_address : const str&  (unused in this routine; required by VM utility signature)
        - current_memory : SectionMemory&  memory section where list variables are stored
     2) Description:
        - Creates a new list in the SectionMemory. Stores the list size under
          "___LIST___ENGINE___<name>___SIZE___" and stores each element under
          "___LIST___ENGINE___<name>___<idx>___".
        - If type == "dynamic", elements are auto-typed based on content (boolean, integer, floating, char, string).
     3) Visualization:
            Call:
              new_list( { "myList", "dynamic", "[1, 2.0, 'a', \"hi\"]" }, ... )
            Memory after:
              ___LIST___ENGINE___myList___SIZE___ => 4
              ___LIST___ENGINE___myList___0___ => 1
              ___LIST___ENGINE___myList___1___ => 2.0
              ___LIST___ENGINE___myList___2___ => 'a'
              ___LIST___ENGINE___myList___3___ => "hi"
            Visual:
            +-----------------------------------------+
            | myList: [1, 2.0, 'a', "hi"]             |
            +-----------------------------------------+
     4) Per-line / block comments:
        - Validate params count.
        - Parse contents using splitContents.
        - Build size variable and per-element variables using naming convention.
        - For dynamic typing: detect booleans, numbers (then choose smallest numeric type), chars, strings.
    */
    static void new_list(
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    ) {
        // verify the correct parameter count
        if (parameters.size() != 3)
            throw std::runtime_error(
                "Parameters take three parameters, syntax: (name, type: [anyBuiltIn, 'dynamic'], values: [iterable])"
            );

        // split the values (third parameter) into individual items
        std::vector<str> contents = splitContents(parameters[2]);
        str listName = parameters[0];

        // size variable name follows internal naming convention
        str size_name = str("___LIST___ENGINE___") + listName + str("___SIZE___");
        if (current_memory.m_containsVariable(size_name)) {
            // error if list already exists
            std::cout << "\n[ERROR_ENGINE] The listName that is causing the error: (" << listName << ")\n";
            throw std::runtime_error("List already exsists!");
        }

        // insert size into memory as int64
        current_memory.m_insertVariable(size_name, { static_cast<int64_t>(contents.size()) , ENUM_TYPE_I64 });

        // handle dynamic typed list only (as per current implementation)
        if (parameters[1] == "dynamic") {
            int it_count = 0; // element index

            // iterate each item in contents and insert typed variable
            for (auto item : contents) {
                bool containsDot = false;
                // element variable name pattern
                str name = str("___LIST___ENGINE___") + listName + str("___") + std::to_string(it_count) + str("___");

                // boolean literals
                if (item == "true" || item == "false") {
                    current_memory.m_insertVariable(name, { item == "true", ENUM_TYPE_BOOL });
                }
                // numeric detection: integer vs float
                else if (isNum(item, containsDot)) {
                    long double num = std::stold(item);

                    if (!containsDot) {
                        // pick narrowest integer type that fits
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
                        // floating types based on magnitude
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
                // character literal detection
                else if (___isChar___(item)) {
                    // reuse DeclaratorEngine::new_char to create char variable (keeps behavior consistent)
                    DeclaratorEngine::new_char({ name, item }, "nullptr", current_memory);
                }
                // string literal detection: starts and ends with double quotes
                else if (item.front() == '"' && item.back() == '"') {
                    item = item.substr(1, item.length() - 2); // remove surrounding quotes
                    current_memory.m_insertVariable(name, { item, ENUM_TYPE_STRING });
                }
                else {
                    // unknown element token => error
                    std::cerr << "\n[ERROR_ENGINE] The item causing exception: " << item << '\n';
                    throw std::runtime_error("Bad value for reAssign_list");
                }

                it_count++; // increment element index
            }
        }
    }

    /*
     reAssign_list
     1) Parameters:
        - parameters : (name, type, values)
        - return_address : const str& (unused in body)
        - current_memory : SectionMemory&
     2) Description:
        - Replaces the contents of an existing list: updates the stored size and reassigns each element
          variable using memory->m_reInsertVariable (so it expects existing variables by name).
        - Implementation mirrors new_list but uses reInsert for existing variables.
     3) Visualization:
            Call:
              reAssign_list({ "myList", "dynamic", "[5, 'b']" }, ...)
            Effect:
              size updated to 2; each element re-inserted using same naming scheme.
     4) Per-line:
        - Validate list exists by checking size_name, then reInsert size and elements.
    */
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
            // list must exist to reassign
            std::cout << "\n[ERROR_ENGINE] The listName that is causing the error: (" << listName << ")\n";
            throw std::runtime_error("Couldn't find the list!");
        }
        // update the size variable
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
                    // use DeclaratorEngine::reAssign_char for consistent char re-assignment behavior
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

    /*
     delete_list
     1) Parameters:
        - parameters : vector<str> -> list of list names to delete
        - return_address : const str& (unused)
        - current_memory : SectionMemory&
     2) Description:
        - For each listName provided, removes every stored element variable and the list-size variable.
     3) Visualization:
            Before:
              size var = N, elements 0..N-1 exist
            After:
              no size var, no elements for that list remain
            Visual:
            +----------------------------+
            | remove myList[0..N-1], size|
            +----------------------------+
     4) Per-line:
        - For each listName, read size var, loop over indices and remove variables by naming convention, then remove size var.
    */
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
            // fetch size (stored as int64)
            int64_t list_size = std::get<int64_t>(current_memory.m_getVariable(size_name).first);
            // remove each element variable
            for (int64_t idx = 0; idx < list_size; ++idx) {
                str at_name = str("___LIST___ENGINE___") +
                    listName +
                    str("___") +
                    std::to_string(idx) +
                    str("___");
                current_memory.m_removeVariable(at_name); // remove element variable
            }
            // remove the size variable itself
            current_memory.m_removeVariable(size_name);
        }
    }

    /*
     get
     1) Parameters:
        - parameters : (list_name, indexStr)
        - return_address : const str& -> destination variable to hold the retrieved value
        - current_memory : SectionMemory&
     2) Description:
        - Retrieves element at zero-based index from the named list and re-inserts it into return_address.
        - Accepts index either as literal or as string reference ($var) or quoted literal reference.
     3) Visualization:
            get({ "myList", "2" }, "dest", memory) -> dest = myList[2]
            Visual:
            +----------------------+
            | dest <- myList[2]    |
            +----------------------+
     4) Per-line:
        - Validate params and return_address presence.
        - Resolve index (allow stringized variables).
        - Bounds check and perform variable fetch and reInsert into return_address.
    */
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

        // if indexStr is a quoted or referenced string, fetch its stringized value
        if (!indexStr.empty() && (indexStr.front() == '\'' || indexStr.front() == '"')) {
            indexStr = current_memory.m_fetchStringalizedValue(indexStr);
        }

        long long idx = 0;
        try {
            idx = std::stoll(indexStr); // convert to integer
        }
        catch (...) {
            throw std::runtime_error("Index for get() is not a valid integer");
        }

        // validate that list exists by checking size var
        str size_name = str("___LIST___ENGINE___") + listName + str("___SIZE___");
        if (!current_memory.m_containsVariable(size_name))
            throw std::runtime_error("Couldn't find the list in get()!");

        int64_t list_size = std::get<int64_t>(current_memory.m_getVariable(size_name).first);
        if (idx < 0 || idx >= list_size)
            throw std::runtime_error("Index out of range in get()");

        // compute element variable name and move the variable to return_address
        str at_name = str("___LIST___ENGINE___") + listName + str("___") + std::to_string(idx) + str("___");
        auto var_pair = current_memory.m_getVariable(at_name);
        current_memory.m_reInsertVariable(return_address, var_pair);
    }

    /*
     push
     1) Parameters:
        - parameters : (listName, itemToken)
          itemToken may be literal number, boolean, char literal, string literal, or $reference
        - return_address : const str& (unused)
        - current_memory : SectionMemory&
     2) Description:
        - Appends an item to the named list. Creates a new element variable at index == current size,
          updates size variable by +1.
        - Detects type (boolean, integer, float, char, string) and inserts with smallest fitting numeric type.
     3) Visualization:
            push({ "myList", "42" }, ...) => new element at index N with value 42, size = N+1
            Visual:
            +------------------+
            | myList -> add 42 |
            +------------------+
     4) Per-line:
        - Validate list exists, compute new element name, detect token type, insert new variable and update size.
    */
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

        // fetch current size
        int64_t list_size = std::get<int64_t>(current_memory.m_getVariable(size_name).first);

        // resolve item token (may be a reference like $x)
        str item = parameters[1];
        if (item.front() == '$')
            item = current_memory.m_fetchStringalizedValue(item);

        // new element variable name
        str new_item_name = str("___LIST___ENGINE___") + listName + str("___")
            + std::to_string(list_size) + str("___");
        bool containsDot = false;

        // boolean literal
        if (item == "true" || item == "false") {
            current_memory.m_insertVariable(new_item_name, {
                item == "true" ? true : false, ENUM_TYPE_BOOL
                });
        }
        // numeric detection & insertion (choose smallest numeric type that fits)
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
        // char literal
        else if (___isChar___(item))
        {
            DeclaratorEngine::new_char({ new_item_name , item }, "nullptr", current_memory);
        }
        // string literal
        else if (!item.empty() && item.front() == '"' && item.back() == '"') {
            str inner = item.substr(1, item.length() - 2);
            current_memory.m_insertVariable(new_item_name, { inner , ENUM_TYPE_STRING });
        }
        else {
            // unknown token -> error
            std::cerr << "\n[ERROR_ENGINE] The item which is causing exception is: " << item << '\n';
            throw std::runtime_error("Bad value for push()");
        }
        // update stored size (increment)
        current_memory.m_reInsertVariable(size_name, { static_cast<int64_t>(list_size + 1), ENUM_TYPE_I64 });
    }

    /*
     pop
     1) Parameters:
        - parameters : (listName)
        - return_address : const str& (unused)
        - current_memory : SectionMemory&
     2) Description:
        - Removes and discards the last element of the named list (if not empty).
        - Decrements stored size.
     3) Visualization:
            pop({ "myList" }, ...) -> removes myList[N-1], size -> N-1
     4) Per-line:
        - Validate list exists, ensure non-empty, remove last element variable and decrement size.
    */
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
        current_memory.m_removeVariable(last_element_name); // remove last element
        current_memory.m_reInsertVariable(size_name, { static_cast<int64_t>(list_size - 1), ENUM_TYPE_I64 }); // update size
    }

    /*
     printList
     1) Parameters:
        - parameters : (listName, head, tail)
           - head : string (displayed before list contents)
           - tail : string (displayed after list contents)
        - return_address : const str& (unused)
        - current_memory : SectionMemory&
     2) Description:
        - Prints the list with given head and tail formatting. Resolves $ references for head/tail.
        - Each element is printed using makeValueToString and quoted if string/char.
     3) Visualization:
            printList({ "myList", "start: ", " :end" }, ... )
            Output:
              start: ["a", 2, 3] :end
     4) Per-line:
        - Resolve head/tail (allow $var references and quoted literals).
        - Validate list exists, iterate elements, print comma-separated elements and apply quoting for strings/chars.
    */
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

        // resolve possible references for head and tail (leading $)
        if (head.front() == '$')
            head = current_memory.m_fetchStringalizedValue(head);

        if (tail.front() == '$')
            tail = current_memory.m_fetchStringalizedValue(tail);

        // remove surrounding quotes if present for head and tail
        if (head.front() == '"' && head.front() == '"')
            head = head.substr(1, head.length() - 2);

        if (tail.front() == '"' && tail.front() == '"')
            tail = tail.substr(1, tail.length() - 2);

        // validate list exists
        str size_name = str("___LIST___ENGINE___") + listName + str("___SIZE___");
        if (!current_memory.m_containsVariable(size_name)) {
            std::cout << "\n[ERROR_ENGINE] The listName that is causing the error: (" << listName << ")\n";
            throw std::runtime_error("Couldn't find the list!");
        }

        // read list size
        int64_t list_size = std::get<int64_t>(current_memory.m_getVariable(size_name).first);
        bool mask = false; // used to print commas between elements
        std::cout << head << "[";
        for (int64_t index = 0; index < list_size; ++index) {
            str curr_var_name = str("___LIST___ENGINE___") + listName + str("___") +
                std::to_string(index) + str("___");
            if (mask)
                std::cout << ", "; // separator for elements
            mask = true;
            auto val = current_memory.m_getVariable(curr_var_name); // fetch element
            if (val.second == ENUM_TYPE_STRING || val.second == ENUM_TYPE_CHAR)
                std::cout << '"' << makeValueToString(val) << '"'; // quote strings and chars
            else std::cout << makeValueToString(val); // print other types as-is
        }
        std::cout << "]" << tail; // closing bracket and tail displayed
    }

};
#endif
