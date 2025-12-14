#include "includes.h"

#ifndef ___VERLIGHT___COMPILER___H___
#define ___VERLIGHT___COMPILER___H___


#define  ___STATIC__MEMBER___ static
#define ___CONST___ const
#define ___NO___EXCEPTION___ noexcept
#define ___OPERATOR__SET___ {',' , '!' ,'.'  , ':' , '@' , '(' , ')' , '{' , '}' , '[' , ']' , '$' , '-'  , '~' , '<' , '>', ';' , '"' , '_'}

/*
Overview & Order of member functions (DICC_Compiler)
  - Type & macro declarations
  - Struct ___INSTRUCTION___T___  (instruction representation)
  - Public static methods:
      1) ___M___IS___VALID___OPERATOR___
         - Check whether a character is a language operator
      2) ___M___EXTRACT___PROGRAMS___
         - Extract program names and raw program body text from source code
      3) ___M___EXTRACT___PORGRAM___LINES___
         - Split each program body into semicolon-terminated lines (preserving quoted semicolons)
      4) ___M___READ___HIGH___LEVEL___INSTRUCTION___
         - Parse a single high-level instruction string into an ___INSTRUCTION___T___ structure
      5) ___M___CONVERT___TO___INSTRUCTIONS___FINALIZER___
         - Convert all program-lines into vectors of parsed instructions
      6) ___M___COMPILE___
         - Top-level compile helper that runs extraction + parsing and optionally prints status
  Rationale:
    - Small helper/isValid first.
    - Extraction of raw programs -> split into lines -> parse lines to instructions -> finalize/compile.
    - No behavioral changes — comments only.
*/

#ifndef ___HAS___DECLARED___TYPES___
#define ___HAS___DECLARED___TYPED___ 1

typedef bool ___BOOLEAN___;
typedef char ___CHAR___;
typedef std::vector<std::string> ___STRING___VECTOR___;
typedef robin_hood::unordered_set<char> ___CHAR___SET___;
typedef std::string ___STRING___;
typedef robin_hood::unordered_map<std::string, std::string> ___UMAP___STRING___STRING___T___;
typedef robin_hood::unordered_map<std::string, std::vector<std::string>> ___UMAP___STRING___VECTOR___STRING___T___;

/* Instruction representation used by compiler -> VM boundary */
struct ___INSTRUCTION___T___
{
    ___STRING___ ___VM___WILL___EXECUTE___ = "true";     // boolean guard string (e.g. "true" or "$someVar")
    ___STRING___ ___TODO___ = "nan";                     // the action token (e.g. "@new_i32")
    ___STRING___VECTOR___ ___PARAMETERS___ = { "nan" };  // parameter tokens
    ___STRING___ ___RETURN___ADDRESS___ = "nullptr";    // return address or identifier used by instructions
};

typedef  robin_hood::unordered_map<std::string, std::vector<___INSTRUCTION___T___>> ___UMAP___STRING___VECTOR___INSTRUCTIONS___T___;
typedef std::vector<___INSTRUCTION___T___> ___INSTRUCTIONS___VECTOR___T___;

#endif

// Compiler Engine
class DICC_Compiler {

public:

    /*
     ___M___IS___VALID___OPERATOR___(C)
     1) Parameters:
        - C : const char  -> a single character to test
     2) Description:
        - Returns true if the character is in the operator character set
          (commas, punctuation, bracket characters, quotes, underscore, etc).
     3) Visualization:
            Input: '('  -> true
                   'A'  -> false (unless underscore or operator char)
            Visual box:
            +--------+
            | ___OPERATOR__SET___ contains '(' ? -> yes |
            +--------+
     4) Per-line explanation:
        - Create a local operator set from macro, then check membership using find().
        - noexcept: function promises not to throw exceptions.
    */
    ___STATIC__MEMBER___ ___BOOLEAN___ ___M___IS___VALID___OPERATOR___
    (___CONST___ ___CHAR___ ___C)
        ___NO___EXCEPTION___
    {
    ___CHAR___SET___ ___PA___CONST___OPERATORS___ = ___OPERATOR__SET___; // local set from macro
    return ___PA___CONST___OPERATORS___.find(___C) != ___PA___CONST___OPERATORS___.end(); // membership check
    }

        /*
         ___M___EXTRACT___PROGRAMS___(__CODE__)
         1) Parameters:
            - __CODE__ : const std::string&  -> entire source code as a single string
         2) Description:
            - Scans source text and extracts named program blocks.
            - Program naming begins with '#' followed by identifier characters (alpha/underscore)
            - Program definition body begins at '{' and extends up to matching '}'.
            - Ignores spaces inside quoted strings (toggle with ___IGSPACES___).
            - Returns a map: program_name -> raw program body text (string).
         3) Visualization:
                Source: ... #main { ... } ... #util { ... }
                Output map:
                  "main" -> "{ ... }"
                  "util" -> "{ ... }"
                Box:
                +--------------------------+
                | "main" : code-for-main   |
                | "util" : code-for-util   |
                +--------------------------+
         4) Per-line/block comments:
            - Maintains phases: reading program name, reading program body, and whether quoted spaces are significant.
            - Builds temporary word (program name) and temp code block and inserts into result map when finished.
        */
        ___STATIC__MEMBER___ ___UMAP___STRING___STRING___T___ ___M___EXTRACT___PROGRAMS___
        (___CONST___ ___STRING___& __CODE__)
        ___NO___EXCEPTION___
    {
        ___UMAP___STRING___STRING___T___ __RESULT__;   // output: program_name -> raw code

    // state flags
    ___BOOLEAN___ ___PHASE___PRGNAME___ = false; // reading program name after '#'
    ___BOOLEAN___ ___PHASE___PRGDEF___ = false;  // inside a '{' program definition
    ___BOOLEAN___ ___IGSPACES___ = true;         // whether spaces are ignored (true when not inside quotes)
    ___STRING___ __TEMP__WORD__ = "";            // temporary accumulator for program name
    ___STRING___ __TEMP__CODE__ = "";            // temporary accumulator for code body

    // iterate over each character of the input code string
    for (auto item : __CODE__)
    {
        // when reading program name, non-alpha/non-underscore ends the name:
        if (___PHASE___PRGNAME___ && !std::isalpha(item) && item != '_')
        {
            if (!__TEMP__WORD__.empty())
                __RESULT__.insert({__TEMP__WORD__ , __TEMP__CODE__}); // insert mapping name->code
            ___PHASE___PRGNAME___ = false; // leave program-name phase
        }

        // when inside program def and encounter closing brace, finalize program entry
        if (___PHASE___PRGDEF___ && item == '}')
        {
            __RESULT__[__TEMP__WORD__] = __TEMP__CODE__; // store accumulated code in results
            __TEMP__CODE__.clear();                     // reset temp code
            __TEMP__WORD__.clear();                     // reset temp name
            ___PHASE___PRGDEF___ = false;               // leave program definition phase
        }

        // while in program-name phase append alphabetic/underscore characters to program name
        if (___PHASE___PRGNAME___)
            __TEMP__WORD__ += ___STRING___(1 , item);

        // Build code characters into __TEMP__CODE__ when:
        //   - inside a program definition and either the char is alpha or an operator
        //   - or when spaces are not to be ignored (inside quotes)
        //   - or when the char is a digit (allow numbers)
        if ((___PHASE___PRGDEF___ &&
            (
                std::isalpha(item) ||
               ___M___IS___VALID___OPERATOR___(item)
            )
            ) ||
            (!___IGSPACES___) ||
            std::isdigit(item)
        )
        __TEMP__CODE__ += ___STRING___(1, item);

        // Toggle quote-state when encountering quotes and currently ignoring spaces
        if (item == '"' && ___IGSPACES___) {
            ___IGSPACES___ = false;
            continue;
        }

        // Toggle back when encountering closing quote
        if (item == '"' && !___IGSPACES___)
        ___IGSPACES___ = true;

        // If '{' occurs and not already in a program-def, switch to program definition phase
        if (item == '{' && !___PHASE___PRGDEF___)
            ___PHASE___PRGDEF___ = true;

        // If '#' occurs and not already reading a program name, start reading program name
        if (item == '#' && !___PHASE___PRGNAME___)
            ___PHASE___PRGNAME___ = true;
    }
    return __RESULT__;
    }

        /*
         ___M___EXTRACT___PORGRAM___LINES___(___CODE___)
         1) Parameters:
            - ___CODE___ : const map<string, string>&  -> mapping program_name -> raw program code
         2) Description:
            - For each program body string, split into semicolon-terminated "lines".
            - Semicolons inside quoted strings are NOT treated as line terminators.
            - Returns map: program_name -> vector<string> (each entry is a single "line" including its trailing ';').
         3) Visualization:
                Input: { "main":"x = 1; print(\"a; b\"); y=2;" }
                Output: { "main": ["x = 1;", "print(\"a; b\");", "y=2;"] }
                Visual:
                +------------------------------+
                | "main" : [line1, line2, ...] |
                +------------------------------+
         4) Per-line explanation:
            - Iterate programs; for each char accumulate until semicolon found (unless inside quotes).
            - Push accumulated string to vector and clear buffer when semicolon completes a line.
        */
        ___STATIC__MEMBER___ ___UMAP___STRING___VECTOR___STRING___T___
        ___M___EXTRACT___PORGRAM___LINES___
        (___CONST___ ___UMAP___STRING___STRING___T___& ___CODE___)
    {
        ___UMAP___STRING___VECTOR___STRING___T___ __RESULT__; // program_name -> vector<lines>

        ___STRING___ ___TEMP___STRING___ = "";             // accumulator for a single line
        ___STRING___VECTOR___ ___TEMP___VECTOR___(0, "");  // vector to collect lines for a program

        // iterate over each program entry in the map
        for (auto item : ___CODE___)
        {
            ___BOOLEAN___ ___PHASE___READING___QUOTE___ = false; // inside quote toggle

            // iterate over every character of the program body (item.second)
            for (auto __item : item.second) {

                // If current char is not ';' or it is a ';' but we are inside quotes, append to accumulator
                if (__item != ';' ||
                    (
                        __item == ';' && ___PHASE___READING___QUOTE___
                        )
                    )
                    ___TEMP___STRING___ += ___STRING___(1, __item);

                // else if ';' encountered and not inside quotes, finalize current line
                else if (!___PHASE___READING___QUOTE___ && __item == ';')
                {
                    ___TEMP___STRING___ += ___STRING___(1, __item); // include the semicolon

                    if (!___TEMP___STRING___.empty())
                        ___TEMP___VECTOR___.push_back(___TEMP___STRING___); // push line into vector

                    ___TEMP___STRING___.clear(); // clear for next line
                }

                // Toggle quote-phase on unescaped quotes
                if (__item == '"' && !___PHASE___READING___QUOTE___) {
                    ___PHASE___READING___QUOTE___ = true;
                    continue;
                }

                if (__item == '"' && ___PHASE___READING___QUOTE___)
                    ___PHASE___READING___QUOTE___ = false;

            }

            // insert the vector of lines into the result map under program name
            __RESULT__.insert({ item.first , ___TEMP___VECTOR___ });
            ___TEMP___VECTOR___.clear(); // clear for next program
        }

        return __RESULT__;
    }

    /*
     ___M___READ___HIGH___LEVEL___INSTRUCTION___(__CODE___)
     1) Parameters:
        - ___CODE___ : const std::string&  -> a single high-level instruction string (one "line")
     2) Description:
        - Parses a high-level instruction into its components:
            <will_execute> @todo : (params) ~ return_address ;
          - < > : optional will-execute guard (may be "true" or "$var")
          - @...: todo (action) token, ended by ':' (e.g. @new_i32 :)
          - ( ... ): parameter list separated by commas, respecting quotes and bracket nesting
          - ~ ... ; : return address specification between '~' and ';'
        - Returns an ___INSTRUCTION___T___ struct with parsed fields.
     3) Visualization:
            Input: "<true> @add : (1, $x) ~ result;"
            Output: {
              ___VM___WILL___EXECUTE___: "true",
              ___TODO___: "add",
              ___PARAMETERS___: ["1", "$x"],
              ___RETURN___ADDRESS___: "result"
            }
            Visual block:
            +--------------------------------------------+
            | <true> | @add : | (1,$x) | ~ result ;      |
            +--------------------------------------------+
     4) Per-line / block comments:
        - Uses boolean phase flags to track which part is being read.
        - Respects quoted strings and nested brackets when parsing parameters (commas inside quotes/brackets are ignored).
    */
    ___STATIC__MEMBER___ ___INSTRUCTION___T___
        ___M___READ___HIGH___LEVEL___INSTRUCTION___
        (___CONST___ ___STRING___& ___CODE___)
        ___NO___EXCEPTION___
    {
        ___INSTRUCTION___T___ ___RESULT___;

    // initialize result fields to empty/defaults (avoid leftover values)
    ___RESULT___.___VM___WILL___EXECUTE___ = "";
    ___RESULT___.___TODO___ = "";
    ___RESULT___.___PARAMETERS___ = {};
    ___RESULT___.___RETURN___ADDRESS___ = "";

    // Parser phase flags:
    ___BOOLEAN___ ___PHASE___READ___WILLEXE___ = false;    // inside < ... >
    ___BOOLEAN___ ___PHASE___READ___TODO___ = false;       // reading @todo until ':'
    ___BOOLEAN___ ___PHASE___READ___PARAMETERS___ = false; // reading parameters inside ( ... )
    ___BOOLEAN___ ___PHASE___READ___RETURNADD___ = false;  // reading return address between '~' and ';'
    ___BOOLEAN___ ___PHASE___READING___QUOTE___ = false;    // inside double quotes
    ___BOOLEAN___ ___PHASE___READING___BRACKETS___ = false; // inside [ ... ] bracket, affects param splitting
    ___STRING___  ___TEMP___STRING___ = "";                // accumulator for a parameter token

    // iterate characters in instruction string
    for (auto item : ___CODE___)
    {

        // start reading will-execute guard when encountering '<' (and not already in that phase nor inside quotes)
        if
        (item == '<'
            && !___PHASE___READ___WILLEXE___
            && !___PHASE___READING___QUOTE___
        )
        {
            ___PHASE___READ___WILLEXE___ = true;
            continue;
        }

        // close will-execute guard when encountering '>'
        if
        (item == '>'
            && ___PHASE___READ___WILLEXE___
            && !___PHASE___READING___QUOTE___
        )
        {
            ___PHASE___READ___WILLEXE___ = false;
            continue;
        }
        // open parameters '('
        if
        (
            item == '('
            && !___PHASE___READ___PARAMETERS___
            && !___PHASE___READING___QUOTE___
        )
        {
            ___PHASE___READ___PARAMETERS___ = true;
            continue;
        }

        // close parameters ')': push last temp parameter if present
        if
        (
            item == ')'
            && ___PHASE___READ___PARAMETERS___
            && !___PHASE___READING___QUOTE___
        )
        {
            if (!___TEMP___STRING___.empty())
                ___RESULT___.___PARAMETERS___.push_back(___TEMP___STRING___);

            ___TEMP___STRING___.clear();
            ___PHASE___READ___PARAMETERS___ = false;
            continue;
        }
        // start reading TODO token when encountering '@'
        if
        (
            item == '@'
            && !___PHASE___READ___TODO___
            && !___PHASE___READING___QUOTE___
        )
        {
            ___PHASE___READ___TODO___ = true;
            continue;
        }

        // end reading TODO token at ':' (when in TODO phase)
        if
        (
            item == ':'
            && ___PHASE___READ___TODO___
            && !___PHASE___READING___QUOTE___
        )
        {
            ___PHASE___READ___TODO___ = false;
            continue;
        }
        // start reading return address when encounter '~'
        if
        (
            item == '~'
            && !___PHASE___READ___RETURNADD___
            && !___PHASE___READING___QUOTE___
        )
        {
            ___PHASE___READ___RETURNADD___ = true;
            continue;
        }
        // stop reading return address at ';'
        if
        (
            item == ';'
            && ___PHASE___READ___RETURNADD___
            && !___PHASE___READING___QUOTE___
        )
        {
            ___PHASE___READ___RETURNADD___ = false;
            continue;
        }


        // when inside will-execute guard and not a space, append char to VM_WILL_EXECUTE
        if (___PHASE___READ___WILLEXE___ && item != ' ')
        ___RESULT___.___VM___WILL___EXECUTE___ += ___STRING___(1 , item);

        // when reading TODO token, append non-space chars into TODO
        if (___PHASE___READ___TODO___ && item != ' ')
        ___RESULT___.___TODO___ += ___STRING___(1 , item);

        // when reading return address, append non-space chars into RETURN_ADDRESS
        if (___PHASE___READ___RETURNADD___ && item != ' ')
        ___RESULT___.___RETURN___ADDRESS___ += ___STRING___(1 , item);


        // Parameter parsing: accumulate chars into ___TEMP___STRING___ until an unprotected comma splits parameters
        if (___PHASE___READ___PARAMETERS___)
        {
            if (
                item != ',' || // if not a comma, accumulate
                (
                    item == ',' &&
                    (
                    ___PHASE___READING___QUOTE___ ||   // if inside quotes, comma is literal
                     ___PHASE___READING___BRACKETS___   // or inside brackets, comma is literal
                    )
                )
            )
                ___TEMP___STRING___ += ___STRING___(1 , item); // accumulate into current param token



            else if
            (
                item == ','
                && !___PHASE___READING___BRACKETS___
                && !___PHASE___READING___QUOTE___
            )
            {

                if (!___TEMP___STRING___.empty())
                    ___RESULT___.___PARAMETERS___.push_back(___TEMP___STRING___); // push completed parameter

                ___TEMP___STRING___.clear(); // reset accumulator for next param
            }
        }

        // Toggle quoted-string reading phase on encountering '"' (unescaped)
        if (item == '"' && !___PHASE___READING___QUOTE___)
        {
            ___PHASE___READING___QUOTE___ = true;
            continue;
        }

        if (item == '"' && ___PHASE___READING___QUOTE___)
            ___PHASE___READING___QUOTE___ = false;


        // bracket handling: '[' opens a bracket region, ']' closes it — commas inside brackets are not separators
        if (item == '[' && !___PHASE___READING___BRACKETS___ && !___PHASE___READING___QUOTE___)
        {
            ___PHASE___READING___BRACKETS___ = true;
            continue;
        }

        if (item == ']' && ___PHASE___READING___BRACKETS___ && !___PHASE___READING___QUOTE___)
        ___PHASE___READING___BRACKETS___ = false;


    }
    // Post-parse defaults: if no will-execute token found, default to "true"
    if (___RESULT___.___VM___WILL___EXECUTE___ == "")
    ___RESULT___.___VM___WILL___EXECUTE___ = "true";

    // default return address if none supplied: "nullptr"
    if (___RESULT___.___RETURN___ADDRESS___ == "")
    ___RESULT___.___RETURN___ADDRESS___ = "nullptr";

    return ___RESULT___;
    }

        /*
         ___M___CONVERT___TO___INSTRUCTIONS___FINALIZER___(__CODE___)
         1) Parameters:
            - ___CODE___ : const map<string, vector<string>>&  -> program -> vector<raw lines>
         2) Description:
            - Converts each raw line string into an ___INSTRUCTION___T___ by calling the reader.
            - Returns map: program_name -> vector<parsed instructions>.
         3) Visualization:
                Input: { "main": ["<true> @x : (1) ~ r;"] }
                Output: { "main": [ parsedInstruction ] }
                Visual:
                +------------------------------+
                | "main" : [ instructionObj ]  |
                +------------------------------+
         4) Per-line:
            - iterate programs and their lines, parse each line via reader, collect results into vector.
        */
        ___STATIC__MEMBER___ ___UMAP___STRING___VECTOR___INSTRUCTIONS___T___
        ___M___CONVERT___TO___INSTRUCTIONS___FINALIZER___
        (___CONST___ ___UMAP___STRING___VECTOR___STRING___T___& ___CODE___)
        ___NO___EXCEPTION___
    {
        ___UMAP___STRING___VECTOR___INSTRUCTIONS___T___ ___RESULT___; // output: program -> vector<instructions>
        ___INSTRUCTIONS___VECTOR___T___ ___TEMP___VECTOR___;          // temporary vector for instructions

        for (auto item : ___CODE___)
        {
            for (auto __item : item.second)
            ___TEMP___VECTOR___.push_back
           (___M___READ___HIGH___LEVEL___INSTRUCTION___(__item)); // parse each line and append to temp vector

           ___RESULT___.insert({item.first , ___TEMP___VECTOR___}); // insert program mapping
           ___TEMP___VECTOR___.clear(); // clear for next program
        }
        return ___RESULT___;
    }

        /*
         ___M___COMPILE___(__CODE___, ___GIVE___STATUS___)
         1) Parameters:
            - ___CODE___ : const std::string&  -> full source code text
            - ___GIVE___STATUS___ : bool (optional, default=false)
                If true, prints a human-readable dump of parsed programs and instructions to stdout.
         2) Description:
            - Top-level helper that runs:
                 extract programs -> extract program lines -> convert lines to instructions
              and returns the final compiled mapping program -> vector<instruction>.
            - If ___GIVE___STATUS___ is true, prints a formatted dump of the result for debugging.
         3) Visualization:
                Input: raw source text
                Pipeline:
                  extract programs -> split into lines -> parse instructions -> output final map
                Visual:
                +----------------------------------+
                | SOURCE TEXT -> [programs] -> [lines] -> [instructions] |
                +----------------------------------+
         4) Per-line:
            - Compose the three-step pipeline using the static helpers above, optionally print status.
        */
        ___STATIC__MEMBER___ ___UMAP___STRING___VECTOR___INSTRUCTIONS___T___
        ___M___COMPILE___
        (___CONST___ ___STRING___& ___CODE___,
            ___CONST___ ___BOOLEAN___ ___GIVE___STATUS___ = false
        )
        ___NO___EXCEPTION___
    {

        // Run the pipeline: extract programs -> split into lines -> parse into instructions
        auto ___RESULT___ = DICC_Compiler::___M___CONVERT___TO___INSTRUCTIONS___FINALIZER___
        (DICC_Compiler::___M___EXTRACT___PORGRAM___LINES___
        (DICC_Compiler::___M___EXTRACT___PROGRAMS___(___CODE___))
        );

    // If status requested, print a formatted dump of programs and their parsed instructions
    if (___GIVE___STATUS___) {
    for (auto item : ___RESULT___) {
        std::cout << "|===============================================|" << "\n\n";
        std::cout << "PROGRAM_NAME: " << item.first << "\n";
        std::cout << "PROGRAM_CODE: " << "\n\n";
        int idx = 0;
        for (auto __item : item.second) {
            idx++;
            std::cout << "     LINE[" << idx << "]: \n";
            std::cout << "          ___VM___WILL___EXECUTE___: " << __item.___VM___WILL___EXECUTE___ << '\n';
            std::cout << "          ___TODO___: " << __item.___TODO___ << '\n';
            std::cout << "          ___PARAMETERS___: ";
            bool ____mask = false;
            for (auto ____item : __item.___PARAMETERS___)
            {
                if (____mask) std::cout << ", ";
                ____mask = true;
                std::cout << '\'' << ____item << '\'';
            }
            std::cout << '\n';
            std::cout << "          ___RETURN___ADDRESS____: " << __item.___RETURN___ADDRESS___ << "\n\n";
        }
    }
    std::cout << "|===============================================|";
    }

    return ___RESULT___;
    }

};
#define compileVerlight DICC_Compiler::___M___COMPILE___
#define compilerCollection DICC_Compiler

#endif

