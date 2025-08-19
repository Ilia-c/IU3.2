
#!/usr/bin/env python3
import argparse, struct, sys, math
from datetime import timedelta

# === Layout parameters (MUST match your Hard_fault.h) ===
MAGIC          = 0xA5A5A5A5
BT_DEPTH       = 32
STACK_WORDS    = 256    # 256 words = 1 KiB

def pick_files_with_gui():
    try:
        import tkinter as tk
        from tkinter import filedialog
        root = tk.Tk(); root.withdraw()
        paths = filedialog.askopenfilenames(
            title="Выберите дампы HARD FAULT (*.ERR/*.BIN)",
            filetypes=[("Error/Raw dumps", "*.ERR *.BIN"), ("All files", "*.*")],
        )
        root.destroy()
        return list(paths)
    except Exception:
        return []

def pick_file_from_console():
    try:
        p = input("Укажите путь к файлу дампа (*.ERR/*.BIN): ").strip().strip('\"')
        return [p] if p else []
    except Exception:
        return []

def pretty_hex(x): return f"0x{x:08X}"
def dhms(ms):
    td = timedelta(milliseconds=int(ms))
    d = td.days; s = td.seconds
    h, s = divmod(s, 3600); m, s = divmod(s, 60)
    return f"{d}d {h:02}:{m:02}:{s:02}"

def bits(x, indices): return {name: bool(x & (1<<bit)) for name, bit in indices.items()}

def decode_cfsr(cfsr):
    mmfsr = cfsr & 0xFF
    bfsr  = (cfsr >> 8) & 0xFF
    ufsr  = (cfsr >> 16) & 0xFFFF
    mm = bits(mmfsr, {"IACCVIOL":0,"DACCVIOL":1,"MUNSTKERR":3,"MSTKERR":4,"MLSPERR":5,"MMARVALID":7})
    bf = bits(bfsr,  {"IBUSERR":0,"PRECISERR":1,"IMPRECISERR":2,"UNSTKERR":3,"STKERR":4,"LSPERR":5,"BFARVALID":7})
    uf = bits(ufsr,  {"UNDEFINSTR":0,"INVSTATE":1,"INVPC":2,"NOCP":3,"UNALIGNED":8,"DIVBYZERO":9})
    return mm,bf,uf

def decode_hfsr(hfsr): return bits(hfsr, {"VECTTBL":1, "FORCED":30})
def decode_dfsr(dfsr): return bits(dfsr, {"HALTED":0, "BKPT":1, "DWTTRAP":2, "VCATCH":3, "EXTERNAL":4})
def decode_shcsr(shcsr):
    names = {"USGFAULTENA":18,"BUSFAULTENA":17,"MEMFAULTENA":16,"SVCALLPENDED":15,"BUSFAULTPENDED":14,
             "MEMFAULTPENDED":13,"USGFAULTPENDED":12,"SYSTICKACT":11,"PENDSVACT":10,"MONITORACT":8,
             "SVCALLACT":7,"USGFAULTACT":3,"BUSFAULTACT":1,"MEMFAULTACT":0}
    return bits(shcsr, names)

def decode_icsr(icsr):
    return {
        "VECTACTIVE":  icsr & 0x1FF,
        "VECTPENDING": (icsr >> 12) & 0x1FF,
        "NMIPENDSET":  bool(icsr & (1<<31)),
        "PENDSVSET":   bool(icsr & (1<<28)),
        "PENDSTSET":   bool(icsr & (1<<26)),
    }

def decode_exc_return(x):
    return {
        "raw": x,
        "return_to":    "Thread" if (x & (1<<2)) else "Handler",
        "stack_used":   "PSP"    if (x & (1<<3)) else "MSP",
        "fp_ctx_saved": bool(x & (1<<4)),
    }

# === Binary layout (packed, little-endian) ===
# Order exactly as in Hard_fault.h:
#   uint32_t magic;
#   uint32_t err_lo, err_hi;
#   uint32_t cfsr,hfsr,dfsr,afsr,shcsr,icsr,mmfar,bfar;   (8 * I)
#   uint64_t state;
#   uint32_t r0,r1,r2,r3,r12,lr,pc,psr;                   (8 * I)
#   uint32_t exc_return;
#   uint32_t timestamp;
#   uint32_t task_id;
#   uint32_t sys_tick;
#   uint32_t sp_at_fault, msp, psp, control, ccr;         (5 * I)
#   uint32_t bt_count;
#   uint32_t bt[BT_DEPTH];
#   uint32_t stack[STACK_WORDS];
FMT = '<' + \
      'I'*3 + \
      'I'*8 + \
      'Q'    + \
      'I'*8 + \
      'I'    + \
      'I'    + \
      'I'    + \
      'I'    + \
      'I'*5  + \
      'I'    + \
      f'{BT_DEPTH}I' + \
      f'{STACK_WORDS}I'

STRUCT_SIZE = struct.calcsize(FMT)

def parse(path):
    b = open(path, 'rb').read()
    if len(b) < STRUCT_SIZE:
        raise ValueError(f"file too small ({len(b)} < expected {STRUCT_SIZE}) — прошивка собрана со STACK_WORDS={STACK_WORDS}, BT_DEPTH={BT_DEPTH}?")

    tup = struct.unpack_from(FMT, b, 0)
    i = 0
    magic, err_lo, err_hi = tup[i:i+3]; i += 3
    cfsr,hfsr,dfsr,afsr,shcsr,icsr,mmfar,bfar = tup[i:i+8]; i += 8
    state = tup[i]; i += 1  # Q
    r0,r1,r2,r3,r12,lr,pc,psr = tup[i:i+8]; i += 8
    exc_return = tup[i]; i += 1
    timestamp  = tup[i]; i += 1
    task_id    = tup[i]; i += 1
    sys_tick   = tup[i]; i += 1
    sp_at_fault, msp, psp, control, ccr = tup[i:i+5]; i += 5
    bt_count = tup[i]; i += 1
    bt = list(tup[i:i+BT_DEPTH]); i += BT_DEPTH
    stack = list(tup[i:i+STACK_WORDS]); i += STACK_WORDS

    return {
        "magic":magic,
        "err_lo":err_lo,"err_hi":err_hi,
        "cfsr":cfsr,"hfsr":hfsr,"dfsr":dfsr,"afsr":afsr,"shcsr":shcsr,"icsr":icsr,"mmfar":mmfar,"bfar":bfar,
        "state":state,
        "r0":r0,"r1":r1,"r2":r2,"r3":r3,"r12":r12,"lr":lr,"pc":pc,"psr":psr,
        "exc_return":exc_return,
        "timestamp":timestamp,"task_id":task_id,"sys_tick":sys_tick,
        "sp_at_fault":sp_at_fault,"msp":msp,"psp":psp,"control":control,"ccr":ccr,
        "bt_count":bt_count,"bt":bt,
        "stack":stack,
        "raw_bytes":b,
    }

def addr_ok(a, lo, hi):
    a_t = a & ~1
    return lo <= a_t < hi

def print_report(path, d, flash_lo, flash_hi):
    print("="*72)
    print(f"File: {path}")
    print(f"Size: {len(d['raw_bytes'])} bytes (struct {STRUCT_SIZE} bytes; bt {BT_DEPTH} * 4; stack {STACK_WORDS*4} bytes)")
    if d["magic"] != MAGIC:
        print(f"[!] MAGIC mismatch: {pretty_hex(d['magic'])} != {pretty_hex(MAGIC)}")
    if (STRUCT_SIZE % 8) != 0:
        print(f"[i] Note: struct size {STRUCT_SIZE} is not 8-byte aligned — с L4 двойными словами последняя запись захватит 4 паддинг-байта.")

    print("-"*72)
    print("Header")
    print("------")
    print(f"{'magic':18}: {pretty_hex(d['magic'])}")
    print(f"{'state':18}: 0x{d['state']:016X}")
    print(f"{'err_lo':18}: {pretty_hex(d['err_lo'])}")
    print(f"{'err_hi':18}: {pretty_hex(d['err_hi'])}")

    print("\nSystem Fault Registers")
    print("----------------------")
    print(f"{'CFSR':18}: {pretty_hex(d['cfsr'])}")
    mm,bf,uf = decode_cfsr(d['cfsr'])
    print("  MMFSR:", ", ".join([k for k,v in mm.items() if v]) or "none")
    print("  BFSR :", ", ".join([k for k,v in bf.items() if v]) or "none")
    print("  UFSR :", ", ".join([k for k,v in uf.items() if v]) or "none")
    print(f"{'HFSR':18}: {pretty_hex(d['hfsr'])}  -> " + (", ".join([k for k,v in decode_hfsr(d['hfsr']).items() if v]) or "none"))
    print(f"{'DFSR':18}: {pretty_hex(d['dfsr'])}  -> " + (", ".join([k for k,v in decode_dfsr(d['dfsr']).items() if v]) or "none"))
    print(f"{'AFSR':18}: {pretty_hex(d['afsr'])}")
    print(f"{'SHCSR':18}: {pretty_hex(d['shcsr'])}")
    icsr = decode_icsr(d['icsr'])
    print(f"{'ICSR':18}: {pretty_hex(d['icsr'])}")
    print(f"  VECTACTIVE : {icsr['VECTACTIVE']}")
    print(f"  VECTPENDING: {icsr['VECTPENDING']}")
    print(f"  NMIPENDSET : {icsr['NMIPENDSET']}")
    print(f"  PENDSVSET  : {icsr['PENDSVSET']}")
    print(f"  PENDSTSET  : {icsr['PENDSTSET']}")
    print(f"{'MMFAR':18}: {pretty_hex(d['mmfar'])}")
    print(f"{'BFAR':18}: {pretty_hex(d['bfar'])}")

    print("\nStack Frame & Context")
    print("---------------------")
    for reg in ("r0","r1","r2","r3","r12","lr","pc","psr"):
        print(f"{reg:18}: {pretty_hex(d[reg])}")
    exc = decode_exc_return(d['exc_return'])
    print(f"{'EXC_RETURN':18}: {pretty_hex(d['exc_return'])}  -> return_to={exc['return_to']}, stack={exc['stack_used']}, fp_ctx_saved={exc['fp_ctx_saved']}")

    print("\nSP & System")
    print("-----------")
    print(f"{'sp_at_fault':18}: {pretty_hex(d['sp_at_fault'])}")
    print(f"{'msp':18}: {pretty_hex(d['msp'])}")
    print(f"{'psp':18}: {pretty_hex(d['psp'])}")
    print(f"{'control':18}: {pretty_hex(d['control'])}")
    print(f"{'ccr':18}: {pretty_hex(d['ccr'])}")

    print("\nRTOS")
    print("----")
    print(f"{'task_id':18}: 0x{d['task_id']:08X}")
    print(f"{'sys_tick(ms)':18}: {d['sys_tick']} ({dhms(d['sys_tick'])} since boot)")
    print(f"{'timestamp(ms)':18}: {d['timestamp']}")

    # Hints for PC/LR
    pc_t = d['pc'] & ~1
    lr_t = d['lr'] & ~1
    in_range_pc = addr_ok(d['pc'], flash_lo, flash_hi)
    in_range_lr = addr_ok(d['lr'], flash_lo, flash_hi)
    print("\nHints")
    print("-----")
    print(f"PC in flash range: {hex(pc_t)}  -> {'YES' if in_range_pc else 'NO'}")
    print(f"LR in flash range: {hex(lr_t)}  -> {'YES' if in_range_lr else 'NO'}")
    print("addr2line (PC & LR):")
    print(f"  arm-none-eabi-addr2line -e firmware.elf -f -C -i {hex(pc_t)} {hex(pc_t-1 if pc_t else 0)} {hex(lr_t)} {hex(lr_t-1 if lr_t else 0)}")

    # Backtrace
    print("\nBacktrace (raw return addresses)")
    print("--------------------------------")
    n = max(0, min(d['bt_count'], len(d['bt'])))
    if n == 0:
        print("(empty)")
    else:
        for idx, a in enumerate(d['bt'][:n]):
            ok = addr_ok(a, flash_lo, flash_hi)
            print(f"#{idx:02d} {pretty_hex(a)}  {'OK' if ok else '??'}")

        # Also prepare grouped addr2line command lines (to avoid super long command)
        addrs = [hex(a & ~1) for a in d['bt'][:n]]
        if addrs:
            print("\naddr2line (bt):")
            chunk = []
            for h in addrs:
                chunk.append(h)
                if len(chunk) == 8:
                    print("  arm-none-eabi-addr2line -e firmware.elf -f -C -i " + " ".join(chunk))
                    chunk = []
            if chunk:
                print("  arm-none-eabi-addr2line -e firmware.elf -f -C -i " + " ".join(chunk))

    print("\nRaw Stack Snapshot (first 128 bytes)")
    print("------------------------------------")
    words_to_show = min(len(d['stack']), 32)  # 32 words = 128 bytes
    for i in range(0, words_to_show, 8):
        chunk = d['stack'][i:i+8]
        print(f"{i*4:06X}:", " ".join(f"{w:08X}" for w in chunk))

def main():
    ap = argparse.ArgumentParser(description="Decode FaultLog_t (new layout: backtrace + extended stack)")
    ap.add_argument("files", nargs="*", help="Paths to *.ERR dumps")
    ap.add_argument("--flash-lo", type=lambda x:int(x,0), default=0x08000000)
    ap.add_argument("--flash-hi", type=lambda x:int(x,0), default=0x08200000)
    args = ap.parse_args()

    files = list(args.files) or pick_files_with_gui() or pick_file_from_console()
    if not files:
        print("[!] Не выбраны файлы.", file=sys.stderr)
        sys.exit(2)

    for p in files:
        try:
            d = parse(p)
            print_report(p, d, args.flash_lo, args.flash_hi)
        except Exception as e:
            print(f"[!] {p}: {e}", file=sys.stderr)

if __name__ == "__main__":
    main()
