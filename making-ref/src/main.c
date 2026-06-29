#include "cub3d.h"

static int	check_extension(char *path)
{
	int	len;

	len = ft_strlen(path);
	if (len < 5)
		return (1);
	if (ft_strncmp(path + len - 4, ".cub", 4) != 0)
		return (1);
	return (0);
}

static int	setup_game(t_game *game)
{
	if (init_mlx(game))
		return (cleanup_exit(game, "MLX init failed"));
	if (load_textures(game))
		return (cleanup_exit(game, "Failed to load textures"));
	if (load_door_tex(game))
		return (cleanup_exit(game, "Failed to load door textures"));
	init_player(game);
	init_doors(game);
	init_sprites(game);
	if (load_sp_tex(game))
		return (cleanup_exit(game, "Failed to load sprite texture"));
	if (load_gun_tex(game))
		return (cleanup_exit(game, "Failed to load gun texture"));
	init_npc(game);
	init_jumpscare(game);
	if (load_png_tex(&game->menu_bg, "./resource/textures/menu_bg.png"))
		game->menu_bg.tex = NULL;
	init_cheat(game);
	music_start();
	game->last_mouse_x = WIDTH / 2;
	game->state = STATE_MENU;
	game->menu_index = 0;
	game->sens_level = SENS_DEFAULT;
	game->difficulty = DIFF_NORMAL;
	game->settings_index = 0;
	game->high_score = load_high_score();
	return (0);
}

int	main(int argc, char **argv)
{
	t_game	game;

	if (argc != 2)
		return (cleanup_exit(NULL, "Usage: ./cub3D <map.cub>"));
	if (check_extension(argv[1]))
		return (cleanup_exit(NULL, "Map must have .cub extension"));
	memset(&game, 0, sizeof(t_game));
	if (parse_cub(&game, argv[1]))
		return (cleanup_exit(&game, NULL));

	if (setup_game(&game))
		return (1);
	mlx_key_hook(game.mlx.mlx, key_hook, &game);
	mlx_mouse_hook(game.mlx.mlx, mouse_hook, &game);
	mlx_close_hook(game.mlx.mlx, close_hook, &game);
	mlx_loop_hook(game.mlx.mlx, render_frame, &game);
	mlx_loop(game.mlx.mlx);
	cleanup_exit(&game, NULL);
	return (0);
}
