#include "gdb.hpp"
#include "nmips.hpp"
#include "log.hpp"
#include <config.hpp>
#include <diskio.hpp>
#include <fpro.h>
#include <kernwin.hpp>
#include <lex.hpp>
#include <loader.hpp>
#include <pro.h>
#include <err.h>
#include <parsejson.hpp>

const char* test_handler(lexer_t* lex, const token_t &keyword, const token_t &value)
{
    int linenum = 0;
    const char* lineptr = NULL;
    lex_get_file_line(lex, &linenum, &lineptr, 0);
    LOG("Handler: %s / %d", keyword.str.c_str(), value.type);
    LOG("Line: %d: %s", linenum, lineptr);
    return NULL;
}

int config_gdb_plugin_t::activate(action_activation_ctx_t *ctx)
{
    int ret = ask_yn(ASKBTN_NO, "WARNING: This will modify your dbg_gdb.cfg file, and potentially brick it! Are you sure you want to proceed?");
    if (ret != ASKBTN_YES) return 1;
    ret = ask_buttons("Add", "Remove", "Cancel", ASKBTN_YES, "Do you want to add or remove the nanoMIPS configuration to the gdb configuration?");
    bool remove = false;
    if (ret == ASKBTN_CANCEL) return 1;
    if (ret == ASKBTN_NO) remove = true;

    LOG("terminating debuggers");
    term_plugins(PLUGIN_DBG);

    config_patcher_t patcher("dbg_gdb.cfg");
    if (!patcher.run(remove)) {
        WARN("Failed to patch config file!");
    }

    init_plugins(PLUGIN_DBG);

    LOG("Reloading debuggers again to be sure");
    term_plugins(PLUGIN_DBG);
    init_plugins(PLUGIN_DBG);

    return 1;
}

const char* config_gdb_plugin_t::set_dbg_option(debugger_t *dbg, const char *keyword, int type, const void *value)
{
    const void* set_val = value;
    lexer_t* lexer = NULL;
    if (type == IDPOPT_CST) {
        lexer = create_lexer(NULL, 0);
        lex_init_string(lexer, (const char*)value);
        set_val = lexer;
    }
   
    auto res = set_dbg_options(dbg, keyword, IDPOPT_PRI_HIGH, type, set_val);

    if (type == IDPOPT_CST) {
        destroy_lexer(lexer);
    }

    if (res == IDPOPT_OK) return NULL;
    if (res == IDPOPT_BADKEY) return "bad key";
    if (res == IDPOPT_BADTYPE) return "bad type";
    if (res == IDPOPT_BADVALUE) return "bad value";

    return res;
}

static const cfgopt_t empty_options[] =
{ //   name               varptr    type/bit
};

// hate globals
static config_patcher_t* current_patcher = nullptr;
static const char* config_patcher_handler(lexer_t* lx, const token_t &keyword, const token_t &value)
{
    if (current_patcher == nullptr) return "No patcher active!";
    return current_patcher->handle(lx, keyword, value);
}

static const cfgopt_t gdb_options[] =
{ //   name               varptr    type/bit
  cfgopt_t("CONFIGURATIONS", (cfgopt_handler_t*)config_patcher_handler),
};

bool config_patcher_t::run(bool remove)
{
    this->remove = remove;
    char filepath[0x4000];
    char* idk = getsysfile(filepath, 0x4000-1, this->filename, CFG_SUBDIR);
    if (idk == NULL) {
        WARN("Failed to find config file %s", filename);
        return false;
    }
    LOG("Found config file at %s", filepath);
    qstring backup_path;
    backup_path.append(filepath);
    backup_path.append(".bak");
    LOG("Backing up original file to %s", backup_path.c_str());
    int ret = qcopyfile(filepath, backup_path.c_str());
    if (ret != 0) {
        WARN("Failed to backup file: %s", qerrstr(ret));
        return false;
    }
    FILE* config = fopenRT(filepath);
    qstring current;
    while (qgetline(&current, config) != -1) {
        input.append(current);
        input.append("\n");
        input_lines.push_back(current);
    }
    qfclose(config);

    current_patcher = this;
    ret = read_config(input.c_str(), false, empty_options, qnumber(empty_options), config_patcher_handler);
    // For some reason, we still return false here, not sure why?
    // if (ret != 0) {
    //     WARN("Failed to read config file: %s", qerrstr(ret));
    //     return false;
    // }
    current_patcher = nullptr;
    // Append final input lines
    append_missing_lines(input_lines.size()-1);
    LOG("Current output: %s", output.c_str());

    WARN("Writing above output to dbg_gdb.cfg");
    config = fopenWT(filepath);
    qfwrite(config, output.c_str(), output.size());
    qfclose(config);
    LOG("Done Writing!");
    // LOG("Read config file: %s", input.c_str());
    return true;
}

void config_patcher_t::append_missing_lines(int current)
{
    for (int line = previous_line; line < current; line++) {
        output.append(input_lines[line]);
        output.append("\n");
    }
    
    previous_line = current;
}

error_t config_patcher_t::parse_json(lexer_t *lx, const token_t &value, jvalue_t *val)
{
    tokenstack_t stk;
    // token_t open = {};
    // open.num = '{';
    // open.type = lx_char;
    // stk.push(open);
    // token_t empty = {};
    stk.push(value);

    error_t err = ::parse_json(val, lx, &stk);
    LOG("Stack after parsing: %ld", stk.size());
    if (err != 0) {
        WARN("Failed to parse json: %s", qerrstr(err));
        return err;
    }

    LOG("Parsed json");

    return 0;
}

int config_patcher_t::get_line(lexer_t* lx)
{
    int linenum = 0;
    const char* lineptr = NULL;
    lex_get_file_line(lx, &linenum, &lineptr, 0);

    return linenum;
}

#define MARKER_START "// !!!WARNING DO NOT DELETE THIS!!! nmips PATCH START MARKER "
#define MARKER_END "// !!!WARNING DO NOT DELETE THIS!!! nmips PATCH END MARKER "

void config_patcher_t::append_marked(const char *lines)
{
    // easiest way to remove :)
    if (remove) return;
    output.append(MARKER_START);
    output.append("\n");
    output.append(lines);
    output.append("\n");
    output.append(MARKER_END);
    output.append("\n");
}

void config_patcher_t::add_lines_between(int startline, int endline, const char *lines, qstring before_marker)
{
    // check if already present, if so remove
    qvector<qstring> current_lines;
    int insertat = endline;
    bool skip = false;

    // Remove marked section if present from lines.
    for (int line = startline; line < endline; line++) {
        qstring curr = input_lines[line];
        if (curr.find(MARKER_START) != qstring::npos) {
            skip = true;
        }
        if (before_marker.size() > 0 && curr.find(before_marker) != qstring::npos) {
            insertat = current_lines.size();
        }
        if (!skip) {
            current_lines.push_back(curr);
        }
        if (curr.find(MARKER_END) != qstring::npos) {
            skip = false;
        }
    }

    int curr_pos = 0;
    bool inserted = false;

    for (auto &line: current_lines) {
        if (curr_pos == insertat) {
            append_marked(lines);
            inserted = true;
        }

        output.append(line);
        output.append("\n");
        curr_pos++;
    }

    if (!inserted) append_marked(lines);
}

void config_patcher_t::rid_of_json(lexer_t *lx, const token_t &value)
{
    jvalue_t val;
    error_t err = parse_json(lx, value, &val);
}

void config_patcher_t::append_lines(int startline, lexer_t *lx, const token_t &value, const char *lines, qstring before_marker)
{
    rid_of_json(lx, value);
    int endline = get_line(lx);

    add_lines_between(startline, endline, lines, before_marker);
    previous_line = endline;
}

const char* config_line = R"("nanoMIPS":                [ PLFM_MIPS,      0,        4,       0,         "mipsl",        "nanomips",         "nanomips-linux.xml",    "1010",     1 ],)";
const char* feature_line = R"(
  "nanomips":
  {
    "org.gnu.gdb.nanomips.cpu":
    {
      "title": "General registers",
      "rename":
      {
        "r0":  "zero",
        "r1":  "at",
        "r2":  "t4",
        "r3":  "t5",
        "r4":  "a0",
        "r5":  "a1",
        "r6":  "a2",
        "r7":  "a3",
        "r8":  "a4",
        "r9":  "a5",
        "r10": "a6",
        "r11": "a7",
        "r12": "t0",
        "r13": "t1",
        "r14": "t2",
        "r15": "t3",
        "r16": "s0",
        "r17": "s1",
        "r18": "s2",
        "r19": "s3",
        "r20": "s4",
        "r21": "s5",
        "r22": "s6",
        "r23": "s7",
        "r24": "t8",
        "r25": "t9",
        "r26": "k0",
        "r27": "k1",
        "r28": "gp",
        "r29": "sp",
        "r30": "fp",
        "r31": "ra"
      },
      "stack_ptr": "sp",
      "frame_ptr": "fp",
      "code_ptr": "pc",
      "data_ptr":
      [
        "zero",
        "at",
        "t4", "t5",
        "a0", "a1", "a2", "a3",
        "a4", "a5", "a6", "a7", "t0", "t1", "t2", "t3",
        "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
        "t8", "t9",
        "k0", "k1",
        "gp", "sp", "ra"
      ]
    },
    "org.gnu.gdb.nanomips.fpu":
    {
      "title": "FPU registers"
    },
    "org.gnu.gdb.nanomips.cp0":
    {
      "title": "CP0 registers"
    },
    "org.gnu.gdb.nanomips.dsp":
    {
      "title": "DSP registers"
    },
    "org.gnu.gdb.nanomips.linux":
    {
      "title": "Linux registers"
    }
  },
  )";

const char* arch_line = R"(  "nanomips":         [ PLFM_MIPS,       0,    0,   -1 ],)";
const char* feature_map_line = R"(  "org.gnu.gdb.nanomips.cpu":       [ PLFM_MIPS,       0,    0,   -1 ],)";

const char* config_patcher_t::handle(lexer_t *lx, const token_t &keyword, const token_t &value)
{
    int startline = get_line(lx);
    append_missing_lines(startline);
    qstring token_str;
    lex_print_token(&token_str, &value);
    LOG("Handler: %s / %s", keyword.str.c_str(), token_str.c_str());

    qstring key = keyword.str;

    if (key == "CONFIGURATIONS") {
        append_lines(startline, lx, value, config_line, "MIPS Little-endian");
    }
    if (key == "DEFAULT_CONFIG") {
        rid_of_json(lx, value);
    }
    if (key == "IDA_FEATURES") {
        append_lines(startline, lx, value, feature_line, R"("mips":)");
    }
    if (key == "ARCH_MAP") {
        append_lines(startline, lx, value, arch_line, R"("mips":)");
    }
    if (key == "FEATURE_MAP") {
        append_lines(startline, lx, value, feature_map_line, R"("org.gnu.gdb.mips.cpu":)");
    }
    if (key == "UNIQUE_REGISTER_MAP") {
        rid_of_json(lx, value);
    }

    return NULL;
}