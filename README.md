<div align="center">
  <img src="https://capsule-render.vercel.app/api?type=rect&height=130&color=0:0ea5e9,100:22c55e&text=Network%20Diagnostics%20Mini-Tool&fontSize=34&fontColor=ffffff&animation=fadeIn&fontAlignY=55" />
  <img src="https://readme-typing-svg.demolab.com?font=Fira+Code&size=14&duration=2200&pause=700&color=22C55E&center=true&vCenter=true&width=860&lines=Ping+%7C+Traceroute+Sederhana+%7C+TCP+Port+Check;Validasi+Host%2FIP+%2B+Timeout+Jelas+%2B+Error+Informatif;Ringkas+hasil+diagnosa+untuk+analisis+Network+Support" />
  <br/>
  <img src="https://skillicons.dev/icons?i=c&perline=1" />
</div>

---

## Deskripsi
**Network Diagnostics Mini-Tool (C)** adalah **CLI** ringan untuk diagnosa koneksi cepat: **ping**, **traceroute sederhana**, dan **TCP port check**. Tool ini membantu teknisi/network support mencatat hasil uji konektivitas secara rapi sekaligus memperlihatkan dasar **OSI/socket** (DNS resolve, latency, hop, koneksi TCP). Alur umumnya: **terima target → (opsional) resolve DNS → jalankan tes sesuai pilihan → hitung/rekap hasil → tampilkan status sukses/gagal + kode/penyebab error**.

---

## Tujuan, Manfaat, dan Bahasa Pemrograman
- **Tujuan**: memberi “tool serbaguna” untuk cek koneksi tanpa aplikasi berat, cukup via terminal.
- **Manfaat**:
  - Mempercepat troubleshooting (cek reachable/latency, jalur hop, dan port service).
  - Membantu analisis (hasil diringkas, mudah dicopy ke tiket/notes).
  - Aman dipakai (validasi input + timeout + error message yang jelas saat jaringan tidak terjangkau).
- **Bahasa**: **C (C99)** dengan dukungan Windows socket (**ws2_32**) untuk port check.

---

## Lokasi Folder (sesuai permintaan)
Folder proyek disimpan dan dinamai **sesuai bahasa pemrograman**:

- **Windows**: `C:\Users\ASUS\Desktop\proyek\c`
- **Nama folder root**: `c`

---

## Instalasi & Persiapan Proyek

---

## ✅ Requirements
- **Windows**: MSYS2 + GCC + Make *(disarankan UCRT64)*  
- **Linux/Mac**: `gcc`/`clang` + `make`

---

## Windows (MSYS2 UCRT64) — Install GCC & Make
> Buka **MSYS2 UCRT64** lalu jalankan:

```bash
pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make
```

Cek berhasil:
```bash
gcc --version
make --version
which gcc
```

---

## Linux/Mac — Install Compiler
Contoh (Ubuntu/Debian):
```bash
sudo apt update
sudo apt install -y build-essential
```

---

## Struktur Proyek
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

## 🎯 Fitur Utama
- ✅ **Ping**: jalankan ping OS bawaan dan ringkas hasil *(loss + min/avg/max jika tersedia)*.
- ✅ **Traceroute (sederhana)**: jalankan traceroute OS bawaan dan ringkas hop.
- ✅ **Port Check (TCP)**: uji koneksi TCP ke port target dengan timeout yang jelas.
- ✅ **Validasi input**: cek format **host/IP**, port (1–65535), dan penanganan input kosong.
- ✅ **DNS resolve (opsional)**: tampilkan IP hasil resolve jika tersedia.
- ✅ **Error informatif**: pesan jelas untuk kasus *host invalid*, *DNS gagal*, *timeout*, atau *network unreachable*.

---

## 🖼️ Preview
> Taruh file preview ini di repo kamu (folder `assets/`) supaya README bisa menampilkan contoh output ringkas.

- Preview output: `assets/preview_output.txt`

```text
assets/
└─ preview_output.txt
```

Contoh tampilkan preview di README:
```md
```text
# (isi file assets/preview_output.txt)
```
```

---

## 🛠️ Build & Compile

---

## Opsi A — Build via Makefile (MSYS2 / Linux / Mac)
Masuk folder proyek:
```bash
cd /c/Users/ASUS/Desktop/proyek/c
```

Build:
```bash
make
```

Run:
```bash
./netdiag.exe
```

> Di Linux/Mac, output bisa saja bernama `netdiag` (tergantung Makefile kamu).

---

## Opsi B — Build via GCC (MSYS2 UCRT64)
```bash
cd /c/Users/ASUS/Desktop/proyek/c
gcc -O2 -Wall -Wextra -std=c99 -o netdiag.exe src/main.c src/menu.c src/validators.c src/net_utils.c src/ping_runner.c src/trace_runner.c src/portcheck.c -lws2_32
./netdiag.exe
```

---

## Opsi C — Build dari PowerShell (Auto-detect UCRT64 / MINGW64)
> Paste ini di **PowerShell** (sekali jalan): auto set PATH → cek gcc → compile → run.

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

## ▶️ Cara Menjalankan (Interaktif)
Saat program jalan, kamu akan melihat menu:

```text
1) Ping
2) Traceroute (sederhana)
3) Port Check (TCP)
4) Exit
```

### Contoh alur cepat (yang umum dipakai)
- **Ping** → masukkan `google.com` → Enter untuk default count/timeout.
- **Traceroute** → masukkan `8.8.8.8` → Enter untuk default hops/timeout.
- **Port Check** → masukkan `google.com` → port `443` → Enter untuk default timeout.

---

## 🧯 Troubleshooting (Error umum & solusi)

---

## “gcc is not recognized” di PowerShell
Artinya PATH Windows belum mengarah ke MSYS2. Solusi cepat: pakai **Opsi C** (PowerShell auto-detect PATH) atau compile dari **MSYS2 UCRT64** (Opsi B).

---

## VS Code error “cannot open source file ctype.h / includePath”
Itu masalah IntelliSense belum menunjuk toolchain.
- `Ctrl+Shift+P` → **C/C++: Select IntelliSense Configuration**
- Pilih compiler:
  - UCRT64: `C:\msys64\ucrt64\bin\gcc.exe`
  - MINGW64: `C:\msys64\mingw64\bin\gcc.exe`

---

## Timeout / Network unreachable
- Pastikan target bisa diakses (coba ganti `8.8.8.8`).
- Cek koneksi internet / firewall kantor.
- Untuk port check: pastikan port memang terbuka dan tidak diblokir.

---

## Catatan Teknis (Best-Effort & Fallback)
Tool ini dibuat supaya tidak crash:
- Jika DNS resolve gagal → tampilkan pesan jelas dan kembali ke menu.
- Jika ping/traceroute gagal → tampilkan status gagal + ringkasan yang masuk akal.
- Input tidak valid → ditolak dengan contoh format yang benar.

---
