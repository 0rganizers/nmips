/*
 *  This is the nanoMIPS plugin module.
 *  It extends the MIPS processor module to disassemble nanoMIPS instructions.
 *
 */

#include "nmips.hpp"
#include <algorithm>
#include <config.hpp>
#include <cstddef>
#include <ida.hpp>
#include <idp.hpp>
#include <bytes.hpp>
#include <loader.hpp>
#include <iterator>
#include <kernwin.hpp>
#include <map>
#include <nalt.hpp>
#include <stdarg.h>
#include "elf_ldr.hpp"
#include "funcs.hpp"
#include "hexrays.hpp"
#include "log.hpp"
#include "mopt.hpp"
#include "nanomips-dis.h"
#include <allins.hpp>
#include <ua.hpp>
#include "constants.hpp"
#include "pro.h"
#include <loader.hpp>
#include "ins.hpp"
#include <vector>

#include "elf/elfbase.h"
#include "elf/elf.h"
#include "loguru.hpp"
#include "segment.hpp"
#include "segregs.hpp"
#include "typeinf.hpp"
#include "reg.hpp"
#include <dbg.hpp>
#include <lex.hpp>


// Need these for mprotecting stuff!
#if !defined(_MSC_VER)

#include <unistd.h>
#include <sys/mman.h>

#else

#include <windows.h>

#endif

int data_id;

static const char node_name[] = "$ nanoMIPS Hooked";

std::vector<std::string> nanomips_gpr_names = {
  "zero", "at",   "t4",   "t5",   "a0",   "a1",   "a2",   "a3",
  "a4",   "a5",   "a6",   "a7",   "t0",   "t1",   "t2",   "t3",
  "s0",   "s1",   "s2",   "s3",   "s4",   "s5",   "s6",   "s7",
  "t8",   "t9",   "k0",   "k1",   "gp",   "sp",   "fp",   "ra"
}; 

/**
 * @brief  Used for disassembling, reads memory via ida's api.
 * @note   
 * @param  memaddr: 
 * @param  *myaddr: 
 * @param  length: 
 * @param  *info: 
 * @retval 
 */
int ida_read_memory (bfd_vma memaddr,
		    bfd_byte *myaddr,
		    unsigned int length,
		    struct disassemble_info *info)
{
    size_t nbytes = get_bytes(myaddr, length, memaddr);
    if (nbytes < length) {
        ERR("Read %d, expected %d", nbytes, length);
        return EIO;
    }
    return 0;
}

int ida_printf(void*, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vmsg(fmt, args);
    va_end(args);
    return ret;
}

bool supported_machine()
{
    ea_t machine_ea = get_fileregion_ea(ELF_MACHINE_OFFSET);
    if (machine_ea != BADADDR) {
        ushort machine_id = get_word(machine_ea);
        LOG("Loaded with ID: 0x%x", machine_id);
        return machine_id == ELF_NANOMIPS;
    }
    return false;
}

uint32 get_feature(insn_t& insn)
{
    nanomips_extra_inst_t inst = (nanomips_extra_inst_t)insn.itype;
    auto it = nanomips_insn.find(inst);
    if (it != nanomips_insn.end())
    {
        return it->second.features;
    }

    if (insn.itype >= CUSTOM_INSN_ITYPE) {
        // Otherwise we would segfault!
        return 0;
    }

    return insn.get_canon_feature(PH);
}

//--------------------------------------------------------------------------
// Return the instruction mnemonics
const char *plugin_ctx_t::get_insn_mnem(const insn_t &insn)
{
    if (insn.itype == nMIPS_todo)
    {
        struct nanomips_opcode op = {};
        nanomips_decoded_op operands[MAX_NUM_OPS] = {};
        size_t insn_size = nanomips_disasm_instr(insn.ea, &disasm_info, &op, operands);
        //LOG("Decoded instruction of size: %d", insn_size);

        return op.name;
    }

    auto it = nanomips_insn.find((nanomips_extra_inst_t)insn.itype);
    if (it != nanomips_insn.end()) {
        return it->second.mnemonic;
    }

    return "unknown";
}

//--------------------------------------------------------------------------
// This function can be hooked to various kernel events.
// In this particular plugin we hook to the HT_IDP group.
// As soon the kernel needs to decode and print an instruction, it will
// generate some events that we intercept and provide our own response.
//
// We extend the processor module to disassemble opcode 0x0F
// (This is a hypothetical example)
// There are 2 different possible approaches for the processor extensions:
//  A. Quick & dirty
//       Implement reaction to ev_ana_insn and ev_out_insn.
//       The first checks if the instruction is valid.
//       The second generates its text.
//  B. Thourough and clean
//       Implement all relevant callbacks.
//       ev_ana_insn fills the 'insn' structure.
//       ev_emu_insn creates all xrefs using ua_add_[cd]ref functions.
//       ev_out_insn generates the textual representation of the instruction.
//          It is required only if the instruction requires special processing
//          or the processor module cannot handle the custom instruction for
//          any reason.
//       ev_out_operand generates the operand representation (only if the
//          operand requires special processing).
//       ev_out_mnem generates the instruction mnemonics.
// The main difference between these 2 approaches is in the creation of
// cross-references and the amount of special processing required by the
// new instructions.

// The quick & dirty approach.
// We just produce the instruction mnemonics along with its operands.
// No cross-references are created. No special processing.
ssize_t idaapi plugin_ctx_t::on_event(ssize_t code, va_list va)
{
    // need to check, to enable if we load an old file and hence don't get an ELF callback
    if (code == processor_t::ev_oldfile)
    {
        char* fname = va_arg(va, char*);
        LOG("loaded old file %s", fname);
        load_from_idb();
        
        return 0;
    }

    if (code == processor_t::ev_loader_elf_machine)
    {
        linput_t* li = va_arg(va, linput_t*);
        int machine_type = va_arg(va, int);
        const char** p_procname = va_arg(va, const char**);
        proc_def_t** p_pd = va_arg(va, proc_def_t**);
        LOG("loader_elf_machine(0x%x)", machine_type);
        if (machine_type == ELF_NANOMIPS) {
            LOG("nanoMIPS elf detected!");
            *p_procname = "mipsl";
            // LOG("loader_elf_machine(%p)", *p_pd);
            elf_mips_t* elf_mips = (elf_mips_t*)*p_pd;
            elf_nmips = new elf_nanomips_t(elf_mips, relocations);
            *p_pd = elf_nmips;
            // Forcibly enable plugin
            enable_plugin(true);

            return ELF_NANOMIPS;
        }
    }
    
    if (!hooked) return 0;

    switch ( code )
    {
        case processor_t::ev_ana_insn:
        {   
            insn_t *insn = va_arg(va, insn_t *);
            size_t length = ana(*insn);
            if ( length )
            {
                insn->size = (uint16)length;
                return insn->size;       // event processed
            }
        }
        break;
        case processor_t::ev_out_mnem:
        {
            outctx_t *ctx = va_arg(va, outctx_t *);
            const insn_t &insn = ctx->insn;
            if ( insn.itype >= CUSTOM_INSN_ITYPE )
            {
                ctx->out_custom_mnem(get_insn_mnem(insn));
                return 1;
            }
        }
        break;
        case processor_t::ev_is_switch:
        {
            switch_info_t* si = va_arg(va, switch_info_t*);
            const insn_t* insn = va_arg(va, const insn_t*);

            return is_switch(si, insn) ? 1 : 0;
        }
        break;
        case processor_t::ev_may_be_func:
        {
            insn_t* insn = va_arg(va, insn_t*);
            int state = va_arg(va, int);

            int ret = may_be_func(insn, state);
            return ret;
        }
        break;
        case processor_t::ev_emu_insn:
        {
            insn_t *insn = va_arg(va, insn_t*);
            return emu(*insn);
        }
        break;
        case processor_t::ev_is_basic_block_end:
        {
            insn_t *insn = va_arg(va, insn_t *);
            if (insn->itype < nMIPS_todo) return 0;
            uint32 feature = get_feature(*insn);
            if ((feature & CF_JUMP) == CF_JUMP) return 1;
            return -1;
        }
        break;
        case processor_t::ev_delay_slot_insn:
        {
            ea_t *ea = va_arg(va, ea_t *);
            *ea = BADADDR;
            // We don't have delay slots in nanoMIPS, so can safely always return -1 here!
            return -1;
        }
        break;
        case processor_t::ev_is_cond_insn:
        {
            insn_t *insn = va_arg(va, insn_t *);
            if (insn->itype > nMIPS_todo)
            {
                switch (insn->itype) {
                case nMIPS_bltic:
                case nMIPS_bltiuc:
                case nMIPS_beqic:
                case nMIPS_bgeic:
                case nMIPS_bgeiuc:
                case nMIPS_bneic:
                case nMIPS_bltc:
                case nMIPS_bltuc:
                case nMIPS_bgec:
                case nMIPS_bgeuc:
                case nMIPS_beqc:
                case nMIPS_bnec:
                case nMIPS_bgezc:
                case nMIPS_blezc:
                    return 1;

                default:
                    return -1;
                }
            }
        }
        break;
        case processor_t::ev_is_call_insn:
        {
            insn_t* insn = va_arg(va, insn_t*);
            if (insn->itype < nMIPS_todo) return 0;
        }
        break;
        case processor_t::ev_get_reg_name:
        {
            qstring *buf = va_arg(va, qstring *);
            int reg = va_arg(va, int);
            size_t width = va_arg(va, size_t);
            if (reg >= nanomips_gpr_names.size()) return -1;
            std::string reg_name = nanomips_gpr_names[reg];
            if (buf == NULL)
                buf = new qstring;
            buf->append(reg_name.c_str(), reg_name.size());
            return reg_name.size();
        }
        break;
        case processor_t::ev_str2reg:
        {
            const char* regname = va_arg(va, const char*);
            std::string regs(regname);
            std::transform(regs.begin(), regs.end(), regs.begin(),
                [](unsigned char c){ return std::tolower(c); });
            auto it = std::find(nanomips_gpr_names.begin(), nanomips_gpr_names.end(), regs);
            if (it != nanomips_gpr_names.end())
            {
                return std::distance(nanomips_gpr_names.begin(), it) + 1;
            } else {
                // WARN("Could not convert %s to a reg num", regname);
            }
        }
        break;
        case processor_t::ev_calc_retloc:
        {
            argloc_t *retloc    = va_arg(va, argloc_t *);
            const tinfo_t *type = va_arg(va, const tinfo_t *);
            cm_t cc             = va_argi(va, cm_t);
            bool arg = false;
            return calc_retloc(retloc, *type, cc, arg) ? 1 : -1;
        }
        break;
        case processor_t::ev_get_cc_regs:
        {
            callregs_t* callregs = va_arg(va, callregs_t*);
            cm_t cc = va_arg(va, cm_t);
            const int *regs;
            get_arg_regs(&regs);
            // TODO: Figure out why this only works if ARGREGS_FP_CONSUME_GP is set!
            callregs->set(ARGREGS_FP_CONSUME_GP, regs, NULL);
            return 1;
        }
        break;
        case processor_t::ev_calc_arglocs:
        {
            func_type_data_t *fti = va_arg(va, func_type_data_t *);
            return calc_arglocs(fti) ? 1 : -1;
        }
        break; 
        case processor_t::ev_calc_varglocs:
        {
            func_type_data_t *fti = va_arg(va, func_type_data_t *);
            regobjs_t *regargs    = va_arg(va, regobjs_t *);
            relobj_t *stkargs = va_arg(va, relobj_t *);
            int nfixed = va_arg(va, int);
            // LOG("calc_varglocs(%p, %p, %p, %d)", fti, regargs, stkargs, nfixed);
            return calc_varglocs(fti, regargs, nfixed) ? 1 : -1;
        }
        break;

        case processor_t::ev_next_exec_insn:
        {
            ea_t* target = va_arg(va, ea_t*);
        }
        break;
        case processor_t::ev_calc_next_eas:
        {
            eavec_t* res = va_arg(va, eavec_t*);
            insn_t* insn = va_arg(va, insn_t*);
            bool over = va_arg(va, bool);
            uint32 feature = get_feature(*insn);
            bool is_jump = (feature & CF_JUMP) == CF_JUMP;
            bool is_call = (feature & CF_CALL) == CF_CALL;
            bool is_stop = (feature & CF_STOP) == CF_STOP;
            int call_count = 0;
            if (!over) {
                if (is_call) {
                    if (insn->itype == MIPS_jalrc) {
                        // indirect!
                        return -1;
                    }
                    if (insn->itype == MIPS_bal) {
                        res->push_back(insn->Op1.addr);
                        call_count++;
                    }
                }
            }
            //TODO: more?
            bool flow = !is_stop;
            if (flow) {
                ea_t next = insn->ea + insn->size;
                res->push_back(next);
            }
            if (insn->itype == MIPS_b || insn->itype == nMIPS_bc) {
                ea_t next = insn->Op1.addr;
                res->push_back(next);
            }
            // Did I misinterpret the CF_JUMP flag?
            if (!is_jump) {
                for (int i = 3; i >= 0; i--) {
                    if (insn->ops[i].type == o_near) {
                        // very risky lmao
                        res->push_back(insn->ops[i].addr);
                        break;
                    }
                }
            }

            return call_count;

        }
        break;
        case processor_t::ev_assemble:
        {
            // Does not work :(
            // output buffer
            uchar* bin = va_arg(va, uchar*);
            ea_t ea = va_arg(va, ea_t);
            ea_t cs = va_arg(va, ea_t);
            ea_t ip = va_arg(va, ea_t);
            bool use32 = va_arg(va, bool);
            const char* line = va_arg(va, const char*);
            LOG("assemble %s at 0x%x / 0x%x", line, ea, ip);
        }
        break;
    }
    return 0;                     // event is not processed
}

//--------------------------------------------------------------------------
// Initialize the plugin.
// IDA will call this function only once.
// If this function returns nullptr, IDA will unload the plugin.
// Otherwise the plugin returns a pointer to a newly created context structure.
//
// To be able to hook the ELF callback, we always instantiate the plugin, but might not intercept an IDP events yet!
static plugmod_t *idaapi init()
{
    // Add the ability to assemble things :)
    // get_ph()->flag |= PR_ASSEMBLE;
    const char* log_file = get_plugin_options("nmips_log_file");
    int argc = 0;
    if (log_file != nullptr)
        loguru::add_file(log_file, loguru::Truncate, loguru::Verbosity_MAX);
    LOG("Logging to log file %s", log_file);
    // LOG("Assembler: %s", get_ph()->assemblers[0]->name);

    auto plugmod = new plugin_ctx_t;
    set_module_data(&data_id, plugmod);
    return plugmod;
}

//-------------------------------------------------------------------------
plugin_ctx_t::plugin_ctx_t()
{
    relocations = new elf_nanomips_relocations_t;
    // Always hook IDP for ELF callback.
    hook_event_listener(HT_IDP, this);
    // get_ph()->flag |= PR_ASSEMBLE;
    // LOG("Assembler: %s", get_ph()->assemblers[0]->name);

    load_from_idb();

    init_disassemble_info(&disasm_info, NULL, ida_printf);
    disasm_info.arch = bfd_arch_nanomips;
    disasm_info.mach = bfd_mach_nanomipsisa32r6;

    disasm_info.read_memory_func = ida_read_memory;

    disassemble_init_for_target(&disasm_info);
}

//--------------------------------------------------------------------------
// Terminate the plugin.
// This destructor will be called before unloading the plugin.
plugin_ctx_t::~plugin_ctx_t()
{
    clr_module_data(data_id);
    delete mgen;
    // listeners are uninstalled automatically
    // when the owner module is unloaded
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

int config_gdb_plugin_t::activate(action_activation_ctx_t *)
{
    const dbg_info_t* dbg_plugins = NULL;
    size_t num = get_debugger_plugins(&dbg_plugins);
    LOG("Have %d dbg plugins", num);

    debugger_t* gdb_dbg = nullptr;

    for (int i = 0; i < num; i++)
    {
        const dbg_info_t* plugin = &dbg_plugins[i];
        // LOG("plugin: %s, %s, %s", plugin->pi->org_name, plugin->pi->path, plugin->dbg->name);
        if (strcmp(plugin->dbg->name, "gdb") == 0) gdb_dbg = plugin->dbg;
    }

    if (gdb_dbg != nullptr)
    {
        // Note, that the opening parenthesis will be added by gdb itself!
        const char configs[] = R"(
            "nanoMIPS":                [ 12,      0,        4,       0,         "mipsl",        "nanomips",         "nanomips-linux.xml",    "1010",     0 ]
        })";
        auto res = set_dbg_option(gdb_dbg, "CONFIGURATIONS", IDPOPT_CST, configs);
        if (res != NULL)
            LOG("failed setting option CONFIGURATIONS: %s", res);

        const char ida_features[] = R"(
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
                }
            }
        )";
        res = set_dbg_option(gdb_dbg, "IDA_FEATURES", IDPOPT_CST, ida_features);
        if (res != NULL)
            LOG("failed setting option IDA_FEATURES: %s", res);

        const char arch_map[] = R"(
                "nanomips":         [ 12,       0,    0,   -1 ]
            }
        )";

        res = set_dbg_option(gdb_dbg, "ARCH_MAP", IDPOPT_CST, arch_map);
        if (res != NULL)
            LOG("failed setting option ARCH_MAP: %s", res);

        const char feature_map[] = R"(
                "org.gnu.gdb.nanomips.cpu":       [ 12,       0,    0,   -1 ]
            }
        )";

        res = set_dbg_option(gdb_dbg, "FEATURE_MAP", IDPOPT_CST, feature_map);
        if (res != NULL)
            LOG("failed setting option FEATURE_MAP: %s", res);

        res = set_dbg_option(gdb_dbg, "DEFAULT_CONFIG", IDPOPT_STR, "nanoMIPS");
        if (res != NULL)
            LOG("failed setting option DEFAULT_CONFIG: %s", res);

        uval_t val = 0;
        res = set_dbg_option(gdb_dbg, "NOACK_MODE", IDPOPT_NUM, &val);
        if (res != NULL)
            LOG("failed setting option NOACK_MODE: %s", res);
        //set_dbg_options(gdb_dbg, "DEFAULT_CONFIG", IDPOPT_PRI_HIGH, IDPOPT_CST, "nanoMIPS");
        //LOG("setting option res: %p", res);
    } else {
        LOG("GDB plugin not loaded!");
    }

    return 1;
}

void plugin_ctx_t::ensure_mgen_installed()
{
    if (this->did_check_hexx) return;
    TRACE("Installing mgen filter!");

    if ( !init_hexrays_plugin() )
    {
        TRACE("Hexrays not detected, not installing microcode filter!");
        return;
    }

    this->did_check_hexx = true;

    mgen = new nmips_microcode_gen_t;
    bool result = install_microcode_filter(mgen);
    if (!result)
    {
        ERR("Failed to install microcode filter!");
    } else {
        LOG("Successfully installed mgen filter!");
    }

    install_optinsn_handler(&mopt);

    segment_t* got = get_segm_by_name(".got");
    LOG("Found got segment: 0x%x", got->start_ea);
    got_location = got->start_ea;

    if (!set_default_sreg_value(NULL, GP_SREG, got_location))
    {
        ERR("failed to set default sreg value to: 0x%x", got_location);
    } else {
        LOG("Successfully set default sreg value to: 0x%x", got_location);
    }
}

size_t page_size()
{
#if defined(_MSC_VER)
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
#else
    return getpagesize();
#endif
}

bool protect_data(void* addr, size_t size, bool write_enable)
{
#if defined(_MSC_VER)
    DWORD old_prot = 0;
    DWORD flags = PAGE_READONLY;
    if (write_enable) flags = PAGE_READWRITE;
    bool ret = VirtualProtect(addr, size, flags, &old_prot);
    return ret;
#else
    int flags = PROT_READ;
    if (write_enable) flags |= PROT_WRITE;
    int ret = mprotect(addr, size, flags);
    return ret != -1;
#endif
}

void plugin_ctx_t::enable_plugin(bool enable)
{
    if (enable) {
        relocations->enable_hooks(true);
        // this is very hacky, but I think needed so that we can change the names everywhere :/
        size_t idx = 0;
        const char** reg_names = (const char**)PH.reg_names;

        // Pls ilfak let me override the register names fully, so I don't have to do this.
        void* reg_addr = (void*) reg_names;
        size_t mask =  ~(page_size() - 1);
        void* reg_page = (void*)((size_t)reg_addr & mask);
        protect_data(reg_page, page_size()*2, true);
        // TODO: error checking
        processor_t* curr = get_ph();
        LOG("Processor: %d", curr->id);
        size_t max_regs = curr->regs_num;
        // LOG("Assembler: %s", get_ph()->assemblers[0]->name);
        for (auto &name: nanomips_gpr_names)
        {
            if (idx >= max_regs) {
                WARN("Processor only has %ld registers, but we expect %ld! Are you sure you loaded a mips processor?", max_regs, nanomips_gpr_names.size());
            }
            reg_names[idx] = name.c_str();
            idx++;
        }

        // I am too lazy to write a cross platform way to detect if the page was actually already writeable before.
        // Blame Ilfak for this one.
        // protect_data(reg_page, page_size()*2, false);

        bool res = register_action(config_gdb_plugin_desc);
        if (!res) {
            ERR("Failed to register gdb action");
        }
        res = attach_action_to_menu("Debugger/Debugger Options", "nmips:ConfigGDB", 0);
        if (!res) {
            ERR("Failed to attach action to menu");
        }
    } else {
        relocations->enable_hooks(false);
        unregister_action("nmips:ConfigGDB");
    }
    hooked = enable;
    nec_node.create(node_name);
    nec_node.altset(0, hooked);
}

void plugin_ctx_t::load_from_idb()
{
    nec_node.create(node_name);
    bool enable = nec_node.altval(0);
    enable_plugin(enable);
    relocations->load_from_idb();
}

//--------------------------------------------------------------------------
// The plugin method
// This is the main function of plugin.
// It will be called when the user selects the plugin from the menu.
// The input argument is usually zero. Non-zero values can be specified
// by using load_and_run_plugin() or through plugins.cfg file (discouraged).
// It forcibly disables / enables the plugin.
bool idaapi plugin_ctx_t::run(size_t)
{
    enable_plugin(!hooked);
    info("AUTOHIDE NONE\n"
        "nanoMIPS processor support is %s", hooked ? "enabled" : "disabled");
    return true;
}

//--------------------------------------------------------------------------
static const char comment[] = "nanoMIPS processor support";
static const char help[] = "Hooks analysis events to provide nanoMIPS support in IDA.\n";

//--------------------------------------------------------------------------
// This is the preferred name of the plugin module in the menu system
// The preferred name may be overridden in plugins.cfg file

static const char desired_name[] = "nanoMIPS processor support";

// This is the preferred hotkey for the plugin module
// The preferred hotkey may be overridden in plugins.cfg file

static const char desired_hotkey[] = "";

//--------------------------------------------------------------------------
//
//      PLUGIN DESCRIPTION BLOCK
//
//--------------------------------------------------------------------------
plugin_t PLUGIN =
{
    IDP_INTERFACE_VERSION,
    PLUGIN_FIX           // this is a processor extension plugin
    | PLUGIN_MULTI,         // this plugin can work with multiple idbs in parallel
    init,                 // initialize
    nullptr,
    nullptr,
    comment,              // long comment about the plugin. not used.
    help,                 // multiline help about the plugin. not used.
    desired_name,         // the preferred short name of the plugin
    desired_hotkey        // the preferred hotkey to run the plugin
};
