#ifndef __GDB_H
#define __GDB_H

// #define dont_use_snprintf snprintf
// #include <sstream>
// #include <iostream>

#include <ida.hpp>
#include <dbg.hpp>
#include <kernwin.hpp>
#include <pro.h>
#include <parsejson.hpp>

struct plugin_ctx_t;
struct config_gdb_plugin_t : public action_handler_t
{
    config_gdb_plugin_t() {}
    virtual int idaapi activate(action_activation_ctx_t *) override;
    virtual action_state_t idaapi update(action_update_ctx_t *) override
    {
        return AST_ENABLE_ALWAYS;
    }

    const char* set_dbg_option(debugger_t *dbg, const char *keyword, int type, const void *value);
};

struct config_patcher_t
{
    const char* filename = NULL;

    int previous_line = 0;

    bool remove = false;

    qstring output;
    qstring input;
    qvector<qstring> input_lines;

    config_patcher_t(const char* filename) : filename(filename) {}

    bool run(bool remove);
    const char* handle(lexer_t* lx, const token_t &keyword, const token_t &value);
    void append_missing_lines(int current);

    void add_lines_between(int startline, int endline, const char* lines, qstring before_marker = "");
    void append_marked(const char* lines);
    void append_lines(int startline, lexer_t* lx, const token_t &value, const char* lines, qstring before_marker = "");

    error_t parse_json(lexer_t* lx, const token_t &value, jvalue_t *val);

    void rid_of_json(lexer_t* lx, const token_t &value);

    int get_line(lexer_t* lx);
};

#endif /* __GDB_H */
