#include "cub3d.h"

static void	set_notif(t_game *game, char *text)
{
	int	i;

	i = 0;
	while (text[i] && i < 15)
	{
		game->notif_text[i] = text[i];
		i++;
	}
	game->notif_text[i] = 0;
	game->notif_timer = 90;
}

static int	any_ghost_alive_sp(t_game *game)
{
	int	i;

	i = 0;
	while (i < game->sprite_count)
	{
		if (game->sprites[i].alive && game->sprites[i].type == SP_GHOST)
			return (1);
		i++;
	}
	return (0);
}

static void	consume(t_game *game, t_sprite *sp)
{
	if (sp->type == SP_HEAL)
	{
		game->player.health += HEAL_AMOUNT;
		if (game->player.health > HP_MAX)
			game->player.health = HP_MAX;
		set_notif(game, "+30 HEALTH");
		sp->alive = 0;
		sp->respawn_timer = PICKUP_RESPAWN_FRAMES;
		snd_pickup();
	}
	else if (sp->type == SP_AMMO)
	{
		game->player.ammo += AMMO_AMOUNT;
		if (game->player.ammo > AMMO_MAX)
			game->player.ammo = AMMO_MAX;
		set_notif(game, "+8 AMMO");
		sp->alive = 0;
		sp->respawn_timer = PICKUP_RESPAWN_FRAMES;
		snd_pickup();
	}
	else if (sp->type == SP_EXIT)
	{
		if (any_ghost_alive_sp(game))
			set_notif(game, "kill them all");
		else
		{
			game->won = 1;
			snd_pickup();
		}
	}
}

static void	tick_respawn(t_sprite *sp)
{
	if (sp->alive || sp->respawn_timer <= 0)
		return ;
	sp->respawn_timer--;
	if (sp->respawn_timer == 0)
	{
		sp->x = sp->orig_x;
		sp->y = sp->orig_y;
		sp->alive = 1;
	}
}

/*
** Walk-over collection + respawn tick.
** Pickups respawn after PICKUP_RESPAWN_FRAMES; ghosts have no timer
** (set to 0 on death) so they stay dead.
*/
void	update_pickups(t_game *game)
{
	int		i;
	double	dx;
	double	dy;

	i = 0;
	while (i < game->sprite_count)
	{
		if (game->sprites[i].type != SP_GHOST)
			tick_respawn(&game->sprites[i]);
		if (!game->player.dead && game->sprites[i].alive
			&& game->sprites[i].type != SP_GHOST)
		{
			dx = game->sprites[i].x - game->player.pos_x;
			dy = game->sprites[i].y - game->player.pos_y;
			if (dx * dx + dy * dy < PICKUP_RADIUS * PICKUP_RADIUS)
				consume(game, &game->sprites[i]);
		}
		i++;
	}
}
