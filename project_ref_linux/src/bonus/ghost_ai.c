#include "cub3d.h"

/*
** Try to move ghost by (dx, dy). Wall-aware: split axes so ghost slides
** along walls instead of getting stuck on corners.
*/
static void	step_ghost(t_game *game, t_sprite *g, double dx, double dy)
{
	double	nx;
	double	ny;

	nx = g->x + dx;
	ny = g->y + dy;
	if (!is_blocked(game, (int)nx, (int)g->y))
		g->x = nx;
	if (!is_blocked(game, (int)g->x, (int)ny))
		g->y = ny;
}

/*
** Chase only when ghost can actually see the player.
** Two gates: detect range (proximity) AND line-of-sight (no wall between).
** Outside either, the ghost stands still — has to walk into range first.
*/
static void	chase_player(t_game *game, t_sprite *g)
{
	double	dx;
	double	dy;
	double	len;
	double	wobble;
	double	spd;

	dx = game->player.pos_x - g->x;
	dy = game->player.pos_y - g->y;
	len = sqrt(dx * dx + dy * dy);
	if (len < GHOST_MIN_DIST)
		return ;
	if (len > GHOST_DETECT_RANGE)
		return ;
	if (!has_los(game, g->x, g->y))
		return ;
	wobble = sin(game->anim_tick * 0.05 + g->x * 1.7) * 0.3;
	dx = dx / len + (-dy / len) * wobble;
	dy = dy / len + (dx / len) * wobble;
	spd = enemy_speed(g->type);
	step_ghost(game, g, dx * spd, dy * spd);
}

/*
** Random ambient growl when any ghost is within 4 units of the player.
** Fires roughly every 5s (300 frames) to avoid spam.
*/
static void	growl_tick(t_game *game)
{
	int		i;
	double	dx;
	double	dy;

	if (game->anim_tick % 300 != 0)
		return ;
	i = 0;
	while (i < game->sprite_count)
	{
		if (game->sprites[i].alive && game->sprites[i].type == SP_GHOST)
		{
			dx = game->sprites[i].x - game->player.pos_x;
			dy = game->sprites[i].y - game->player.pos_y;
			if (dx * dx + dy * dy < 16.0)
			{
				snd_growl();
				return ;
			}
		}
		i++;
	}
}

void	update_ghosts(t_game *game)
{
	int	i;

	if (game->player.dead)
		return ;
	i = 0;
	while (i < game->sprite_count)
	{
		if (game->sprites[i].alive
			&& is_enemy_type(game->sprites[i].type))
			chase_player(game, &game->sprites[i]);
		i++;
	}
	growl_tick(game);
}
