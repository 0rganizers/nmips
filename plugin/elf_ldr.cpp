#include "elf_ldr.hpp"
#include "bytes.hpp"
#include "elf/elf.h"
#include "funcs.hpp"
#include "idp.hpp"
#include "log.hpp"
#include "name.hpp"
#include "ua.hpp"
#include "segregs.hpp"
#include "nmips.hpp"

const char *elf_nanomips_t::proc_handle_reloc(const rel_data_t &rel_data, const sym_rel *symbol, const elf_rela_t *reloc, reloc_tools_t *tools)
{   
    LOG("handle_relocation(0x%x, 0x%x, 0x%x, t: %d): %s, %s, 0x%x", rel_data.P, rel_data.S, rel_data.Sadd, rel_data.type, symbol->name.c_str(), symbol->original_name.c_str(), symbol->value);
    if (rel_data.type == 10)
    {
        auto got_entry = rel_data.P;
        // hopefully this is actually correct lmao.
        patch_dword(got_entry, rel_data.S);
        return "R_NANOMIPS_GLOBAL";
    }
    if (rel_data.type == 11)
    {
        // this is not really true, but whatever, it works lmao.
        uval_t A = rel_data.P - tools->got.start_ea;
        auto got_entry = rel_data.P;
        ea_t target_fn = rel_data.S;
        patch_dword(got_entry, target_fn);
        // if (!set_default_sreg_value(NULL, str2reg("gp"), tools->got.start_ea))
        // {
        //     ERR("failed to set default sreg value to: 0x%x", tools->got.start_ea);
        // }
        // func_t* fn = get_func(target_fn);
        // if (fn != NULL)
        // {
        //     qstring buf;
        //     get_ea_name(&buf, target_fn);
        //     LOG("[0x%x] got_fn: %s", target_fn, buf.c_str());
        // }
        // get_flags(got_entry);
        return "R_NANOMIPS_JUMP_SLOT";
    }

    return base->proc_handle_reloc(rel_data, symbol, reloc, tools);
}

bool elf_nanomips_t::proc_create_got_offsets(const elf_shdr_t *gotps, reloc_tools_t *tools)
{
    LOG("create_got_offsets(0x%llx)", gotps->sh_addr);
    return base->proc_create_got_offsets(gotps, tools);
}

const char *elf_nanomips_t::proc_describe_flag_bit(uint32 *e_flags)
{
    LOG("describe_flag_bit(%x)", *e_flags);
    return base->proc_describe_flag_bit(e_flags);
}

bool elf_nanomips_t::proc_load_unknown_sec(Elf64_Shdr *sh, bool force)
{
    LOG("load_unknown_sec(0x%llx)", sh->sh_addr);
    return base->proc_load_unknown_sec(sh, force);
}

int elf_nanomips_t::proc_handle_special_symbol(sym_rel *st, const char *name, ushort type)
{
    return base->proc_handle_special_symbol(st, name, type);
}

const char *elf_nanomips_t::proc_handle_dynamic_tag(const Elf64_Dyn *dyn)
{
    return base->proc_handle_dynamic_tag(dyn);
}

bool elf_nanomips_t::proc_is_acceptable_image_type(ushort filetype)
{
    return base->proc_is_acceptable_image_type(filetype);
}

void elf_nanomips_t::proc_on_start_data_loading(elf_ehdr_t &header)
{
    base->proc_on_start_data_loading(header);
}

bool elf_nanomips_t::proc_on_end_data_loading()
{
    return base->proc_on_end_data_loading();
}

bool elf_nanomips_t::proc_handle_symbol(sym_rel &sym, const char *symname)
{
    return base->proc_handle_symbol(sym, symname);
}

void elf_nanomips_t::proc_handle_dynsym(const sym_rel &symrel, elf_sym_idx_t isym, const char *symname)
{
    return base->proc_handle_dynsym(symrel, isym, symname);
}

bool elf_nanomips_t::proc_on_create_section(const elf_shdr_t &sh, const qstring &name, ea_t *sa)
{
    LOG("on_create_section(%s): 0x%x (0x%llx)", name.c_str(), *sa, sh.sh_addr);
    // set gp pointer
    if (name == ".got")
    {
        got_location = sh.sh_addr;
        plugin->got_location = got_location;
        LOG("saved got location: 0x%x", plugin->got_location);
        if (!set_default_sreg_value(NULL, 0x44, plugin->got_location))
        {
            ERR("failed to set default sreg value to: 0x%x", got_location);
        }
    }
    return base->proc_on_create_section(sh, name, sa);
}

const char *elf_nanomips_t::calc_procname(uint32 *e_flags, const char *procname)
{
    LOG("calc_procname(%s)", procname);
    return base->calc_procname(e_flags, procname);
}

ea_t elf_nanomips_t::proc_adjust_entry(ea_t entry)
{
    return base->proc_adjust_entry(entry);
}

bool elf_nanomips_t::proc_can_convert_pic_got() const
{
    bool ret_val = base->proc_can_convert_pic_got();
    LOG("can_convert_pic_got() = %s", ret_val ? "true" : "false");
    return ret_val;
}

size_t elf_nanomips_t::proc_convert_pic_got(const segment_t *gotps, reloc_tools_t *tools)
{
    LOG("convert_pic_got(0x%x", gotps->start_ea);
    return base->proc_convert_pic_got(gotps, tools);
}

bool elf_nanomips_t::proc_should_load_section(const elf_shdr_t &sh, elf_shndx_t idx, const qstring &name)
{
    return base->proc_should_load_section(sh, idx, name);
}

void elf_nanomips_t::proc_on_loading_symbols()
{
    base->proc_on_loading_symbols();
}

bool elf_nanomips_t::proc_perform_patching(const elf_shdr_t *plt, const elf_shdr_t *gotps)
{
    LOG("perform_patching(0x%llx)", plt->sh_addr);
    return base->proc_perform_patching(plt, gotps);
}

bool elf_nanomips_t::proc_supports_relocs() const
{
    bool ret_val = base->proc_supports_relocs();
    LOG("supports_relocs() = %s", ret_val ? "true" : "false");
    return ret_val;
}