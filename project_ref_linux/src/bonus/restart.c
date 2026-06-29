#include "cub3d.h"

static void	reset_player(t_player *p)
{
	p->pos_x = p->spawn_x;
	p->pos_y = p->spawn_y;
	p->dir_x = p->spawn_dir_x;
	p->dir_y = p->spawn_dir_y;
	p->plane_x = p->spawn_plane_x;
	p->plane_y = p->spawn_plane_y;
	p->health = HP_MAX;
	p->stamina = STAMINA_MAX;
	p->ammo = AMMO_START;
	p->kills = 0;
	p->weapon = WEP_MACHETE;
	p->dead = 0;
	p->dmg_flash = 0;
	p->shoot_flash = 0;
	p->recoil = 0;
	p->shake = 0;
	p->move_fb = 0;
	p->move_lr = 0;
	p->rotate = 0;
	p->sprint = 0;
	p->aim_on_target = 0;
}

static void	reset_sprites(t_game *game)
{
	int	i;

	i = 0;
	while (i < game->sprite_count)
	{
		game->sprites[i].x = game->sprites[i].orig_x;
		game->sprites[i].y = game->sprites[i].orig_y;
		game->sprites[i].alive = 1;
		game->sprites[i].hp = SPRITE_HP;
		game->sprites[i].respawn_timer = 0;
		i++;
	}
}

static void	reset_doors(t_game *game)
{
	int	i;

	i = 0;
	while (i < game->door_count)
	{
		game->doors[i].state = DOOR_CLOSED;
		game->doors[i].timer = 0;
		game->map.grid[game->doors[i].y][game->doors[i].x] = 'D';
		i++;
	}
}

void	restart_game(t_game *game)
{
	reset_player(&game->player);
	reset_sprites(game);
	reset_doors(game);
	game->won = 0;
	game->active_weapon = WEP_MACHETE;
	game->weapon_anim_timer = 0;
	game->weapon_anim_frame = 0;
	game->jscare.active = 0;
	game->jscare.frame = 0;
	game->jscare.cooldown = 0;
	game->npc.dialog_done = 0;
	game->npc.gun_unlocked = 0;
	game->npc.minimap_unlocked = 0;
	game->weapons[WEP_PISTOL].locked = 1;
	game->weapons[WEP_BOLTER].locked = 1;
	dialog_start(game);
}
