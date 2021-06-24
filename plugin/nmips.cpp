/*
 *  This is the nanoMIPS plugin module.
 *  It extends the MIPS processor module to disassemble nanoMIPS instructions.
 *
 */

#include "nmips.hpp"
#include <algorithm>
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

int data_id;

static const char node_name[] = "$ nanoMIPS Hooked";

#define GPR_A0 (4)
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

uint32 get_feature(nanomips_extra_inst_t inst)
{
    auto it = nanomips_insn.find(inst);
    if (it != nanomips_insn.end())
    {
        return it->second.features;
    }

    return 0;
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

void plugin_ctx_t::on_hooked()
{
    // this is very hacky, but I think needed so that we can change the names everywhere :/
    size_t idx = 0;
    const char** reg_names = (const char**)PH.reg_names;
    for (auto &name: nanomips_gpr_names)
    {
        reg_names[idx] = name.c_str();
        idx++;
    }
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
    if (code == processor_t::ev_oldfile)
    {
        char* fname = va_arg(va, char*);
        LOG("loaded old file %s", fname);
        
        nec_node.create(node_name);
        hooked = nec_node.altval(0);
        if (hooked)
        {
            on_hooked();
        }
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
            /*elf_loader_t loader(li, 0);
            reader_t reader(li);
            elf_nmips = new elf_nanomips_t(loader, reader);*/
            LOG("loader_elf_machine(%p)", *p_pd);
            elf_mips_t* elf_mips = (elf_mips_t*)*p_pd;
            elf_nmips = new elf_nanomips_t(elf_mips);
            elf_nmips->plugin = this;
            *p_pd = elf_nmips;
            // ensure we hook next time as well!
            hooked = true;
            nec_node.create(node_name);
            nec_node.altset(0, hooked);
            on_hooked();
            return ELF_NANOMIPS;
        }
    }
    if (!hooked) return 0;
    switch ( code )
    {
        case processor_t::ev_ana_insn:
        {
            // msg("Hooked analysis event");
            
            insn_t *insn = va_arg(va, insn_t *);
            // LOG("[0x%x] analysis", insn->ea);
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
            // msg("Hooked out event");
            outctx_t *ctx = va_arg(va, outctx_t *);
            const insn_t &insn = ctx->insn;
            if ( insn.itype >= CUSTOM_INSN_ITYPE )
            {
                ctx->out_custom_mnem(get_insn_mnem(insn));
                return 1;
            } else {
            // LOG("instruction @0x%x, is not custom", insn.ea);
            }
        }
        break;
        case processor_t::ev_out_operand:
        {
            outctx_t* outctx = va_arg(va, outctx_t*);
            op_t* op = va_arg(va, op_t*);
            
            // LOG("[0x%x] out_operand (%d)", outctx->insn_ea, outctx->insn.itype);
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
            // LOG("[0x%x] may_be_func = %d", insn->ea, ret);
            return ret;
        }
        break;
        case processor_t::ev_emu_insn:
        {
            insn_t *insn = va_arg(va, insn_t*);
            // LOG("[0x%x] emu", insn->ea);
            return emu(*insn);
        }
        break;
        case processor_t::ev_is_basic_block_end:
        {
            insn_t *insn = va_arg(va, insn_t *);
            // if (insn != NULL)
                // LOG("[0x%x] is_basic_block_end", insn->ea);
            if (insn->itype < nMIPS_todo) return 0;
            uint32 feature = get_feature((nanomips_extra_inst_t)insn->itype);
            if ((feature & CF_JUMP) == CF_JUMP) return 1;
            return -1;
        }
        break;
        case processor_t::ev_delay_slot_insn:
        {
            ea_t *ea = va_arg(va, ea_t *);
            // if (ea != NULL)
                // LOG("[0x%x] delay_slot_insn", *ea);
            *ea = BADADDR;
            // We don't have delay slots in nanoMIPS, so can safely always return -1 here!
            return -1;
        }
        break;
        case processor_t::ev_is_cond_insn:
        {
            insn_t *insn = va_arg(va, insn_t *);
            // LOG("[0x%x] is_cond_insn", insn->ea);
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
            // LOG("get_reg_name: reg: %d, width: %ld, buf: %p", reg, width, buf);
            if (reg >= nanomips_gpr_names.size()) return -1;
            std::string reg_name = nanomips_gpr_names[reg];
            if (buf == NULL)
                buf = new qstring;
            buf->append(reg_name.c_str(), reg_name.size());
            return reg_name.size();
            // 
        }
        break;
        case processor_t::ev_str2reg:
        {
            const char* regname = va_arg(va, const char*);
            auto it = std::find(nanomips_gpr_names.begin(), nanomips_gpr_names.end(), regname);
            if (it != nanomips_gpr_names.end())
            {
                return std::distance(nanomips_gpr_names.begin(), it) + 1;
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
            // LOG("get_cc_regs(%p)", callregs);
            get_arg_regs(&regs);
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
        case processor_t::ev_calc_cdecl_purged_bytes:
        // calculate number of purged bytes after call
        {
            // ea_t ea                     = va_arg(va, ea_t);
            return 0;
        }
        break;
        case processor_t::ev_get_stkarg_offset:
        {
            return 0;
        }                            // get offset from SP to the first stack argument
        break;                           // args: none
                                    // returns: the offset+2
    }
    return 0;                     // event is not processed
}

//--------------------------------------------------------------------------
// Initialize the plugin.
// IDA will call this function only once.
// If this function returns nullptr, IDA will unload the plugin.
// Otherwise the plugin returns a pointer to a newly created context structure.
//
// In this example we check the processor type and make the decision.
// You may or may not check any other conditions to decide what you do:
// whether your plugin wants to work with the database or not.
static plugmod_t *idaapi init()
{
    const char* log_file = get_plugin_options("nmips_log_file");
    int argc = 0;
    // loguru::init(argc, nullptr);
    if (log_file != nullptr)
        loguru::add_file(log_file, loguru::Truncate, loguru::Verbosity_MAX);
    LOG("Logging to log file %s", log_file);
    processor_t &ph = PH;
    /*if ( ph.id != PLFM_MIPS )
        return nullptr;*/
    auto plugmod = new plugin_ctx_t;
    set_module_data(&data_id, plugmod);
    return plugmod;
}

//-------------------------------------------------------------------------
plugin_ctx_t::plugin_ctx_t()
{
    bool result = nec_node.create(node_name);
    if (result)
    {
        LOG("Created new nec_node");
    } else {
        LOG("Nec node already exists");
    }
    hooked = nec_node.altval(0) != 0;
    if ( hooked || true)
    {
        hook_event_listener(HT_IDP, this);
        msg("nanoMIPS processor support is enabled\n");
    }
    if (hooked)
    {
        on_hooked();
    }
    /*if (!supported_machine())
    {
        LOG("Warning: enabled, but this is not a nanoMIPS elf!");
    }*/
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

void plugin_ctx_t::ensure_mgen_installed()
{
    if (this->did_check_hexx) return;
    LOG("Installing mgen filter!");

    if ( !init_hexrays_plugin() )
    {
        ERR("Hexrays not detected, not installing microcode filter!");
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

    if (!set_default_sreg_value(NULL, 0x44, got_location))
    {
        ERR("failed to set default sreg value to: 0x%x", got_location);
    } else {
        LOG("Successfully set default sreg value to: 0x%x", got_location);
    }
}

//--------------------------------------------------------------------------
// The plugin method
// This is the main function of plugin.
// It will be called when the user selects the plugin from the menu.
// The input argument is usually zero. Non-zero values can be specified
// by using load_and_run_plugin() or through plugins.cfg file (discouraged).
bool idaapi plugin_ctx_t::run(size_t)
{
    if ( hooked )
        unhook_event_listener(HT_IDP, this);
    else
        hook_event_listener(HT_IDP, this);
    hooked = !hooked;
    nec_node.create(node_name);
    nec_node.altset(0, hooked);
    if (hooked)
    {
        on_hooked();
    }
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
