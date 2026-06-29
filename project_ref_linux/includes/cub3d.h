#ifndef CUB3D_H
# define CUB3D_H

# include <math.h>
# include <fcntl.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <stdint.h>
# include "MLX42/MLX42.h"

/* ─── Screen ─────────────────────────────── */
# define WIDTH      1280
# define HEIGHT     720
# define TITLE      "Abyss"

/* ─── Movement ───────────────────────────── */
# define MOVE_SLOW   0.025
# define MOVE_FAST   0.055
# define ROT_SPEED   0.03
# define WALL_MARGIN 0.2

/* ─── Survival horror ────────────────────── */
# define HP_MAX         100.0
# define DMG_RADIUS     1.4
# define DMG_PER_TICK   0.10
# define REGEN_PER_TICK 0.12
# define STAMINA_MAX    100.0
# define STAMINA_DRAIN  0.55
# define STAMINA_REGEN  0.22
# define SPRINT_MIN     8.0
# define SHOOT_RANGE    10.0
# define SHOOT_RADIUS   0.4
# define SPRITE_HP      3
# define MUZZLE_FRAMES  6
# define GHOST_SPEED    0.010
# define GHOST_MIN_DIST 0.6
# define GHOST_DETECT_RANGE 6.0
# define LOS_STEP       0.1

/* ─── Weapons ────────────────────────────── */
# define MAX_WEAPONS    3
# define WEAPON_FRAMES  8
# define WEP_MACHETE    0
# define WEP_PISTOL     1
# define WEP_BOLTER     2
# define WEAPON_ANIM_TICKS 24

/* ─── Fog (horror atmosphere) ────────────── */
# define FOG_START  1.0
# define FOG_END    6.0
# define FOG_FLOOR  0.02

/* ─── Texture indices ────────────────────── */
# define NO         0
# define SO         1
# define WE         2
# define EA         3

/* ─── Wall types ─────────────────────────── */
# define MAX_WALL_TYPES 4
# define HIT_DOOR       99
# define DOOR_ANIM_MAX  30

/* ─── Key bindings (MLX42 keysyms) ───────── */
# define KEY_W      MLX_KEY_W
# define KEY_A      MLX_KEY_A
# define KEY_S      MLX_KEY_S
# define KEY_D      MLX_KEY_D
# define KEY_E      MLX_KEY_E
# define KEY_R      MLX_KEY_R
# define KEY_SPACE  MLX_KEY_SPACE
# define KEY_1      MLX_KEY_1
# define KEY_2      MLX_KEY_2
# define KEY_3      MLX_KEY_3
# define KEY_LCTRL  MLX_KEY_LEFT_CONTROL
# define KEY_LSHIFT MLX_KEY_LEFT_SHIFT
# define KEY_LEFT   MLX_KEY_LEFT
# define KEY_RIGHT  MLX_KEY_RIGHT
# define KEY_UP     MLX_KEY_UP
# define KEY_DOWN   MLX_KEY_DOWN
# define KEY_ENTER  MLX_KEY_ENTER
# define KEY_ESC    MLX_KEY_ESCAPE
# define KEY_T      MLX_KEY_T
# define KEY_O      MLX_KEY_O
# define KEY_BACKSPACE MLX_KEY_BACKSPACE

/* ─── Game state ─────────────────────────── */
# define STATE_MENU    0
# define STATE_PLAYING 1
# define STATE_CHEAT   2
# define STATE_SETTINGS 3

/* ─── Cheat ──────────────────────────────── */
# define CHEAT_CODE    "42bkk"
# define CHEAT_MAX_LEN 16

/* ─── Sensitivity ────────────────────────── */
# define SENS_MIN     2
# define SENS_MAX     100
# define SENS_DEFAULT 25
# define SENS_STEP    2

/* ─── Difficulty ─────────────────────────── */
# define DIFF_EASY    0
# define DIFF_NORMAL  1
# define DIFF_HARD    2
# define DIFF_ASIAN   3
# define DIFF_COUNT   4

/* ─── Mouse buttons ──────────────────────── */
# define MOUSE_LEFT 1

/* ─── GNL ────────────────────────────────── */
# define BUFFER_SIZE    1024

/* ─── Bonus limits ───────────────────────── */
# define MAX_DOORS    64
# define MAX_SPRITES  64
# define MINIMAP_T    6
# define MINIMAP_PAD  5
# define MAX_STR_IMGS 256

/* ─── MLX42 font metrics ─────────────────── */
# define FONT_W       10
# define FONT_H       20

/* ─── Sprite types ───────────────────────── */
# define SP_GHOST    0
# define SP_HEAL     1
# define SP_AMMO     2
# define SP_EXIT     3
# define SP_NPC      4
# define SP_JSCARE   5
# define SP_TROOP    6
# define SP_CACO     7
# define SP_TEX_N    8
# define JSCARE_ANIM_N 8

/* ─── Pickup tuning ──────────────────────── */
# define PICKUP_RADIUS 0.6
# define HEAL_AMOUNT   30
# define AMMO_AMOUNT   8
# define AMMO_START    16
# define AMMO_MAX      40
# define PICKUP_RESPAWN_FRAMES 600

/* ─── NPC / Jumpscare ────────────────────── */
# define NPC_INTERACT_RADIUS 1.6
# define JSCARE_TRIGGER_DIST 2.5
# define JSCARE_FRAMES       70
# define JSCARE_COOLDOWN     300
# define JSCARE_ANIM_FRAMES  8

/* ──────────────────────────────────────────
   Structs
   ────────────────────────────────────────── */

typedef struct s_img
{
	mlx_image_t		*img;
	mlx_texture_t	*tex;
	int				width;
	int				height;
}	t_img;

typedef struct s_mlx
{
	mlx_t	*mlx;
	t_img	screen;
}	t_mlx;

typedef struct s_map
{
	char	**grid;
	int		width;
	int		height;
}	t_map;

typedef struct s_player
{
	double	pos_x;
	double	pos_y;
	double	dir_x;
	double	dir_y;
	double	plane_x;
	double	plane_y;
	double	spawn_x;
	double	spawn_y;
	double	spawn_dir_x;
	double	spawn_dir_y;
	double	spawn_plane_x;
	double	spawn_plane_y;
	int		move_fb;
	int		move_lr;
	int		rotate;
	int		sprint;
	double	health;
	double	stamina;
	int		ammo;
	int		kills;
	int		weapon;
	int		shake;
	int		dmg_flash;
	int		shoot_flash;
	int		recoil;
	int		aim_on_target;
	int		dead;
}	t_player;

typedef struct s_ray
{
	double	camera_x;
	double	ray_dir_x;
	double	ray_dir_y;
	int		map_x;
	int		map_y;
	double	delta_dist_x;
	double	delta_dist_y;
	double	side_dist_x;
	double	side_dist_y;
	int		step_x;
	int		step_y;
	int		hit;
	int		hit_type;
	int		side;
	double	perp_wall_dist;
	int		line_height;
	int		draw_start;
	int		draw_end;
	double	wall_x;
	int		tex_x;
}	t_ray;

typedef struct s_door
{
	int		x;
	int		y;
	int		timer;
	int		state;
}	t_door;

# define DOOR_CLOSED   0
# define DOOR_OPENING  1
# define DOOR_OPEN     2
# define DOOR_CLOSING  3

typedef struct s_sprite
{
	double	x;
	double	y;
	double	orig_x;
	double	orig_y;
	double	dist;
	int		hp;
	int		alive;
	int		type;
	int		respawn_timer;
}	t_sprite;

typedef struct s_npc
{
	double	x;
	double	y;
	int		spawned;
	int		dialog_done;
	int		dialog_active;
	int		gun_unlocked;
	int		minimap_unlocked;
}	t_npc;

typedef struct s_jumpscare
{
	int	active;
	int	frame;
	int	cooldown;
}	t_jumpscare;

typedef struct s_cheat
{
	int		god_mode;
	char	input[CHEAT_MAX_LEN + 1];
	int		input_len;
	int		msg_timer;
	int		msg_ok;
}	t_cheat;

typedef struct s_weapon
{
	const char	*name;
	t_img		frames[WEAPON_FRAMES];
	int			frame_count;
	double		range;
	double		radius;
	int			uses_ammo;
	int			locked;
}	t_weapon;

typedef struct s_game
{
	t_mlx			mlx;
	t_map			map;
	t_player		player;
	t_img			tex[MAX_WALL_TYPES][4];
	char			*tex_path[MAX_WALL_TYPES][4];
	int				floor_color;
	int				ceil_color;
	t_door			doors[MAX_DOORS];
	int				door_count;
	t_img			door_tex[4];
	char			*door_path[4];
	t_sprite		sprites[MAX_SPRITES];
	int				sprite_count;
	t_img			sp_tex[SP_TEX_N];
	char			*sp_path[SP_TEX_N];
	t_img			gun_tex;
	char			*gun_path;
	t_img			knife_tex;
	char			*knife_path;
	t_img			menu_bg;
	double			zbuf[WIDTH];
	int				anim_tick;
	int				last_mouse_x;
	int				state;
	int				menu_index;
	int				sens_level;
	int				difficulty;
	int				settings_index;
	int				won;
	int				notif_timer;
	char			notif_text[16];
	int				high_score;
	int				dialog_index;
	int				dialog_timer;
	t_npc			npc;
	t_jumpscare		jscare;
	t_cheat			cheat;
	double			light_mult;
	int				flicker_timer;
	t_img			jscare_frames[JSCARE_ANIM_N];
	int				jscare_frame_count;
	t_weapon		weapons[MAX_WEAPONS];
	int				active_weapon;
	int				weapon_anim_timer;
	int				weapon_anim_frame;
	mlx_image_t		*str_imgs[MAX_STR_IMGS];
	int				str_count;
}	t_game;

/* ──────────────────────────────────────────
   Prototypes
   ────────────────────────────────────────── */

/* parsing */
int		parse_cub(t_game *game, char *path);
int		parse_elements(t_game *game, int fd, char **first_map_line);
int		parse_element_line(t_game *game, char *line);
int		parse_map(t_game *game, int fd, char *first_line);
int		validate_map(t_game *game);

/* mlx */
int		init_mlx(t_game *game);
void	put_pixel(t_img *img, int x, int y, int color);
int		get_pixel(t_img *img, int x, int y);
void	clear_strings(t_game *game);
void	put_string(t_game *game, int x, int y, const char *str);

/* textures */
int		load_textures(t_game *game);
int		load_png_tex(t_img *t, const char *path);

/* player */
void	init_player(t_game *game);
void	update_player(t_game *game);
int		is_blocked(t_game *game, int x, int y);
int		has_los(t_game *game, double sx, double sy);
int		is_enemy_type(int type);
double	enemy_speed(int type);
double	enemy_dmg(int type);

/* events */
void	key_hook(mlx_key_data_t kd, void *param);
void	mouse_hook(mouse_key_t btn, action_t act,
			modifier_key_t mods, void *param);
void	close_hook(void *param);

/* raycast */
void	init_ray(t_ray *ray, t_player *p, int x);
void	init_step(t_ray *ray, t_player *p);
void	perform_dda(t_ray *ray, t_game *game);
void	calc_wall_data(t_ray *ray);

/* render */
void	render_frame(void *param);
void	draw_column(t_game *game, t_ray *ray, int x);
void	draw_flat(t_img *screen, int x, int y_start, int y_end, int color);

/* bonus */
void	render_minimap(t_game *game);
void	init_doors(t_game *game);
void	use_door(t_game *game);
void	update_doors(t_game *game);
int		load_door_tex(t_game *game);
t_door	*door_at(t_game *game, int x, int y);
void	init_sprites(t_game *game);
void	render_sprites(t_game *game);
void	update_ghosts(t_game *game);
void	update_pickups(t_game *game);
void	update_aim(t_game *game);
void	restart_game(t_game *game);
int		load_sp_tex(t_game *game);
void	update_mouse_look(t_game *game);
void	render_menu(t_game *game);
void	handle_menu_key(t_game *game, int key);

/* audio + ambience */
void	music_start(void);
void	music_stop(void);
void	music_set_volume(int vol);
int		music_get_volume(void);
void	ambient_scare_tick(t_game *game);
void	update_lighting(t_game *game);
double	current_light(t_game *game);

/* settings */
void	open_settings(t_game *game);
void	close_settings(t_game *game);
void	settings_keypress(t_game *game, mlx_key_data_t kd);
void	settings_click(t_game *game);
void	render_settings(t_game *game);

/* sound */
void	snd_shoot(void);
void	snd_hit(void);
void	snd_pickup(void);
void	snd_hurt(void);
void	snd_door(void);
void	snd_death(void);
void	snd_footstep(void);
void	snd_growl(void);

/* highscore */
int		load_high_score(void);
void	save_high_score(int score);

/* dialog + hud */
void	dialog_start(t_game *game);
void	dialog_update(t_game *game);
void	dialog_render(t_game *game);
void	render_hud(t_game *game);
void	render_text_overlay(t_game *game);
void	render_gun(t_game *game);
int		load_gun_tex(t_game *game);
void	shoot(t_game *game);
void	update_combat(t_game *game);
void	apply_vignette(t_game *game);
void	update_weapon_anim(t_game *game);
void	switch_weapon(t_game *game, int idx);

/* NPC */
void	init_npc(t_game *game);
void	update_npc(t_game *game);
void	render_npc(t_game *game);
void	npc_interact(t_game *game);
void	render_npc_dialog(t_game *game);

/* Jumpscare */
void	init_jumpscare(t_game *game);
void	update_jumpscare(t_game *game);
void	render_jumpscare(t_game *game);
void	trigger_death_jumpscare(t_game *game);

/* Cheat */
void	init_cheat(t_game *game);
void	open_cheat(t_game *game);
void	close_cheat(t_game *game, int submitted);
void	cheat_keypress(t_game *game, mlx_key_data_t kd);
void	cheat_click(t_game *game);
void	render_cheat(t_game *game);
int		cheat_button_at(int mx, int my);

/* cleanup */
int		cleanup_exit(t_game *game, char *msg);

/* utils */
int		ft_strlen(const char *s);
char	*ft_strdup(const char *s);
char	*ft_strndup(const char *s, int n);
char	*ft_strjoin(char *s1, char *s2);
char	*ft_strchr(const char *s, int c);
int		ft_strncmp(const char *s1, const char *s2, int n);
int		ft_atoi(const char *str);
int		ft_isdigit(int c);
int		ft_isspace(int c);
char	**ft_split(const char *s, char c);
void	ft_free_split(char **arr);
char	*ft_strtrim(const char *s1, const char *set);

/* gnl */
char	*get_next_line(int fd);

#endif
