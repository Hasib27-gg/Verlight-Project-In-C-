#include "includes.h"

#ifndef ___VERLIGHT___COMPILER___H___
#define ___VERLIGHT___COMPILER___H___


#define  ___STATIC__MEMBER___ static
#define ___CONST___ const
#define ___NO___EXCEPTION___ noexcept
#define ___OPERATOR__SET___ {',' , '!' ,'.'  , ':' , '@' , '(' , ')' , '{' , '}' , '[' , ']' , '$' , '-'  , '~' , '<' , '>', ';' , '"' , '_'}

#ifndef ___HAS___DECLARED___TYPES___
#define ___HAS___DECLARED___TYPED___ 1

typedef bool ___BOOLEAN___;
typedef char ___CHAR___;
typedef std::vector<std::string> ___STRING___VECTOR___;
typedef robin_hood::unordered_set<char> ___CHAR___SET___;
typedef std::string ___STRING___;
typedef robin_hood::unordered_map<std::string, std::string> ___UMAP___STRING___STRING___T___;
typedef robin_hood::unordered_map<std::string, std::vector<std::string>> ___UMAP___STRING___VECTOR___STRING___T___;

struct ___INSTRUCTION___T___
{
    ___STRING___ ___VM___WILL___EXECUTE___ = "true";
    ___STRING___ ___TODO___ = "nan";
    ___STRING___VECTOR___ ___PARAMETERS___ = { "nan" };
    ___STRING___ ___RETURN___ADDRESS___ = "nullptr";
};

typedef  robin_hood::unordered_map<std::string, std::vector<___INSTRUCTION___T___>> ___UMAP___STRING___VECTOR___INSTRUCTIONS___T___;
typedef std::vector<___INSTRUCTION___T___> ___INSTRUCTIONS___VECTOR___T___;

#endif

class DICC_Compiler {

public:

    ___STATIC__MEMBER___ ___BOOLEAN___ ___M___IS___VALID___OPERATOR___
    (___CONST___ ___CHAR___ ___C)
        ___NO___EXCEPTION___
    {
    ___CHAR___SET___ ___PA___CONST___OPERATORS___ = ___OPERATOR__SET___;
    return ___PA___CONST___OPERATORS___.find(___C) != ___PA___CONST___OPERATORS___.end();
    }

        ___STATIC__MEMBER___ ___UMAP___STRING___STRING___T___ ___M___EXTRACT___PROGRAMS___
        (___CONST___ ___STRING___& __CODE__)
        ___NO___EXCEPTION___
    {
        ___UMAP___STRING___STRING___T___ __RESULT__;

    ___BOOLEAN___ ___PHASE___PRGNAME___ = false;
    ___BOOLEAN___ ___PHASE___PRGDEF___ = false;
    ___BOOLEAN___ ___IGSPACES___ = true;
    ___STRING___ __TEMP__WORD__ = "";
    ___STRING___ __TEMP__CODE__ = "";

    for (auto item : __CODE__)
    {
        if (___PHASE___PRGNAME___ && !std::isalpha(item) && item != '_')
        {
            if (!__TEMP__WORD__.empty())
                __RESULT__.insert({__TEMP__WORD__ , __TEMP__CODE__});
            ___PHASE___PRGNAME___ = false;
        }

        if (___PHASE___PRGDEF___ && item == '}')
        {
            __RESULT__[__TEMP__WORD__] = __TEMP__CODE__;
            __TEMP__CODE__.clear();
            __TEMP__WORD__.clear();
            ___PHASE___PRGDEF___ = false;
        }

        if (___PHASE___PRGNAME___)
            __TEMP__WORD__ += ___STRING___(1 , item);

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

        if (item == '"' && ___IGSPACES___) {
            ___IGSPACES___ = false;
            continue;
        }

        if (item == '"' && !___IGSPACES___)
        ___IGSPACES___ = true;

        if (item == '{' && !___PHASE___PRGDEF___)
            ___PHASE___PRGDEF___ = true;

        if (item == '#' && !___PHASE___PRGNAME___)
            ___PHASE___PRGNAME___ = true;
    }
    return __RESULT__;
    }

        ___STATIC__MEMBER___ ___UMAP___STRING___VECTOR___STRING___T___
        ___M___EXTRACT___PORGRAM___LINES___
        (___CONST___ ___UMAP___STRING___STRING___T___& ___CODE___)
    {
        ___UMAP___STRING___VECTOR___STRING___T___ __RESULT__;

        ___STRING___ ___TEMP___STRING___ = "";
        ___STRING___VECTOR___ ___TEMP___VECTOR___(0, "");

        for (auto item : ___CODE___)
        {
            ___BOOLEAN___ ___PHASE___READING___QUOTE___ = false;

            for (auto __item : item.second) {

                if (__item != ';' ||
                    (
                        __item == ';' && ___PHASE___READING___QUOTE___
                        )
                    )
                    ___TEMP___STRING___ += ___STRING___(1, __item);

                else if (!___PHASE___READING___QUOTE___ && __item == ';')
                {
                    ___TEMP___STRING___ += ___STRING___(1, __item);

                    if (!___TEMP___STRING___.empty())
                        ___TEMP___VECTOR___.push_back(___TEMP___STRING___);

                    ___TEMP___STRING___.clear();
                }

                if (__item == '"' && !___PHASE___READING___QUOTE___) {
                    ___PHASE___READING___QUOTE___ = true;
                    continue;
                }

                if (__item == '"' && ___PHASE___READING___QUOTE___)
                    ___PHASE___READING___QUOTE___ = false;

            }

            __RESULT__.insert({ item.first , ___TEMP___VECTOR___ });
            ___TEMP___VECTOR___.clear();
        }

        return __RESULT__;
    }

    ___STATIC__MEMBER___ ___INSTRUCTION___T___
        ___M___READ___HIGH___LEVEL___INSTRUCTION___
        (___CONST___ ___STRING___& ___CODE___)
        ___NO___EXCEPTION___
    {
        ___INSTRUCTION___T___ ___RESULT___;

    ___RESULT___.___VM___WILL___EXECUTE___ = "";
    ___RESULT___.___TODO___ = "";
    ___RESULT___.___PARAMETERS___ = {};
    ___RESULT___.___RETURN___ADDRESS___ = "";

    ___BOOLEAN___ ___PHASE___READ___WILLEXE___ = false;
    ___BOOLEAN___ ___PHASE___READ___TODO___ = false;
    ___BOOLEAN___ ___PHASE___READ___PARAMETERS___ = false;
    ___BOOLEAN___ ___PHASE___READ___RETURNADD___ = false;
    ___BOOLEAN___ ___PHASE___READING___QUOTE___ = false;
    ___BOOLEAN___ ___PHASE___READING___BRACKETS___ = false;
    ___STRING___  ___TEMP___STRING___ = "";

    for (auto item : ___CODE___)
    {

        if
        (item == '<'
            && !___PHASE___READ___WILLEXE___
            && !___PHASE___READING___QUOTE___
        )
        {
            ___PHASE___READ___WILLEXE___ = true;
            continue;
        }

        if
        (item == '>'
            && ___PHASE___READ___WILLEXE___
            && !___PHASE___READING___QUOTE___
        )
        {
            ___PHASE___READ___WILLEXE___ = false;
            continue;
        }

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


        if (___PHASE___READ___WILLEXE___ && item != ' ')
        ___RESULT___.___VM___WILL___EXECUTE___ += ___STRING___(1 , item);

        if (___PHASE___READ___TODO___ && item != ' ')
        ___RESULT___.___TODO___ += ___STRING___(1 , item);

        if (___PHASE___READ___RETURNADD___ && item != ' ')
        ___RESULT___.___RETURN___ADDRESS___ += ___STRING___(1 , item);


        if (___PHASE___READ___PARAMETERS___)
        {
            if (
                item != ',' ||
                (
                    item == ',' &&
                    (
                    ___PHASE___READING___QUOTE___ ||
                     ___PHASE___READING___BRACKETS___
                    )
                )
            )
                ___TEMP___STRING___ += ___STRING___(1 , item);



            else if
            (
                item == ','
                && !___PHASE___READING___BRACKETS___
                && !___PHASE___READING___QUOTE___
            )
            {

                if (!___TEMP___STRING___.empty())
                    ___RESULT___.___PARAMETERS___.push_back(___TEMP___STRING___);

                ___TEMP___STRING___.clear();
            }
        }

        if (item == '"' && !___PHASE___READING___QUOTE___)
        {
            ___PHASE___READING___QUOTE___ = true;
            continue;
        }

        if (item == '"' && ___PHASE___READING___QUOTE___)
            ___PHASE___READING___QUOTE___ = false;


        if (item == '[' && !___PHASE___READING___BRACKETS___ && !___PHASE___READING___QUOTE___)
        {
            ___PHASE___READING___BRACKETS___ = true;
            continue;
        }

        if (item == ']' && ___PHASE___READING___BRACKETS___ && !___PHASE___READING___QUOTE___)
        ___PHASE___READING___BRACKETS___ = false;


    }

    if (___RESULT___.___VM___WILL___EXECUTE___ == "")
    ___RESULT___.___VM___WILL___EXECUTE___ = "true";

    if (___RESULT___.___RETURN___ADDRESS___ == "")
    ___RESULT___.___RETURN___ADDRESS___ = "nullptr";

    return ___RESULT___;
    }

        ___STATIC__MEMBER___ ___UMAP___STRING___VECTOR___INSTRUCTIONS___T___
        ___M___CONVERT___TO___INSTRUCTIONS___FINALIZER___
        (___CONST___ ___UMAP___STRING___VECTOR___STRING___T___& ___CODE___)
        ___NO___EXCEPTION___
    {
        ___UMAP___STRING___VECTOR___INSTRUCTIONS___T___ ___RESULT___;
        ___INSTRUCTIONS___VECTOR___T___ ___TEMP___VECTOR___;

        for (auto item : ___CODE___)
        {
            for (auto __item : item.second)
            ___TEMP___VECTOR___.push_back
           (___M___READ___HIGH___LEVEL___INSTRUCTION___(__item));

           ___RESULT___.insert({item.first , ___TEMP___VECTOR___});
           ___TEMP___VECTOR___.clear();
        }
        return ___RESULT___;
    }

        ___STATIC__MEMBER___ ___UMAP___STRING___VECTOR___INSTRUCTIONS___T___
        ___M___COMPILE___
        (___CONST___ ___STRING___& ___CODE___,
            ___CONST___ ___BOOLEAN___ ___GIVE___STATUS___ = false
        )
        ___NO___EXCEPTION___
    {

        auto ___RESULT___ = DICC_Compiler::___M___CONVERT___TO___INSTRUCTIONS___FINALIZER___
        (DICC_Compiler::___M___EXTRACT___PORGRAM___LINES___
        (DICC_Compiler::___M___EXTRACT___PROGRAMS___(___CODE___))
        );

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
