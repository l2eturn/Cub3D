#include "cub3d.h"

int	init_mlx(t_game *game)
{
	t_img	*s;

	game->mlx.mlx = mlx_init(WIDTH, HEIGHT, TITLE, false);
	if (!game->mlx.mlx)
		return (1);
	s = &game->mlx.screen;
	s->img = mlx_new_image(game->mlx.mlx, WIDTH, HEIGHT);
	if (!s->img)
		return (1);
	if (mlx_image_to_window(game->mlx.mlx, s->img, 0, 0) < 0)
		return (1);
	s->width = WIDTH;
	s->height = HEIGHT;
	s->tex = NULL;
	return (0);
}
