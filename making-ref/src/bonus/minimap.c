#include "cub3d.h"

static int	door_color(t_game *game, int x, int y)
{
	t_door	*d;

	d = door_at(game, x, y);
	if (!d || d->state == DOOR_CLOSED)
		return (0xAA6600);
	if (d->state == DOOR_OPEN)
		return (0x44DD44);
	return (0xFFCC22);
}

static int	tile_color(t_game *game, int x, int y, char c)
{
	if (c == '1')
		return (0x888888);
	if (c == '2')
		return (0x6a4220);
	if (c == '3')
		return (0x6a0a10);
	if (c == '4')
		return (0xc4b89c);
	if (c == 'D')
		return (door_color(game, x, y));
	if (c == ' ')
		return (0x000000);
	return (0x222222);
}

static void	draw_tile(t_game *game, int mx, int my, int color)
{
	int	px;
	int	py;
	int	dx;
	int	dy;

	px = MINIMAP_PAD + mx * MINIMAP_T;
	py = MINIMAP_PAD + my * MINIMAP_T;
	dy = 0;
	while (dy < MINIMAP_T)
	{
		dx = 0;
		while (dx < MINIMAP_T)
		{
			put_pixel(&game->mlx.screen, px + dx, py + dy, color);
			dx++;
		}
		dy++;
	}
}

static void	draw_player_dot(t_game *game)
{
	int	px;
	int	py;
	int	d;

	px = MINIMAP_PAD + (int)(game->player.pos_x * MINIMAP_T);
	py = MINIMAP_PAD + (int)(game->player.pos_y * MINIMAP_T);
	d = -1;
	while (d <= 1)
	{
		put_pixel(&game->mlx.screen, px + d, py, 0xFF4444);
		put_pixel(&game->mlx.screen, px, py + d, 0xFF4444);
		d++;
	}
}

void	render_minimap(t_game *game)
{
	int		x;
	int		y;
	int		row_len;

	y = 0;
	while (y < game->map.height)
	{
		row_len = ft_strlen(game->map.grid[y]);
		x = 0;
		while (x < row_len)
		{
			draw_tile(game, x, y, tile_color(game, x, y, game->map.grid[y][x]));
			x++;
		}
		y++;
	}
	draw_player_dot(game);
}
