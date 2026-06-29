#include "cub3d.h"

/*
** Write to writable image (mlx_image_t).
** Input color is ARGB (0xAARRGGBB) for backward compat with existing code.
** MLX42 expects RGBA32 (0xRRGGBBAA) — convert. Force alpha=0xFF on writes.
*/
void	put_pixel(t_img *img, int x, int y, int color)
{
	uint32_t	r;
	uint32_t	g;
	uint32_t	b;

	if (!img || !img->img)
		return ;
	if (x < 0 || x >= img->width || y < 0 || y >= img->height)
		return ;
	r = (color >> 16) & 0xFF;
	g = (color >> 8) & 0xFF;
	b = color & 0xFF;
	mlx_put_pixel(img->img, x, y, (r << 24) | (g << 16) | (b << 8) | 0xFF);
}

/*
** Read pixel from either mlx_texture_t (loaded PNG) or mlx_image_t (screen).
** Returns ARGB format where A=0xFF means opaque, A=0 means transparent —
** matches the convention existing code expects for skip-transparent checks.
*/
int	get_pixel(t_img *img, int x, int y)
{
	uint8_t		*pixels;
	int			w;
	int			h;
	uint8_t		*p;

	if (!img)
		return (0);
	if (img->tex)
	{
		pixels = img->tex->pixels;
		w = (int)img->tex->width;
		h = (int)img->tex->height;
	}
	else if (img->img)
	{
		pixels = img->img->pixels;
		w = (int)img->img->width;
		h = (int)img->img->height;
	}
	else
		return (0);
	if (x < 0 || x >= w || y < 0 || y >= h)
		return (0);
	p = &pixels[(y * w + x) * 4];
	return (((int)p[3] << 24) | ((int)p[0] << 16)
		| ((int)p[1] << 8) | p[2]);
}
