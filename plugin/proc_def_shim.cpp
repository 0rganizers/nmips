#include "elf_ldr.hpp"
#include "elf/elf.h"
#include "log.hpp"

proc_def_t::proc_def_t(elf_loader_t &ldr, reader_t &reader) : ldr(ldr), reader(reader) {}

const char *proc_def_t::proc_handle_reloc(const rel_data_t &rel_data, const sym_rel *symbol, const elf_rela_t *reloc, reloc_tools_t *tools)
{
    return nullptr;
}

bool proc_def_t::proc_create_got_offsets(const elf_shdr_t *gotps, reloc_tools_t *tools)
{
    return false;
}

const char *proc_def_t::proc_describe_flag_bit(uint32 *e_flags)
{
    return nullptr;
}

bool proc_def_t::proc_load_unknown_sec(Elf64_Shdr *sh, bool force)
{
    return false;
}

int proc_def_t::proc_handle_special_symbol(sym_rel *st, const char *name, ushort type)
{
    return 0;
}

const char *proc_def_t::proc_handle_dynamic_tag(const Elf64_Dyn *dyn)
{
    return nullptr;
}

bool proc_def_t::proc_is_acceptable_image_type(ushort filetype)
{
    return false;
}

void proc_def_t::proc_on_start_data_loading(elf_ehdr_t &header)
{

}

bool proc_def_t::proc_on_end_data_loading()
{
    return false;
}

bool proc_def_t::proc_handle_symbol(sym_rel &sym, const char *symname)
{
    return false;
}

void proc_def_t::proc_handle_dynsym(const sym_rel &symrel, elf_sym_idx_t isym, const char *symname)
{
    
}

bool proc_def_t::proc_on_create_section(const elf_shdr_t &sh, const qstring &name, ea_t *sa)
{
    return false;
}

const char *proc_def_t::calc_procname(uint32 *e_flags, const char *procname)
{
    return nullptr;
}

ea_t proc_def_t::proc_adjust_entry(ea_t entry)
{
    return 0;
}

bool proc_def_t::proc_can_convert_pic_got() const
{
    return false;
}

size_t proc_def_t::proc_convert_pic_got(const segment_t *gotps, reloc_tools_t *tools)
{
    return 0;
}

bool proc_def_t::proc_should_load_section(const elf_shdr_t &sh, elf_shndx_t idx, const qstring &name)
{
    return false;
}

void proc_def_t::proc_on_loading_symbols()
{

}

bool proc_def_t::proc_perform_patching(const elf_shdr_t *plt, const elf_shdr_t *gotps)
{
    return false;
}

bool proc_def_t::proc_supports_relocs() const
{
    return false;
}