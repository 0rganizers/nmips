#ifndef __NMIPS_H
#define __NMIPS_H

#include "mopt.hpp"
#include "nanomips-dis.h"
#include <ida.hpp>
#include <idp.hpp>
#include <map>
#include <set>
#include "mgen.hpp"
#include "ins.hpp"
#include "elf_ldr.hpp" 

uint32 get_feature(insn_t& inst);

struct insn_analysis_state_t
{
    bool seen_dest = false;
    unsigned int dest_reg = 0;
    unsigned int last_reg = 0;
    bfd_vma last_int = 0;
    bool last_instr_48bits = false;

    bool need_fake_nop_because_mips_is_trash = false;

    void record_register(unsigned int reg);
    void record_int(bfd_vma val);
};

struct plugin_ctx_t;
struct config_gdb_plugin_t : public action_handler_t
{
    plugin_ctx_t &plg;
    config_gdb_plugin_t(plugin_ctx_t &p) : plg(p) {}
    virtual int idaapi activate(action_activation_ctx_t *) override;
    virtual action_state_t idaapi update(action_update_ctx_t *) override
    {
        return AST_ENABLE_ALWAYS;
    }

    const char* set_dbg_option(debugger_t *dbg, const char *keyword, int type, const void *value);
};

//--------------------------------------------------------------------------
// Context data for the plugin. This object is created by the init()
// function and hold all local data.
struct plugin_ctx_t : public plugmod_t, public event_listener_t
{
   /**
    * @brief  Stores whether the plugin is active or not inside the database.
    */
    netnode nec_node;

   /**
    * @brief  Whether we currently hooked the processor module or not.
    */
    bool hooked = false;

   /**
    * @brief  Used for disassembling instructions.
    */
    struct disassemble_info disasm_info = {};

    // state analyzing instruction.
    insn_analysis_state_t ana_state = {};

    nmips_microcode_gen_t* mgen = nullptr;
    large_stk_opt_t mopt;
    bool did_check_hexx = false;

    std::map<ea_t, size_t> fake_jrc_insn;
    // some instructions combine two ordinary mips instructions.
    // if we encounter them, we write out the first to ida, and store the second here.
    std::map<ea_t, insn_t> fake_secondary_insn;

    elf_nanomips_t* elf_nmips = nullptr;
    elf_nanomips_relocations_t* relocations = nullptr;

    sel_t got_location = 0;

    bool calc_arglocs_recursion = false;

    /**
     * Actions.
     * 
     */
    config_gdb_plugin_t config_gdb_plugin_ah = config_gdb_plugin_t(*this);
  const action_desc_t config_gdb_plugin_desc = ACTION_DESC_LITERAL_PLUGMOD(
        "nmips:ConfigGDB",
        "Configure GDB",
        &config_gdb_plugin_ah,
        this,
        "Ctrl+Shift+Meta+G",
        NULL,
        -1);

    plugin_ctx_t();
    ~plugin_ctx_t();

    
   /**
    * @brief  This function is called when the user invokes the plugin.
    */
    virtual bool idaapi run(size_t) override;
    // This function is called upon some events.
    virtual ssize_t idaapi on_event(ssize_t code, va_list va) override;

   /**
    * @brief  Analyze the given instruction and fill it if we can disassemble the location.
    * @note   Can change insn.
    * @param  &insn: The instruction.
    * @retval The size of the instruction in bytes.
    */
    size_t ana(insn_t &insn);

   /**
    * @brief  Get the mnemonic for the given instruction.
    * @note   If the type of the instruction is TODO, disassemble again, then just return the name as is.
    * @param  &insn: The instruction.
    * @retval Mnemonic of the instruction.
    */
    const char* get_insn_mnem(const insn_t &insn);

   /**
    * @brief  Sets the opcode of insn based on the opcode decoded in op.
    * @note   
    * @param  &insn: The instruction to fill.
    * @param  op: The opcode of the disassembled instruction.
    * @retval Whether we could correctly map the instruction.
    */
    bool fill_opcode(insn_t &insn, struct nanomips_opcode& op);

   /**
    * @brief  Converts the given nanomips operand to an ida operand.
    * @note   
    * @param  &insn: The instruction where we want to store the result operand.
    * @param  op: The decoded operand.
    * @param  idx: The idx of the operand in the instruction.
    * @retval next operand index
    */
    int fill_operand(insn_t &insn, struct nanomips_opcode& opcode, nanomips_decoded_op& op, int idx);

    void post_process(insn_t &insn);

    void handle_operand(insn_t &insn, op_t &op);

    bool is_switch(switch_info_t* si, const insn_t* insn);

    int may_be_func(insn_t* insn, int state);

    bool prev_insn(insn_t& insn, ea_t curr);

    int emu(insn_t &insn);

    void ensure_mgen_installed();

    insn_t* add_fake_secondary(insn_t& curr);

   /**
    * @brief  Enable / disable the plugin from working.
    * @note   The plugin always hooks the IDP events, to get a callback when ELF files are loaded.
    *         However, it is only automatically active in nanoMIPS ELF files.
    *         This function manually enables / disables the plugin for working in other files.
    *         It is used by the automatic ELF detection as well though.
    * @param  enable: 
    * @retval None
    */
    void enable_plugin(bool enable);

   /**
    * @brief  Loads the plugin from the IDB.
    * @note   This loads the plugin information from an IDB.
    *         It is called at multiple points, but sometimes the idb is not loaded yet.
    * @retval None
    */
    void load_from_idb();

};

#endif /* __NMIPS_H */
