#ifndef __NMIPS_H
#define __NMIPS_H

#include "nanomips-dis.h"
#include <ida.hpp>
#include <idp.hpp>
#include <map>
#include <set>
#include "mgen.hpp"

enum nanomips_extra_inst_t
{
    nMIPS_todo = CUSTOM_INSN_ITYPE,

    // save / restore
    nMIPS_save,
    nMIPS_restore_jrc,

    // branch instructions
    nMIPS_bc,
    nMIPS_beqic,
    nMIPS_bgeic,
    nMIPS_bgeiuc,
    nMIPS_bltic,
    nMIPS_bltiuc,
    nMIPS_bneic,
};


uint32 get_feature(nanomips_extra_inst_t inst);


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
    bool did_check_hexx = false;

    std::map<ea_t, size_t> fake_jrc_insn;

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

    int emu(insn_t &insn);

    void ensure_mgen_installed();
};

#endif /* __NMIPS_H */
