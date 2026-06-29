#include "cub3d.h"

int	load_png_tex(t_img *t, const char *path)
{
	t->img = NULL;
	t->tex = mlx_load_png(path);
	if (!t->tex)
		return (1);
	t->width = (int)t->tex->width;
	t->height = (int)t->tex->height;
	return (0);
}

static int	load_one(t_game *game, int type, int side)
{
	if (load_png_tex(&game->tex[type][side], game->tex_path[type][side]))
		return (printf("Error\nCannot load texture: %s\n",
				game->tex_path[type][side]), 1);
	return (0);
}

/*
** Wall type 0 (digit '1') is mandatory — must have all 4 paths set.
** Higher types are optional; if a side is unset, fall back to type-0 path.
*/
static int	validate_and_fallback(t_game *game)
{
	int	type;
	int	side;

	side = 0;
	while (side < 4)
	{
		if (!game->tex_path[0][side])
			return (printf("Error\nMissing required texture for wall type 1\n"),
				1);
		side++;
	}
	type = 1;
	while (type < MAX_WALL_TYPES)
	{
		side = 0;
		while (side < 4)
		{
			if (!game->tex_path[type][side])
				game->tex_path[type][side] = ft_strdup(game->tex_path[0][side]);
			side++;
		}
		type++;
	}
	return (0);
}

int	load_textures(t_game *game)
{
	int	type;
	int	side;

	if (validate_and_fallback(game))
		return (1);
	type = 0;
	while (type < MAX_WALL_TYPES)
	{
		side = 0;
		while (side < 4)
		{
			if (load_one(game, type, side))
				return (1);
			side++;
		}
		type++;
	}
	return (0);
}
