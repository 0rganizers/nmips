#ifndef __ELF_LDR_H
#define __ELF_LDR_H

#include <pro.h>
#include <elf/elfbase.h>
#include <elf/elf.h>
#include <idp.hpp>

struct plugin_ctx_t;

/**
 * @brief Symbol encountered by the elf_nanomips_t loader
 * We save this, so that when rebasing the GOT / extern segment, we can correctly update the locations.
 */
struct got_symbol_t
{
    /**
     * @brief Offset from the base of the GOT section, where this symbol is.
     * This might be invalid after a rebasing of the got segment.
     */
    ea_t got_offset = BADADDR;

    /**
     * @brief Actual address of the symbol in the got.
     * We use both, got_symbol_t::got_offset and this, since we might not yet know the start of the got when we encounter a symbol.
     */
    ea_t got_addr;

    /**
     * @brief Offset from the base of the extern section, where this symbol is.
     * This might be invalid after a rebasing of the extern segment.
     */
    ea_t extern_offset = BADADDR;

    /**
     * @brief Actual address of the symbol in the extern section.
     * We use both, got_symbol_t::extern_offset and this, since we might not yet know the start of the extern section when we encounter a symbol.
     */
    ea_t extern_addr;

    /**
     * @brief Name of the symbol.
     * 
     */
    const char* name;
};

/**
 * @brief Keeps track of relocations, so that when we rebase the program, we can update them correctly.
 * This is a separate class to elf_nanomips_t, because we need to have this even without the ELF loader.
 * This happens if we open an existing IDB and want to continue working on something.
 * It saves the relocation information inside th IDB, so we can rebase even loading from an IDB!
 */
struct elf_nanomips_relocations_t : public event_listener_t
{
public:
    elf_nanomips_relocations_t();

    ea_t got_base = BADADDR;
    ea_t extern_base = BADADDR;

    qvector<got_symbol_t> relocated_symbols;

    virtual ssize_t idaapi on_event(ssize_t code, va_list va) override;

    void enable_hooks(bool enable);

    void segments_updated();

    /**
     * @brief Change the internally managed got base address.
     * If not previously set, it will calculate the correct GOT offsets for got symbols.
     * It also updates the GP segmentation register, so that relocations are correctly detected.
     * @param new_base 
     */
    void update_got_base(ea_t new_base);

    /**
     * @brief Change the internally managed extern base address.
     * If not previously set, it will calculate the correct extern offsets for got symbols.
     * DOES not automatically patch the GOT again, you have to do that yourself by calling elf_nanomips_t::patch_got().
     * @param new_base 
     */
    void update_extern_base(ea_t new_base);

    /**
     * @brief Patches up the symbols in the got, so that they are resolved to the extern symbols.
     * Can be called whenever really.
     */
    void patch_got();

    /**
     * @brief Patches a single symbol inside the GOT.
     * 
     * @param symbol 
     */
    void patch_got_symbol(got_symbol_t& symbol);

    void set_offsets(got_symbol_t& got_sym);

    void save_to_idb();
    void load_from_idb();

private:
    /**
     * @brief Storage inside the IDB.
     * 
     */
    netnode storage;
    netnode symbol_storage;
};

/**
 * @brief  ELF loader responsible for handling nanoMIPS relocations.
 * @note   It uses elf_mips_t* as a proxy for most functions.
 */
struct elf_nanomips_t : public proc_def_t
{
public:
    elf_nanomips_t(elf_mips_t* base, elf_nanomips_relocations_t* relocations) : proc_def_t(base->ldr, base->reader), base(base), relocations(relocations) {};
    elf_mips_t* base;
    elf_nanomips_relocations_t* relocations;

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
