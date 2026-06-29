#include "cub3d.h"

static void	fill_bg(t_img *img, int color)
{
	int	x;
	int	y;

	y = 0;
	while (y < HEIGHT)
	{
		x = 0;
		while (x < WIDTH)
		{
			put_pixel(img, x, y, color);
			x++;
		}
		y++;
	}
}

static void	draw_menu_bg(t_game *g)
{
	t_img	*t;
	int		x;
	int		y;
	int		color;

	t = &g->menu_bg;
	y = 0;
	while (y < HEIGHT)
	{
		x = 0;
		while (x < WIDTH)
		{
			color = get_pixel(t, x * t->width / WIDTH,
					y * t->height / HEIGHT);
			put_pixel(&g->mlx.screen, x, y, 0xFF000000 | (color & 0xFFFFFF));
			x++;
		}
		y++;
	}
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

static int	text_cx(const char *s, int center_x)
{
	return (center_x - (ft_strlen(s) * FONT_W) / 2);
}

static void	put_centered(t_game *g, int cx, int y, const char *s)
{
	put_string(g, text_cx(s, cx), y, s);
}

static void	draw_mist(t_game *game)
{
	int		x;
	int		y;
	int		v;
	int		color;

	y = 0;
	while (y < HEIGHT)
	{
		x = 0;
		while (x < WIDTH)
		{
			v = ((x * 31 + (y + game->anim_tick) * 17) ^ (y * 7)) & 0x1F;
			if (v > 26)
			{
				color = (v * 4) << 16 | (v * 2) << 8 | v;
				put_pixel(&game->mlx.screen, x, y, color);
			}
			x += 3;
		}
		y += 2;
	}
}

static void	itoa3(int n, char *buf)
{
	int	h;
	int	t;
	int	o;

	if (n < 0)
		n = 0;
	if (n > 999)
		n = 999;
	h = n / 100;
	t = (n / 10) % 10;
	o = n % 10;
	buf[0] = (h > 0) ? ('0' + h) : ' ';
	buf[1] = (h > 0 || t > 0) ? ('0' + t) : ' ';
	buf[2] = '0' + o;
	buf[3] = 0;
}

static void	draw_creature(t_game *g, t_img *t, int ox, int oy)
{
	int	sw;
	int	sh;
	int	x;
	int	y;
	int	color;

	if (!t || !t->tex)
		return ;
	sh = 120;
	sw = t->width * sh / t->height;
	ox -= sw / 2;
	y = 0;
	while (y < sh)
	{
		x = 0;
		while (x < sw)
		{
			color = get_pixel(t, x * t->width / sw, y * t->height / sh);
			if ((color & 0xFF000000) == 0xFF000000
				&& (color & 0xFFFFFF) != 0)
				put_pixel(&g->mlx.screen, ox + x, oy + y, color);
			x++;
		}
		y++;
	}
}

static void	draw_menu_creatures(t_game *g)
{
	int	cy;

	cy = HEIGHT / 2 - 195;
	draw_creature(g, &g->sp_tex[SP_TROOP], WIDTH / 2 - 340, cy);
	draw_creature(g, &g->sp_tex[SP_CACO], WIDTH / 2 + 340, cy);
}

static void	draw_title_panel(t_game *game)
{
	int	panel[4];

	panel[0] = WIDTH / 2 - 280;
	panel[1] = HEIGHT / 2 - 220;
	panel[2] = 560;
	panel[3] = 130;
	fill_rect(&game->mlx.screen, panel, 0x180404);
	draw_frame(&game->mlx.screen, panel, 0x882222);
}

static void	draw_title(t_game *game)
{
	char	hs_buf[8];
	char	full[32];
	int		i;
	int		cx;

	cx = WIDTH / 2;
	put_centered(game, cx, HEIGHT / 2 - 200, "=== A B Y S S ===");
	put_centered(game, cx, HEIGHT / 2 - 170,
		"a survival horror demo");
	itoa3(game->high_score, hs_buf);
	i = 0;
	while (i < 13)
	{
		full[i] = "BEST KILLS: "[i];
		i++;
	}
	full[12] = ' ';
	full[13] = hs_buf[0];
	full[14] = hs_buf[1];
	full[15] = hs_buf[2];
	full[16] = 0;
	put_centered(game, cx, HEIGHT / 2 - 135, full);
}

static void	draw_option_row(t_game *game, int y, int selected,
	const char *label)
{
	int	bar[4];
	int	cx;

	cx = WIDTH / 2;
	if (selected)
	{
		bar[0] = cx - 220;
		bar[1] = y - 4;
		bar[2] = 440;
		bar[3] = FONT_H + 8;
		fill_rect(&game->mlx.screen, bar, 0x401010);
		draw_frame(&game->mlx.screen, bar, 0xAA3333);
	}
	put_centered(game, cx, y, label);
}

static const char	*difficulty_name(int d)
{
	if (d == DIFF_EASY)
		return ("EASY");
	if (d == DIFF_NORMAL)
		return ("NORMAL");
	if (d == DIFF_HARD)
		return ("HARD");
	return ("ASIAN");
}

static void	build_sens_label(int sens, char *label)
{
	char	buf[8];
	int		i;

	itoa3(sens, buf);
	i = 0;
	while (i < 13)
	{
		label[i] = "SENSITIVITY: "[i];
		i++;
	}
	label[13] = buf[0];
	label[14] = buf[1];
	label[15] = buf[2];
	label[16] = 0;
}

static void	build_diff_label(int diff, char *label)
{
	const char	*name;
	int			i;

	name = difficulty_name(diff);
	i = 0;
	while (i < 12)
	{
		label[i] = "DIFFICULTY: "[i];
		i++;
	}
	i = 0;
	while (name[i] && i < 8)
	{
		label[12 + i] = name[i];
		i++;
	}
	label[12 + i] = 0;
}

static void	draw_options(t_game *game)
{
	char	label[32];

	if (game->menu_index == 0)
		draw_option_row(game, HEIGHT / 2 - 30, 1, ">> START GAME <<");
	else
		draw_option_row(game, HEIGHT / 2 - 30, 0, "   START GAME   ");
	build_sens_label(game->sens_level, label);
	draw_option_row(game, HEIGHT / 2 + 20, game->menu_index == 1, label);
	build_diff_label(game->difficulty, label);
	draw_option_row(game, HEIGHT / 2 + 60, game->menu_index == 2, label);
}

static void	draw_controls_panel(t_game *game)
{
	int	panel[4];
	int	cx;

	panel[0] = WIDTH / 2 - 380;
	panel[1] = HEIGHT - 220;
	panel[2] = 760;
	panel[3] = 170;
	fill_rect(&game->mlx.screen, panel, 0x0c0c14);
	draw_frame(&game->mlx.screen, panel, 0x3a3a55);
	cx = WIDTH / 2;
	put_centered(game, cx, HEIGHT - 200, "--- CONTROLS ---");
	put_centered(game, cx, HEIGHT - 170,
		"WASD: move    CTRL: sprint    MOUSE: look    LCLICK: shoot");
	put_centered(game, cx, HEIGHT - 140,
		"E: interact    R: restart    ESC: quit");
	put_centered(game, cx, HEIGHT - 80,
		"UP/DOWN: choose    LEFT/RIGHT: adjust    ENTER: start");
}

static void	draw_side_accents(t_game *game)
{
	int	bar[4];

	bar[0] = 0;
	bar[1] = 0;
	bar[2] = 6;
	bar[3] = HEIGHT;
	fill_rect(&game->mlx.screen, bar, 0x440000);
	bar[0] = WIDTH - 6;
	fill_rect(&game->mlx.screen, bar, 0x440000);
}

void	render_menu(t_game *game)
{
	if (game->menu_bg.tex)
		draw_menu_bg(game);
	else
	{
		fill_bg(&game->mlx.screen, 0x080608);
		draw_mist(game);
		draw_side_accents(game);
	}
	draw_menu_creatures(game);
	draw_title_panel(game);
	draw_title(game);
	draw_options(game);
	draw_controls_panel(game);
	game->anim_tick++;
}

void	handle_menu_key(t_game *game, int key)
{
	if (key == KEY_ESC)
		cleanup_exit(game, NULL);
	if (key == KEY_UP && game->menu_index > 0)
		game->menu_index--;
	if (key == KEY_DOWN && game->menu_index < 2)
		game->menu_index++;
	if (key == KEY_ENTER && game->menu_index == 0)
	{
		game->state = STATE_PLAYING;
		mlx_set_cursor_mode(game->mlx.mlx, MLX_MOUSE_DISABLED);
		mlx_set_mouse_pos(game->mlx.mlx, WIDTH / 2, HEIGHT / 2);
		dialog_start(game);
	}
	if (game->menu_index == 1)
	{
		if (key == KEY_LEFT && game->sens_level > SENS_MIN)
			game->sens_level -= SENS_STEP;
		if (key == KEY_RIGHT && game->sens_level < SENS_MAX)
			game->sens_level += SENS_STEP;
	}
	if (game->menu_index == 2)
	{
		if (key == KEY_LEFT && game->difficulty > 0)
			game->difficulty--;
		if (key == KEY_RIGHT && game->difficulty < DIFF_COUNT - 1)
			game->difficulty++;
	}
}
