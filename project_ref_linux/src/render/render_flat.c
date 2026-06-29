#include "cub3d.h"

/*
** Vertical depth fog for ceiling/floor strips.
** Distance from horizon (HEIGHT/2) approximates depth — pixels nearer
** the center (vanishing point) are darker, pixels near top/bottom edges
** are closer to the player and stay brighter.
*/
static int	depth_dim(int color, int y)
{
	double	d;
	double	f;
	int		r;
	int		g;
	int		b;

	d = fabs((double)(y - HEIGHT / 2)) / (HEIGHT / 2.0);
	f = d * 0.95 + FOG_FLOOR;
	if (f > 1.0)
		f = 1.0;
	if (f < FOG_FLOOR)
		f = FOG_FLOOR;
	r = (int)(((color >> 16) & 0xFF) * f);
	g = (int)(((color >> 8) & 0xFF) * f);
	b = (int)((color & 0xFF) * f);
	return ((r << 16) | (g << 8) | b);
}

void	draw_flat(t_img *screen, int x, int y_start, int y_end, int color)
{
	int	y;

	y = y_start;
	while (y <= y_end)
	{
		put_pixel(screen, x, y, depth_dim(color, y));
		y++;
	}
}
