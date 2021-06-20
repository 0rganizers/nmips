/*
 *  This is the nanoMIPS plugin module.
 *  It extends the MIPS processor module to disassemble nanoMIPS instructions.
 *
 */

#include "nmips.hpp"
#include <ida.hpp>
#include <idp.hpp>
#include <bytes.hpp>
#include <loader.hpp>
#include <kernwin.hpp>
#include <stdarg.h>
#include "log.hpp"
#include "nanomips-dis.h"
#include <allins.hpp>
#include <ua.hpp>
#include "constants.hpp"
#include "pro.h"
#include <loader.hpp>

int data_id;

static const char node_name[] = "nanoMIPS Hooked";

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

//--------------------------------------------------------------------------
// Return the instruction mnemonics
const char *plugin_ctx_t::get_insn_mnem(const insn_t &insn)
{
    struct nanomips_opcode op = {};
    nanomips_decoded_op operands[MAX_NUM_OPS] = {};
    size_t insn_size = nanomips_disasm_instr(insn.ea, &disasm_info, &op, operands);
    //LOG("Decoded instruction of size: %d", insn_size);

    return op.name;
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
            ctx->out_line(get_insn_mnem(insn), COLOR_INSN);
            return 1;
            } else {
            // LOG("instruction @0x%x, is not custom", insn.ea);
            }
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
// In this example we check the processor type and make the decision.
// You may or may not check any other conditions to decide what you do:
// whether your plugin wants to work with the database or not.
static plugmod_t *idaapi init()
{
    processor_t &ph = PH;
    if ( ph.id != PLFM_MIPS )
        return nullptr;
    auto plugmod = new plugin_ctx_t;
    set_module_data(&data_id, plugmod);
    return plugmod;
}

//-------------------------------------------------------------------------
plugin_ctx_t::plugin_ctx_t()
{
    nec_node.create(node_name);
    hooked = nec_node.altval(0) != 0;
    if ( hooked )
    {
        hook_event_listener(HT_IDP, this);
        msg("nanoMIPS processor support is enabled\n");
    }
    if (!supported_machine())
    {
        LOG("Warning: enabled, but this is not a nanoMIPS elf!");
    }
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
    // listeners are uninstalled automatically
    // when the owner module is unloaded
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
    PLUGIN_PROC           // this is a processor extension plugin
    | PLUGIN_MULTI,         // this plugin can work with multiple idbs in parallel
    init,                 // initialize
    nullptr,
    nullptr,
    comment,              // long comment about the plugin. not used.
    help,                 // multiline help about the plugin. not used.
    desired_name,         // the preferred short name of the plugin
    desired_hotkey        // the preferred hotkey to run the plugin
};
