"""Lift the verified TS2 Ver.B explosion coroutine omitted by upstream's dispatch.

This is a build-time translation, not a runtime interpreter. Only the exact ROM
range verified below is accepted. Original instruction boundaries, CCR effects,
stack writes, scheduler polls and coroutine return addresses are preserved.
The code rejoins upstream at its valid 0x4AB98 instruction boundary; upstream's
0x4AB80 entry is a disassembly starting in the middle of an ADDI instruction.
"""
from pathlib import Path
import hashlib

ROOT = Path(__file__).resolve().parents[1]
START, END = 0x4A4A2, 0x4AB98
SHA256 = '58f24c5576f64cfa60f3a3d131fedf61b99e9bd5e7a873c343559b3d8e58fa68'


def translate(rom):
    if hashlib.sha256(rom[START:END]).hexdigest() != SHA256:
        raise ValueError('Explosion routine does not match Time Crisis TS2 Ver.B')
    def word(a): return int.from_bytes(rom[a:a+2], 'big')
    def long(a): return int.from_bytes(rom[a:a+4], 'big')
    def signed16(v): return v if v < 0x8000 else v - 0x10000
    def reg(n): return 0x20 + 4*n
    def mem(n, disp): return f'(uint32_t)(RG4({reg(n)}) + ({signed16(disp)}))'
    def flags(value, size):
        return f'RS1(0x44, SX{size}({value}) < 0); RS1(0x45, ({value}) == 0); RS1(0x46, 0); RS1(0x47, 0);'
    instructions = []
    pc = START
    while pc < END:
        op, size, body = word(pc), 0, ''
        if op == 0x4E75:  # RTS: a coroutine may return beyond the immediate caller.
            size = 2
            body = 'RS4(0x50, MRD4((uint32_t)RG4(0x3C))); RS4(0x3C, RG4(0x3C)+4); if(rr_return((uint32_t)RG4(0x50)))return; RR_POLL(); pc_=(uint32_t)RG4(0x50); goto resume_;'
        elif op == 0x4EB9 or op == 0x6100:  # JSR absolute / BSR word
            size = 6 if op == 0x4EB9 else 4
            target = long(pc+2) if op == 0x4EB9 else pc+2+signed16(word(pc+2))
            ret = pc+size
            body = f'RS4(0x3C, RG4(0x3C)-4); MWR4((uint32_t)RG4(0x3C), 0x{ret:X}); int j=rr_call_push(0x{ret:X}); RR_POLL(); rr_call_ind(0x{target:X}, 0x{pc:X}); if(rr_after_call(j))return; pc_=rr_ret_to; goto resume_;'
        elif op >> 8 in (0x67, 0x6A, 0x6B, 0x6C):
            assert 0 < (op & 255) < 128  # all branches in this routine are forward, byte displacement
            size = 2
            cond = {0x67:'RG1(0x45)',0x6A:'!RG1(0x44)',0x6B:'RG1(0x44)',0x6C:'RG1(0x44)==RG1(0x46)'}[op >> 8]
            body = f'if({cond}){{pc_=0x{pc+2+(op&255):X};goto resume_;}}'
        elif op & 0xF1FF in (0x41F9, 0x41FA):  # LEA absolute / PC relative
            size = 6 if op & 0x3F == 0x39 else 4
            target = long(pc+2) if size == 6 else pc+2+signed16(word(pc+2))
            body = f'RS4({reg((op>>9)&7)}, 0x{target:X});'
        elif op & 0xF1FF in (0x217C, 0x317C):  # MOVE immediate to d16(An)
            width = 4 if op >> 12 == 2 else 2
            size = 4+width
            value = long(pc+2) if width == 4 else word(pc+2)
            dst = mem((op>>9)&7, word(pc+2+width))
            body = f'MWR{width}({dst}, 0x{value:X}); '+flags(f'0x{value:X}u',width)
        elif op in (0x243C, 0x343C):  # MOVE immediate to D2 (word writes preserve upper half)
            width = 4 if op == 0x243C else 2
            size = 2+width
            value = long(pc+2) if width == 4 else word(pc+2)
            body = f'RS{width}({8+4-width}, 0x{value:X}); '+flags(f'0x{value:X}u',width)
        elif op & 0xFFF8 == 0x0C68:  # CMPI.W #imm,d16(An), leaves X unchanged
            size = 6
            body = f'uint32_t a=MRD2({mem(op&7,word(pc+4))}), b=0x{word(pc+2):X}, r=(a-b)&65535; RS1(0x44, (int16_t)r<0); RS1(0x45,r==0); RS1(0x46,SBORROW(a,b,2)); RS1(0x47,a<b);'
        elif op & 0xFFF8 == 0x06A8:  # ADDI.L #imm,d16(An)
            size = 8
            body = f'uint32_t ea={mem(op&7,word(pc+6))}, a=MRD4(ea), b=0x{long(pc+2):X}, r=a+b; RS1(0x47,CARRY(a,b,4)); RS1(0x43,RG1(0x47)); RS1(0x46,SCARRY(a,b,4)); MWR4(ea,r); RS1(0x44,(int32_t)r<0); RS1(0x45,r==0);'
        elif op & 0xFFF8 == 0x42A8:  # CLR.L d16(An)
            size = 4
            body = f'MWR4({mem(op&7,word(pc+2))},0); '+flags('0',4)
        elif op == 0xC34D:  # EXG A1,A5; no flags
            size = 2
            body = 'uint32_t a=(uint32_t)RG4(0x24); RS4(0x24,RG4(0x34)); RS4(0x34,a);'
        else:
            raise ValueError(f'Unsupported verified instruction {op:04X} at {pc:06X}')
        instructions.append((pc, size, body))
        pc += size
    assert pc == END and len(instructions) == 318
    code = ['/* Generated from the supplied ROM by tools/translate_crate.py. */',
            '#include "lift_rt.h"', '#include <stddef.h>', '#include <stdio.h>',
            'extern uint32_t rr_frame;',
            'void rr_jump(uint32_t,uint32_t);',
            'static void crate_run(uint32_t pc_){',
            f'if(pc_==0x{START:X})fprintf(stderr,"[TCVR] explosion coroutine entered at frame %u\\n",rr_frame);',
            'resume_: switch(pc_){']
    code += [f'case 0x{pc:X}: goto A_{pc:X};' for pc,_,_ in instructions]
    code += ['default: rr_jump(pc_,pc_);return;', '}']
    for pc,size,body in instructions:
        code += [f'A_{pc:X}: RR_INS(0x{pc:X}); {{ {body} }}']
    code += [f'rr_jump(0x{END:X},0x{END-6:X});', '}',
             'void (*tc_crate_entry(uint32_t pc))(uint32_t){switch(pc){']
    code += [f'case 0x{pc:X}:' for pc,_,_ in instructions]
    code += ['return crate_run; default: return NULL; }}', '']
    return '\n'.join(code)


def main():
    # Construct the program from the same verified byte lanes used by tc_game.c.
    rom = bytearray(0x400000)
    for lane in range(4):
        rom[lane::4] = (ROOT/f'upstream/timecris/extracted/ts2verb.{4-lane}').read_bytes()
    source = translate(rom)
    target = ROOT/'build/tc_crate.c'
    target.parent.mkdir(exist_ok=True)
    if not target.exists() or target.read_text() != source:
        target.write_text(source,encoding='utf-8')
    print('Explosion coroutine: 318 verified instructions translated')


if __name__ == '__main__': main()
