#include "cub3d.h"

static const double	g_regen_mult[DIFF_COUNT] = {1.5, 1.0, 0.3, 0.0};

static int	is_hit(t_game *game, t_sprite *sp, double *out_dist)
{
	t_player	*p;
	t_weapon	*w;
	double		dx;
	double		dy;
	double		fwd;
	double		lat;

	p = &game->player;
	w = &game->weapons[game->active_weapon];
	dx = sp->x - p->pos_x;
	dy = sp->y - p->pos_y;
	fwd = dx * p->dir_x + dy * p->dir_y;
	lat = dx * (-p->dir_y) + dy * p->dir_x;
	if (fwd <= 0 || fwd > w->range)
		return (0);
	if (fabs(lat) > w->radius)
		return (0);
	*out_dist = fwd;
	return (1);
}

static int	find_target(t_game *game)
{
	int		i;
	int		best;
	double	best_dist;
	double	d;

	best = -1;
	best_dist = SHOOT_RANGE + 1;
	i = 0;
	while (i < game->sprite_count)
	{
		if (game->sprites[i].alive
			&& is_enemy_type(game->sprites[i].type)
			&& is_hit(game, &game->sprites[i], &d)
			&& d < best_dist)
		{
			best = i;
			best_dist = d;
		}
		i++;
	}
	return (best);
}

void	shoot(t_game *game)
{
	t_weapon	*w;
	int			best;

	w = &game->weapons[game->active_weapon];
	if (w->uses_ammo)
	{
		if (game->player.ammo <= 0 && !game->cheat.god_mode)
			return ;
		if (!game->cheat.god_mode)
			game->player.ammo--;
		game->player.shoot_flash = MUZZLE_FRAMES;
		snd_shoot();
	}
	else
		snd_hit();
	game->player.recoil = 8;
	game->weapon_anim_timer = WEAPON_ANIM_TICKS;
	best = find_target(game);
	if (best < 0)
		return ;
	game->sprites[best].hp--;
	if (w->uses_ammo)
		snd_hit();
	if (game->sprites[best].hp <= 0)
	{
		game->sprites[best].alive = 0;
		game->player.kills++;
	}
}

void	update_aim(t_game *game)
{
	game->player.aim_on_target = (find_target(game) >= 0);
}

static double	nearby_enemy_dmg(t_game *game)
{
	int		i;
	double	dx;
	double	dy;
	double	max_dmg;
	double	d;

	max_dmg = 0;
	i = 0;
	while (i < game->sprite_count)
	{
		if (game->sprites[i].alive
			&& is_enemy_type(game->sprites[i].type))
		{
			dx = game->sprites[i].x - game->player.pos_x;
			dy = game->sprites[i].y - game->player.pos_y;
			if (dx * dx + dy * dy < DMG_RADIUS * DMG_RADIUS
				&& has_los(game, game->sprites[i].x, game->sprites[i].y))
			{
				d = enemy_dmg(game->sprites[i].type);
				if (d > max_dmg)
					max_dmg = d;
			}
		}
		i++;
	}
	return (max_dmg);
}

void	update_combat(t_game *game)
{
	double	dmg;

	if (game->player.dead || game->won)
		return ;
	if (game->cheat.god_mode)
	{
		game->player.health = HP_MAX;
		game->player.stamina = STAMINA_MAX;
		if (game->player.ammo < AMMO_MAX)
			game->player.ammo = AMMO_MAX;
	}
	dmg = game->cheat.god_mode ? 0 : nearby_enemy_dmg(game);
	if (dmg > 0)
	{
		game->player.health -= dmg;
		if (game->player.dmg_flash == 0)
			snd_hurt();
		if (game->player.dmg_flash < 8)
			game->player.dmg_flash = 8;
		if (game->player.shake < 4)
			game->player.shake = 4;
	}
	else if (game->player.health < HP_MAX && !game->cheat.god_mode)
		game->player.health += REGEN_PER_TICK * g_regen_mult[game->difficulty];
	if (game->player.health > HP_MAX)
		game->player.health = HP_MAX;
	if (game->player.health <= 0)
	{
		if (!game->player.dead)
		{
			snd_death();
			trigger_death_jumpscare(game);
			if (game->player.kills > game->high_score)
			{
				game->high_score = game->player.kills;
				save_high_score(game->high_score);
			}
		}
		game->player.health = 0;
		game->player.dead = 1;
	}
	if (game->player.dmg_flash > 0)
		game->player.dmg_flash--;
	if (game->player.shoot_flash > 0)
		game->player.shoot_flash--;
	if (game->player.recoil > 0)
		game->player.recoil--;
	if (game->player.shake > 0)
		game->player.shake--;
}
