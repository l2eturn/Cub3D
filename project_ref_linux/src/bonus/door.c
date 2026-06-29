#include "cub3d.h"

void	init_doors(t_game *game)
{
	int	x;
	int	y;

	y = 0;
	while (y < game->map.height && game->door_count < MAX_DOORS)
	{
		x = 0;
		while (x < (int)ft_strlen(game->map.grid[y]))
		{
			if (game->map.grid[y][x] == 'D')
			{
				game->doors[game->door_count].x = x;
				game->doors[game->door_count].y = y;
				game->doors[game->door_count].timer = 0;
				game->doors[game->door_count].state = DOOR_CLOSED;
				game->door_count++;
			}
			x++;
		}
		y++;
	}
}

/*
** Default door textures (one PNG used for all 4 sides). Loaded from
** the horror pack; will silently leave NULLs if file missing so
** the renderer can fall back to wall type 0.
*/
int	load_door_tex(t_game *game)
{
	int	i;

	i = 0;
	while (i < 4)
	{
		game->door_path[i] = ft_strdup("./resource/textures/horror/wooden_door.png");
		if (!game->door_path[i])
			return (1);
		if (load_png_tex(&game->door_tex[i], game->door_path[i]))
		{
			printf("Warning: missing door texture: %s\n", game->door_path[i]);
			game->door_tex[i].tex = NULL;
		}
		i++;
	}
	return (0);
}

t_door	*door_at(t_game *game, int x, int y)
{
	int	i;

	i = 0;
	while (i < game->door_count)
	{
		if (game->doors[i].x == x && game->doors[i].y == y)
			return (&game->doors[i]);
		i++;
	}
	return (NULL);
}

static t_door	*find_adjacent_door(t_game *game, int px, int py)
{
	t_door	*d;

	d = door_at(game, px + 1, py);
	if (d)
		return (d);
	d = door_at(game, px - 1, py);
	if (d)
		return (d);
	d = door_at(game, px, py + 1);
	if (d)
		return (d);
	return (door_at(game, px, py - 1));
}

void	use_door(t_game *game)
{
	t_door	*door;

	door = find_adjacent_door(game, (int)game->player.pos_x,
			(int)game->player.pos_y);
	if (!door)
		return ;
	if (door->state == DOOR_CLOSED || door->state == DOOR_CLOSING)
		door->state = DOOR_OPENING;
	else if (door->state == DOOR_OPEN || door->state == DOOR_OPENING)
		door->state = DOOR_CLOSING;
	snd_door();
}

/*
** Tick animation. Map cell stays 'D' the whole time; passability is
** decided by door->state (checked in is_blocked / perform_dda), so the
** animation can be visually smooth instead of binary.
**
** Safety guard: if the player is standing on this door cell when it
** starts to close, revert to opening — prevents trapping the player
** in their own door.
*/
static void	tick_one(t_game *game, t_door *d)
{
	if (d->state == DOOR_OPENING)
	{
		d->timer++;
		if (d->timer >= DOOR_ANIM_MAX)
		{
			d->timer = DOOR_ANIM_MAX;
			d->state = DOOR_OPEN;
		}
	}
	else if (d->state == DOOR_CLOSING)
	{
		if ((int)game->player.pos_x == d->x
			&& (int)game->player.pos_y == d->y)
		{
			d->state = DOOR_OPENING;
			return ;
		}
		d->timer--;
		if (d->timer <= 0)
		{
			d->timer = 0;
			d->state = DOOR_CLOSED;
		}
	}
}

void	update_doors(t_game *game)
{
	int	i;

	i = 0;
	while (i < game->door_count)
	{
		tick_one(game, &game->doors[i]);
		i++;
	}
}
