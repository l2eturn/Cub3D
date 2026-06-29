#include "cub3d.h"

static void	handle_play_press(t_game *game, int key)
{
	if (key == KEY_ESC)
		cleanup_exit(game, NULL);
	if (game->player.dead || game->won)
	{
		if (key == KEY_R)
			restart_game(game);
		return ;
	}
	if (key == KEY_W)
		game->player.move_fb = 1;
	if (key == KEY_S)
		game->player.move_fb = -1;
	if (key == KEY_D)
		game->player.move_lr = 1;
	if (key == KEY_A)
		game->player.move_lr = -1;
	if (key == KEY_RIGHT)
		game->player.rotate = 1;
	if (key == KEY_LEFT)
		game->player.rotate = -1;
	if (key == KEY_LCTRL || key == KEY_LSHIFT)
		game->player.sprint = 1;
	if (key == KEY_E)
	{
		use_door(game);
		npc_interact(game);
	}
	if (key == KEY_1)
		switch_weapon(game, WEP_MACHETE);
	if (key == KEY_2)
		switch_weapon(game, WEP_PISTOL);
	if (key == KEY_3)
		switch_weapon(game, WEP_BOLTER);
	if (key == KEY_T)
		open_cheat(game);
	if (key == KEY_O)
		open_settings(game);
	if (key == KEY_SPACE)
	{
		game->dialog_timer = 0;
		game->dialog_index++;
	}
}

static void	handle_play_release(t_game *game, int key)
{
	if (key == KEY_W || key == KEY_S)
		game->player.move_fb = 0;
	if (key == KEY_A || key == KEY_D)
		game->player.move_lr = 0;
	if (key == KEY_LEFT || key == KEY_RIGHT)
		game->player.rotate = 0;
	if (key == KEY_LCTRL || key == KEY_LSHIFT)
		game->player.sprint = 0;
}

void	key_hook(mlx_key_data_t kd, void *param)
{
	t_game	*game;

	game = (t_game *)param;
	if (game->state == STATE_CHEAT)
	{
		cheat_keypress(game, kd);
		return ;
	}
	if (game->state == STATE_SETTINGS)
	{
		settings_keypress(game, kd);
		return ;
	}
	if (kd.action == MLX_PRESS)
	{
		if (game->state == STATE_MENU)
		{
			handle_menu_key(game, kd.key);
			return ;
		}
		handle_play_press(game, kd.key);
	}
	else if (kd.action == MLX_RELEASE)
	{
		if (game->state == STATE_PLAYING)
			handle_play_release(game, kd.key);
	}
}

void	mouse_hook(mouse_key_t btn, action_t act,
	modifier_key_t mods, void *param)
{
	t_game		*game;
	t_weapon	*w;

	(void)mods;
	game = (t_game *)param;
	if (game->state == STATE_CHEAT)
	{
		if (act == MLX_PRESS && btn == MLX_MOUSE_BUTTON_LEFT)
			cheat_click(game);
		return ;
	}
	if (game->state == STATE_SETTINGS)
	{
		if (act == MLX_PRESS && btn == MLX_MOUSE_BUTTON_LEFT)
			settings_click(game);
		return ;
	}
	if (game->state != STATE_PLAYING)
		return ;
	if (act == MLX_PRESS && btn == MLX_MOUSE_BUTTON_LEFT
		&& !game->player.dead)
	{
		w = &game->weapons[game->active_weapon];
		if (w->locked)
			return ;
		shoot(game);
	}
}

void	close_hook(void *param)
{
	cleanup_exit((t_game *)param, NULL);
}
