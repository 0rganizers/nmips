import idaapi
import ida_funcs
import idautils
import ida_kernwin
import ida_allins
import ida_ua

type_map = {
    idaapi.o_void: "void",
    idaapi.o_reg: "reg",
    idaapi.o_mem: "mem",
    idaapi.o_phrase: "phrase",
    idaapi.o_displ: "displ",
    idaapi.o_imm: "imm",
    idaapi.o_far: "far",
    idaapi.o_near: "near",
    idaapi.o_idpspec0: "idpspec0",
    idaapi.o_idpspec1: "idpspec1",
    idaapi.o_idpspec2: "idpspec2",
    idaapi.o_idpspec3: "idpspec3",
    idaapi.o_idpspec4: "idpspec4",
    idaapi.o_idpspec5: "idpspec5"
}

dt_map = {
    idaapi.dt_byte: "byte",
    idaapi.dt_word: "word",
    idaapi.dt_dword: "dword",
    idaapi.dt_float: "float",
    idaapi.dt_double: "double",
    idaapi.dt_tbyte: "tbyte",
    idaapi.dt_packreal: "packreal",
    idaapi.dt_qword: "qword",
    idaapi.dt_byte16: "byte16",
    idaapi.dt_code: "code",
    idaapi.dt_void: "void",
    idaapi.dt_string: "string"
}


class instr_info_plugin_t(idaapi.plugin_t):
    comment = "instr_info v0.0.1"
    help = "Display information about the current instruction, such as opcode and operands"
    wanted_name = "instr_info"
    wanted_hotkey = "Ctrl+Meta+Shift+I"
    flags = idaapi.PLUGIN_KEEP

    def init(self):
        return idaapi.PLUGIN_KEEP

    def cache_mnemonics(self):
        pass

    def format_type(self, t):
        return type_map[t]
    
    def format_dt(self, dt):
        return dt_map[dt]

    def get_value(self, op: idaapi.op_t):
        if op.type == idaapi.o_reg:
            return f"{idaapi.get_reg_name(op.reg, 4)} ({op.reg})"
        if op.type == idaapi.o_phrase:
            return op.phrase
        if op.type == idaapi.o_imm:
            return op.value
        if op.type == idaapi.o_mem or op.type == idaapi.o_near:
            return f"0x{op.addr:x}"
        if op.type == idaapi.o_displ:
            return f"({op.addr}, {op.phrase})"
        return op.specval

    def get_prop(self, op: idaapi.op_t):
        if op.type == idaapi.o_reg:
            return "reg"
        if op.type == idaapi.o_phrase:
            return "phrase"
        if op.type == idaapi.o_imm:
            return "value"
        if op.type == idaapi.o_mem or op.type == idaapi.o_near:
            return "addr"
        if op.type == idaapi.o_displ:
            return "(addr, phrase)"
        return "specval"

    def format_specval(self, op: idaapi.op_t):
        if op.specval == 0:
            return ""
        return f" (spec: {op.specval:x})"

    def format_flags(self, op: idaapi.op_t):
        return f"{op.specflag1}, {op.specflag2}, {op.specflag3}, {op.specflag4}"

    def format_value(self, op: idaapi.op_t):
        return f"{self.get_prop(op)}: {self.get_value(op)}"

    def format_operand(self, i: idaapi.insn_t, op: idaapi.op_t):
        return f"[{self.format_type(op.type)}] {self.format_value(op)} ({self.format_dt(op.dtype)}) ({self.format_flags(op)}){self.format_specval(op)}"

    def run(self, arg):
        ea = idaapi.get_screen_ea()
        i = idaapi.insn_t()
        if idaapi.decode_insn(i, ea):
            mnemonic = ida_ua.ua_mnem(ea)
            out_text = f"Instruction {mnemonic}, Operands:"
            op_lines = []
            for idx in range(8):
                op: idaapi.op_t = i.ops[idx]
                if op.type == idaapi.o_void:
                    break
                op_lines.append(f"Op{idx+1}: " + self.format_operand(i, op))
            if len(op_lines) > 0:
                out_text += "\n" + "\n".join(op_lines)
                ida_kernwin.ask_form(f"""BUTTON YES* Ok
{out_text}
""")
        else:
            ida_kernwin.warning(f"Instruction at 0x{ea:x} could not be decoded!")

    def term(self):
        pass

def PLUGIN_ENTRY():
    return instr_info_plugin_t()