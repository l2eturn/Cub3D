#include "cub3d.h"

static void	free_map(t_map *map)
{
	int	i;

	if (!map->grid)
		return ;
	i = 0;
	while (i < map->height)
		free(map->grid[i++]);
	free(map->grid);
	map->grid = NULL;
}

static void	free_tex_one(t_img *t)
{
	if (t->tex)
		mlx_delete_texture(t->tex);
	t->tex = NULL;
	t->img = NULL;
}

static void	free_wall_textures(t_game *game)
{
	int	type;
	int	side;

	type = 0;
	while (type < MAX_WALL_TYPES)
	{
		side = 0;
		while (side < 4)
		{
			free_tex_one(&game->tex[type][side]);
			if (game->tex_path[type][side])
				free(game->tex_path[type][side]);
			game->tex_path[type][side] = NULL;
			side++;
		}
		type++;
	}
}

static void	free_textures(t_game *game)
{
	int	i;

	free_wall_textures(game);
	i = 0;
	while (i < SP_TEX_N)
	{
		free_tex_one(&game->sp_tex[i]);
		if (game->sp_path[i])
			free(game->sp_path[i]);
		game->sp_path[i] = NULL;
		i++;
	}
	free_tex_one(&game->gun_tex);
	if (game->gun_path)
		free(game->gun_path);
	free_tex_one(&game->knife_tex);
	if (game->knife_path)
		free(game->knife_path);
	free_tex_one(&game->menu_bg);
	i = 0;
	while (i < 4)
	{
		free_tex_one(&game->door_tex[i]);
		if (game->door_path[i])
			free(game->door_path[i]);
		game->door_path[i] = NULL;
		i++;
	}
	i = 0;
	while (i < JSCARE_ANIM_N)
	{
		free_tex_one(&game->jscare_frames[i]);
		i++;
	}
	i = 0;
	while (i < MAX_WEAPONS)
	{
		int	f;

		f = 0;
		while (f < WEAPON_FRAMES)
		{
			free_tex_one(&game->weapons[i].frames[f]);
			f++;
		}
		i++;
	}
}

static void	free_mlx(t_game *game)
{
	clear_strings(game);
	if (game->mlx.screen.img)
		mlx_delete_image(game->mlx.mlx, game->mlx.screen.img);
	game->mlx.screen.img = NULL;
	if (game->mlx.mlx)
		mlx_terminate(game->mlx.mlx);
	game->mlx.mlx = NULL;
}

/*
** Single exit point. msg == NULL → clean exit (0).
*/
int	cleanup_exit(t_game *game, char *msg)
{
	music_stop();
	if (msg)
		printf("Error\n%s\n", msg);
	if (!game)
		exit(msg ? 1 : 0);
	free_map(&game->map);
	if (game->mlx.mlx)
		free_textures(game);
	free_mlx(game);
	exit(msg ? 1 : 0);
}
