#ifndef ___IOENGINE___H___
#define ___IOENGINE___H___

#include "MemoryModel.h"

/*
Order of member functions (quick reference)
  1) print    - print items without newline
  2) println  - print items with newline after each item
  3) flush    - flush stdout
  4) input    - prompt for user input, store into return_address (string)

Rationale:
- IO functions are grouped by output (print/println/flush) then input.
- All functions accept the same signature used by the VM: (parameters, return_address, current_memory).
- No behavior changed — only extensive comments added below.
*/

class IOEngine {
public:

    /*
     print(...)
     1) Parameters:
        - parameters : const std::vector<str>&
            A list of items to print. Each item is either:
              * a literal string (quoted) like "\"hello\""
              * a variable reference beginning with '$' like "$x"
              * an unquoted token printed verbatim
        - return_address : const str&
            The VM "return address" variable name used for context validation.
            It must exist in current_memory or be the literal "nullptr".
        - current_memory : SectionMemory&
            The SectionMemory instance used to resolve "$" references.
     2) Description:
        - For each parameter, resolve variable references (strings that start with '$'),
          strip quotes for quoted string literals, or print tokens verbatim.
        - Does NOT append newlines automatically.
     3) Visualized example:
            Input parameters: ["\"hi\"", "$name", ":", "42"]
            Memory: $name -> "Alice"
            Output stream:  hiAlice:42
            (no trailing newline)
            Visual box:
            +--------------------------------+
            | std::cout: "hi" "Alice" ":" "42" |
            +--------------------------------+
     4) Per-line explanation (each code line described inline below).
    */
    static void print
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        // Validate that the return_address exists in memory, or is the literal "nullptr".
        // This keeps behavior consistent with other engine functions that require a return slot.
        if (!current_memory.m_containsVariable(return_address) &&
            return_address != "nullptr")
        {
            // Throw with descriptive error if return address not present.
            throw std::runtime_error(
                "Return Address '" + return_address + "' was not found in the memory!"
            );
        }

        // Iterate through each provided parameter token and print appropriately.
        for (auto item : parameters)
        {
            // If item begins with '$' it is a variable reference: fetch its stringified value and print it.
            if (item.front() == '$')
            {
                std::cout << current_memory.m_fetchStringalizedValue(item);
            }
            // Else if item looks like a quoted string (starts and ends with double quotes) print inner text.
            else if (item.size() >= 2 && item.front() == '"' && item.back() == '"')
            {
                std::cout << item.substr(1, item.size() - 2);
            }
            // Otherwise print the token verbatim (useful for separators or numeric literals).
            else
            {
                std::cout << item;
            }
        }
    }

    /*
     println(...)
     1) Parameters:
        - parameters : const std::vector<str>&
            Same shape as print(): quoted literals, $refs, or tokens.
        - return_address : const str&
            Must exist in current_memory or be "nullptr".
        - current_memory : SectionMemory&
     2) Description:
        - Like print(), but appends a newline ('\n') after each printed item.
        - Useful for printing multiple values each on its own line.
     3) Visualized example:
            Input parameters: ["\"Line\"", "$user"]
            Memory: $user -> "Bob"
            Output:
              Line
              Bob
            Visual box:
            +-----------+
            | "Line\n"  |
            | "Bob\n"   |
            +-----------+
     4) Per-line explanation: each line below is described inline.
    */
    static void println
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        // Validate return address presence (or allow "nullptr")
        if (!current_memory.m_containsVariable(return_address) &&
            return_address != "nullptr")
        {
            // Throw if invalid
            throw std::runtime_error(
                "Return Address '" + return_address + "' was not found in the memory!"
            );
        }

        // Iterate and print each parameter followed by newline
        for (auto item : parameters)
        {
            // If it's a variable reference, fetch its string value then write and newline
            if (item.front() == '$')
            {
                std::cout << current_memory.m_fetchStringalizedValue(item) << '\n';
            }
            // If it's a quoted literal, strip quotes and write with newline
            else if (item.size() >= 2 && item.front() == '"' && item.back() == '"')
            {
                std::cout << item.substr(1, item.size() - 2) << '\n';
            }
            // Otherwise print the token directly and append newline
            else
            {
                std::cout << item << '\n';
            }
        }
    }

    /*
     flush(...)
     1) Parameters:
        - parameters : const std::vector<str>&
            Expected to be empty. flush() does not accept parameters.
        - return_address : const str&
            Must exist in current_memory or be "nullptr".
        - current_memory : SectionMemory&
     2) Description:
        - Validates return_address and parameter count, then flushes std::cout.
        - Use to force the output buffer to write to the terminal/device immediately.
     3) Visualized example:
            Call: flush([], "nullptr", mem)
            Action: std::cout << std::flush
            Visual box:
            +------------------+
            | std::cout buffer |
            +------------------+
            (contents pushed out)
     4) Per-line explanation: described inline below.
    */
    static void flush
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        // Validate that the return address exists or is "nullptr".
        if (!current_memory.m_containsVariable(return_address) &&
            return_address != "nullptr")
        {
            // Throw if missing
            throw std::runtime_error(
                "Return Address '" + return_address + "' was not found in the memory!"
            );
        }

        // flush() accepts no parameters — enforce that now
        if (!parameters.empty())
            throw std::runtime_error("Error! flush() doesn't take any parameters.");

        // Perform the actual flush on the standard output stream
        std::cout << std::flush;
    }

    /*
     input(...)
     1) Parameters:
        - parameters : const std::vector<str>&
            Optionally one display string (quoted literal, $ref, or token) shown as prompt.
        - return_address : const str&
            If not "nullptr", the inputted line will be stored into this variable as ENUM_TYPE_STRING.
            Must exist in current_memory or be "nullptr".
        - current_memory : SectionMemory&
     2) Description:
        - Displays optional prompt, reads a full line from stdin, and optionally stores
          it as a string value in current_memory at return_address.
        - If the prompt is a variable reference (starts with '$'), it is resolved.
        - If the prompt is a quoted literal, quotes are stripped before display.
     3) Visualized example:
            parameters: ["\"Enter name: \""], return_address: "name"
            Behavior:
              prints: Enter name:
              waits for user, reads "Charlie" then stores variable name = "Charlie"
            Visual box:
              +-----------------------------+
              | std::cout: "Enter name:"     |
              | std::cin: reads "Charlie\n"  |
              | memory["name"] = "Charlie"   |
              +-----------------------------+
     4) Per-line explanation: inlined below.
    */
    static void input
    (
        const std::vector<str>& parameters,
        const str& return_address,
        SectionMemory& current_memory
    )
    {
        // Validate return_address exists in memory or allow "nullptr"
        if (!current_memory.m_containsVariable(return_address) &&
            return_address != "nullptr")
        {
            // Throw if return address is missing
            throw std::runtime_error(
                "Return Address '" + return_address + "' was not found in the memory!"
            );
        }

        // At most one display string is allowed: check that parameter count <= 1
        if (parameters.size() > 1)
            throw std::runtime_error("Error! input() takes at most one display string.");

        // Determine prompt to display (either empty or the single parameter)
        str display = parameters.empty() ? "" : parameters[0];

        // If prompt is a variable reference, resolve it via memory
        if (display.front() == '$')
            display = current_memory.m_fetchStringalizedValue(display);

        // If prompt looks like a quoted literal, strip quotes
        if (display.front() == '"' && display.back() == '"')
            display = display.substr(1, display.size() - 2);

        // Print prompt and flush the output so the user sees it immediately
        std::cout << display << std::flush;

        // Read a full line of input from stdin into inpStr
        str inpStr;
        std::getline(std::cin, inpStr);

        // If return_address is not "nullptr", store the input string into memory as ENUM_TYPE_STRING
        if (return_address != "nullptr")
        {
            value_t value = { inpStr, ENUM_TYPE_STRING };
            current_memory.m_reInsertVariable(return_address, value);
        }
    }
};

#endif
