<<<<<<< HEAD
# c
🔌 C: Buat “Network Diagnostics Mini-Tool”. Pakai: input host/IP+port→pilih ping/trace/port-check. Manfaat: diagnosa koneksi cepat + latihan OSI/socket. Alur: validasi target→(ops) DNS→tes ICMP/TCP→ukur latency/hop→ringkas status+kode. Preview: ringkasan hasil tes rapi. Anti-error: cek format host, timeout tegas, fallback jaringan putus.
=======
<div align="center">
  <img src="https://capsule-render.vercel.app/api?type=rect&height=130&color=0:0ea5e9,100:22c55e&text=Network%20Diagnostics%20Mini-Tool&fontSize=34&fontColor=ffffff&animation=fadeIn&fontAlignY=55" />
  <img src="https://readme-typing-svg.demolab.com?font=Fira+Code&size=14&duration=2200&pause=700&color=22C55E&center=true&vCenter=true&width=900&lines=Ping%20%7C%20Traceroute%20Sederhana%20%7C%20Port%20Check;Tool%20ringan%20untuk%20diagnosa%20koneksi%20cepat%20(Network%20Support);Validasi%20host%2FIP%20%2B%20timeout%20jelas%20%2B%20error%20informatif" />
  <br/>
  <img src="https://skillicons.dev/icons?i=c&perline=1" />
</div>

---

## Tujuan, Manfaat, dan Bahasa Pemrograman
- ✅ **Tujuan**: CLI ringan untuk menjalankan **Ping**, **Traceroute (sederhana)**, dan **Port Check (TCP)** supaya diagnosa koneksi bisa cepat dan terstruktur.
- ✅ **Manfaat**:
  - Mengetahui apakah host reachable, melihat hop/rute, dan mengecek port terbuka/tertutup.
  - Menunjukkan dasar **OSI/socket** (DNS resolve opsional + TCP connect untuk port check).
  - Output ringkas bisa dicatat/dilampirkan ke tiket untuk analisis.
- ✅ **Bahasa**: **C (C99)**

---

## Instalasi & Persiapan Proyek
- ✅ **Windows**: disarankan pakai **MSYS2** (UCRT64/MINGW64) agar `gcc` tersedia.
- ✅ **Linux/Mac**: gunakan `gcc`/`clang` bawaan + `make` (opsional).

---

## Build & Run (Windows PowerShell) — Auto-detect MSYS2 (UCRT64/MINGW64)
```powershell
# 1) Deteksi gcc (UCRT64 atau MINGW64) + set PATH otomatis
if (Test-Path "C:\msys64\ucrt64\bin\gcc.exe") {
  $env:Path = "C:\msys64\ucrt64\bin;" + $env:Path
} elseif (Test-Path "C:\msys64\mingw64\bin\gcc.exe") {
  $env:Path = "C:\msys64\mingw64\bin;" + $env:Path
} else {
  Write-Host "ERROR: gcc.exe tidak ditemukan. Install MSYS2 toolchain dulu (UCRT64/MINGW64)." -ForegroundColor Red
  exit 1
}

# 2) Cek gcc
gcc --version

# 3) Compile + Run
cd C:\Users\ASUS\Desktop\proyek\c
gcc -O2 -Wall -Wextra -std=c99 -o netdiag.exe src/main.c src/menu.c src/validators.c src/net_utils.c src/ping_runner.c src/trace_runner.c src/portcheck.c -lws2_32
.\netdiag.exe
```

---

## Build (MSYS2 UCRT64) — Alternatif
```bash
cd /c/Users/ASUS/Desktop/proyek/c
gcc -O2 -Wall -Wextra -std=c99 -o netdiag.exe src/main.c src/menu.c src/validators.c src/net_utils.c src/ping_runner.c src/trace_runner.c src/portcheck.c -lws2_32
./netdiag.exe
```

---

## Build (Linux/Mac) — Makefile
```bash
cd ~/proyek/c
make
./netdiag
```

---

## Struktur File Proyek
```text
c/
├─ README.md
├─ Makefile
├─ src/
│  ├─ main.c
│  ├─ menu.c
│  ├─ menu.h
│  ├─ validators.c
│  ├─ validators.h
│  ├─ net_utils.c
│  ├─ net_utils.h
│  ├─ ping_runner.c
│  ├─ ping_runner.h
│  ├─ trace_runner.c
│  ├─ trace_runner.h
│  ├─ portcheck.c
│  └─ portcheck.h
└─ assets/
   └─ preview_output.txt
```

---

## Cara Menjalankan Proyek
- ✅ Jalankan `netdiag.exe` / `netdiag` lalu pilih menu.
- ✅ Isi target host/IP, lalu jalankan tes sesuai kebutuhan.

---

## Mode Menu (Interaktif)
- ✅ Isi angka menu di `Pilih:`
  - `1` = Ping
  - `2` = Traceroute (sederhana)
  - `3` = Port Check (TCP)
  - `4` = Exit
- ✅ Setelah pilih tes, isi:
  - Host/IP (contoh: `google.com` atau `8.8.8.8`)
  - (Opsional) port / jumlah ping / hops / timeout (bisa Enter untuk default)

---

## Mode Cepat (Non-interaktif)
```bash
# Ping
./netdiag --ping google.com --count 4 --timeout 1500

# Traceroute
./netdiag --trace 8.8.8.8 --hops 15 --timeout 2000

# Port check (TCP)
./netdiag --port example.com 443 --timeout 1500
```

---

## Preview Output (Ringkas & Rapi)
- File: `assets/preview_output.txt`

```text
[NETDIAG] Target      : google.com
[NETDIAG] Resolved IP : 142.250.xxx.xxx

[PING]   Count        : 4
[RESULT] Sent/Recv    : 4/4
[RESULT] Loss         : 0%
[RESULT] RTT (ms)     : min=12  avg=15  max=20
[STATUS] OK

[TRACE]  Max hops     : 15
[RESULT] Hops sample  : 1) 192.168.1.1  2ms | 2) 10.0.0.1  8ms | ...

[PORT]   Target       : example.com:443
[RESULT] TCP connect  : OPEN (latency 34ms)
```

---

## Alur Program
- ✅ Terima target → validasi format host/IP.
- ✅ (Opsional) resolve DNS → dapatkan IPv4 target (jika domain).
- ✅ Jalankan tes sesuai pilihan:
  - **Ping**: jalankan command OS, parse ringkas RTT + loss (best-effort).
  - **Traceroute**: jalankan `tracert` (Windows) / `traceroute` (Linux/macOS), ringkas hop (best-effort).
  - **Port Check**: TCP connect dengan timeout → status `OPEN/CLOSED/FILTERED`.
- ✅ Tampilkan status sukses/gagal + pesan error yang informatif.

---

## Perbaikan Error Jika Proyek Gagal (Troubleshooting)

---

## PowerShell: `gcc` is not recognized
- ✅ Jalankan bagian **Build & Run (Windows PowerShell) — Auto-detect MSYS2** agar PATH ter-set untuk sesi PowerShell.
- ✅ Alternatif: compile dari MSYS2 UCRT64 langsung (bagian **Build (MSYS2 UCRT64)**).

---

## VS Code IntelliSense: `#include` error / `ctype.h` tidak ditemukan
- ✅ Ini biasanya error **konfigurasi IntelliSense**, bukan error compile.
- ✅ Solusi cepat:
  - Pastikan toolchain MSYS2 sudah terpasang dan `gcc --version` bisa jalan.
  - VS Code → `Ctrl+Shift+P` → **C/C++: Select IntelliSense Configuration** → pilih konfigurasi yang mengarah ke GCC MSYS2.
  - Atau set `compilerPath` ke:
    - `C:\msys64\ucrt64\bin\gcc.exe` (UCRT64), atau
    - `C:\msys64\mingw64\bin\gcc.exe` (MINGW64)

---

## DNS resolve gagal / jaringan tidak terjangkau
- ✅ Pastikan internet aktif.
- ✅ Coba gunakan IP langsung (mis. `8.8.8.8`).
- ✅ Jika corporate network, kemungkinan DNS/Firewall membatasi.

---

## Traceroute tidak tersedia (Linux)
```bash
sudo apt install traceroute
```

---

## Port check selalu timeout
- ✅ Naikkan timeout (contoh: `1500` → `3000`).
- ✅ Pastikan port benar (contoh: `80`, `443`, `22`).
- ✅ Cek firewall/proxy/ISP yang mungkin memblok koneksi.

---

## Host/IP tidak valid
- ✅ Contoh domain valid: `google.com`
- ✅ Contoh IPv4 valid: `8.8.8.8`
>>>>>>> bf65ef0 (first)
