# Linux vs macOS — cub3D Port Comparison

`making-ref` (Linux) → `project_ref_mac` (macOS)

---

## 1. Makefile — Linker Flags

**ไฟล์:** `Makefile` บรรทัด 9

### Linux (making-ref)
```makefile
MLX_LINK = $(MLX_LIB) -ldl -lglfw -pthread -lm
```

### macOS (project_ref_mac)
```makefile
GLFW_FLAGS = $(shell pkg-config --libs glfw3 2>/dev/null || echo "-L/opt/homebrew/lib -lglfw")
MLX_LINK = $(MLX_LIB) $(GLFW_FLAGS) -framework Cocoa -framework OpenGL -framework IOKit -lm
```

### ทำไมต่างกัน

| Flag | Linux | macOS | เหตุผล |
|------|-------|-------|--------|
| `-ldl` | ✅ ต้องใช้ | ❌ ไม่มี | Linux ใช้ `dlopen()` โหลด shared lib แบบ dynamic — macOS ไม่ต้องการ flag แยก |
| `-pthread` | ✅ ต้องใช้ | ❌ ไม่ต้องการ | Linux linker ต้องการ explicit flag สำหรับ POSIX threads — macOS รวม libc อยู่แล้ว |
| `-framework Cocoa` | ❌ ไม่มี | ✅ ต้องใช้ | macOS window system และ event loop ผ่าน Cocoa framework |
| `-framework OpenGL` | ❌ ไม่มี | ✅ ต้องใช้ | OpenGL บน macOS อยู่ใน framework ไม่ใช่ `-lGL` |
| `-framework IOKit` | ❌ ไม่มี | ✅ ต้องใช้ | GLFW ต้องการ IOKit สำหรับ input device (keyboard/mouse) บน macOS |
| `GLFW_FLAGS` | ❌ ไม่มี | ✅ เพิ่ม | Homebrew ติดตั้ง GLFW ที่ `/opt/homebrew/lib` (Apple Silicon) หรือ `/usr/local/lib` (Intel) — ใช้ `pkg-config` detect path อัตโนมัติ |

---

## 2. sound.c — Sound Effects Player

**ไฟล์:** `src/bonus/sound.c` ฟังก์ชัน `play_sound()`

### Linux (making-ref)
```c
prefix = "aplay -q ";
// → "aplay -q ./resource/sounds/shoot.wav >/dev/null 2>&1 &"
```

### macOS (project_ref_mac)
```c
prefix = "afplay ";
// → "afplay ./resource/sounds/shoot.wav >/dev/null 2>&1 &"
```

### ทำไมต่างกัน

| | `aplay` (Linux) | `afplay` (macOS) |
|-|-----------------|------------------|
| ที่มา | ALSA (Advanced Linux Sound Architecture) | built-in macOS (AudioToolbox) |
| ติดตั้ง | มากับ `alsa-utils` | มีใน macOS ทุก version |
| format รองรับ | WAV (PCM) เป็นหลัก | WAV, MP3, AAC, AIFF ฯลฯ |
| quiet flag | `-q` ปิด output | เงียบ default ไม่ต้องใส่ flag |
| ใช้บน macOS ได้มั้ย | ❌ ไม่มีใน macOS | ✅ built-in |

---

## 3. audio.c — Background Music

**ไฟล์:** `src/bonus/audio.c` ฟังก์ชัน `build_music_cmd()` และ `music_stop()`

### Linux (making-ref)
```c
// build_music_cmd
prefix = "ffplay -nodisp -autoexit -loop 0 -volume ";
suffix = " ./resource/horror_game_music.mp3 >/dev/null 2>&1 & echo $! > /tmp/cub3d_music.pid";
// volume = integer 0–100
// → "ffplay -nodisp -autoexit -loop 0 -volume 30 ./resource/horror_game_music.mp3 >/dev/null 2>&1 & echo $! > /tmp/cub3d_music.pid"

// music_stop
system("kill $(cat /tmp/cub3d_music.pid 2>/dev/null) 2>/dev/null; rm -f /tmp/cub3d_music.pid");
```

### macOS (project_ref_mac)
```c
// build_music_cmd
p1 = "(while :; do afplay -v ";
p2 = " ./resource/horror_game_music.mp3; done) >/dev/null 2>&1 & echo $! > /tmp/cub3d_music.pid";
// volume = float 0.00–1.00  (scaled จาก 0–100)
// → "(while :; do afplay -v 0.30 ./resource/horror_game_music.mp3; done) >/dev/null 2>&1 & echo $! > /tmp/cub3d_music.pid"

// music_stop
system("pkill -f 'afplay.*horror_game_music' 2>/dev/null; rm -f /tmp/cub3d_music.pid");
```

### ทำไมต่างกัน

**ปัญหา 1: `ffplay` ไม่ใช่ built-in**
- Linux: `ffplay` มาจาก `ffmpeg` package ที่นิยมติดตั้งใน lab 42
- macOS: `ffplay` ต้องติดตั้งผ่าน `brew install ffmpeg` เอง — ไม่ guaranteed
- `afplay` built-in ทุก macOS ไม่ต้องติดตั้งอะไรเพิ่ม

**ปัญหา 2: `afplay` ไม่มี `-loop` flag**
- `ffplay -loop 0` = loop ไม่จำกัด
- `afplay` เล่นครั้งเดียวแล้วจบ — ต้องห่อด้วย shell loop `(while :; do afplay ...; done)`

**ปัญหา 3: Volume scale ต่างกัน**
- `ffplay -volume 30` = integer 0–100
- `afplay -v 0.30` = float 0.0–1.0

| vol (g_music_volume) | ffplay | afplay |
|----------------------|--------|--------|
| 0 | `-volume 0` | `-v 0.00` |
| 30 | `-volume 30` | `-v 0.30` |
| 100 | `-volume 100` | `-v 1.00` |

**ปัญหา 4: `music_stop` — kill ไม่ถึง child process**

Linux version ใช้ `kill $(cat /tmp/cub3d_music.pid)` — kill แค่ `ffplay` process ตรงๆ ได้เพราะ PID ของ `ffplay` คือ process หลัก

macOS version ต้องใช้ `pkill -f 'afplay.*horror_game_music'` เพราะ:
```
shell (PID ที่เก็บใน .pid)
└── while loop
    └── afplay (child — PID ต่างออกไป)
```
ถ้า `kill` แค่ shell parent → `afplay` กลายเป็น orphan เล่นต่อไปจนจบเพลง
`pkill -f` match ด้วย command line string → kill ตรงไปที่ `afplay` process จริงๆ

---

## สรุป — ไฟล์ที่แก้

| ไฟล์ | สิ่งที่เปลี่ยน |
|------|----------------|
| `Makefile` | link flags: เอา `-ldl -pthread` ออก, เพิ่ม `-framework Cocoa/OpenGL/IOKit`, เพิ่ม `GLFW_FLAGS` สำหรับ Homebrew path |
| `src/bonus/sound.c` | `aplay -q` → `afplay` (sound effects) |
| `src/bonus/audio.c` | `ffplay -loop 0 -volume X` → `(while :; do afplay -v 0.XX; done)`, volume scale ปรับใหม่, `music_stop` เปลี่ยนเป็น `pkill -f` |

**ไฟล์ที่ไม่ต้องแก้:** ทุก `.c` อื่น, `cub3d.h`, `resource/` — MLX42 API เหมือนกันทั้งสองแพลตฟอร์ม
