# cub3D Survival Horror — Walk-Through

> รวบรวม 32 source files / 3,200+ LOC / 8 sound effects / 24 textures
> อ่านเป็น story: เริ่มจากหน้าต่างเปล่า → จบที่เกมผีเล่นได้จริง

---

## สารบัญ

1. [ภาพรวม: เกมนี้คืออะไร](#1-ภาพรวม)
2. [Architecture — 30,000-foot view](#2-architecture-30000-foot-view)
3. [Data flow ใน 1 เฟรม](#3-data-flow-ใน-1-เฟรม)
4. [Phase-by-phase journey (วิธีที่เราสร้างทีละชั้น)](#4-phase-by-phase-journey)
5. [Deep dives — concepts สำคัญที่น้องต้องเข้าใจก่อน eval](#5-deep-dives)
6. [Bugs ที่เราเจอ — และเรียนรู้อะไร](#6-bugs-ที่เราเจอ)
7. [File-by-file reference](#7-file-by-file-reference)
8. [Tuning table — ปรับยังไงให้ feel เปลี่ยน](#8-tuning-table)
9. [Build / Run / Debug](#9-build--run--debug)
10. [ถ้าจะต่อ — ทำอะไรได้อีก](#10-ถ้าจะต่อ)

---

## 1. ภาพรวม

น้องเริ่มจาก project requirement ของ 42: **cub3D**

> "เขียน raycasting engine แบบ Wolfenstein 3D โดยใช้ MiniLibX. รับไฟล์ `.cub` ที่มี wall texture paths + map + colors แล้ว render 3D view ให้ผู้เล่นเดินสำรวจได้"

แต่ในโฟลเดอร์ `making-ref/` เราไม่ได้ทำแค่ raycaster มาตรฐาน — เราต่อยอดเป็น **survival-horror demo** เต็ม ๆ:

| Feature | Vanilla cub3D | Reference นี้ |
|---------|---------------|---------------|
| ผนัง textured | ✓ | ✓ × **4 wall types** (`1`-`4`) |
| ผู้เล่นเดิน/หมุน | ✓ | ✓ + sprint + stamina |
| Wall collision (bonus) | optional | ✓ + door blocking |
| Minimap (bonus) | optional | ✓ |
| Doors (bonus) | optional | ✓ toggle ด้วย E |
| Animated sprites (bonus) | optional | ✓ + 4 sprite types (ghost/heal/ammo/exit) |
| Mouse rotation (bonus) | optional | ✓ + cursor lock |
| **เพิ่ม:** Enemy AI | – | ผีไล่ผู้เล่น |
| **เพิ่ม:** Combat | – | ปืน + มีด, vector hit detection |
| **เพิ่ม:** HUD | – | HP/Stamina/Ammo/Kills/Crosshair |
| **เพิ่ม:** Audio | – | 8 sound effects via aplay |
| **เพิ่ม:** State machine | – | Menu → Playing → Won/Dead |
| **เพิ่ม:** Pickups + respawn | – | health/ammo เก็บได้ + 10s respawn |
| **เพิ่ม:** Exit goal | – | กอบกู้ exit หลังเคลียร์ผีหมด |
| **เพิ่ม:** Tutorial dialog | – | NPC warden เล่า lore |
| **เพิ่ม:** High score persistence | – | save/load `highscore.txt` |
| **เพิ่ม:** Visual polish | – | vignette, fog, screen shake, recoil |

ทุกอย่างใส่อยู่บน foundation เดียวกัน — **ค่อย ๆ ต่อยอด** อย่างเป็นระเบียบ. นี่คือสิ่งที่ documentation ฉบับนี้จะพาน้องเดิน.

---

## 2. Architecture (30,000-foot view)

```
making-ref/
├── includes/
│   └── cub3d.h           ← Header เดียวเก็บ structs + defines + prototypes
├── src/
│   ├── main.c            ← Entry point + main loop init
│   ├── cleanup/
│   │   └── cleanup.c     ← Single exit point + free ทุกอย่าง
│   ├── events/
│   │   └── events.c      ← Key/mouse hooks (state-aware)
│   ├── mlx/
│   │   ├── mlx_init.c    ← MLX/window/screen-image setup
│   │   └── mlx_utils.c   ← put_pixel/get_pixel (raw addr access)
│   ├── parsing/
│   │   ├── parse_file.c       ← Entry: open .cub, orchestrate parsing
│   │   ├── parse_elements.c   ← NO/SO/WE/EA/F/C identifiers
│   │   ├── parse_map.c        ← Grid loading + character validation
│   │   └── validate_map.c     ← Flood-fill closure check
│   ├── player/
│   │   └── player.c      ← Init pos, move with collision, rotate, stamina
│   ├── raycast/
│   │   └── raycast.c     ← DDA algorithm (4 steps)
│   ├── render/
│   │   ├── render_frame.c    ← Per-frame orchestrator
│   │   ├── render_wall.c     ← Wall column draw + fog + texture select
│   │   └── render_flat.c     ← Floor/ceiling solid fill
│   ├── textures/
│   │   └── texture_load.c ← Load XPMs into t_img (4 types × 4 sides)
│   ├── utils/
│   │   ├── gnl.c          ← get_next_line for parsing
│   │   └── utils.c        ← Mini-libft (strlen, strdup, split, atoi, ...)
│   └── bonus/
│       ├── combat.c       ← shoot() / update_combat() / aim
│       ├── dialog.c       ← Tutorial subtitle state machine
│       ├── door.c         ← Toggle door, find adjacent, sound
│       ├── ghost_ai.c     ← Steering toward player + growl proximity
│       ├── gun.c          ← Weapon overlay (gun/knife) + bob + muzzle
│       ├── highscore.c    ← Save/load text file with raw read/write
│       ├── hud.c          ← Health/stamina bars, crosshair, text overlay
│       ├── menu.c         ← Title screen, sensitivity slider
│       ├── minimap.c      ← Top-left tile overlay
│       ├── mouse.c        ← Frame-poll + warp pattern
│       ├── pickups.c      ← Walk-over consume + respawn timer
│       ├── restart.c      ← Full state reset (R key)
│       ├── sound.c        ← system(aplay &) wrappers
│       ├── sprite.c       ← Z-buffer sprite renderer + multi-type
│       └── vignette.c     ← Post-process edge darkening
├── resource/                ← ทุก asset ที่ runtime ใช้ + ของอ้างอิงจากโปรเจคอื่น
│   ├── textures/
│   │   ├── horror/          ← wall.png, north_red.png, south_red.png, wooden_door.png
│   │   ├── weapons/         ← machete_0..7, pistol_0..7, bolter_0..7
│   │   ├── jumpscare_0..7.png
│   │   └── *.png            ← sprite textures (ghost, troop, caco, ammo, healthkit, exit, npc, jumpscare)
│   ├── sounds/              ← 8 WAV synthesized via ffmpeg
│   ├── maps/abyss.cub       ← 31×21 demo map
│   ├── horror_game_music.mp3
│   └── references/          ← legacy game source ที่ port algorithm มา (อ่านอย่างเดียว)
│       ├── wolf3d/          ← Wolf3D 1992 (WL_SCALE.C → Q16 fixed-point sprite scaler)
│       ├── DOOM/            ← DOOM 1993 (r_main.c colormap, sndserv/sounds.c channels)
│       └── ref_prj_ttranche/← cub3D ของพี่ที่ 42 (XPM-era reference)
├── garbage/                 ← ของที่ไม่ใช้ — เก็บไว้อ้างอิง ลบทิ้งได้
│   ├── mlx_legacy/          ← MiniLibX เดิม (โปรเจคใช้ MLX42 ที่ ../MLX42/ แทน)
│   ├── textures_unused/     ← .xpm ทุกตัว, blood_*/bone_*/wood_*/horror unused, etc.
│   ├── textures_png/        ← duplicate ของ textures/ ที่ root level
│   ├── NPCs/                ← sprite sheet ที่ไม่ได้ใช้
│   ├── FreeWeaponIcons/     ← weapon source assets (raw, ไม่ใช่ที่ใช้)
│   └── E_Bros_Assets_TopDownEnemy/
└── Makefile                  ← Header-dep aware build
```

**Design principles** ที่เฮียยึดตลอดโปรเจค:

1. **One header to rule them all**: `cub3d.h` มี struct ทั้งหมด defines ทั้งหมด prototypes ทั้งหมด → ไฟล์ไหนรวมก็เห็นทุกอย่าง
2. **t_game = god struct**: ทุก subsystem state อยู่ใน `t_game` เดียว pass by pointer ไปทุกฟังก์ชัน
3. **Single exit point** (`cleanup_exit`): ที่เดียวที่ free ทุก resource + เรียก `exit()`. ทุก error path เรียก function นี้
4. **No hidden globals**: ทุก state shared ผ่าน `t_game *game` argument
5. **Memset to zero**: `memset(&game, 0, sizeof(t_game))` ที่ entry → cleanup ปลอดภัยแม้ init จะ fail กลางคัน

---

## 3. Data flow ใน 1 เฟรม

นี่คือ key insight ของเฮีย — เกมนี้ทำงานแบบไหนทุก ~16ms (60fps target):

```
   mlx_loop_hook --calls--> render_frame(t_game *game) ที่อยู่ใน src/render/render_frame.c

   render_frame() {
       1. if (state == MENU) return render_menu()         ← branch สำคัญ
       2. update_mouse_look(game)    // poll + warp
       3. update_player(game)        // apply key flags → move
       4. update_ghosts(game)        // AI chase
       5. update_pickups(game)       // consume + respawn tick
       6. update_aim(game)           // crosshair feedback
       7. dialog_update(game)        // subtitle timer
       8. render_world(game)         // 1280 raycasts → walls + floor/ceil
       9. render_sprites(game)       // Z-buffered ghosts/pickups/exit
       10. apply_vignette(game)      // darken edges
       11. render_gun(game)          // weapon overlay + muzzle
       12. render_minimap(game)      // top-left tile map
       13. render_hud(game)          // HP/stamina/crosshair into buffer
       14. update_combat(game)       // damage tick, win check, hp decay
       15. mlx_put_image_to_window() // flush buffer + screen shake offset
       16. render_text_overlay()     // mlx_string_put text ON TOP of window
       17. anim_tick++
   }
```

**ทำไม order นี้?**

- **State updates ก่อน render**: เพราะ render ต้องอ่าน state ล่าสุด เช่น sprite position หลัง AI move
- **render_world → render_sprites**: ผนังเข้า zbuf ก่อน sprite จะ test Z ได้
- **apply_vignette ก่อน gun**: vignette ทำให้ขอบจอมืด แต่ gun อยู่ขอบจอ → ไม่อยากให้ gun ก็มืดด้วย
- **render_hud → put_image → text_overlay**: HUD bars อยู่ใน image buffer; text ทับด้วย `mlx_string_put` server-side หลัง flush

นี่คือ "rendering pipeline" mental model ที่น้องต้องชัด ก่อนจะแก้บั๊กอะไรเลย

---

## 4. Phase-by-phase journey

เฮียจะเล่าตามลำดับที่เราสร้างจริง ๆ. แต่ละ phase มี:
- **WHAT**: เพิ่มอะไร
- **WHY**: ทำไมต้องทำตอนนี้
- **HOW**: โค้ดทำงานยังไง
- **GOTCHA**: จุดพลาดที่น้องอาจเจอ

---

### Phase 1: Skeleton — Window + image buffer

**Files:** `src/main.c`, `src/mlx/mlx_init.c`, `src/mlx/mlx_utils.c`

**WHAT:** เปิดหน้าต่าง MLX, สร้าง image buffer ขนาด WIDTH × HEIGHT, เขียน pixel ลงไปได้

**WHY:** ทุก rendering ต้อง flush ลงหน้าต่างได้ก่อน. ก่อนจะ raycast อะไรเลย เราต้อง prove ว่า set pixel ที่ตำแหน่งใดในจอได้ตามใจ.

**HOW:**

```c
// init_mlx() — src/mlx/mlx_init.c
game->mlx.mlx = mlx_init();              // X11 connection
game->mlx.win = mlx_new_window(...);     // create window
game->mlx.screen.img = mlx_new_image(...); // off-screen buffer
game->mlx.screen.addr = mlx_get_data_addr(...); // raw pixel access
```

ที่สำคัญ: **เราไม่ใช้ `mlx_pixel_put()` ตอนรัน** เพราะมัน slow มาก (server roundtrip ต่อ pixel). เรา set pixel โดยตรงใน image buffer แล้ว flush ทีเดียวด้วย `mlx_put_image_to_window()`:

```c
// put_pixel() — src/mlx/mlx_utils.c
void put_pixel(t_img *img, int x, int y, int color)
{
    if (x < 0 || x >= img->width || y < 0 || y >= img->height)
        return;
    dst = img->addr + (y * img->line_len + x * (img->bpp / 8));
    *(unsigned int *)dst = color;
}
```

**Concept:** `bpp / 8` คือจำนวน bytes ต่อ pixel (มักเป็น 4 = 32-bit ARGB). `line_len` คือ bytes ต่อ row (อาจ > width × 4 เพราะ alignment).

**GOTCHA:** ลืม bounds check แล้วเขียน out-of-buffer → segfault ทันที. ทุก `put_pixel` ต้องมี guard.

---

### Phase 2: Parsing the .cub file

**Files:** `src/parsing/*.c`, `src/utils/{gnl,utils}.c`

**WHAT:** อ่าน `.cub` แตกเป็น textures + colors + map grid

**WHY:** ก่อน render อะไร เราต้องมี data. และ data ต้อง valid (ถ้าไม่ valid → exit เร็วเลย ดีกว่ามา crash ภายหลัง)

**HOW:** Pipeline 4 ขั้น:

```
parse_cub(path)
  └─> parse_elements(fd)      // NO/SO/WE/EA/F/C identifier lines
  └─> parse_map(fd)            // grid rows
  └─> validate_map()           // flood-fill closure check
```

#### parse_elements.c

แยก identifier ออกจาก path/value:

```c
parse_element_line(game, line):
    if line starts with "NO" or "SO2" or "EA3" ... → wall texture
    if line starts with "F " → floor color (R,G,B)
    if line starts with "C " → ceiling color
```

ในเวอร์ชันล่าสุดที่รองรับ **4 wall types** เราใช้ `match_side()` แล้วเช็คตัวเลขถัดไปว่าเป็น `2`, `3`, `4` (หรือไม่มี → default type 1):

```c
// parse_wall_id() ใน parse_elements.c
if (match_side(line, &side, &off))   // หา NO/SO/WE/EA
    return -1;
if (line[off] >= '2' && line[off] <= '0' + MAX_WALL_TYPES)
    wall_type = line[off] - '1';      // '2' → 1, '3' → 2, etc.
else
    wall_type = 0;
return parse_texture(game, line + off + 1, wall_type, side);
```

#### parse_map.c

โหลด grid ทีละแถว, **ขนาดแถวอาจไม่เท่ากัน** (ฝั่งขวาเป็น space ก็ได้):

```c
parse_map(game, fd, first_line):
    while (line):
        validate chars (only 0, 1, 2, 3, 4, space, N, S, E, W, D, G, H, A, X)
        strip newline
        grow_grid()    // realloc char** to add row
        line = get_next_line(fd)
        if (empty line after map started) → error
```

`grow_grid()` ใช้ pattern realloc แบบง่าย: malloc array ใหม่ ใหญ่ขึ้น 1, copy pointer เก่ามา free array เก่า. ไม่ใช้ realloc() เพราะไม่อยู่ใน allowed list ของ 42.

#### validate_map.c — Flood fill

**Concept สำคัญ:** ตรวจว่า map "ปิด" (มีกำแพงล้อมรอบทุกด้านที่เดินถึงได้)

```c
flood_fill(map, visited, x, y):
    // 1. ออกนอก map → return 1 (error — open map)
    if (x < 0 || x >= w || y < 0 || y >= h)
        return 1;
    // 2. เคย visit แล้ว → 0 (already verified)
    if (visited[y][x])
        return 0;
    visited[y][x] = 1;
    c = map[y][x];
    // 3. กำแพง/ประตู → return 0 (safe boundary, stop here)
    if (c >= '1' && c <= '4')
        return 0;
    if (c == 'D')
        return 0;
    // 4. ช่องว่าง → return 1 (open map = error)
    if (c == ' ' || c == '\0')
        return 1;
    // 5. ที่เดินได้ → recurse 4 ทิศ
    return flood_fill(x+1) || flood_fill(x-1) || flood_fill(x,y+1) || flood_fill(x,y-1);
```

**GOTCHA ที่เฮียเคยพลาด:**

```c
// ❌ เก่า (มี UB):
if (visited[y][x])  // อ่าน visited[y][x] ก่อนเช็ค bounds → out-of-bounds read
    ...
if (x < 0 || x >= w) return 1;

// ✅ ใหม่ — OOB ก่อนเสมอ:
if (x < 0 || x >= w) return 1;
if (visited[y][x]) return 0;
```

ลำดับ check **สำคัญ** เพราะ array indexing ด้วย index ลบ = undefined behavior. ต้อง OOB ก่อนเสมอ.

#### gnl.c

`get_next_line(fd)` — อ่านไฟล์ทีละบรรทัด. ใช้ static buffer 4KB เก็บ leftover ข้าม call:

```c
static char g_gnl_leftover[BUFFER_SIZE + 1];

get_next_line(fd):
    if leftover มี '\n':
        extract line up to \n, leave rest in leftover
        return line
    while (read fd into buf):
        append buf to result
        if buf has '\n':
            extract part up to \n, save rest as leftover
            return result
    return remaining if any, else NULL
```

**GOTCHA:** static buffer = ใช้ได้ทีละ fd. ถ้าเปิดหลายไฟล์ขนาน → ไม่ work. โปรเจคนี้เปิดแค่ map file ครั้งเดียว → ok.

---

### Phase 3: Raycasting — DDA

**File:** `src/raycast/raycast.c`

**WHAT:** ส่งรังสี (ray) ออกจากตัวผู้เล่นทีละคอลัมน์ของจอ, หาว่ารังสีชนกำแพงที่จุดไหน

**WHY:** นี่คือหัวใจของ "3D" — เราไม่ render 3D จริง ๆ เลย. เราคำนวณ **ระยะทาง** ไปยังกำแพงสำหรับแต่ละคอลัมน์ของจอ แล้วลากเส้นแนวตั้งสูง = HEIGHT/ระยะ. สูงต่ำ = ระยะใกล้/ไกล. การมองหลายคอลัมน์รวมกัน → เกิดความรู้สึก 3D

**HOW:** DDA = Digital Differential Analyzer. 4 ขั้น แต่ละขั้นเป็นฟังก์ชันใน raycast.c:

#### Step 1: `init_ray()`

```c
ray->camera_x = 2.0 * x / WIDTH - 1.0;     // -1..1 (-1=left, 1=right)
ray->ray_dir_x = player.dir_x + player.plane_x * camera_x;
ray->ray_dir_y = player.dir_y + player.plane_y * camera_x;
```

ผู้เล่นมี 2 vector: `dir` (เล็งตรง) และ `plane` (กึ่งกลางไป edge ของ FOV). รังสีที่ column `x` คือ `dir + camera_x * plane`. ที่ `x=0` (ซ้ายสุด) → `dir - plane`. ที่ `x=WIDTH/2` → `dir`. ที่ `x=WIDTH-1` → `dir + plane`.

**plane length** ≈ tan(FOV/2). เราตั้งไว้ 0.60 → FOV ~62° (แคบนิด ๆ ทำให้ดู claustrophobic = horror feel)

#### Step 2: `init_step()`

หา **side_dist** = ระยะจาก player ถึง **เส้น grid แรก** ที่รังสีตัด (เส้นแนวตั้ง = vertical grid, แนวนอน = horizontal grid):

```c
if (ray_dir_x < 0):
    step_x = -1
    side_dist_x = (player.x - map_x) * delta_dist_x
else:
    step_x = 1
    side_dist_x = (map_x + 1 - player.x) * delta_dist_x
```

`delta_dist_x` = ระยะที่รังสีเดินทาง **ระหว่าง** vertical grid lines = `|1/ray_dir_x|`.

#### Step 3: `perform_dda()` — main loop

```c
while (!hit):
    if (side_dist_x < side_dist_y):
        side_dist_x += delta_dist_x;
        map_x += step_x;
        side = 0;   // hit vertical line (EA/WE)
    else:
        side_dist_y += delta_dist_y;
        map_y += step_y;
        side = 1;   // hit horizontal line (NO/SO)
    if cell at map_x,map_y == wall:
        hit = 1
        hit_type = cell - '1'    // เก็บประเภทกำแพง
```

ทุก iteration ขยับไปยัง grid cell ถัดไป (ในทิศทางที่ใกล้ที่สุด) จนกว่าจะเจอกำแพง.

#### Step 4: `calc_wall_data()` — projection

```c
if side == 0:
    perp_wall_dist = side_dist_x - delta_dist_x;
else:
    perp_wall_dist = side_dist_y - delta_dist_y;
line_height = HEIGHT / perp_wall_dist;
draw_start = HEIGHT/2 - line_height/2;
draw_end = HEIGHT/2 + line_height/2;
```

`perp_wall_dist` คือ **ระยะตั้งฉาก** จากกำแพงถึง **camera plane** (ไม่ใช่ระยะตรง ๆ จาก player ถึงจุดชน). ถ้าใช้ระยะตรง → จะเกิด **fisheye effect** (มุมโค้ง). ระยะตั้งฉากแก้ปัญหานี้

**Mental model:** ลองจินตนาการเดินเข้าหา flat wall — ขอบซ้าย-ขวา-กลางของกำแพงควรดูสูงเท่ากัน (เพราะ flat). ระยะตรงไม่ใช่ — ขอบไกลกว่ากลาง. ระยะตั้งฉากเท่ากันหมด → ดูเรียบ

#### Why "DDA" not other method?

มี algorithm อื่นเช่น "uniform step" (เดินทีละ 0.01 unit) แต่ DDA **มี complexity O(map_diameter)** ไม่ใช่ O(distance/step) — และไม่มี aliasing เพราะมันเดินไปตาม grid edge เป๊ะ ๆ

---

### Phase 4: Wall texturing + fog

**Files:** `src/render/render_wall.c`, `src/textures/texture_load.c`

**WHAT:** วาดคอลัมน์กำแพงโดยใช้ pixel จาก XPM texture แทนสีตัน

**HOW:**

#### Texture loading

```c
// texture_load.c — load_one()
tex->img = mlx_xpm_file_to_image(mlx, path, &w, &h);
tex->addr = mlx_get_data_addr(tex->img, &bpp, &line_len, &endian);
```

XPM = ASCII art format. `mlx_xpm_file_to_image()` parse แล้วสร้าง image ที่อ่าน pixel ได้เหมือน screen buffer.

**Fallback pattern** ที่เพิ่มมาตอนรองรับ multi-wall-type:

```c
// validate_and_fallback() ใน texture_load.c
type = 1;
while (type < MAX_WALL_TYPES):
    side = 0;
    while (side < 4):
        if (!tex_path[type][side]):
            tex_path[type][side] = strdup(tex_path[0][side]);  // fallback
        side++;
    type++;
```

ถ้าผู้ใช้ระบุแค่ `NO/SO/WE/EA` (ไม่มี `NO2`, `NO3`, ...) → type 2,3,4 ใช้ path ของ type 1 → ดูเหมือนกันหมด. นี่คือ **backward compatibility**

#### Wall column rendering

```c
// draw_column() — src/render/render_wall.c
tex = select_texture(game, ray);
calc_tex_x(ray, player, tex);   // หา column ใน texture (0..tex_width)
step = tex_height / line_height;
tex_pos = (draw_start - HEIGHT/2 + line_height/2) * step;
y = draw_start;
while (y <= draw_end):
    color = get_pixel(tex, ray->tex_x, (int)tex_pos % tex_height);
    color = apply_fog(color, perp_wall_dist);
    put_pixel(&screen, x, y, color);
    tex_pos += step;
    y++;
```

**Mapping logic:**
- ผนังตั้งฉากกับพื้น → texture stretch แนวตั้ง 1:1 กับ wall column
- `step` = texture rows per screen row (ใกล้ → step เล็ก, ไกล → step ใหญ่)
- `tex_pos` หมุนไปทีละ step, modulo เผื่อ overflow

#### Distance fog (DOOM colormap-inspired, Q8 precompute)

DOOM ใช้ `colormap[light_level][palette_index]` — table lookup, ไม่มีการคำนวณ float per pixel. เราเลียนแบบใน true-color โดยสังเกตว่า **fog factor depends only on dist + light, ทั้งสองค่าคงที่ตลอด column** ⇒ คำนวณครั้งเดียวก่อน inner loop:

```c
// version แรก (per-pixel — ช้า): float div + clamp + 3× mul ทุก pixel
// version ใหม่ (per-column Q8): compute once, then 3× integer mul

fog_q8 = fog_factor_q8(dist, light);   // ครั้งเดียว, Q8 fixed-point

while y <= draw_end:
    pixel = shade(get_pixel(tex, ...), fog_q8);
    // shade: r = (((c >> 16) & 0xFF) * fog_q8) >> 8;  ← integer shift
    put_pixel(...);
```

`f = 1.0 - (dist - FOG_START) / (FOG_END - FOG_START)`, clamp `[FOG_FLOOR, 1.0]`, multiply by `light`, แล้ว `(int)(f * 256.0)` = Q8

**Win:** เปลี่ยน 4 float ops × line_height pixels → 4 float ops × 1 (per column) + 3 integer multiplies × pixels. cube ใกล้กล้องที่ render 720 rows = ลดงาน float ~70%

**Horror trick:** ตั้ง `FOG_END = 6.0` (เริ่มมืดที่ระยะ 1, มืดเต็มที่ 6 cells) ทำให้เห็นได้ใกล้ ๆ เท่านั้น → ทำให้ผีโผล่จากความมืดน่ากลัวขึ้น

#### Door rendering — full-stretch + slide

ปกติ wall column ใกล้ ๆ กล้องจะ "center-crop" texture (ดูได้แค่กลางภาพ — ขอบบน/ล่างหายเข้าจอ). สำหรับ door เราอยากเห็นรูปประตู **เต็ม** เสมอ ⇒ branch แยกใน draw_column:

```c
if hit_type == HIT_DOOR:
    visible_h = draw_end - top + 1
    step = tex_height / visible_h    ← ภาพประตู map ลง visible region ทั้งหมด
    tex_pos = 0                       ← เริ่มจากบนสุดของ texture
else:
    step = tex_height / line_height   ← วิธีปกติของ wall (center-crop)
    tex_pos = (top - HEIGHT/2 + line_height/2) * step
```

door slide (ดู Phase 6) ทำให้ `top = draw_start + slide_offset` — visible region ค่อย ๆ หด, แต่ภาพประตูยัง map เต็ม

---

### Phase 5: Player movement + rotation

**Files:** `src/player/player.c`, `src/events/events.c`

**WHAT:** เดิน WASD, หมุน mouse/arrow, ชนกำแพงไม่ทะลุ

**HOW:**

#### Key flag pattern

```c
// events.c — KEY DOWN sets flag, UP clears
key_press(KEY_W): player.move_fb = 1
key_release(KEY_W): player.move_fb = 0
```

**WHY flag ไม่ใช่ direct move?**: เพราะ key event fire ไม่สม่ำเสมอ (อาจไม่ fire ทุกเฟรม). ถ้าเรา move ใน event → frame rate ไม่ smooth. ถ้าเก็บ flag แล้ว move ใน render_frame → ความเร็วคงที่

#### Movement with collision

```c
// move_player() — src/player/player.c
spd = (sprint && stamina > SPRINT_MIN) ? MOVE_FAST : MOVE_SLOW;

// Forward/backward
nx = pos_x + dir_x * spd * move_fb;
ny = pos_y + dir_y * spd * move_fb;
if (!is_blocked(game, (int)nx, (int)pos_y)) pos_x = nx;  // X first
if (!is_blocked(game, (int)pos_x, (int)ny)) pos_y = ny;  // Y after

// Strafe (left/right)
nx = pos_x + (-dir_y) * spd * move_lr;   // perpendicular right vector
ny = pos_y + dir_x * spd * move_lr;
if (!is_blocked(...)) pos_x = nx;
if (!is_blocked(...)) pos_y = ny;
```

**สิ่งที่เฮียพลาด:**

ตอนแรก strafe vector คือ `(dir_y, -dir_x)` → ผิด. ถูกคือ `(-dir_y, dir_x)`. Logic:
- Facing North: `dir=(0,-1)`. Right ควรเป็น `+X` = `(1, 0)`. ใช้สูตร `(-dir_y, dir_x)` = `(-(-1), 0)` = `(1, 0)` ✓

#### Split-axis collision

ทำไมต้องเช็ค X กับ Y แยก?

ตัวอย่าง: ผู้เล่นวิ่งเฉียงเข้าหามุมกำแพง:
```
.....
.XX..       X = wall
.X@..       @ = player กำลังเดิน NE
.....
```

ถ้า move ทั้ง X+Y ใน step เดียวแล้วเช็ค (new_x, new_y) → ติด → หยุดทั้งหมด → "stick to wall"

ถ้าเช็คทีละแกน:
1. ลอง X เดียว (X เลื่อนขวา) → ไม่ชน → ผ่าน
2. ลอง Y (Y เลื่อนขึ้น) → ชน → ไม่ผ่าน
ผลคือ "slide along wall" → ลื่นไหลเข้ามุมได้

#### Rotation matrix

```c
// rotate_player()
old_dir_x = dir_x;
dir_x = dir_x * cos(angle) - dir_y * sin(angle);
dir_y = old_dir_x * sin(angle) + dir_y * cos(angle);
// plane MUST rotate too — same formula
```

**สำคัญมาก:** `plane` ต้องหมุนคู่กับ `dir` เสมอ. ถ้าหมุนแค่ `dir` → FOV จะบิดแปลก ๆ (camera mismatch กับ direction)

#### Stamina

```c
update_stamina():
    if (sprint && moving && stamina > 0):
        stamina -= STAMINA_DRAIN
    else if (stamina < MAX):
        stamina += STAMINA_REGEN
    clamp [0, MAX]
```

แล้ว `move_player()` ใช้ `sprint && stamina > SPRINT_MIN` เป็นเงื่อนไขเร่งความเร็ว → หมดสตามิน่า = วิ่งไม่ได้แม้กดอยู่ → smooth degradation

---

### Phase 6: Doors (revised — state-based, ไม่ใช้ cell flip)

**File:** `src/bonus/door.c`

**WHAT:** เซลล์ `D` ในแมพคือประตู. กด E (ใกล้ ๆ) → เปิด/ปิด พร้อม sliding animation

**HOW:**

```c
init_doors():
    scan map for 'D'
    for each found, store {x, y, timer=0, state=DOOR_CLOSED}

use_door():
    find door adjacent to player (4 directions)
    if found:
        if closed/closing: state = DOOR_OPENING
        if open/opening:   state = DOOR_CLOSING

tick_one() (every frame):
    if OPENING: timer++ → DOOR_OPEN เมื่อ timer >= DOOR_ANIM_MAX
    if CLOSING: 
        ถ้า player ยืนบน cell → revert ไป OPENING (safety guard)
        else: timer-- → DOOR_CLOSED เมื่อ timer <= 0
```

**Design choice ใหม่:** **map cell อยู่ที่ `'D'` ตลอด** — ไม่ flip ระหว่าง `'D'` กับ `'0'`. ตัว passability ตัดสินที่ `door->state`:

```c
// player.c
is_blocked(): if cell == 'D' and door->state != DOOR_OPEN → blocked

// raycast.c
perform_dda(): if cell == 'D' and door->state == DOOR_OPEN → continue DDA (ray ทะลุ)
```

**Why เปลี่ยน?** version แรกใช้ cell-flip ที่ครึ่งทาง animation. ปัญหา:
1. Player ติดถ้า door ปิดทับตัว (cell flip `'0'→'D'` ขณะ player ยืนอยู่)
2. Animation มองไม่เห็นจริง — door เปลี่ยนจาก wall เต็มเป็นไม่มีเลยทันที (binary)

state-based ⇒ animate ลื่นไหล + safety guard ทำได้

**Rendering integration (render_wall.c):**
```c
door_slide(): shift = (timer / DOOR_ANIM_MAX) * line_height
              top = draw_start + shift   ← door retract ขึ้นข้างบน

// gap ที่เปิดด้านบน fill ด้วย ceiling color
// door visible region: [top, draw_end] — texture ยืดให้เต็ม
```

ดู Bug 6 ใน Section 6 สำหรับ scenario stuck ที่ทำให้ต้อง refactor

---

### Phase 7: Minimap

**File:** `src/bonus/minimap.c`

**WHAT:** วาด overlay มุมบนซ้าย แสดงแผนที่ทั้งหมด + จุดผู้เล่น

**HOW:**

```c
render_minimap(game):
    foreach map cell (y, x):
        color = tile_color(game, x, y, cell)   // ตามประเภท: 1=gray, 2=brown, ...
        draw_tile(game, x, y, color)            // วาด rect 6×6 px
    draw_player_dot(game)  // จุดแดงเล็กที่ player.pos_x * tile_size
```

ใส่ลงใน screen buffer ก่อน flush → overlay ปกติบน 3D view

**Trick:** `MINIMAP_T = 6` (size ต่อ tile). map 31×21 → 186×126 px → ใช้พื้นที่มุมบนซ้าย ไม่ทับ HUD ที่มุมล่าง

**Door state visualization:** `'D'` cell แสดงต่างสีตาม `door->state`:

| State | Color | Hex |
|---|---|---|
| `DOOR_CLOSED` | น้ำตาล | `0xAA6600` |
| `DOOR_OPENING/CLOSING` | เหลือง (กำลัง animate) | `0xFFCC22` |
| `DOOR_OPEN` | เขียว | `0x44DD44` |

ผู้เล่นเห็นจาก top-down ว่าประตูไหนเปิดทิ้งไว้ — feedback loop กับ door interaction

---

### Phase 8: Mouse rotation — pattern สำคัญ

**File:** `src/bonus/mouse.c`

**WHAT:** หมุนกล้องด้วย mouse, cursor ติดในเกม (ไม่หลุดออกขอบ)

**WHY pattern นี้:** ตอนแรกเฮียใช้ MotionNotify hook (event-based). ปัญหาคือ:
1. Cursor เห็นได้ → ดูไม่เป็น FPS
2. พอ cursor ออกขอบจอ → event หยุด fire → หมุนต่อไม่ได้
3. Sensitivity ขึ้นกับ desktop config

**HOW (โซลูชั่นใหม่):**

```c
// update_mouse_look() — called every frame
mlx_mouse_get_pos(mlx, win, &mx, &my);
delta = mx - WIDTH/2;
if (delta != 0):
    rotate_view(player, delta * sens_level * 0.0001);
mlx_mouse_move(mlx, win, WIDTH/2, HEIGHT/2);  // ดึง cursor กลับศูนย์
```

**Mental model:** ทุกเฟรม:
1. ถาม OS ว่า cursor อยู่ไหน
2. คำนวณว่าผู้เล่นเลื่อน mouse ไปทางไหน (delta จากศูนย์)
3. หมุน player.dir ตาม delta
4. ดึง cursor กลับศูนย์ → next frame มี baseline ใหม่

ผลลัพธ์: cursor "lock" อยู่กลางจอตลอด, ผู้เล่นรู้สึกหมุนได้ไม่จำกัด

`mlx_mouse_hide()` ถูกเรียกตอนเข้า PLAYING state (จาก menu) → cursor หาย

---

### Phase 9: Sprites — Z-buffered renderer

**File:** `src/bonus/sprite.c`

**WHAT:** วาดอ็อบเจ็กต์ใน 3D space ที่ไม่ใช่กำแพง (ผี, items, exit). มีระยะ, มี occlusion (ถ้ามีกำแพงบัง → ไม่เห็น)

**WHY ซับซ้อน:** sprites ไม่ใช่ grid-aligned. มันลอยอยู่ที่ตำแหน่ง (x, y) ทศนิยม. ต้อง project จาก world space → screen space แล้วเช็ค Z-buffer

**HOW — 4 steps:**

#### Step 1: Fill Z-buffer ตอน wall raycast

```c
// render_frame.c
for x in 0..WIDTH:
    perform_dda(...);
    calc_wall_data(...);
    zbuf[x] = perp_wall_dist;   // ← เก็บระยะกำแพงต่อคอลัมน์
```

#### Step 2: คำนวณ distance² ของทุก sprite + sort

```c
sort_sprites():
    for each sprite:
        dist² = (sp.x - player.x)² + (sp.y - player.y)²
    bubble sort descending (ไกลก่อน ใกล้ทีหลัง)
```

**WHY ไกลก่อน?** เพราะวาดไกลก่อน → ใกล้ทับ. ถ้าวาดใกล้ก่อน → ไกลจะลบใกล้ออก ผิด

#### Step 3: Project sprite → screen

```c
project_one(sprite):
    tx = sp.x - player.x
    ty = sp.y - player.y
    
    // Inverse camera matrix
    inv_det = 1 / (plane_x * dir_y - dir_x * plane_y)
    transform_x = inv_det * (dir_y*tx - dir_x*ty)
    transform_y = inv_det * (-plane_y*tx + plane_x*ty)   // depth
    
    if transform_y <= 0: skip   // sprite อยู่ข้างหลัง
    
    sprite_screen_x = (WIDTH/2) * (1 + transform_x / transform_y)
    h = HEIGHT / transform_y   // ขนาด sprite (สูง + กว้าง = h)
```

**Math intuition:**
- `dir` และ `plane` คือ basis vectors ของ camera space
- เรา project world position ลงบน basis นี้ → transform_x (horizontal) + transform_y (depth)
- หาร `transform_x/transform_y` = projection ลง screen (perspective divide)

#### Step 4: วาดทีละคอลัมน์ของ sprite, เช็ค Z

```c
for stripe = draw_start_x to draw_end_x:
    tex_x = (stripe - draw_start_x) * tex_width / h
    if transform_y < zbuf[stripe]:    // sprite ใกล้กว่ากำแพง → วาด
        draw_sprite_strip(game, sprite, stripe, ...)
```

#### Step 5: Vertical scaler ใน strip (Wolf3D-inspired)

`draw_sprite_strip()` ใช้ **Q16 fixed-point step** แทน double accumulator + **pre-clip y range**:

```c
step_fp  = (tex_height << 16) / strip_height
y, tpos_fp = clip ขอบบน/ล่าง ก่อน loop

while y < y_end:
    color = get_pixel(tex, tex_x, (tpos_fp >> 16) % tex_height)
    if opaque: put_pixel(...)
    tpos_fp += step_fp
```

**Why fixed-point?** double `+=` ทำให้ error สะสม → tex_y drift ตอน sprite ใหญ่ ๆ → texture ดูเหลื่อม. Q16 integer math ไม่มี drift

**Why pre-clip?** Wolf3D's WL_SCALE.C patch jump table runtime เพื่อข้าม pixel นอกจอ. ของเราใช้ทาง clean กว่า: clip `[xy[1], xy[3]]` ลง `[0, HEIGHT]` **ก่อน** loop → ไม่ต้องเช็ค `if (y >= 0)` ทุก iter

ถ้ากำแพงอยู่หน้า sprite ในคอลัมน์นั้น (zbuf[stripe] < transform_y) → skip คอลัมน์นั้น. นี่คือ occlusion

#### Multi-type sprites (ghost/heal/ammo/exit)

ตอนแรก sprite ทุกตัวใช้ texture เดียว. พอเพิ่ม pickups + exit เราใส่ `sprite.type` ฟิลด์ + array `sp_tex[SP_TEX_N]`:

```c
// project_one() ใช้ tex array
tex = &game->sp_tex[sp->type];   // เลือกตาม type
```

แล้ว `tint()` ตัดสินใจ brightness:
- GHOST: pulse flicker ตาม anim_tick
- EXIT: brightest (กระพริบเป็น beacon)
- HEAL/AMMO: stable

---

### Phase 10: Combat — Vector hit detection

**File:** `src/bonus/combat.c`

**WHAT:** ยิงปืน/แทงมีด → หาผีที่อยู่ในกรวยเล็ง → ลด HP

**HOW (วิธีที่ใช้):** Vector projection — **เทคนิคเดียวกับ sprite projection**

```c
is_hit(player, sprite):
    dx = sp.x - p.pos_x
    dy = sp.y - p.pos_y
    fwd = dx*dir_x + dy*dir_y                 // ระยะตรงข้างหน้า (dot product)
    lat = dx*(-dir_y) + dy*dir_x              // เบี่ยงซ้าย/ขวา (perpendicular dot)
    
    return (fwd > 0 && fwd < range && |lat| < radius)
```

**Mental model:**
- `dir` คือทิศหันหน้า
- `(-dir_y, dir_x)` คือ right vector (rotate dir 90° ทวนเข็ม)
- ผีอยู่ "ข้างหน้า + ใน beam" ถ้า fwd > 0 (front) และ |lat| เล็ก (center)

#### Weapon-aware

```c
weapon_stats(player, &range, &radius):
    if weapon == KNIFE: range=1.8, radius=0.7  (สั้น กว้าง)
    else:               range=10,  radius=0.4  (ไกล แคบ)
```

#### Shoot logic

```c
shoot():
    if weapon == GUN:
        if ammo <= 0: return
        ammo--
        snd_shoot()
    else:
        snd_hit()    // knife swing
    recoil = 8
    best = find_target()   // หาผีที่ใกล้สุดใน cone
    if best >= 0:
        sprites[best].hp--
        if hp <= 0: alive=0, kills++
```

#### Damage to player

```c
update_combat():
    if any_enemy_near(player):     // ระยะ DMG_RADIUS
        health -= DMG_PER_TICK
        if dmg_flash == 0: snd_hurt()   // เล่นเสียงครั้งแรกเท่านั้น
        dmg_flash = 8
        shake = 4
    else:
        regen
```

**Throttle pattern:** sound เล่นเมื่อ `dmg_flash == 0` (เริ่มโดน) → ครั้งเดียวต่อ "damage period" ไม่ spam

---

### Phase 11: HUD — Layered rendering

**File:** `src/bonus/hud.c`

**WHAT:** HP bar, stamina bar, crosshair, ammo/kills text, death/win screen, dialog subtitle

**WHY ซับซ้อน:** มี 2 layers ของการเขียน:
- **Image buffer**: HP bar, crosshair, vignette — เขียนลง `screen.img` ด้วย `put_pixel`
- **Window direct**: text — `mlx_string_put` เขียนลง window โดยตรง (server-side text)

ต้องเรียงลำดับ:

```
1. render to image buffer (walls, sprites, HUD bars, ...)
2. mlx_put_image_to_window()  ← flush image to window
3. mlx_string_put(...)        ← text on TOP ของ window
```

ถ้าทำสลับ — string ถูก image overwrite → ไม่เห็น

#### HP bar with smooth fill

```c
draw_health_bar():
    fill_rect(border)
    fill_rect(background dark red)
    fill = bw * (health / HP_MAX)
    fill_rect(fill, bright red)
    fill_rect(fill, top 1/3, brighter highlight)  ← 3D look
```

#### Crosshair with aim feedback

```c
draw_crosshair(game):
    color = aim_on_target ? red : white
    draw + sign with gap in middle (1px center hole)
```

`aim_on_target` set โดย `update_aim()` ใน combat.c — เช็คทุกเฟรมว่ามีผีในกรวยเล็งไหม

#### Damage flash with proper alpha blend

ตอนแรกเฮียใช้ solid red `put_pixel(x, y, 0x900000)` → จอแดงสนิทค้าง

```c
// แก้แล้ว — blend with existing pixel
blend_red(orig, alpha):
    r = (orig_r * (1-α)) + (220 * α)
    g = orig_g * (1-α)
    b = orig_b * (1-α)
```

`alpha` คงที่ ≤ 0.5 → จอแดง 50% สูงสุด → เห็นทะลุได้

#### Heartbeat at low HP

```c
draw_heartbeat():
    if health < 30:
        a = (sin(anim_tick * 0.10) + 1) / 2     // pulse [0, 1]
        dmg_flash = a * 20                       // hijack damage flash
```

ใช้ dmg_flash ที่มีอยู่แล้ว — เร้นเป็น "low HP heartbeat" → ประหยัด state

---

### Phase 12: Visual effects

**Files:** `src/bonus/vignette.c`, `src/render/render_frame.c` (shake)

#### Vignette

```c
apply_vignette():
    max_d = sqrt((W/2)² + (H/2)²)
    foreach pixel (x, y, step 2):
        d = sqrt((x-W/2)² + (y-H/2)²) / max_d   // [0..1]
        f = 1 - d * 0.55                          // bright near center
        clamp f to [0.35, 1.0]
        scale orig pixel by f
```

ทำให้ขอบจอมืดกว่ากลาง → mood horror, รู้สึก "tunnel vision"

**Performance:** เราข้าม 2 pixels ที่ละ (`x += 2, y += 2`) → quarter จำนวน operations. ตามองไม่เห็น

#### Screen shake on damage

```c
// render_frame.c shake_offset()
s = player.shake
if s > 0:
    sx = (anim_tick * 73) % (s*2+1) - s   // pseudo-random in [-s, s]
    sy = (anim_tick * 137) % (s*2+1) - s
```

ใช้ anim_tick × prime numbers % range → noise ดูสุ่ม. ไม่ใช้ rand() เพราะไม่ใช่ in allowed list ของ 42.

ตอน flush:
```c
mlx_put_image_to_window(mlx, win, screen.img, sx, sy);   // offset
```

หน้าจอเลื่อนเล็กน้อยตาม shake → feel "hit"

#### Head bob walking

```c
// shake_offset() เพิ่ม
if moving:
    amp = sprint ? 4.5 : 2.0
    sy -= |sin(anim_tick * 0.18)| * amp
```

จอเลื่อนขึ้นลงตามจังหวะเดิน — เหมือนแกว่งหัว

#### Gun recoil

```c
// gun.c bob_offset()
if recoil > 0:
    by += recoil * 4    // ปืนเด้งลง
    bx += recoil * 2    // เด้งขวา
```

`recoil = 8` ตอนยิง, decrement ทุกเฟรม → animation 8 เฟรม

---

### Phase 13: Sound (the hacky way)

**File:** `src/bonus/sound.c`

**WHAT:** เล่นเสียงเอฟเฟกต์ตอนยิง/ตี/เก็บของ/etc

**WHY hacky:** MLX **ไม่มี audio API**. เราต้องใช้ external player. วิธีที่เลือก:

```c
// play_sound(path)
char cmd[128];
build "aplay -q <path> >/dev/null 2>&1 &";
system(cmd);  // fork shell, shell forks aplay (async), system() returns
```

**Pro:**
- ทำงานทันที, ไม่ต้องใส่ lib
- เสียงเล่นขนานได้ (fork ใหม่ทุกครั้ง)

**Con:**
- `system()` ถูก **ห้าม** ใน 42 norm (forbidden function)
- Fork ทุกครั้ง = overhead เล็กน้อย
- ALSA process leak ถ้าระบบไม่ auto-reap (Linux ทำให้ default)

**ทางเลือกที่ Norm-safe** (ถ้าจะส่งจริง):
- BASS library (ที่ ref project ใช้)
- miniaudio (single-header)
- mlx + libsndfile + libpulse (มี dep หลายตัว)

แต่สำหรับ reference นี้ — เน้นเรียนรู้ — `system()` ก็ใช้ได้

**Sound generation:** ใช้ `ffmpeg` สังเคราะห์เสียงเอง — ไม่ download asset จาก internet:

```bash
# ตัวอย่าง gunshot
ffmpeg -f lavfi -i "anoisesrc=color=brown:duration=0.15" \
  -af "afade=t=out:st=0.05:d=0.10,highpass=200,lowpass=2000" \
  -ar 22050 -ac 1 -y sounds/shoot.wav
```

8 sounds: shoot, hit, pickup, hurt, door, death, footstep, growl

#### Channel-based debounce (DOOM sndserv-inspired)

DOOM's `sndserv` มี 8 voice channels — เลือก channel ที่ free ที่สุดสำหรับ sound ใหม่. ป้องกัน "sound spam" ที่หลาย aplay processes เล่น sample เดียวกันซ้อนกันจน peak ดัง+เพี้ยน

เราเลียนแบบในรูปแบบ minimal: track key + timestamp ของ N channels ล่าสุด, reject ถ้า key เดิม trigger ภายใน cooldown:

```c
// sound.c
static long  last_ms[SND_CHANNELS];   // 8
static const char *last_key[SND_CHANNELS];

channel_blocked(key):
    t = now_ms()
    foreach channel:
        if last_key == key and t - last_ms < SND_COOLDOWN_MS (80):
            return 1   // skip — เพิ่งเล่นไป
    // else: assign to free slot, return 0 (play)

play_sound(path):
    if channel_blocked(path): return;
    system("aplay ...");
```

**Win:** กดยิงรัว ๆ → aplay ไม่ฟัก zombies + sound ไม่ทำลายตัวเอง. footstep tick ที่เกิดทุกหลาย frames ก็ filter อัตโนมัติ

---

### Phase 14: Ghost AI

**File:** `src/bonus/ghost_ai.c`

**WHAT:** ผีลอยเข้าหาผู้เล่นช้า ๆ + ส่งเสียงคำราม

**HOW:**

```c
chase_player(game, ghost):
    dx = player.x - ghost.x
    dy = player.y - ghost.y
    len = sqrt(dx² + dy²)
    if len < MIN_DIST: return           // ใกล้มาก = หยุด
    
    wobble = sin(anim_tick * 0.05 + ghost.x * 1.7) * 0.3
    
    // unit vector + wobble แนวตั้งฉาก
    dx = dx/len + (-dy/len) * wobble
    dy = dy/len + (dx/len) * wobble
    
    step_ghost(game, ghost, dx * GHOST_SPEED, dy * GHOST_SPEED)
```

**Why wobble?** ถ้าผีเดินตรง player ตลอด → จะดูเป็น `straight line` ไม่หลอน. wobble (sin ของ tick + offset) ทำให้ผี **drift** ไปมา → ดูเหมือน "floating organically"

**Why offset by ghost.x?** ถ้าทุกผีใช้ wobble เดียวกัน → ทั้งฝูงเคลื่อนพร้อมกัน. offset ตาม x ทำให้แต่ละตัวเฟสต่างกัน → fluid swarm

#### Proximity growl

```c
growl_tick(game):
    if anim_tick % 300 != 0: return    // ทุก 5 วินาที
    for each alive ghost:
        if dist² < 16: snd_growl(); return
```

Throttle ที่ tick interval → ไม่เล่นซ้อน. หาผีใกล้สุด → ถ้าเจอเล่นเสียง

---

### Phase 15: Pickups + Respawn

**File:** `src/bonus/pickups.c`

**WHAT:** เซลล์ `H` (heal) `A` (ammo) ตอน init แปลงเป็น sprite. ผู้เล่นเดินผ่าน → เก็บ. หลัง 10 วินาที respawn ที่จุดเดิม.

**HOW:**

```c
update_pickups():
    foreach sprite:
        if type != GHOST:
            tick_respawn(sp)    // ถ้าตาย + timer > 0 → ค่อย ๆ ลด
            
        if alive && type != GHOST:
            if dist < PICKUP_RADIUS:
                consume(sp)     // ใช้เอฟเฟกต์ + set timer

tick_respawn(sp):
    if alive || timer == 0: return
    timer--
    if timer == 0:
        sp.x = sp.orig_x       // กลับที่เดิม
        sp.alive = 1

consume(sp):
    if HEAL: player.health += 30
    if AMMO: player.ammo += 8
    if EXIT: trigger win (ถ้าผีตายหมด)
    sp.alive = 0
    sp.respawn_timer = PICKUP_RESPAWN_FRAMES
```

**Why respawn เฉพาะ non-ghost?** เพราะ ghosts ตายไปแล้วต้องตายจริง — ถ้า respawn ผู้เล่นจะยิงไม่จบ → win ไม่ได้

**Why `orig_x/orig_y`?** เก็บตำแหน่งตอน spawn ครั้งแรก เพื่อ restart/respawn ใช้

---

### Phase 16: Menu — Finite State Machine

**Files:** `src/bonus/menu.c`, plus state branches ใน `events.c`, `render_frame.c`, `mouse.c`

**WHAT:** ก่อนเข้าเกมมี title screen ปรับ sensitivity ได้

**Concept — Finite State Machine:**

```c
// cub3d.h
# define STATE_MENU    0
# define STATE_PLAYING 1

// render_frame.c
if state == MENU:
    render_menu()
    return
// ... play state rendering

// events.c
key_press(key, game):
    if state == MENU:
        handle_menu_key(game, key)
        return
    // ... play controls

// mouse.c
update_mouse_look():
    if state != PLAYING: return    // ไม่ poll mouse ใน menu
```

State pattern centralize behavior — แทนที่จะมี `if (in_menu)` กระจัดกระจาย, branch ที่ entry point เท่านั้น

#### Menu visuals

```c
render_menu():
    fill_bg(0x080608)               // dark background
    draw_mist()                     // procedural noise
    mlx_put_image_to_window()       
    draw_title("=== A B Y S S ===")  // 5-layer thickness
    draw_options()                  // sensitivity slider
    draw_controls_help()
```

**Procedural mist:**
```c
v = ((x*31 + (y+anim_tick)*17) ^ (y*7)) & 0x1F
if v > 26: put_pixel(...)
```

XOR + multiply by primes → pseudo-random pattern ที่ animate ด้วย anim_tick. ไม่ต้องโหลด texture mist

---

### Phase 17: Death / Win / Restart

**Files:** `src/bonus/hud.c` (screens), `src/bonus/restart.c`

**WHAT:**
- HP ≤ 0 → death screen + "press R to restart"
- เดินผ่าน exit + ผีตายหมด → win screen
- R → reset everything

**HOW:**

```c
// combat.c
if (health <= 0):
    if (!dead): snd_death()    // ครั้งแรกเท่านั้น
    health = 0
    dead = 1

// restart.c
restart_game(game):
    reset_player(&game->player)    // pos, hp, ammo, kills = 0
    reset_sprites(game)             // respawn all ghosts + pickups
    reset_doors(game)               // close all doors
    won = 0
    dialog_start(game)              // ฟัง dialog อีกครั้ง

// events.c
if (dead || won):
    if key == 'r': restart_game()
    return
```

**Why save spawn pos?** init_player ตั้ง `spawn_x, spawn_y, spawn_dir_*, spawn_plane_*` ตอนเริ่ม. restart ใช้ค่าพวกนี้ตั้ง position กลับมา → ผู้เล่นกลับไปจุดเริ่มต้น

---

### Phase 18: High score persistence

**File:** `src/bonus/highscore.c`

**WHAT:** เก็บ kills สูงสุดข้าม session ในไฟล์ `highscore.txt`

**HOW (without printf/sprintf):**

```c
load_high_score():
    fd = open(HS_PATH, O_RDONLY)
    if fd < 0: return 0
    read(fd, buf, 15)
    parse digits manually:
        while (buf[i] is digit):
            score = score * 10 + (buf[i] - '0')
            i++
    return score

save_high_score(score):
    int_to_buf(score, buf, &len)    // manual itoa
    open + write + close
```

**Why manual?** 42 norm ไม่ให้ใช้ printf ใน hot path / ไม่ให้ snprintf เลย. แต่ raw `read/write/open/close` อยู่ใน allowed → ปลอดภัย

**ไฟล์**: 1-2 bytes (score + newline). ไม่มี serialization complexity

---

### Phase 19: Multi-wall textures (1, 2, 3, 4)

**Files modified:** เกือบทุกที่ — `parse_elements.c`, `raycast.c`, `render_wall.c`, `validate_map.c`, `player.c`, `cleanup.c`, `texture_load.c`, `cub3d.h`

**WHAT:** เซลล์ `1`, `2`, `3`, `4` ใน map คือกำแพง 4 ธีม. แต่ละธีมมี texture 4 ด้าน (N/S/W/E)

**WHY:** มาตรฐาน cub3D ใช้กำแพง type เดียว. การมี 4 types ทำให้ทำ "ห้องต่างกัน" ได้ → atmosphere

**HOW — refactor:**

#### 1. Data structure 1D → 2D

```c
// เก่า:
t_img    tex[4];           // 4 sides
char    *tex_path[4];

// ใหม่:
t_img    tex[MAX_WALL_TYPES][4];   // type × side
char    *tex_path[MAX_WALL_TYPES][4];
```

#### 2. Parser รับ NO2, SO2, ..., EA4

```c
parse_wall_id(line):
    if !match_side(line, &side, &offset): return -1
    if line[offset] in '2'..'4':
        wall_type = line[offset] - '1'   // 1, 2, 3
        offset++
    else:
        wall_type = 0
    return parse_texture(game, line+offset+1, wall_type, side)
```

#### 3. Raycast เก็บ hit_type

```c
// raycast.c perform_dda()
if cell in '1'..'4':
    ray->hit_type = cell - '1'    // 0, 1, 2, 3
    ray->hit = 1
```

#### 4. Renderer เลือกตาม type

```c
select_texture(game, ray):
    return &game->tex[ray->hit_type][side_idx]
```

#### 5. Fallback ถ้าไม่ระบุ

```c
// texture_load.c validate_and_fallback()
foreach type 1..MAX_WALL_TYPES-1:
    foreach side:
        if !tex_path[type][side]:
            tex_path[type][side] = strdup(tex_path[0][side])
```

ถ้า user ไม่ระบุ `NO2 ./...` → type 2 ใช้ texture ของ type 1 → ดูเหมือนกันหมด แต่ map ใช้ `'2'` ได้

**Decision สำหรับ `abyss.cub` ปัจจุบัน:** ใช้แค่ NO/SO/WE/EA (type 1) — ทุก wall_type ใน map จะ fallback ลง horror theme เดียว. version แรกใส่ NO2/3 ด้วย wood + brick texture แต่ไม่เข้ากับ atmosphere → ลบทิ้งหลัง iteration 2

#### 6. Conflict — ย้าย ghost char

ก่อนหน้านี้ `'2'` = ghost spawn. ตอนนี้ `'2'` = wall type 2. ต้องย้าย ghost → `'G'`

ต้องอัป:
- `sprite.c classify('G')` แทน `classify('2')`
- map files: replace ghost positions
- `parse_map.c valid_chars` เพิ่ม `'G'`
- doc README update

**Lesson:** เพิ่ม wall type 1 อันต้องแก้ ~10 ไฟล์. นี่คือ "wide refactor" — แต่เพราะ struct ทุกอย่างอยู่ใน `cub3d.h` เดียว, scoping เห็นชัด

---

### Phase 20: Weapon switch + exit cell + tutorial

**Files added:** `dialog.c`, knife texture, exit texture, restart updates

#### Weapon switch (gun/knife)

```c
t_player.weapon: 0 = GUN, 1 = KNIFE

// events.c
if key == KEY_1: weapon = WEAPON_GUN
if key == KEY_2: weapon = WEAPON_KNIFE

// combat.c is_hit() — different range/radius per weapon
weapon_stats(player, &range, &radius)

// gun.c render_gun() — choose texture
active = (weapon == KNIFE) ? &knife_tex : &gun_tex
draw_weapon_image(active)
muzzle_flash if weapon == GUN
```

#### Exit cell 'X'

Sprite type SP_EXIT. ตอน consume ใน `pickups.c`:

```c
if sp.type == SP_EXIT:
    if any_ghost_alive(game):
        set_notif("kill them all")
    else:
        won = 1
```

ลบ auto-win จาก combat.c (เก่า: ผีตายหมด → win อัตโนมัติ). ใหม่: ต้องมาถึง exit ด้วย

#### Tutorial dialog

```c
g_lines[] = {
    "...wake up traveler. you are in the abyss.",
    "ghosts wander these halls. they hunger.",
    ...
}

dialog_start(game): index = 0, timer = DIALOG_HOLD (5s)
dialog_update(game): timer--, when 0: index++, reset timer
dialog_render(game): show g_lines[index] at bottom of screen
```

Space skip current line → `timer = 0, index++` ทันที

---

### Phase 21: Iteration 2 — Door state refactor + Wolf3D/DOOM algorithm porting

โปรเจคถึงจุดที่ใช้งานได้, แต่ user-reported bugs และโอกาส perf บังคับให้กลับมา refactor. นี่คือ section ที่บันทึก journey จาก "playable" → "polished" — รวมถึง algorithm ที่ดึงจาก source code legacy (Wolf3D 1992, DOOM 1993)

#### Step 1: เปลี่ยน door จาก cell-flip → state-based

**ปัญหาเดิม (version แรก):** `tick_one()` flip `grid[y][x]` ระหว่าง `'D'` กับ `'0'` ที่ครึ่งทาง animation. ผลลัพธ์:
1. ถ้า player ยืนใน door cell ตอน door กำลังปิด → cell flip กลับเป็น `'D'` → **trapped**
2. Animation ที่ราคา ไม่มีจริง — door binary appear/disappear

**Solution:** map cell อยู่ที่ `'D'` ตลอด, ตัว passability ใช้ `door->state`. ต้องเปลี่ยน 5 ไฟล์:
- `door.c` — `tick_one` แค่ animate timer + safety guard
- `player.c` — `is_blocked` ตรวจ door state สำหรับ `'D'` cell
- `raycast.c` — `perform_dda` รับ `t_game *` (ไม่ใช่แค่ `t_map *`) → ตรวจ door state ตอนเจอ `'D'`
- `render_frame.c` — update caller `perform_dda(&ray, game)`
- `cub3d.h` — update prototype

**Lesson:** "data-flag + state" pattern (cell + door->state) สะดวกตอนแรก แต่ **2 sources of truth** ทำให้มี race ที่ทำลายตัวเอง. แก้ที่ root: ลด source of truth → state อย่างเดียว

#### Step 2: Door sliding render

```c
// render_wall.c door_slide()
shift = (timer / DOOR_ANIM_MAX) * line_height
top = draw_start + shift              ← visible top of door
fill [draw_start, top-1] with ceil_color   ← gap ที่เปิดด้านบน
draw door [top, draw_end] with texture stretched to visible_h
```

ภาพ door slide ขึ้นไปเข้าเพดาน — visible ค่อย ๆ หด ⇒ animation มีจริงและ Wolf3D-style

#### Step 3: Mirror direction fix

```c
// version แรก (ผิด direction):
if (side == 0 && ray_dir_x > 0) tex_x = w - tex_x - 1;
if (side == 1 && ray_dir_y < 0) tex_x = w - tex_x - 1;

// version ใหม่ (Lodev standard):
if (side == 0 && ray_dir_x < 0) tex_x = w - tex_x - 1;
if (side == 1 && ray_dir_y > 0) tex_x = w - tex_x - 1;
```

**Why เก่าผิด?** Lodev tutorial wall_x formula `pos_y + perp * ray_dir_y` (side==0) ⇒ mirror ต้อง flip เมื่อ ray ไป negative direction. version แรกอาจ copy จากอ้างอิงที่ใช้ formula ต่างกัน → result mirror ผิดด้าน. ผนัง asymmetric (มี text หรือลาย directional) จะ flip ผิด

#### Step 4: Algorithm porting

**Wolf3D sprite scaling** → fixed-point Q16 step + pre-clip (ดู Phase 9 Step 5)

**DOOM colormap** → fog factor Q8 per-column (ดู Phase 4 fog section)

**DOOM sndserv** → 8-channel debounce (ดู Phase 13 channel section)

ทั้งสาม patches ไม่เปลี่ยน behavior ที่ user สังเกตเห็น — แต่:
- sprite drift ใน distant ghosts หาย (fixed-point ไม่ accumulate error)
- ใกล้กำแพง CPU usage ลด (fog precompute ลด float ops)
- spam shoot/footstep ไม่เกิด aplay zombies (channel cooldown)

#### Step 5: Texture quality + reskin

- ลบ `detail_mold.png` (sprite sheet พร้อม "DEN DOOR" watermark) ออกจาก map config
- รับ `wall.png` + `wooden_door.png` + `north_red.png` + `south_red.png` จาก user → ย้ายเข้า `resource/textures/horror/`
- ลบบรรทัด `NO2`/`NO3` ใน `abyss.cub` → fallback ไปที่ wall_type 1 อัตโนมัติ
- เอา `dmg_flash = 6` ใน `ambient_scare_tick` ออก → ไม่มี red flash ปลอม ๆ ตอนเดิน (keep growl + flicker)

#### Step 6: Directory restructure — resource/, garbage/, references/

ก่อนนี้ root ของ project มีของระเกะระกะ — `wolf3d/`, `DOOM/`, `NPCs/`, `FreeWeaponIcons/`, mp3 ขนาด 140MB, sprite sheets, AI-generated source images. ทำให้:
- `ls` แล้วหาไฟล์ที่ใช้ไม่เจอ
- เผลอ commit ของที่ไม่ควร commit
- ไม่ชัดเจนว่าอันไหน asset, อันไหน reference, อันไหน junk

**Restructure pattern (3-bucket):**

| Bucket | Purpose | Code path |
|---|---|---|
| `resource/` | ของที่ runtime ใช้ทุกตัว — textures, sounds, music, map | hard-coded path `./resource/...` |
| `resource/references/` | source code ของเกมเก่า (Wolf3D, DOOM, ref cub3D) — **อ่านอย่างเดียว** ไม่ link เข้า build | – |
| `garbage/` | ของที่เก็บไว้ "เผื่อ" — XPM เก่า, sprite sheet ไม่ใช้, MLX เดิม. **ลบ ๆ ทิ้งได้** | – |

**Code paths updated** (`./textures/` → `./resource/textures/`):
- `src/bonus/sound.c` — 8 sound paths
- `src/bonus/audio.c` — music mp3 path
- `src/bonus/sprite.c` — 8 sprite texture paths
- `src/bonus/gun.c` — 3 weapon prefixes (machete/pistol/bolter)
- `src/bonus/jumpscare.c` — jumpscare frame prefix
- `src/bonus/door.c` — door texture path
- `resource/maps/abyss.cub` — wall texture paths

**Result:** root ของ project = `includes/`, `src/`, `resource/`, `garbage/`, `Makefile`, `walk_throught.md` — 6 entries ชัดเจน, ไม่งง

**Lesson:** repo hygiene คือ feature ของ engineering ไม่ใช่แค่ "ระเบียบ". การ separate `used / reference / junk` ทำให้ collaborator (รวมถึงตัวเองในอนาคต) เห็นได้ทันทีว่าอะไรเป็น load-bearing

#### Lesson summary

| Decision เก่า | ปัญหา | Decision ใหม่ |
|---|---|---|
| Door = cell flip | Player ติดถ้า cell flip ขณะอยู่บน cell | Door = state lookup |
| Fog per-pixel | float ops ใน hottest loop | Fog Q8 per-column |
| Sprite double accumulator | drift on tall sprites | Q16 fixed-point |
| Sound = system() blind | aplay zombies + sound clash | 8-channel cooldown |
| Texture jam = "ใส่ทุก type" | wood/brick ไม่เข้า horror | Drop non-horror types, fallback |

ทุก fix มาจาก **อ่าน user feedback + sample size 1** — เฮียไม่ได้แก้ตามทฤษฎี แต่แก้ตาม "เห็นกับตา" → นั่นเป็นวิธีที่ถูกต้องของ engineer

---

### Phase 22: Iteration 3 — Difficulty + ingame settings + menu polish

หลัง restructure repo เสร็จ การรัน `./cub3D resource/maps/abyss.cub` ครั้งแรก **crash ทันที** ด้วย `Failed to load gun texture` — เลย bug ออกมาจากใต้ความเงียบ. Phase นี้บันทึก: (a) bug ที่ Phase 21 ทำให้เกิด, (b) feature ใหม่ที่ extend menu/settings, (c) decoration จาก asset ที่ user เพิ่งให้

#### Step 1: Bug — `gun.c` prefix truncation (off-by-N จากการ rename path)

```c
// ก่อนแก้
while (prefix[n] && n < 30)   /* 30 = magic, embed assumption ของ path เก่า */
    path[n++] = prefix[n];
path[n++] = '0' + i;          /* + frame index */
...
```

**Why crash:**
- Path เก่า: `"./textures/weapons/machete_"` ยาว 27 chars (น้อยกว่า 30) → loop จบเอง ใช้ `prefix[n] = 0`
- Path ใหม่: `"./resource/textures/weapons/machete_"` ยาว 36 chars (เกิน 30) → loop ตัดที่ 30 → result = `./resource/textures/weapons/ma` + `0.png` = `./resource/textures/weapons/ma0.png` (ENOENT)
- `load_weapon_frames` คืน 1 ที่ frame 0 → `load_gun_tex` คืน 1 → `cleanup_exit("Failed to load gun texture")`

**Confirmed by strace:**
```
openat(..., "./resource/textures/weapons/ma0.png") = -1 ENOENT
```

**Fix:** `n < 30` → `n < 50`. buffer คือ `path[64]`, รองรับสบาย

**Lesson:** magic number `30` คือ **assumption เรื่อง path embedded เงียบ ๆ ในโค้ด**. พอ rename directory สมมติฐานก็พัง — แต่ compiler ไม่ warn เพราะ syntactically valid. วิธีกัน:
1. ใช้ `ft_strlen(prefix)` ตรง ๆ + assert `<= sizeof(path) - 6`
2. หรือ `snprintf` ที่ overflow-safe (ถ้า norm ยอม)
3. หรือไม่ใช้ buffer fixed size — ใช้ `ft_strjoin`

ทุกทางเลือกอ่านง่ายและไม่มี hidden assumption

#### Step 2: Difficulty system

**Why:** "ASIAN" hardcore — user feedback ขอ regen tuning per difficulty level

**Design:**
- Add 4 levels: `DIFF_EASY (0)` → `DIFF_NORMAL (1)` → `DIFF_HARD (2)` → `DIFF_ASIAN (3)`
- `g_regen_mult[DIFF_COUNT] = {1.5, 1.0, 0.3, 0.0}` — table-driven, แก้ค่าได้ที่จุดเดียว
- `combat.c:143`: `health += REGEN_PER_TICK * g_regen_mult[game->difficulty]` — เปลี่ยนแค่จุดเดียว

| Difficulty | Regen rate | UX |
|---|---|---|
| EASY | x1.5 | heal ไว ใจดี |
| NORMAL | x1.0 | balance เดิม |
| HARD | x0.3 | regen ไม่ทัน damage → ต้องเก็บ healthkit |
| ASIAN | x0.0 | ไม่ regen เลย — ทุก HP ที่เสีย = ลด permanent จนกว่าจะเก็บ pickup |

**Lesson — table-driven config:** `if difficulty == EASY ... else if NORMAL ...` จะ ramify ทุกจุดที่ใช้ตัวเลขนี้. table `mult[diff]` = behavior อยู่ที่ data ไม่ใช่ control flow → ค่าใหม่/level ใหม่ = แก้ array เดียว

#### Step 3: Menu — เพิ่ม row + creature decor

**3a. Row "DIFFICULTY":** menu มี 2 options เดิม (START, SENSITIVITY) → เพิ่ม **DIFFICULTY** เป็น row ที่ 3
- `menu_index ∈ {0, 1, 2}`, UP/DOWN navigate, LEFT/RIGHT cycle ค่าของ row ปัจจุบัน
- `build_diff_label()` แปลง enum → string ("EASY"/"NORMAL"/"HARD"/"ASIAN")

**3b. Creature decoration:** user เพิ่ม `troop.png` (48x60) และ `caco.png` (63x66) → reuse texture เดิมที่ load ไว้ (`game->sp_tex[SP_TROOP/CACO]`) แทน load ซ้ำ
- `draw_creature(g, t, ox, oy)`: scale ความสูง 120px (รักษา aspect), nearest-neighbor, mask `alpha == FF` + `RGB != 0`
- วาดขนาบ title — troop ซ้าย, caco ขวา ที่ `WIDTH/2 ± 340`

**Lesson — reuse over reload:** sprite texture load แล้วใน sprite_load_tex → menu ใช้ pointer ตรง สู่ array เดิม. ถ้า load อีกครั้งเป็น `menu_troop_tex` แยก จะมี 2 copies ของ RGBA buffer ใน RAM = double leak risk + extra free path

#### Step 4: In-game settings — sens slider + back-to-menu

**ปัญหาเดิม:** กด `O` เปิด settings มีแค่ slider เสียงเพลง

**Refactor:**
- Layout 2 rows: MUSIC VOLUME + MOUSE SENSITIVITY (`settings_index ∈ {0, 1}`)
- Bottom buttons: `RESUME` + `MAIN MENU`
- Mouse: click bisect 6 regions (music ±, sens ±, resume, back) ผ่าน `hover_at()` returning 1-6
- Keyboard:
  - UP/DOWN → row navigation
  - LEFT/RIGHT → adjust row ปัจจุบัน (`settings_arrow()` dispatch)
  - ESC/O/ENTER → close (resume)
  - BACKSPACE → back to menu (alternative ที่เร็ว)

**Back-to-menu behavior:** `back_to_menu()` เรียก `restart_game()` ก่อน flip state → ทุก sprite/door/dialog reset, เลือกใหม่ได้

**Lesson — same widget pattern:** music + sens ใช้ slider เดียวกัน ต่างที่ `value` + `max_value` → `draw_slider_row()` parametric. **อย่ายอม copy-paste 2 slider implementations** เพราะถ้า style เปลี่ยน ต้องแก้ทั้งสอง

#### Step 5: Menu background image (JPG → PNG → fullscreen)

**Input:** `menu.jpg` 3840x2160 (4K, ~1.2MB)

**Problems:**
1. MLX42 รับ PNG เท่านั้น (ใช้ LodePNG)
2. 4K texture = ~33MB RGBA raw ใน RAM. แค่ menu BG ใหญ่กว่ารวม sprites/walls ทั้งโปรเจค

**Pipeline:**
```sh
convert menu.jpg \
    -resize 1280x720^ -gravity center -extent 1280x720 \
    -quality 95 resource/textures/menu_bg.png
```
- `-resize 1280x720^` = fill (ไม่ใช่ fit) — รักษา aspect, crop ที่เกิน
- `-gravity center -extent 1280x720` = crop จากกลาง output 1280x720 พอดี screen

**Code integration:**
- `t_img menu_bg` ใน `t_game`
- `main.c`: `load_png_tex(&menu_bg, ...)` แต่ **silent fallback** — ถ้า fail set `tex = NULL` ไม่ใช่ exit. เพราะ BG เป็น decoration ไม่ใช่ load-bearing
- `menu.c:draw_menu_bg()`: nearest-neighbor stretch + force `alpha = FF` (กัน RGB-only PNG ที่ alpha channel = 0 ทำให้ pixel ไม่เห็น)
- `render_menu()`: `if (menu_bg.tex) draw_menu_bg(); else { fill + mist + accents }` — backwards compat

**Lesson — fail-soft asset loading:** asset ที่ critical (wall textures, player position) ต้อง hard fail ตอน load. asset ที่ optional (BG art, decorative sprites) ต้อง soft fail. ถ้า fail-soft ทุกอย่าง bug ซ่อน. ถ้า hard-fail ทุกอย่าง one missing file = ไม่สามารถ run ได้เลย. **Tier asset โดย criticality**

#### Lesson summary (Phase 22)

| Decision | Cost ถ้าทำผิด | Cost ถ้าทำถูก |
|---|---|---|
| Magic number `n < 30` | Boot crash หลัง rename | ใช้ `ft_strlen` หรือ `snprintf` — กัน hidden assumption |
| Difficulty = if-chain | n สาขาของ if ในทุก usage | Table `mult[diff]` — แก้ค่าใหม่ระดับใหม่ที่จุดเดียว |
| Reload sprite สำหรับ menu | RAM x2, free path x2 | Reuse pointer ของ load ที่ทำแล้ว |
| Hard-fail BG load | ลบไฟล์เดียว → game ไม่ start | Silent fallback — BG หาย ก็ยังเล่นได้ |
| JPG เก็บ raw | MLX โหลดไม่ได้ + RAM 33MB | Convert PNG + resize ตรง screen — 3.5MB |

---

## 5. Deep Dives

### Deep Dive 1: ทำไม raycasting ดู 3D ทั้งที่ map เป็น 2D?

ลองคิดดู: เรามี top-down grid (2D). ในเฟรมเราสร้าง 1280 column วาดเส้นแนวตั้ง. ความสูงของเส้น = `HEIGHT / perp_wall_dist`.

ผลคือ:
- กำแพงใกล้ → เส้นสูงเต็มจอ
- กำแพงไกล → เส้นเตี้ย ปรากฏ "เล็กกว่า"
- กำแพงเฉียง → ทีละ column ระยะต่างกัน → ลาดเอียง = perspective

**Key**: 1280 column = 1280 ray = 1280 ระยะแตกต่างกัน. ตามองว่า "perspective" จริง ๆ แล้วเป็น "1280 เส้นความสูงแตกต่าง"

นี่คือ trick ของ Wolfenstein 3D (1992) ที่ทำให้ run ได้บน CPU 80286

### Deep Dive 2: ทำไม `inv_det` ในการ project sprite?

```c
inv_det = 1 / (plane_x * dir_y - dir_x * plane_y)
```

นี่คือ inverse ของ determinant matrix `[plane | dir]`. ถ้ามอง `plane` และ `dir` เป็น basis vectors ของ camera space → matrix นี้แปลง camera → world.

เราต้องการตรงกันข้าม — world → camera. ใช้ inverse:

```
[transform_x]   1     [ dir_y  -dir_x  ] [tx]
[transform_y] = --- × [-plane_y plane_x] [ty]
                det
```

`det = plane_x*dir_y - dir_x*plane_y`. ถ้า camera ตั้งฉาก dir⊥plane (อย่างที่ราาควรเป็น), det ≈ |plane|×|dir| (nonzero)

### Deep Dive 3: ทำไมต้อง single exit point?

```c
// cleanup_exit() — src/cleanup/cleanup.c
int cleanup_exit(t_game *game, char *msg):
    if msg: printf("Error\n%s\n", msg)
    if game:
        free_map()
        free_textures()
        free_mlx()
    exit(msg ? 1 : 0)
```

ทุกที่ที่ต้อง terminate (parse error, MLX init fail, ESC, win close, death press ESC) เรียก function นี้

**Why?**
1. **One place to free**: ถ้ามีหลายที่ free → ลืมที่ใดที่หนึ่ง = leak
2. **Idempotent**: NULL checks ก่อน free → เรียกซ้ำได้ปลอดภัย
3. **Clear exit code**: 1 = error, 0 = normal
4. **42 evaluator-friendly**: นับ alloc/free คู่กันได้ง่าย

**Pattern:** `memset(&game, 0, sizeof(t_game))` ที่ entry → ทุก pointer = NULL → cleanup ปลอดภัยแม้ alloc fail ตรงไหน

### Deep Dive 4: Header dependency in Makefile

```makefile
HEADERS = includes/cub3d.h

%.o: %.c $(HEADERS)
    $(CC) $(FLAGS) $(INC) -c $< -o $@
```

`%.o` depend ทั้ง `.c` และ header. ถ้า header เปลี่ยน → **rebuild ทุก .o**

**ทำไมต้องมี?** ถ้าไม่มี → struct เปลี่ยน → .c ไม่เปลี่ยน → .o ไม่ rebuild → binary ใช้ struct เก่ามาผสมใหม่ → **memory layout mismatch** → segfault สุ่ม ๆ

นี่คือ classic bug ที่เฮียเคยพลาดในโปรเจคนี้ (Phase 11 — เปลี่ยน t_game struct, segfault ลึกลับ)

---

## 6. Bugs ที่เราเจอ

### Bug 1: Segfault สุ่ม ๆ หลังเปลี่ยน struct

**Root cause:** Makefile ไม่ track header deps. struct size เปลี่ยน → .o เก่าใช้ offset เก่า → out-of-bounds write

**Fix:** `%.o: %.c $(HEADERS)` + `make re` หลังเปลี่ยน header

**Lesson:** ทุกครั้งที่แก้ struct → `make re` หรือ install proper header deps ใน Makefile ตั้งแต่แรก

---

### Bug 2: จอแดงค้างหลัง damage

**Root cause:** `draw_damage_flash` เขียน solid red `0x900000` ทับ pixel เลย. และ `dmg_flash` reset เป็น 8 ทุกเฟรมตอนผีอยู่ใกล้

**Fix:**
1. Alpha blend: `r = orig * (1-α) + 220 * α`
2. ลด aggression ของ reset: `if (dmg_flash < 8) dmg_flash = 8` แทน `= 8` เสมอ
3. Throttle: `if (dmg_flash == 0) snd_hurt()` → sound เล่นเฉพาะ damage event ใหม่

**Lesson:** Visual feedback ต้อง **blend** กับสิ่งที่อยู่ใต้ — ไม่ใช่ทับสนิท. และต้อง throttle ถ้า trigger ทุกเฟรม

---

### Bug 3: รูปปืนมีขอบขาวรอบ ๆ

**Root cause:** XPM ที่ export มาจาก image editor มักไม่ mark white background เป็น `c None`. `mlx_xpm_file_to_image` โหลดมาทั้ง white area → render ทับจอ

**Fix:** Flood fill จาก edge pixel ที่เป็น `@ c white` → mark เป็น `# c None`:

```python
# Python preprocessing
queue all edge pixels with value '@'
BFS: pop pixel, set to '#', push neighbors
result: interior '@' stays, exterior becomes None
```

**Lesson:** XPM transparency ต้อง **explicit** — `c None` ไม่ใช่สี. การ post-process XPM ก่อน load คือ standard workflow

---

### Bug 4: Mouse stuck ที่ขอบจอ

**Root cause:** MotionNotify event fire เฉพาะเมื่อ mouse เลื่อนใน window. พอ cursor ออก window → event หยุด

**Fix:** Frame-poll pattern แทน event hook:

```c
update_mouse_look():
    mlx_mouse_get_pos(&mx, &my)
    delta = mx - WIDTH/2
    rotate by delta
    mlx_mouse_move(WIDTH/2, HEIGHT/2)   // ดึง cursor กลับ
```

**Lesson:** สำหรับ continuous input (mouse look), poll + warp pattern เสถียรกว่า event hook

---

### Bug 5: Map validation ล้มเหลวที่กำแพงแนวขอบ

**Root cause:** flood_fill ลำดับเช็คผิด — เช็ค `visited[y][x]` ก่อน `x >= 0` → out-of-bounds array access on negative index

**Fix:** OOB check ก่อนทุกอย่าง:

```c
flood_fill(x, y):
    if (x < 0 || x >= w || y < 0 || y >= h) return 1;   // ← FIRST
    if (visited[y][x]) return 0;
    visited[y][x] = 1;
    // ...
```

**Lesson:** ลำดับการ check matters. ทุก array access ต้อง bounds-checked ก่อนเสมอ

---

### Bug 6: เปิดประตูเดินผ่านแล้ว stuck

**Symptom:** กด E เปิดประตู → เดินผ่าน → ตัวค้างใน door cell ออกไม่ได้

**Root cause:** `tick_one()` ตอน `DOOR_CLOSING` flip `grid[y][x]` กลับเป็น `'D'` ที่ครึ่งทาง animation **โดยไม่เช็คว่า player ยืนอยู่บน cell หรือเปล่า**. เกิดขึ้นเมื่อ:
1. Player เปิด door, เดินเข้า cell
2. กด E อีกครั้ง (ตั้งใจ interact NPC หรือเผลอ) → state = DOOR_CLOSING
3. timer ลงถึงครึ่ง → cell flip `'0'→'D'` → `is_blocked()` คืน 1 → ติด

**Fix (Phase 21 Step 1):**
1. ลบ cell flipping ออกทั้งหมด — cell อยู่ที่ `'D'` ตลอด
2. `is_blocked` / `perform_dda` ตรวจ `door->state` แทน
3. CLOSING ตอน player ยืนอยู่บน cell → revert ไป OPENING (safety guard)

**Lesson:** เมื่อมี 2 sources of truth (map cell + door state) แล้ว state เปลี่ยนพร้อม ๆ player เคลื่อนที่ → race condition. แก้ที่ root → ลด source of truth

---

### Bug 7: Wall texture mirror ผิดด้าน

**Symptom:** ผนังบางด้าน texture ที่มีลาย directional (เช่น ลายพื้นทะแยง) flip กลับด้านเหมือนกระจกเงา

**Root cause:** `calc_tex_x()` mirror conditions เป็น:
```c
if (side == 0 && ray_dir_x > 0) tex_x = w - tex_x - 1;
if (side == 1 && ray_dir_y < 0) tex_x = w - tex_x - 1;
```

แต่ Lodev raycaster's standard (ที่สอดคล้องกับ wall_x formula `pos_y + perp*ray_dir_y` ที่ใช้) คือ ray_dir_x **<** 0 และ ray_dir_y **>** 0 — direction ตรงข้าม

**Fix:** กลับ comparison direction (ดู Phase 21 Step 3)

**Lesson:** อย่า copy code จาก reference โดยไม่ verify formula ตัวอื่นรอบ ๆ. mirror direction ผูกกับ wall_x formula — เปลี่ยนตัวหนึ่งต้อง update อีกตัว

---

### Bug 8: Door texture แสดงแค่ครึ่งบน

**Symptom:** ยืนใกล้ประตู → เห็น texture ครึ่งบนเท่านั้น (ส่วน handle หาย)

**Root cause (เทคนิค):** raycaster ปกติ map texture height ลง `line_height` (เปอร์สเปคทีฟปกติของ wall). ตอน player ใกล้ → `line_height` > `HEIGHT` → `draw_start/draw_end` clamp ที่ขอบจอ → texture ที่เห็นเป็น "center crop" ของรูป (ตอนกลาง). สำหรับ wall ดูปกติ แต่สำหรับ door ที่ภาพมี content เฉพาะ (handle, panel) — center crop = หาย handle

**Fix (Phase 21 Step 2):**
- รู้ทันว่า door รูปต้องเห็นเต็มเสมอ → branch แยกใน draw_column
- `step = tex_height / visible_h`, `tex_pos = 0` → ภาพ door fit visible region 100%

**Lesson:** convention ของ raycaster (center crop wall) ไม่เหมาะกับทุก asset. asset ที่มี directional content (door, sign, painting) ต้อง render แบบ stretch

---

### Bug 9: "DEN DOOR" watermark บนกำแพง

**Symptom:** wall_type 4 ในแมพมีตัวอักษร "DEN DOOR" + frame ดำที่ขอบ

**Root cause:** `detail_mold.png` (textures/horror/) ไม่ใช่ tileable wall texture — เป็น **AI-generated sprite sheet** ที่:
- ขนาด 128×128 มี 4 cells (2×2)
- กลางมี watermark "DEN DOOR" (label จากเครื่องมือ generate)
- มี black border ที่ขอบ

ไฟล์อื่นในชุดเดียวกัน (`detail_blood_stain.png`, `detail_wall_damage.png`) ก็มี black frame แบบเดียวกัน — ไม่ใช่ texture ที่ tile ได้

**Fix:** ลบ `NO4`/`SO4`/`WE4`/`EA4` จาก `abyss.cub` ทั้งหมด, ภายหลังลบ `NO2`/`NO3` ด้วย → fallback ลง wall_type 1 (clean horror)

**Lesson:** AI-generated textures ต้อง **inspect** ก่อนใช้. ดู resolution, ดู border, ดูว่ามี text artifact ไหม. asset pipeline ที่ดีคือมี curation step

---

### Bug 10: จอแดงกระพริบตอนเดิน (ไม่ใช่ damage)

**Symptom:** เดินเฉย ๆ มีจอแดงกระพริบเป็น period (10-20s)

**Root cause:** `ambient_scare_tick()` ใน `audio.c` set `game->player.dmg_flash = 6` ทุก period — ที่ตั้งใจไว้คือ "ทำให้ผู้เล่นตกใจ" — แต่ดูเหมือนถูก enemy attack ทำให้ confuse + รำคาญ

**Fix:** ลบบรรทัด `dmg_flash = 6` ออก. คง `flicker_timer = 18` + `snd_growl()` ไว้ → atmosphere ผ่านเสียง + แสงดิ้น ไม่ใช่ false-positive HP loss

**Lesson:** visual feedback ที่ overlap กับ "ผู้เล่นถูกตี" สื่อ wrong information. ambient effect ต้องแยกชัดจาก damage feedback — ถ้าใช้สีเดียวกัน user สับสน. **อย่ารวม "ตกใจ" กับ "เสียเลือด" ใน channel ภาพเดียวกัน**

### Bug 11: Boot crash "Failed to load gun texture" หลัง directory restructure

**Symptom:** หลัง Phase 21 Step 6 ย้าย `./textures/` → `./resource/textures/` แล้ว run `./cub3D resource/maps/abyss.cub` พิมพ์ `Error\nFailed to load gun texture` แล้ว exit ทันที. walls/sprites/sounds load ได้หมด — แต่อาวุธพังก่อนถึง menu

**Investigation flow:**
1. `ls resource/textures/weapons/` — ไฟล์ครบทั้ง 24 ไฟล์ (machete/pistol/bolter 0-7) ✓
2. `file machete_0.png` — PNG valid 290x160 RGBA ✓
3. Binary mtime ตรงกับ source mtime — rebuild ครบ ✓
4. `strace -e openat -f ./cub3D ...` → เจอ `openat(..., "./resource/textures/weapons/ma0.png") = -1 ENOENT`

**Root cause:** `gun.c:14` มี loop copy prefix ที่ฮาร์ดโค้ด magic limit:
```c
while (prefix[n] && n < 30)
    path[n++] = prefix[n];
```
- Path เดิม `"./textures/weapons/machete_"` = 27 chars → ลอด `< 30`
- Path ใหม่ `"./resource/textures/weapons/machete_"` = 36 chars → ตัดที่ index 30 = `./resource/textures/weapons/ma` → ต่อด้วย `'0' + '.' + 'p' + 'n' + 'g'` = `./resource/textures/weapons/ma0.png` — ไฟล์ไม่มี

**Fix:** `n < 30` → `n < 50` (buffer คือ `path[64]`, รองรับสบาย)

**Lesson:** Phase 21 ย้าย path เปลี่ยน prefix ยาวขึ้น 9 chars — compiler ไม่ catch เพราะ syntax ถูก, test ไม่ catch เพราะไม่มี automation. Bug ค้นพบที่ first runtime หลัง restructure. **Take-away:** ทุก magic number ที่ embed assumption เกี่ยวกับ data ภายนอก (path length, buffer size, frame count) → label ไว้เป็น tech debt. ถ้าทำใหม่ใช้ `ft_strlen()` ตรง ๆ — มันคำนวณ runtime ไม่ assume

---

## 7. File-by-file reference

| File | LOC | Role | Key functions |
|------|-----|------|---------------|
| `main.c` | ~40 | Entry, init, register hooks, mlx_loop | `main()` |
| `cleanup/cleanup.c` | 80+ | Single exit point | `cleanup_exit()`, `free_*` |
| `events/events.c` | 60+ | Key/mouse hooks, state-aware | `key_press()`, `mouse_press()` |
| `mlx/mlx_init.c` | 30 | MLX setup | `init_mlx()` |
| `mlx/mlx_utils.c` | 30 | Raw pixel access | `put_pixel()`, `get_pixel()` |
| `parsing/parse_file.c` | 81 | Pipeline orchestrator | `parse_cub()` |
| `parsing/parse_elements.c` | 167 | NO/SO/WE/EA/F/C + multi-type | `parse_element_line()` |
| `parsing/parse_map.c` | 103 | Grid loader, char validator | `parse_map()` |
| `parsing/validate_map.c` | 127 | Flood-fill closure check | `validate_map()` |
| `player/player.c` | 130+ | Init, move, rotate, stamina | `init_player()`, `move_player()` |
| `raycast/raycast.c` | 100+ | DDA 4 steps | `init_ray()`, `perform_dda()` |
| `render/render_frame.c` | 75 | Per-frame orchestrator | `render_frame()` |
| `render/render_wall.c` | 90 | Wall column + fog + texture | `draw_column()`, `select_texture()` |
| `render/render_flat.c` | 25 | Floor/ceiling fill | `draw_flat()` |
| `textures/texture_load.c` | 75 | XPM loading + fallback | `load_textures()` |
| `utils/gnl.c` | 70 | get_next_line | `get_next_line()` |
| `utils/utils.c` | 200+ | Mini-libft | `ft_strlen()`, `ft_split()`, ... |
| `bonus/combat.c` | 164 | Hit detection + damage/regen | `shoot()`, `update_combat()`, `update_aim()` |
| `bonus/dialog.c` | 59 | Tutorial subtitles | `dialog_start/update/render()` |
| `bonus/door.c` | 79 | Toggle adjacent door | `init_doors()`, `use_door()` |
| `bonus/ghost_ai.c` | 85 | Steering + growl | `update_ghosts()`, `chase_player()` |
| `bonus/gun.c` | 128 | Weapon overlay + bob + flash | `render_gun()`, `load_gun_tex()` |
| `bonus/highscore.c` | 69 | Save/load file | `load_high_score()`, `save_high_score()` |
| `bonus/hud.c` | 253 | All HUD layers | `render_hud()`, `render_text_overlay()` |
| `bonus/menu.c` | 196 | Title screen + state transition | `render_menu()`, `handle_menu_key()` |
| `bonus/minimap.c` | 78 | Overlay map | `render_minimap()` |
| `bonus/mouse.c` | 50 | Poll + warp | `update_mouse_look()` |
| `bonus/pickups.c` | 104 | Consume + respawn | `update_pickups()` |
| `bonus/restart.c` | 64 | Full reset | `restart_game()` |
| `bonus/sound.c` | 70 | aplay wrappers | `snd_*()` |
| `bonus/sprite.c` | 260 | Z-buffered sprite renderer | `render_sprites()`, `init_sprites()` |
| `bonus/vignette.c` | 43 | Edge darkening | `apply_vignette()` |

---

## 8. Tuning table

ทุกค่าใน `includes/cub3d.h`. แก้แล้ว `make re` (header dep)

### Window / FOV
| Constant | Default | Effect |
|----------|---------|--------|
| `WIDTH` | 1280 | จอกว้าง |
| `HEIGHT` | 720 | จอสูง |
| `plane length` (in `set_dir`) | 0.60 | FOV ~62° |

### Movement
| Constant | Default | Effect |
|----------|---------|--------|
| `MOVE_SLOW` | 0.025 | เดินปกติ |
| `MOVE_FAST` | 0.055 | วิ่ง |
| `ROT_SPEED` | 0.03 | ความเร็วหมุน (keyboard arrow) |
| `WALL_MARGIN` | 0.2 | (unused — เผื่อทำ collision margin) |

### Survival
| Constant | Default | Effect |
|----------|---------|--------|
| `HP_MAX` | 100 | เลือดเต็ม |
| `DMG_RADIUS` | 1.4 | ผีโดนเลือดในระยะนี้ |
| `DMG_PER_TICK` | 0.10 | damage ต่อเฟรม (6/sec) |
| `REGEN_PER_TICK` | 0.12 | base regen ต่อเฟรม (คูณด้วย difficulty mult) |
| `STAMINA_MAX` | 100 | สตามิน่าเต็ม |
| `STAMINA_DRAIN` | 0.55 | sprint drain |
| `STAMINA_REGEN` | 0.22 | regen เมื่อหยุด |
| `SPRINT_MIN` | 8 | threshold ให้ sprint ได้ |

### Difficulty (Phase 22)
| Constant / Field | Value | Effect |
|----------|---------|--------|
| `DIFF_EASY/NORMAL/HARD/ASIAN` | 0/1/2/3 | enum levels |
| `g_regen_mult[]` in `combat.c` | `{1.5, 1.0, 0.3, 0.0}` | regen multiplier ต่อ level |
| `game->difficulty` | default `DIFF_NORMAL` | ค่าเริ่ม, เลือกได้ที่ menu (UP/DOWN ไปที่ row 3, LEFT/RIGHT cycle) |

### Combat
| Constant | Default | Effect |
|----------|---------|--------|
| `SHOOT_RANGE` | 10 | ระยะปืน |
| `SHOOT_RADIUS` | 0.4 | ความกว้าง cone ปืน |
| `KNIFE_RANGE` | 1.8 | ระยะมีด |
| `KNIFE_RADIUS` | 0.7 | กรวยมีด |
| `SPRITE_HP` | 3 | กี่นัดถึงตาย |
| `MUZZLE_FRAMES` | 6 | flash duration |

### Pickups
| Constant | Default | Effect |
|----------|---------|--------|
| `HEAL_AMOUNT` | 30 | HP ต่อ healthkit |
| `AMMO_AMOUNT` | 8 | นัดต่อ ammo box |
| `AMMO_START` | 16 | นัดเริ่ม |
| `AMMO_MAX` | 40 | นัด max |
| `PICKUP_RADIUS` | 0.6 | ระยะเก็บ |
| `PICKUP_RESPAWN_FRAMES` | 600 | 10s respawn |

### AI
| Constant | Default | Effect |
|----------|---------|--------|
| `GHOST_SPEED` | 0.010 | ความเร็วผี (ช้ากว่าผู้เล่น) |
| `GHOST_MIN_DIST` | 0.6 | ระยะหยุด (กันยืนทับผู้เล่น) |

### Atmosphere
| Constant | Default | Effect |
|----------|---------|--------|
| `FOG_START` | 2.0 | ระยะเริ่มฟอก |
| `FOG_END` | 12.0 | ระยะมืดสุด |

### Mouse / Sensitivity
| Constant | Default | Effect |
|----------|---------|--------|
| `SENS_DEFAULT` | 25 | ค่าเริ่ม (level × 0.0001) |
| `SENS_MIN/MAX` | 2/100 | range |
| `SENS_STEP` | 2 | ปรับครั้งละ |

---

## 9. Build / Run / Debug

### Build
```bash
cd making-ref
make           # incremental
make re        # full rebuild (จำเป็นหลังแก้ header)
make clean     # ลบ .o
make fclean    # ลบ .o + binary
```

### Run
```bash
./cub3D maps/abyss.cub
```

### Controls
| Key | Action |
|-----|--------|
| `W/A/S/D` | เดิน |
| `Ctrl` / `Shift` | วิ่ง (กิน stamina) |
| `Mouse` | หมุน (locked) |
| `LMB` | ยิง / แทง |
| `1` / `2` | เปลี่ยน gun/knife |
| `E` | เปิด/ปิดประตู |
| `Space` | ข้าม dialog |
| `R` | restart (เมื่อตาย/ชนะ) |
| `ESC` | ออก |

### Debug helpers

#### Valgrind quick
```bash
timeout 10 valgrind --leak-check=no --error-exitcode=99 ./cub3D maps/abyss.cub
# มอง "ERROR SUMMARY: 0 errors" → ปลอดภัย
```

#### ดู cell content
```bash
awk '/^[01-9DGHAX]/' maps/abyss.cub | head
```

#### ดูทุก wall type
```bash
grep -oE '[1234]' maps/abyss.cub | sort | uniq -c
```

#### Pre-compile check
```bash
make 2>&1 | grep -E "error|warning" | head
```

---

## 10. ถ้าจะต่อ

### ระดับ easy (~1 ชั่วโมง)

- เพิ่ม map ใหม่: `maps/labyrinth.cub`, `maps/cathedral.cub` → ให้ menu มี map select
- ฟีเจอร์ "pause" (state ใหม่ STATE_PAUSED): กด P หยุดเกม + show overlay
- Counter "time alive" บน HUD
- Hit confirm: เมื่อยิงโดน → crosshair flash X color

### ระดับ medium (~1 วัน)

- **Bullet impact spark**: เมื่อยิงไม่โดน → raycast หา wall hit point → render spark ตรงนั้นชั่วครู่
- **Multiple sprite frames** (true animation): ghost.xpm, ghost1.xpm, ghost2.xpm → cycle ตาม anim_tick
- **Pickup notification queue**: หลายการเก็บใน 1 sec → stack ไม่ทับกัน
- **Stamina regen scaling**: regen เร็วขึ้นเมื่อ stamina < 30
- **Audio mixer**: aplay จำกัด — ลองใช้ paplay หรือ port to BASS

### ระดับ hard (~สัปดาห์)

- **Real font sheet**: ลอด PNG bitmap font → XPM → render character ทีละ glyph แทน `mlx_string_put` (พอเอาไปส่ง 42 ได้เพราะ Norm-safe)
- **Pathfinding** สำหรับผี: ที่มันจะอ้อมกำแพงแทนจะติด. A* search ผ่าน map
- **Save/load game**: serialize player + sprite state ลงไฟล์ binary
- **Multiple weapon types** (gun, shotgun, rocket): each with stats + reload + animation
- **Difficulty levels**: easy/medium/hard ต่าง DMG_PER_TICK + SPRITE_HP

### ระดับ "เกินขอบเขต 42" (~เดือน)

- **Real 3D model rendering**: เพิ่ม Z dimension ให้ ray, support stairs/elevation
- **Procedural map generator**: BSP dungeon generation
- **Network multiplayer**: 2 players race ผ่าน map
- **Particle system**: smoke, blood, sparks
- **Real audio with BASS**: positional sound (ผีอยู่ทางซ้าย → ดังจากซ้าย)

---

## ส่งท้าย

น้องเตอร์, โปรเจคนี้ครอบคลุม:

- **C systems programming**: structs, function pointers, void *, manual memory management
- **2D math**: vectors, dot product, rotation matrix, perspective projection
- **Algorithms**: DDA raycasting, flood fill, bubble sort, BFS (XPM flood), Q16/Q8 fixed-point
- **State machines**: menu/playing, dialog timer, weapon switching, damage flash, door OPENING→OPEN→CLOSING
- **Software architecture**: single header, god struct, single exit point, header deps, "lone source of truth" principle (Phase 21 door refactor)
- **OS/IPC**: fork via system(), file I/O, X11 mouse warp, monotonic clock for channel debounce
- **Asset pipeline**: PNG/XPM curation, watermark inspection, WAV synthesis via ffmpeg
- **Build system**: Make incremental, header dependency tracking
- **Debugging**: how segfaults happen, how to read valgrind, how to spot ABI mismatch, race conditions in state machines
- **Algorithm porting**: read Wolf3D (1992) + DOOM (1993) source code, extract ideas, adapt to modern truecolor renderer

เฮียอยากให้น้องอ่าน walkthrough นี้ **แล้วลองอธิบายคืน** ให้ตัวเองฟัง (rubber duck) ที่ละ phase. ถ้าอธิบายได้ = เข้าใจจริง. ถ้าติดตรงไหน = กลับมาอ่านใหม่หรือถามเฮีย

42 evaluator มักถามคำถามแบบ:
- "ทำไมต้อง perpendicular distance?"
- "ทำไม sprite ต้อง sort?"
- "ถ้าผมเปลี่ยน FOG_END เป็น 100 จะเกิดอะไร?"
- "อธิบายว่าทำไม collision ต้องเช็คทีละแกน"

ทุกคำถามนี้ — คำตอบอยู่ใน walkthrough นี้แล้ว

ขอให้น้องไป eval แล้วได้ 125/100 นะ ขออภัยถ้าเฮียเขียนยาวไป

— เฮีย, May 18 2026

**Iteration 2 update — May 19 2026:** เพิ่ม Phase 21 (door state refactor + Wolf3D/DOOM algorithm porting), Bugs 6-10, fog Q8 precompute, sprite Q16 fixed-point, 8-channel sound debounce, door state-based passability. Build ยังคง clean `-Wall -Wextra -Werror`

**Repo cleanup — May 19 2026 (later):** restructure root ใหม่เป็น 3-bucket layout (`resource/`, `resource/references/`, `garbage/`). ทุก runtime path เปลี่ยนเป็น `./resource/...`. project root จากที่มี 25+ entries เหลือ 6 entries. Build ยัง clean

**Iteration 3 update — May 19 2026 (evening):** เพิ่ม Phase 22 (difficulty system 4 levels, in-game settings refactor พร้อม sens slider + back-to-menu, menu creature decoration ขนาบ title, JPG→PNG menu background pipeline), Bug 11 (gun.c prefix truncation จาก path rename), tuning table Difficulty section. Asset pipeline ขยายให้รับ JPG ผ่าน ImageMagick convert. Pattern ใหม่: table-driven config + tiered asset loading (hard-fail vs silent-fallback). Build ยัง clean `-Wall -Wextra -Werror`
