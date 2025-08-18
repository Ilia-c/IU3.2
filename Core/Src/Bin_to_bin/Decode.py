
#!/usr/bin/env python3
import argparse
import struct
import sys
from datetime import timedelta

# -------- Optional GUI picker --------
def pick_files_with_gui():
    try:
        import tkinter as tk
        from tkinter import filedialog
        root = tk.Tk()
        root.withdraw()
        paths = filedialog.askopenfilenames(
            title="Выберите бинарные дампы HARD FAULT",
            filetypes=[("Binary files", "*.bin"), ("All files", "*.*")],
        )
        root.destroy()
        return list(paths)
    except Exception as e:
        print(f"[i] GUI-файлпикер недоступен ({e}). Используйте аргументы командной строки.", file=sys.stderr)
        return []

# -------- Configuration --------
STATE_FIELDS = [
    "Mode",
    "Communication",
    "RS485_prot",
    "units_mes",
    "screen_sever_mode",
    "USB_mode",
    "Save_in",
    "len",
    "mode_ADC0",
    "block",
]

STATE_ENUMS = {
    "Mode": {0: "Idle", 1: "Measure", 2: "Calib", 3: "Service"},
    "Communication": {0: "None", 1: "USB", 2: "RS485", 3: "BT"},
    "RS485_prot": {0: "Raw", 1: "Modbus-RTU"},
    "units_mes": {0: "Pa", 1: "kPa", 2: "MPa"},
    "screen_sever_mode": {0: "Off", 1: "Dim", 2: "Saver"},
    "USB_mode": {0: "MSC", 1: "CDC/ACM", 2: "HID"},
    "Save_in": {0: "None", 1: "ExtFlash", 2: "EEPROM", 3: "USB"},
    "len": {},
    "mode_ADC0": {0: "Single", 1: "Continuous", 2: "DMA"},
    "block": {0: "Unlocked", 1: "Locked"},
}

def decode_state(n):
    out = []
    for idx, name in enumerate(STATE_FIELDS):
        val = (n >> (4*idx)) & 0xF
        human = STATE_ENUMS.get(name, {}).get(val, None)
        out.append((name, val, human if human is not None else ""))
    return out

def bits(x, indices):
    return {name: bool(x & (1<<bit)) for name, bit in indices.items()}

def decode_cfsr(cfsr):
    mmfsr = cfsr & 0xFF
    bfsr  = (cfsr >> 8) & 0xFF
    ufsr  = (cfsr >> 16) & 0xFFFF
    mm = bits(mmfsr, {"IACCVIOL":0,"DACCVIOL":1,"MUNSTKERR":3,"MSTKERR":4,"MLSPERR":5,"MMARVALID":7})
    bf = bits(bfsr,  {"IBUSERR":0,"PRECISERR":1,"IMPRECISERR":2,"UNSTKERR":3,"STKERR":4,"LSPERR":5,"BFARVALID":7})
    uf = bits(ufsr,  {"UNDEFINSTR":0,"INVSTATE":1,"INVPC":2,"NOCP":3,"UNALIGNED":8,"DIVBYZERO":9})
    return mm, bf, uf

def decode_hfsr(hfsr):
    return bits(hfsr, {"VECTTBL":1, "FORCED":30})

def decode_dfsr(dfsr):
    return bits(dfsr, {"HALTED":0, "BKPT":1, "DWTTRAP":2, "VCATCH":3, "EXTERNAL":4})

def decode_shcsr(shcsr):
    names = {
        "USGFAULTENA":18, "BUSFAULTENA":17, "MEMFAULTENA":16,
        "SVCALLPENDED":15, "BUSFAULTPENDED":14, "MEMFAULTPENDED":13, "USGFAULTPENDED":12,
        "SYSTICKACT":11, "PENDSVACT":10, "MONITORACT":8,
        "SVCALLACT":7, "USGFAULTACT":3, "BUSFAULTACT":1, "MEMFAULTACT":0
    }
    return bits(shcsr, names)

def decode_icsr(icsr):
    vectactive = icsr & 0x1FF
    vectpending = (icsr >> 12) & 0x1FF
    nmipendset = bool(icsr & (1<<31))
    pendstset  = bool(icsr & (1<<26))
    pendsvset  = bool(icsr & (1<<28))
    return {
        "VECTACTIVE": vectactive,
        "VECTPENDING": vectpending,
        "NMIPENDSET": nmipendset,
        "PENDSVSET": pendsvset,
        "PENDSTSET": pendstset,
    }

def decode_exc_return(x):
    return {
        "raw": x,
        "return_to": "Thread" if (x & (1<<2)) else "Handler",
        "stack_used": "PSP" if (x & (1<<3)) else "MSP",
        "fp_context_saved": False if not (x & (1<<4)) else True
    }

def word_list(b):
    out = []
    for i in range(0, len(b), 4):
        chunk = b[i:i+4]
        if len(chunk) < 4: break
        out.append(struct.unpack('<I', chunk)[0])
    return out

def pretty_hex(x):
    return f"0x{x:08X}"

def seconds_to_dhms(ms):
    td = timedelta(milliseconds=ms)
    days = td.days
    seconds = td.seconds
    hours, rem = divmod(seconds, 3600)
    mins, secs = divmod(rem, 60)
    return f"{days}d {hours:02}:{mins:02}:{secs:02}"

def parse_file(path):
    with open(path, 'rb') as f:
        data = f.read()
    if len(data) < 100:
        print(f"[!] File too small ({len(data)} bytes). Expected at least 100 bytes.", file=sys.stderr)
        return 2

    fmt = '<I Q I I ' + 'I'*8 + 'I'*8 + 'I'*4
    header_size = struct.calcsize(fmt)
    if len(data) < header_size:
        print(f"[!] File too small for header ({len(data)} < {header_size}).", file=sys.stderr)
        return 2

    unpacked = struct.unpack_from(fmt, data, 0)
    idx = 0
    magic = unpacked[idx]; idx+=1
    state = unpacked[idx]; idx+=1
    err_lo = unpacked[idx]; idx+=1
    err_hi = unpacked[idx]; idx+=1
    cfsr,hfsr,dfsr,afsr,shcsr,icsr,mmfar,bfar = unpacked[idx:idx+8]; idx+=8
    r0,r1,r2,r3,r12,lr,pc,psr = unpacked[idx:idx+8]; idx+=8
    exc_return = unpacked[idx]; idx+=1
    task_id    = unpacked[idx]; idx+=1
    sys_tick   = unpacked[idx]; idx+=1
    timestamp  = unpacked[idx]; idx+=1

    stack_bytes = data[header_size:]
    stack_words = word_list(stack_bytes)

    print("="*72)
    print(f"File: {path}")
    print(f"Size: {len(data)} bytes (header {header_size}, stack {len(stack_bytes)} bytes)")
    print("-"*72)
    print("Header")
    print("------")
    print(f"{'magic':18}: {pretty_hex(magic)}")
    print(f"{'state':18}: 0x{state:016X}")
    for name, val, human in decode_state(state):
        if human:
            print(f"  - {name:16} = {val} ({human})")
        else:
            print(f"  - {name:16} = {val}")
    print(f"{'err_lo':18}: {pretty_hex(err_lo)}")
    print(f"{'err_hi':18}: {pretty_hex(err_hi)}")

    print("\nSystem Fault Registers")
    print("----------------------")
    print(f"{'CFSR':18}: {pretty_hex(cfsr)}")
    mm,bf,uf = decode_cfsr(cfsr)
    print("  MMFSR:", ", ".join([k for k,v in mm.items() if v]) or "none")
    print("  BFSR :", ", ".join([k for k,v in bf.items() if v]) or "none")
    print("  UFSR :", ", ".join([k for k,v in uf.items() if v]) or "none")
    print(f"{'HFSR':18}: {pretty_hex(hfsr)}  -> " + (", ".join([k for k,v in decode_hfsr(hfsr).items() if v]) or "none"))
    print(f"{'DFSR':18}: {pretty_hex(dfsr)}  -> " + (", ".join([k for k,v in decode_dfsr(dfsr).items() if v]) or "none"))
    print(f"{'AFSR':18}: {pretty_hex(afsr)}")
    print(f"{'SHCSR':18}: {pretty_hex(shcsr)}")
    sh = decode_shcsr(shcsr)
    print("  Flags:", ", ".join([k for k,v in sh.items() if v]) or "none")
    print(f"{'ICSR':18}: {pretty_hex(icsr)}")
    ic = decode_icsr(icsr)
    print(f"  VECTACTIVE : {ic['VECTACTIVE']}")
    print(f"  VECTPENDING: {ic['VECTPENDING']}")
    print(f"  NMIPENDSET : {ic['NMIPENDSET']}")
    print(f"  PENDSVSET  : {ic['PENDSVSET']}")
    print(f"  PENDSTSET  : {ic['PENDSTSET']}")
    print(f"{'MMFAR':18}: {pretty_hex(mmfar)}")
    print(f"{'BFAR':18}: {pretty_hex(bfar)}")

    print("\nStack Frame & Context")
    print("---------------------")
    print(f"{'r0':18}: {pretty_hex(r0)}")
    print(f"{'r1':18}: {pretty_hex(r1)}")
    print(f"{'r2':18}: {pretty_hex(r2)}")
    print(f"{'r3':18}: {pretty_hex(r3)}")
    print(f"{'r12':18}: {pretty_hex(r12)}")
    print(f"{'lr':18}: {pretty_hex(lr)}")
    print(f"{'pc':18}: {pretty_hex(pc)}")
    print(f"{'psr':18}: {pretty_hex(psr)}")

    exc = decode_exc_return(exc_return)
    print(f"{'EXC_RETURN':18}: {pretty_hex(exc_return)}  -> return_to={exc['return_to']}, stack={exc['stack_used']}, fp_ctx_saved={exc['fp_context_saved']}")
    print(f"{'task_id':18}: 0x{task_id:08X}")
    print(f"{'sys_tick(ms)':18}: {sys_tick} ({seconds_to_dhms(sys_tick)} since boot)")
    print(f"{'timestamp(ms)':18}: {timestamp}")

    if stack_words:
        print("\nRaw Stack Snapshot (words, lowest address first)")
        print("-----------------------------------------------")
        per_line = 8
        for i in range(0, len(stack_words), per_line):
            chunk = stack_words[i:i+per_line]
            addr = i*4
            print(f"{addr:06X}:", " ".join(f"{w:08X}" for w in chunk))

    print("="*72)
    return 0

def main():
    ap = argparse.ArgumentParser(description="Decode binary HardFault log exported by device.")
    ap.add_argument("files", nargs="*", help="Path(s) to raw .bin files saved by backup_fault_to_external()")
    ap.add_argument("--gui", action="store_true", help="Открыть диалог выбора файлов (если аргументы не указаны).")
    args = ap.parse_args()

    files = list(args.files)
    if (not files) and args.gui:
        files = pick_files_with_gui()
    # Если пользователь не указал --gui, но и файлов нет — пробуем GUI как удобство.
    if not files:
        files = pick_files_with_gui()

    if not files:
        print("[!] Не выбраны файлы. Укажите пути или используйте --gui.", file=sys.stderr)
        sys.exit(2)

    rc = 0
    for path in files:
        try:
            rc |= parse_file(path)
        except FileNotFoundError:
            print(f"[!] File not found: {path}", file=sys.stderr)
            rc |= 2
        except struct.error as e:
            print(f"[!] Struct unpack error: {e}", file=sys.stderr)
            rc |= 2
        except Exception as e:
            print(f"[!] Error reading {path}: {e}", file=sys.stderr)
            rc |= 1
    sys.exit(rc)

if __name__ == "__main__":
    main()
