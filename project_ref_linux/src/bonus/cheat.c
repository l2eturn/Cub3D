#include "cub3d.h"

#define CHEAT_BOX_W   500
#define CHEAT_BOX_H   240
#define BTN_W         140
#define BTN_H         44

static void	box_rect(int xy[4])
{
	xy[0] = WIDTH / 2 - CHEAT_BOX_W / 2;
	xy[1] = HEIGHT / 2 - CHEAT_BOX_H / 2;
	xy[2] = CHEAT_BOX_W;
	xy[3] = CHEAT_BOX_H;
}

static void	enter_btn(int xy[4])
{
	xy[0] = WIDTH / 2 - BTN_W - 14;
	xy[1] = HEIGHT / 2 + CHEAT_BOX_H / 2 - BTN_H - 18;
	xy[2] = BTN_W;
	xy[3] = BTN_H;
}

static void	cancel_btn(int xy[4])
{
	xy[0] = WIDTH / 2 + 14;
	xy[1] = HEIGHT / 2 + CHEAT_BOX_H / 2 - BTN_H - 18;
	xy[2] = BTN_W;
	xy[3] = BTN_H;
}

void	init_cheat(t_game *game)
{
	game->cheat.god_mode = 0;
	game->cheat.input_len = 0;
	game->cheat.input[0] = 0;
	game->cheat.msg_timer = 0;
	game->cheat.msg_ok = 0;
}

void	open_cheat(t_game *game)
{
	if (game->state != STATE_PLAYING)
		return ;
	game->state = STATE_CHEAT;
	game->cheat.input_len = 0;
	game->cheat.input[0] = 0;
	mlx_set_cursor_mode(game->mlx.mlx, MLX_MOUSE_NORMAL);
	mlx_set_mouse_pos(game->mlx.mlx, WIDTH / 2, HEIGHT / 2);
}

static int	check_code(t_game *game)
{
	int		i;
	char	*expected;

	expected = CHEAT_CODE;
	i = 0;
	while (expected[i] && i < game->cheat.input_len)
	{
		if (game->cheat.input[i] != expected[i])
			return (0);
		i++;
	}
	if (expected[i] || i != game->cheat.input_len)
		return (0);
	return (1);
}

void	close_cheat(t_game *game, int submitted)
{
	if (submitted && check_code(game))
	{
		game->cheat.god_mode = 1;
		game->cheat.msg_ok = 1;
		game->cheat.msg_timer = 90;
	}
	else if (submitted)
	{
		game->cheat.msg_ok = 0;
		game->cheat.msg_timer = 90;
	}
	game->cheat.input_len = 0;
	game->cheat.input[0] = 0;
	game->state = STATE_PLAYING;
	mlx_set_cursor_mode(game->mlx.mlx, MLX_MOUSE_DISABLED);
	mlx_set_mouse_pos(game->mlx.mlx, WIDTH / 2, HEIGHT / 2);
}

static int	keysym_to_char(int k)
{
	if (k >= MLX_KEY_A && k <= MLX_KEY_Z)
		return ('a' + (k - MLX_KEY_A));
	if (k >= MLX_KEY_0 && k <= MLX_KEY_9)
		return ('0' + (k - MLX_KEY_0));
	return (0);
}

void	cheat_keypress(t_game *game, mlx_key_data_t kd)
{
	int	c;

	if (kd.action != MLX_PRESS)
		return ;
	if (kd.key == KEY_ESC)
	{
		close_cheat(game, 0);
		return ;
	}
	if (kd.key == KEY_ENTER)
	{
		close_cheat(game, 1);
		return ;
	}
	if (kd.key == KEY_BACKSPACE)
	{
		if (game->cheat.input_len > 0)
		{
			game->cheat.input_len--;
			game->cheat.input[game->cheat.input_len] = 0;
		}
		return ;
	}
	c = keysym_to_char(kd.key);
	if (c && game->cheat.input_len < CHEAT_MAX_LEN)
	{
		game->cheat.input[game->cheat.input_len++] = c;
		game->cheat.input[game->cheat.input_len] = 0;
	}
}

int	cheat_button_at(int mx, int my)
{
	int	btn[4];

	enter_btn(btn);
	if (mx >= btn[0] && mx < btn[0] + btn[2]
		&& my >= btn[1] && my < btn[1] + btn[3])
		return (1);
	cancel_btn(btn);
	if (mx >= btn[0] && mx < btn[0] + btn[2]
		&& my >= btn[1] && my < btn[1] + btn[3])
		return (2);
	return (0);
}

void	cheat_click(t_game *game)
{
	int32_t	mx;
	int32_t	my;
	int		hit;

	mlx_get_mouse_pos(game->mlx.mlx, &mx, &my);
	hit = cheat_button_at(mx, my);
	if (hit == 1)
		close_cheat(game, 1);
	else if (hit == 2)
		close_cheat(game, 0);
}

static void	fill_rect(t_img *img, int xy[4], int color)
{
	int	dx;
	int	dy;

	dy = 0;
	while (dy < xy[3])
	{
		dx = 0;
		while (dx < xy[2])
		{
			put_pixel(img, xy[0] + dx, xy[1] + dy, color);
			dx++;
		}
		dy++;
	}
}

static void	draw_frame(t_img *img, int xy[4], int color)
{
	int	i;

	i = 0;
	while (i < xy[2])
	{
		put_pixel(img, xy[0] + i, xy[1], color);
		put_pixel(img, xy[0] + i, xy[1] + xy[3] - 1, color);
		i++;
	}
	i = 0;
	while (i < xy[3])
	{
		put_pixel(img, xy[0], xy[1] + i, color);
		put_pixel(img, xy[0] + xy[2] - 1, xy[1] + i, color);
		i++;
	}
}

static void	draw_button(t_game *game, int xy[4], const char *label,
	int hovered)
{
	int	bg;
	int	border;
	int	tx;

	bg = hovered ? 0x442211 : 0x1a0c08;
	border = hovered ? 0xFFCC44 : 0x886622;
	fill_rect(&game->mlx.screen, xy, bg);
	draw_frame(&game->mlx.screen, xy, border);
	tx = xy[0] + (xy[2] - (int)ft_strlen(label) * FONT_W) / 2;
	put_string(game, tx, xy[1] + (xy[3] - FONT_H) / 2, label);
}

void	render_cheat(t_game *game)
{
	int		box[4];
	int		btn[4];
	int32_t	mx;
	int32_t	my;
	int		hover;
	int		cx;

	box_rect(box);
	fill_rect(&game->mlx.screen, box, 0x080404);
	draw_frame(&game->mlx.screen, box, 0xCC3322);
	cx = WIDTH / 2;
	put_string(game, cx - 7 * FONT_W, box[1] + 24, "ENTER CHEAT CODE");
	put_string(game, cx - 11 * FONT_W, box[1] + 56,
		"(letters and digits, ESC to cancel)");
	fill_rect(&game->mlx.screen, (int [4]){cx - 180, box[1] + 90, 360, 36},
		0x000000);
	draw_frame(&game->mlx.screen,
		(int [4]){cx - 180, box[1] + 90, 360, 36}, 0x886622);
	put_string(game, cx - 175, box[1] + 98, game->cheat.input);
	mlx_get_mouse_pos(game->mlx.mlx, &mx, &my);
	hover = cheat_button_at(mx, my);
	enter_btn(btn);
	draw_button(game, btn, "ENTER", hover == 1);
	cancel_btn(btn);
	draw_button(game, btn, "CANCEL", hover == 2);
}
