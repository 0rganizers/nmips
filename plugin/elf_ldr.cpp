#include "elf_ldr.hpp"
#include "bytes.hpp"
#include "constants.hpp"
#include "elf/elf.h"
#include "funcs.hpp"
#include "idp.hpp"
#include "log.hpp"
#include "name.hpp"
#include "ua.hpp"
#include "segregs.hpp"
#include "nmips.hpp"
#include <cstdarg>
#include <netnode.hpp>
#include <pro.h>

static const char relocations_node_name[] = "$ nanoMIPS relocations";
static const char relocations_symbol_name[] = "$ nanoMIPS relocation symbols";

elf_nanomips_relocations_t::elf_nanomips_relocations_t()
{
    bool exists = storage.create(relocations_node_name);
    if (!exists)
    {
        LOG("relocation storage does not exist");
    } else {
        LOG("relocation storage already exists");
    }
    symbol_storage.create(relocations_symbol_name);
}

void elf_nanomips_relocations_t::segments_updated()
{
    segment_t* ext = get_segm_by_name("extern");
    if (ext != NULL)
        this->update_extern_base(ext->start_ea);
    segment_t* got = get_segm_by_name(".got");
    if (got != NULL)
        this->update_got_base(got->start_ea);
    if (ext == NULL || got == NULL) {
        //WARN("either GOT or extern segment not found, cannot patch got for relocations!");
        return;
    }

    this->patch_got();
    // LOG("Updated relocations inside GOT");
}

ssize_t elf_nanomips_relocations_t::on_event(ssize_t code, va_list va)
{
    switch (code) {
    case idb_event::segm_added:
    case idb_event::segm_moved:
    case idb_event::allsegs_moved:
    case idb_event::segm_name_changed:
    {
        segments_updated();
    }
    break;
    }
    return 0;
}

void elf_nanomips_relocations_t::enable_hooks(bool enable)
{
    if (enable) {
        hook_event_listener(HT_IDB, this, this);
    } else {
        unhook_event_listener(HT_IDB, this);
    }
}

void elf_nanomips_relocations_t::update_got_base(ea_t new_base)
{
    got_base = new_base;

    for (auto &sym : relocated_symbols)
    {
        set_offsets(sym);
    }

    set_default_sreg_value(NULL, GP_SREG, got_base);
    save_to_idb();
}

void elf_nanomips_relocations_t::update_extern_base(ea_t new_base)
{
    // LOG("Change extern base from 0x%x to 0x%x", extern_base, new_base);
    extern_base = new_base;

    for (auto &sym : relocated_symbols)
    {
        set_offsets(sym);
    }
    save_to_idb();
}

void elf_nanomips_relocations_t::patch_got()
{
    for (auto &sym : relocated_symbols)
    {
        patch_got_symbol(sym);
    }
}

void elf_nanomips_relocations_t::patch_got_symbol(got_symbol_t& symbol)
{
    auto got_address = symbol.got_addr;
    if (symbol.got_offset != BADADDR && got_base != BADADDR)
    {
        got_address = got_base + symbol.got_offset;
    }

    auto extern_address = symbol.extern_addr;
    if (symbol.extern_offset != BADADDR && extern_base != BADADDR)
    {
        extern_address = extern_base + symbol.extern_offset;
    }

    // LOG("patching symbol %s 0x%x = 0x%x", symbol.name, got_address, extern_address);
    // get_flags(0);
    // patch_dword(got_address, extern_address);
}

void elf_nanomips_relocations_t::set_offsets(got_symbol_t& got_sym)
{
    if (got_base != BADADDR && got_sym.got_offset == BADADDR)
    {
        got_sym.got_offset = got_sym.got_addr - got_base;
    } else {
        // got_sym.got_offset = BADADDR;
    }
    if (extern_base != BADADDR && got_sym.extern_offset == BADADDR)
    {
        got_sym.extern_offset = got_sym.extern_addr - extern_base;
    } else {
        // got_sym.extern_offset = BADADDR;
    }
}

void elf_nanomips_relocations_t::save_to_idb()
{
    storage.create(relocations_node_name);
    symbol_storage.create(relocations_symbol_name);
    int ret = storage.altset(0, got_base);
    // if (ret != 1) {
    //     ERR("Failed to store got_base in relocation storage!");
    // }
    storage.altset(1, extern_base);
    storage.altset(2, relocated_symbols.size());

    size_t idx = 0;
    for (auto &sym : relocated_symbols)
    {
        symbol_storage.supset(idx, &sym, sizeof(sym));
        idx++;
    }
}

void elf_nanomips_relocations_t::load_from_idb()
{
    storage.create(relocations_node_name);
    symbol_storage.create(relocations_symbol_name);
    got_base = storage.altval(0);
    extern_base = storage.altval(1);
    int num_syms = storage.altval(2);
    relocated_symbols.clear();

    for (int idx = 0; idx < num_syms; idx++)
    {
        got_symbol_t sym;
        symbol_storage.supval(idx, &sym, sizeof(sym));
        relocated_symbols.push_back(sym);
    }
}

const char *elf_nanomips_t::proc_handle_reloc(const rel_data_t &rel_data, const sym_rel *symbol, const elf_rela_t *reloc, reloc_tools_t *tools)
{   
    LOG("handle_relocation(0x%x, 0x%x, 0x%x, t: %d): %s, %s, 0x%x", rel_data.P, rel_data.S, rel_data.Sadd, rel_data.type, symbol->name.c_str(), symbol->original_name.c_str(), symbol->value);
    if (rel_data.type == 10 || rel_data.type == 11)
    {
        ea_t got_address = rel_data.P;
        ea_t extern_address = rel_data.S;
        const char* name = symbol->original_name.c_str();
        got_symbol_t& got_sym = relocations->relocated_symbols.push_back();
        got_sym.name = name;
        got_sym.got_addr = got_address;
        got_sym.extern_addr = extern_address;
        relocations->set_offsets(got_sym);
        relocations->patch_got_symbol(got_sym);
        relocations->save_to_idb();

        if (rel_data.type == 10)
        {
            return "R_NANOMIPS_GLOBAL";
        }
        return "R_NANOMIPS_JUMP_SLOT";
    }

    return base->proc_handle_reloc(rel_data, symbol, reloc, tools);
}

bool elf_nanomips_t::proc_create_got_offsets(const elf_shdr_t *gotps, reloc_tools_t *tools)
{
    // LOG("create_got_offsets(0x%llx)", gotps->sh_addr);
    return base->proc_create_got_offsets(gotps, tools);
}

const char *elf_nanomips_t::proc_describe_flag_bit(uint32 *e_flags)
{
    // LOG("describe_flag_bit(%x)", *e_flags);
    return base->proc_describe_flag_bit(e_flags);
}

bool elf_nanomips_t::proc_load_unknown_sec(Elf64_Shdr *sh, bool force)
{
    // LOG("load_unknown_sec(0x%llx)", sh->sh_addr);
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
    if (name == ".got")
    {
        auto got_addr = sh.sh_addr;
        relocations->update_got_base(got_addr);
    }
    return base->proc_on_create_section(sh, name, sa);
}

const char *elf_nanomips_t::calc_procname(uint32 *e_flags, const char *procname)
{
    // LOG("calc_procname(%s)", procname);
    return base->calc_procname(e_flags, procname);
}

ea_t elf_nanomips_t::proc_adjust_entry(ea_t entry)
{
    return base->proc_adjust_entry(entry);
}

bool elf_nanomips_t::proc_can_convert_pic_got() const
{
    bool ret_val = base->proc_can_convert_pic_got();
    // LOG("can_convert_pic_got() = %s", ret_val ? "true" : "false");
    return ret_val;
}

size_t elf_nanomips_t::proc_convert_pic_got(const segment_t *gotps, reloc_tools_t *tools)
{
    // LOG("convert_pic_got(0x%x", gotps->start_ea);
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
    // LOG("perform_patching(0x%llx)", plt->sh_addr);
    return base->proc_perform_patching(plt, gotps);
}

bool elf_nanomips_t::proc_supports_relocs() const
{
    bool ret_val = base->proc_supports_relocs();
    // LOG("supports_relocs() = %s", ret_val ? "true" : "false");
    return ret_val;
}