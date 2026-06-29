#include "cub3d.h"

static void	set_dir(t_player *p, char spawn)
{
	if (spawn == 'N')
	{
		p->dir_x = 0;    p->dir_y = -1;
		p->plane_x = 0.60; p->plane_y = 0;
	}
	else if (spawn == 'S')
	{
		p->dir_x = 0;    p->dir_y = 1;
		p->plane_x = -0.60; p->plane_y = 0;
	}
	else if (spawn == 'E')
	{
		p->dir_x = 1;    p->dir_y = 0;
		p->plane_x = 0;   p->plane_y = 0.60;
	}
	else
	{
		p->dir_x = -1;   p->dir_y = 0;
		p->plane_x = 0;   p->plane_y = -0.60;
	}
}

void	init_player(t_game *game)
{
	int		x;
	int		y;
	char	c;

	y = 0;
	while (y < game->map.height)
	{
		x = 0;
		while (x < (int)ft_strlen(game->map.grid[y]))
		{
			c = game->map.grid[y][x];
			if (c == 'N' || c == 'S' || c == 'E' || c == 'W')
			{
				game->player.pos_x = x + 0.5;
				game->player.pos_y = y + 0.5;
				set_dir(&game->player, c);
				game->player.spawn_x = game->player.pos_x;
				game->player.spawn_y = game->player.pos_y;
				game->player.spawn_dir_x = game->player.dir_x;
				game->player.spawn_dir_y = game->player.dir_y;
				game->player.spawn_plane_x = game->player.plane_x;
				game->player.spawn_plane_y = game->player.plane_y;
				game->player.health = HP_MAX;
				game->player.stamina = STAMINA_MAX;
				game->player.ammo = AMMO_START;
				game->map.grid[y][x] = '0';
				return ;
			}
			x++;
		}
		y++;
	}
}

int	is_blocked(t_game *game, int x, int y)
{
	char	c;
	t_door	*d;

	if (y < 0 || y >= game->map.height)
		return (1);
	if (x < 0 || x >= (int)ft_strlen(game->map.grid[y]))
		return (1);
	c = game->map.grid[y][x];
	if (c >= '1' && c <= '0' + MAX_WALL_TYPES)
		return (1);
	if (c == 'D')
	{
		d = door_at(game, x, y);
		if (d && d->state == DOOR_OPEN)
			return (0);
		return (1);
	}
	return (0);
}

/*
** Line-of-sight from (sx,sy) to player. Steps along the ray in LOS_STEP
** units and returns 0 if any wall cell is hit before reaching the player.
** Used by ghost AI to gate chase/damage on visibility instead of grid
** distance alone (prevents wall-hack chasing).
*/
/*
** Enemy registry — single source of truth for what counts as hostile.
** Centralized so ghost_ai, combat, sprite rendering, and damage tick
** all classify consistently. Stats differ per type:
**
**   GHOST  — baseline; slow, low damage
**   TROOP  — Imp; faster, medium damage (Doom-imp inspired)
**   CACO   — Cacodemon; slow but high damage / tankier feeling
*/
int	is_enemy_type(int type)
{
	return (type == SP_GHOST || type == SP_TROOP || type == SP_CACO);
}

double	enemy_speed(int type)
{
	if (type == SP_TROOP)
		return (GHOST_SPEED * 1.5);
	if (type == SP_CACO)
		return (GHOST_SPEED * 0.7);
	return (GHOST_SPEED);
}

double	enemy_dmg(int type)
{
	if (type == SP_TROOP)
		return (DMG_PER_TICK * 1.3);
	if (type == SP_CACO)
		return (DMG_PER_TICK * 1.8);
	return (DMG_PER_TICK);
}

int	has_los(t_game *game, double sx, double sy)
{
	double	dx;
	double	dy;
	double	dist;
	double	x;
	double	y;
	double	traveled;

	dx = game->player.pos_x - sx;
	dy = game->player.pos_y - sy;
	dist = sqrt(dx * dx + dy * dy);
	if (dist < 0.01)
		return (1);
	dx /= dist;
	dy /= dist;
	x = sx;
	y = sy;
	traveled = 0;
	while (traveled < dist)
	{
		x += dx * LOS_STEP;
		y += dy * LOS_STEP;
		traveled += LOS_STEP;
		if (is_blocked(game, (int)x, (int)y))
			return (0);
	}
	return (1);
}

static void	move_player(t_game *game)
{
	t_player	*p;
	double		nx;
	double		ny;
	double		spd;

	p = &game->player;
	spd = (p->sprint && p->stamina > SPRINT_MIN) ? MOVE_FAST : MOVE_SLOW;
	nx = p->pos_x + p->dir_x * spd * p->move_fb;
	ny = p->pos_y + p->dir_y * spd * p->move_fb;
	if (!is_blocked(game, (int)nx, (int)p->pos_y))
		p->pos_x = nx;
	if (!is_blocked(game, (int)p->pos_x, (int)ny))
		p->pos_y = ny;
	nx = p->pos_x + (-p->dir_y) * spd * p->move_lr;
	ny = p->pos_y + p->dir_x * spd * p->move_lr;
	if (!is_blocked(game, (int)nx, (int)p->pos_y))
		p->pos_x = nx;
	if (!is_blocked(game, (int)p->pos_x, (int)ny))
		p->pos_y = ny;
}

/*
** Rotate using 2D rotation matrix. plane MUST rotate with dir.
*/
static void	rotate_player(t_player *p)
{
	double	angle;
	double	old_dir_x;
	double	old_plane_x;

	angle = ROT_SPEED * p->rotate;
	old_dir_x = p->dir_x;
	p->dir_x = p->dir_x * cos(angle) - p->dir_y * sin(angle);
	p->dir_y = old_dir_x * sin(angle) + p->dir_y * cos(angle);
	old_plane_x = p->plane_x;
	p->plane_x = p->plane_x * cos(angle) - p->plane_y * sin(angle);
	p->plane_y = old_plane_x * sin(angle) + p->plane_y * cos(angle);
}

static void	update_stamina(t_player *p)
{
	int	moving;

	moving = p->move_fb || p->move_lr;
	if (p->sprint && moving && p->stamina > 0)
		p->stamina -= STAMINA_DRAIN;
	else if (p->stamina < STAMINA_MAX)
		p->stamina += STAMINA_REGEN;
	if (p->stamina < 0)
		p->stamina = 0;
	if (p->stamina > STAMINA_MAX)
		p->stamina = STAMINA_MAX;
}

static void	footstep_tick(t_game *game)
{
	int	period;

	if (!(game->player.move_fb || game->player.move_lr))
		return ;
	period = game->player.sprint ? 18 : 28;
	if (game->anim_tick % period == 0)
		snd_footstep();
}

void	update_player(t_game *game)
{
	if (game->player.move_fb || game->player.move_lr)
		move_player(game);
	if (game->player.rotate)
		rotate_player(&game->player);
	update_stamina(&game->player);
	footstep_tick(game);
}
