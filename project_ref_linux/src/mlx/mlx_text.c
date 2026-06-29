#include "cub3d.h"

/*
** MLX42's mlx_put_string returns a new mlx_image_t per call that persists
** on the window until deleted. To emulate MiniLibX-style stateless text,
** clear all string images at the start of each frame's overlay pass.
*/
void	clear_strings(t_game *game)
{
	int	i;

	i = 0;
	while (i < game->str_count)
	{
		if (game->str_imgs[i])
			mlx_delete_image(game->mlx.mlx, game->str_imgs[i]);
		game->str_imgs[i] = NULL;
		i++;
	}
	game->str_count = 0;
}

/*
** Wrapper around mlx_put_string that tracks the returned image so
** clear_strings can release it on the next frame. Color is dropped —
** MLX42's font API does not support per-call coloring without a sheet.
*/
void	put_string(t_game *game, int x, int y, const char *str)
{
	mlx_image_t	*img;

	if (!str || !*str)
		return ;
	if (game->str_count >= MAX_STR_IMGS)
		return ;
	img = mlx_put_string(game->mlx.mlx, str, x, y);
	if (!img)
		return ;
	if (img->instances && img->count > 0)
		mlx_set_instance_depth(&img->instances[0], 100);
	game->str_imgs[game->str_count++] = img;
}
