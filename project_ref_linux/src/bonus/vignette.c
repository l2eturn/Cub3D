#include "cub3d.h"

/*
** Radial darkening: pixels far from center get multiplied by < 1.
** Skip step of 2px for performance — barely visible.
*/
void	apply_vignette(t_game *game)
{
	int		x;
	int		y;
	int		dx;
	int		dy;
	int		color;
	int		r;
	int		g;
	int		b;
	double	d;
	double	f;
	double	max_d;

	max_d = sqrt((WIDTH / 2) * (WIDTH / 2) + (HEIGHT / 2) * (HEIGHT / 2));
	y = 0;
	while (y < HEIGHT)
	{
		x = 0;
		while (x < WIDTH)
		{
			dx = x - WIDTH / 2;
			dy = y - HEIGHT / 2;
			d = sqrt(dx * dx + dy * dy) / max_d;
			f = 1.0 - d * 0.85;
			if (f < 0.15)
				f = 0.15;
			color = get_pixel(&game->mlx.screen, x, y);
			r = (int)(((color >> 16) & 0xFF) * f);
			g = (int)(((color >> 8) & 0xFF) * f);
			b = (int)((color & 0xFF) * f);
			put_pixel(&game->mlx.screen, x, y, (r << 16) | (g << 8) | b);
			x += 2;
		}
		y += 2;
	}
}
