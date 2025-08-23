#!/usr/bin/env python3
import os
import sys
import binascii
import tkinter as tk
from tkinter import ttk, filedialog, messagebox

# ====== Ключ AES-128 для устройства (16 байт в HEX, без пробелов) ======
# НЕ ХРАНИТЕ ПРОДОВЫЙ КЛЮЧ В ПУБЛИЧНОМ РЕПО. Здесь для примера.
FW_KEY_HEX = "30336636817637313236646263399522"  # 16 байт (здесь пример из прошивки, замените!)

# ── Константы ────────────────────────────────────────────────────────────────
MAX_FW_SIZE = 446 * 1024  # 446 KB
EXTERNAL_HEADER_SIZE = 36  # [8 + 4 + 2 + 2 + 16 + 4]
# Отображаемый текст -> (код uint16, ярлык для имени файла)
WARD_INFO_MAP = {
    "IU375 — Версия 3.70/3.75 (0xFAEE)": (0xFAEE, "IU375"),
    "IU380 — Версия 3.80 (0x0EA3)":      (0x0EA3, "IU380"),
}

# ── AES CTR ─────────────────────────────────────────────────────────────────
def aes128_ctr_encrypt(key_bytes: bytes, iv16: bytes, plaintext: bytes) -> bytes:
    """
    Совместимо с tiny-AES CTR:
    - IV 16 байт хранится и инкрементируется как big-endian 128-битный счётчик
      (в tiny-AES инкремент идёт с конца массива Iv[15..0]).
    """
    try:
        from Crypto.Cipher import AES
        from Crypto.Util import Counter
    except Exception as e:
        raise RuntimeError(
            "Требуется pycryptodome: pip install pycryptodome\n" + str(e)
        )

    if len(key_bytes) != 16:
        raise ValueError("Ключ должен быть 16 байт для AES-128")
    if len(iv16) != 16:
        raise ValueError("IV должен быть 16 байт")

    init_value = int.from_bytes(iv16, "big")
    ctr = Counter.new(128, initial_value=init_value)  # big-endian counter
    cipher = AES.new(key_bytes, AES.MODE_CTR, counter=ctr)
    return cipher.encrypt(plaintext)

# ── Логика формирования файла UPDATE.bin ─────────────────────────────────────
def build_update_file(in_file: str, out_name: str,
                      fw_version_u16: int, ward_version_u16: int) -> str:
    """
    Новый формат файла:
      [8 байт size (LE)]
      [4 байта CRC32(plain) (LE)]
      [2 байта версия ПО (LE, uint16)]
      [2 байта версия платы (LE, uint16)]
      [16 байт IV (AES CTR)]
      [4 байта CRC32(cipher) (LE)]
      [ciphertext ...] (длина = size)
    """
    if not os.path.isfile(in_file):
        return f"Ошибка: не найден входной файл:\n{in_file}"

    # Ключ
    key_hex = FW_KEY_HEX.strip().lower()
    if len(key_hex) != 32 or any(c not in "0123456789abcdef" for c in key_hex):
        return "Ошибка: FW_KEY_HEX должен быть 32 hex-символа (16 байт)."
    key_bytes = bytes.fromhex(key_hex)

    with open(in_file, "rb") as f:
        plain = f.read()

    fw_size = len(plain)
    if fw_size == 0 or fw_size > MAX_FW_SIZE:
        return f"Ошибка: размер прошивки ({fw_size} байт) превышает {MAX_FW_SIZE} байт."

    # CRC по открытому тексту (для загрузчика)
    crc_plain = binascii.crc32(plain) & 0xFFFFFFFF

    # Случайный IV (16 байт), совместимый с tiny-AES CTR
    iv = os.urandom(16)

    # Шифрование
    cipher = aes128_ctr_encrypt(key_bytes, iv, plain)

    # CRC по шифртексту (для копирования USB->W25)
    crc_enc = binascii.crc32(cipher) & 0xFFFFFFFF

    # Поля заголовка
    size_bytes = fw_size.to_bytes(8, "little")
    crc_plain_bytes = crc_plain.to_bytes(4, "little")
    fw_ver_bytes = (fw_version_u16 & 0xFFFF).to_bytes(2, "little")
    ward_ver_bytes = (ward_version_u16 & 0xFFFF).to_bytes(2, "little")
    crc_enc_bytes = crc_enc.to_bytes(4, "little")

    out_dir = os.path.dirname(os.path.abspath(in_file))
    if not out_name.lower().endswith(".bin"):
        out_name += ".bin"
    out_file = os.path.join(out_dir, out_name)

    with open(out_file, "wb") as f:
        f.write(size_bytes)       # 0..7
        f.write(crc_plain_bytes)  # 8..11
        f.write(fw_ver_bytes)     # 12..13
        f.write(ward_ver_bytes)   # 14..15
        f.write(iv)               # 16..31
        f.write(crc_enc_bytes)    # 32..35
        f.write(cipher)           # 36..35+size

    status = (
        "Файл сформирован успешно!\n\n"
        f"Исходный файл:\n{in_file}\n"
        f"Выходной файл:\n{out_file}\n\n"
        f"Размер прошивки: {fw_size} байт\n"
        f"CRC32(plain): 0x{crc_plain:08X}\n"
        f"CRC32(cipher): 0x{crc_enc:08X}\n"
        f"IV: {iv.hex()}\n"
        f"Версия ПО (u16): {fw_version_u16}\n"
        f"Версия платы (u16): 0x{ward_version_u16:04X}"
    )
    return status

# ── GUI ──────────────────────────────────────────────────────────────────────
class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Генератор UPDATE.bin (AES-CTR)")
        self.resizable(False, False)

        self.in_file_var = tk.StringVar()
        self.out_name_var = tk.StringVar(value="UPDATE.bin")
        self.fw_version_var = tk.StringVar(value="0")
        default_ward_key = list(WARD_INFO_MAP.keys())[0]
        self.ward_choice_var = tk.StringVar(value=default_ward_key)

        vcmd = (self.register(self._validate_uint16), "%P")

        pad_x = 10
        pad_y = 6

        ttk.Label(self, text="Входной .bin файл:").grid(row=0, column=0, sticky="w", padx=pad_x, pady=pad_y)
        ttk.Entry(self, textvariable=self.in_file_var, width=60).grid(row=0, column=1, sticky="we", padx=(0,pad_x), pady=pad_y)
        ttk.Button(self, text="Обзор…", command=self._browse_in_file).grid(row=0, column=2, padx=(0,pad_x), pady=pad_y)

        ttk.Label(self, text="Имя выходного файла:").grid(row=1, column=0, sticky="w", padx=pad_x, pady=pad_y)
        ttk.Entry(self, textvariable=self.out_name_var, width=60).grid(row=1, column=1, sticky="we", padx=(0,pad_x), pady=pad_y)
        ttk.Label(self, text="(можно изменить)").grid(row=1, column=2, sticky="w", padx=(0,pad_x), pady=pad_y)

        ttk.Label(self, text="Версия ПО (uint16):").grid(row=2, column=0, sticky="w", padx=pad_x, pady=pad_y)
        ttk.Spinbox(self, from_=0, to=65535, textvariable=self.fw_version_var, width=12,
                    validate="key", validatecommand=vcmd).grid(row=2, column=1, sticky="w", padx=(0,pad_x), pady=pad_y)

        ttk.Label(self, text="Версия платы:").grid(row=3, column=0, sticky="w", padx=pad_x, pady=pad_y)
        combo = ttk.Combobox(self, textvariable=self.ward_choice_var, values=list(WARD_INFO_MAP.keys()),
                             state="readonly", width=44)
        combo.grid(row=3, column=1, sticky="w", padx=(0,pad_x), pady=pad_y)
        combo.current(0)

        btn_frame = ttk.Frame(self)
        btn_frame.grid(row=4, column=0, columnspan=3, sticky="we", padx=pad_x, pady=(pad_y, 0))
        ttk.Button(btn_frame, text="Сформировать", command=self._on_generate).pack(side="left", padx=(0,10))
        ttk.Button(btn_frame, text="Выход", command=self.destroy).pack(side="left")

        self.status_lbl = ttk.Label(self, text="", justify="left")
        self.status_lbl.grid(row=5, column=0, columnspan=3, sticky="w", padx=pad_x, pady=(pad_y, pad_y))

        self.fw_version_var.trace_add("write", self._on_deps_changed)
        self.ward_choice_var.trace_add("write", self._on_deps_changed)
        self._refresh_out_name()

    def _validate_uint16(self, new_value: str) -> bool:
        if new_value == "":
            return True
        if not new_value.isdigit():
            return False
        try:
            v = int(new_value)
            return 0 <= v <= 65535
        except ValueError:
            return False

    def _browse_in_file(self):
        path = filedialog.askopenfilename(
            title="Выберите бинарный файл прошивки",
            filetypes=[("BIN файлы", "*.bin"), ("Все файлы", "*.*")]
        )
        if path:
            self.in_file_var.set(path)

    def _on_deps_changed(self, *_):
        self._refresh_out_name()

    def _refresh_out_name(self):
        ward_key = self.ward_choice_var.get()
        fw_ver_str = (self.fw_version_var.get() or "0").strip()
        try:
            fw_ver_int = int(fw_ver_str) if fw_ver_str.isdigit() else 0
        except ValueError:
            fw_ver_int = 0

        ward_info = WARD_INFO_MAP.get(ward_key, (0xFAEE, "IU375"))
        ward_label = ward_info[1]
        name = f"UPDATE_{ward_label}_Ver_{fw_ver_int}.bin"
        self.out_name_var.set(name)

    def _on_generate(self):
        in_file = self.in_file_var.get().strip()
        out_name = (self.out_name_var.get().strip() or "UPDATE.bin")
        if not in_file:
            messagebox.showerror("Ошибка", "Не выбран входной .bin файл.")
            return

        fw_ver_str = (self.fw_version_var.get().strip() or "0")
        try:
            fw_version = int(fw_ver_str)
            if not (0 <= fw_version <= 65535):
                raise ValueError()
        except ValueError:
            messagebox.showerror("Ошибка", "Версия ПО должна быть числом 0..65535.")
            return

        ward_key = self.ward_choice_var.get()
        ward_info = WARD_INFO_MAP.get(ward_key)
        if ward_info is None:
            messagebox.showerror("Ошибка", "Не выбрана версия платы.")
            return
        ward_version = ward_info[0]

        try:
            status = build_update_file(in_file, out_name, fw_version, ward_version)
            if status.startswith("Ошибка"):
                messagebox.showerror("Ошибка", status)
            else:
                messagebox.showinfo("Готово", "Файл сформирован успешно.")
            self.status_lbl.config(text=status)
        except Exception as e:
            messagebox.showerror("Ошибка", str(e))
            self.status_lbl.config(text=f"Ошибка: {e}")

def main():
    app = App()
    app.mainloop()

if __name__ == "__main__":
    main()
