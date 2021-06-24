#ifndef __ELF_LDR_H
#define __ELF_LDR_H

#include <pro.h>
#include <elf/elfbase.h>
#include <elf/elf.h>

struct plugin_ctx_t;

struct elf_nanomips_t : public proc_def_t
{
public:
    elf_nanomips_t(elf_mips_t* base) : proc_def_t(base->ldr, base->reader), base(base) {};
    elf_mips_t* base;
    sel_t got_location = 0;
    plugin_ctx_t* plugin = nullptr;

    // Overridden from elf_mips_t
    virtual const char *proc_handle_reloc(
            const rel_data_t &rel_data,
            const sym_rel *symbol,
            const elf_rela_t *reloc,
            reloc_tools_t *tools) override;
    virtual bool proc_create_got_offsets(
            const elf_shdr_t *gotps,
            reloc_tools_t *tools) override;
    virtual const char *proc_describe_flag_bit(uint32 *e_flags) override;
    virtual bool proc_load_unknown_sec(Elf64_Shdr *sh, bool force) override;
    virtual int proc_handle_special_symbol(
            sym_rel *st,
            const char *name,
            ushort type) override;
    virtual const char *proc_handle_dynamic_tag(const Elf64_Dyn *dyn) override;
    virtual bool proc_is_acceptable_image_type(ushort filetype) override;
    virtual void proc_on_start_data_loading(elf_ehdr_t &header) override;
    virtual bool proc_on_end_data_loading() override;
    virtual bool proc_handle_symbol(sym_rel &sym, const char *symname) override;
    virtual void proc_handle_dynsym(
            const sym_rel &symrel,
            elf_sym_idx_t isym,
            const char *symname) override;
    virtual bool proc_on_create_section(
            const elf_shdr_t &sh,
            const qstring &name,
            ea_t *sa) override;
    virtual const char *calc_procname(uint32 *e_flags, const char *procname) override;

    // Overridden from proc_def_t
    virtual ea_t proc_adjust_entry(ea_t entry) override;
    virtual bool proc_can_convert_pic_got() const override;
    virtual size_t proc_convert_pic_got(
            const segment_t *gotps,
            reloc_tools_t *tools) override;
    virtual bool proc_should_load_section(
        const elf_shdr_t &sh,
        elf_shndx_t idx,
        const qstring &name) override;
    virtual void proc_on_loading_symbols() override;
    virtual bool proc_perform_patching(
        const elf_shdr_t *plt,
        const elf_shdr_t *gotps) override;
    virtual bool proc_supports_relocs() const override;
};

#endif /* __ELF_LDR_H */
