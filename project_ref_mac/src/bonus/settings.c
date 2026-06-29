#include "cub3d.h"

#define SET_BOX_W  640
#define SET_BOX_H  420
#define VOL_STEP   10

static void	box_rect(int xy[4])
{
	xy[0] = WIDTH / 2 - SET_BOX_W / 2;
	xy[1] = HEIGHT / 2 - SET_BOX_H / 2;
	xy[2] = SET_BOX_W;
	xy[3] = SET_BOX_H;
}

static int	row_y(int idx)
{
	return (HEIGHT / 2 - SET_BOX_H / 2 + 110 + idx * 100);
}

static void	music_minus(int xy[4])
{
	xy[0] = WIDTH / 2 - 240;
	xy[1] = row_y(0) - 4;
	xy[2] = 56;
	xy[3] = 40;
}

static void	music_plus(int xy[4])
{
	xy[0] = WIDTH / 2 + 184;
	xy[1] = row_y(0) - 4;
	xy[2] = 56;
	xy[3] = 40;
}

static void	sens_minus(int xy[4])
{
	xy[0] = WIDTH / 2 - 240;
	xy[1] = row_y(1) - 4;
	xy[2] = 56;
	xy[3] = 40;
}

static void	sens_plus(int xy[4])
{
	xy[0] = WIDTH / 2 + 184;
	xy[1] = row_y(1) - 4;
	xy[2] = 56;
	xy[3] = 40;
}

static void	close_btn(int xy[4])
{
	xy[0] = WIDTH / 2 - 200;
	xy[1] = HEIGHT / 2 + SET_BOX_H / 2 - 64;
	xy[2] = 160;
	xy[3] = 44;
}

static void	menu_btn(int xy[4])
{
	xy[0] = WIDTH / 2 + 40;
	xy[1] = HEIGHT / 2 + SET_BOX_H / 2 - 64;
	xy[2] = 160;
	xy[3] = 44;
}

void	open_settings(t_game *game)
{
	if (game->state != STATE_PLAYING)
		return ;
	game->state = STATE_SETTINGS;
	mlx_set_cursor_mode(game->mlx.mlx, MLX_MOUSE_NORMAL);
	mlx_set_mouse_pos(game->mlx.mlx, WIDTH / 2, HEIGHT / 2);
}

void	close_settings(t_game *game)
{
	game->state = STATE_PLAYING;
	mlx_set_cursor_mode(game->mlx.mlx, MLX_MOUSE_DISABLED);
	mlx_set_mouse_pos(game->mlx.mlx, WIDTH / 2, HEIGHT / 2);
}

static void	back_to_menu(t_game *game)
{
	restart_game(game);
	game->state = STATE_MENU;
	game->menu_index = 0;
	mlx_set_cursor_mode(game->mlx.mlx, MLX_MOUSE_NORMAL);
}

static int	hit_btn(int btn[4], int mx, int my)
{
	return (mx >= btn[0] && mx < btn[0] + btn[2]
		&& my >= btn[1] && my < btn[1] + btn[3]);
}

static int	hover_at(int mx, int my)
{
	int	btn[4];

	music_minus(btn);
	if (hit_btn(btn, mx, my))
		return (1);
	music_plus(btn);
	if (hit_btn(btn, mx, my))
		return (2);
	sens_minus(btn);
	if (hit_btn(btn, mx, my))
		return (3);
	sens_plus(btn);
	if (hit_btn(btn, mx, my))
		return (4);
	close_btn(btn);
	if (hit_btn(btn, mx, my))
		return (5);
	menu_btn(btn);
	if (hit_btn(btn, mx, my))
		return (6);
	return (0);
}

static void	bump_sens(t_game *g, int delta)
{
	g->sens_level += delta;
	if (g->sens_level < SENS_MIN)
		g->sens_level = SENS_MIN;
	if (g->sens_level > SENS_MAX)
		g->sens_level = SENS_MAX;
}

void	settings_click(t_game *game)
{
	int32_t	mx;
	int32_t	my;
	int		hit;

	mlx_get_mouse_pos(game->mlx.mlx, &mx, &my);
	hit = hover_at(mx, my);
	if (hit == 1)
		music_set_volume(music_get_volume() - VOL_STEP);
	else if (hit == 2)
		music_set_volume(music_get_volume() + VOL_STEP);
	else if (hit == 3)
		bump_sens(game, -SENS_STEP);
	else if (hit == 4)
		bump_sens(game, SENS_STEP);
	else if (hit == 5)
		close_settings(game);
	else if (hit == 6)
		back_to_menu(game);
}

static void	settings_arrow(t_game *game, int dir)
{
	if (game->settings_index == 0)
		music_set_volume(music_get_volume() + dir * VOL_STEP);
	else
		bump_sens(game, dir * SENS_STEP);
}

void	settings_keypress(t_game *game, mlx_key_data_t kd)
{
	if (kd.action != MLX_PRESS)
		return ;
	if (kd.key == KEY_ESC || kd.key == KEY_O || kd.key == KEY_ENTER)
		close_settings(game);
	else if (kd.key == KEY_UP && game->settings_index > 0)
		game->settings_index--;
	else if (kd.key == KEY_DOWN && game->settings_index < 1)
		game->settings_index++;
	else if (kd.key == KEY_LEFT)
		settings_arrow(game, -1);
	else if (kd.key == KEY_RIGHT)
		settings_arrow(game, 1);
	else if (kd.key == KEY_BACKSPACE)
		back_to_menu(game);
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

static void	draw_button(t_game *g, int xy[4], const char *label, int hovered)
{
	int	bg;
	int	border;
	int	tx;

	bg = hovered ? 0x442211 : 0x1a0c08;
	border = hovered ? 0xFFCC44 : 0x886622;
	fill_rect(&g->mlx.screen, xy, bg);
	draw_frame(&g->mlx.screen, xy, border);
	tx = xy[0] + (xy[2] - (int)ft_strlen(label) * FONT_W) / 2;
	put_string(g, tx, xy[1] + (xy[3] - FONT_H) / 2, label);
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

static void	draw_slider_row(t_game *g, int y, int value, int max_value)
{
	int		bar[4];
	int		fill[4];
	char	buf[8];

	bar[0] = WIDTH / 2 - 160;
	bar[1] = y + 8;
	bar[2] = 320;
	bar[3] = 22;
	fill_rect(&g->mlx.screen, bar, 0x110000);
	draw_frame(&g->mlx.screen, bar, 0x886622);
	fill[0] = bar[0] + 2;
	fill[1] = bar[1] + 2;
	fill[2] = (bar[2] - 4) * value / max_value;
	fill[3] = bar[3] - 4;
	if (fill[2] > 0)
		fill_rect(&g->mlx.screen, fill, 0xAA3322);
	itoa3(value, buf);
	put_string(g, bar[0] + bar[2] + 12, bar[1] + 2, buf);
}

static void	draw_row_label(t_game *g, int y, const char *label, int selected)
{
	int	bar[4];

	if (selected)
	{
		bar[0] = WIDTH / 2 - 280;
		bar[1] = y - 14;
		bar[2] = 560;
		bar[3] = 70;
		fill_rect(&g->mlx.screen, bar, 0x180a06);
		draw_frame(&g->mlx.screen, bar, 0xCC8844);
	}
	put_string(g, WIDTH / 2 - (int)ft_strlen(label) * FONT_W / 2, y - 6, label);
}

static void	draw_rows(t_game *game, int hover)
{
	int	btn[4];
	int	y;

	y = row_y(0);
	draw_row_label(game, y, "MUSIC VOLUME", game->settings_index == 0);
	music_minus(btn);
	draw_button(game, btn, "<<", hover == 1);
	music_plus(btn);
	draw_button(game, btn, ">>", hover == 2);
	draw_slider_row(game, y, music_get_volume(), 100);
	y = row_y(1);
	draw_row_label(game, y, "MOUSE SENSITIVITY", game->settings_index == 1);
	sens_minus(btn);
	draw_button(game, btn, "<<", hover == 3);
	sens_plus(btn);
	draw_button(game, btn, ">>", hover == 4);
	draw_slider_row(game, y, game->sens_level, SENS_MAX);
}

void	render_settings(t_game *game)
{
	int		box[4];
	int		btn[4];
	int32_t	mx;
	int32_t	my;
	int		hover;

	box_rect(box);
	fill_rect(&game->mlx.screen, box, 0x080404);
	draw_frame(&game->mlx.screen, box, 0xCC9922);
	put_string(game, WIDTH / 2 - 8 * FONT_W, box[1] + 30, "=== SETTINGS ===");
	mlx_get_mouse_pos(game->mlx.mlx, &mx, &my);
	hover = hover_at(mx, my);
	draw_rows(game, hover);
	close_btn(btn);
	draw_button(game, btn, "RESUME", hover == 5);
	menu_btn(btn);
	draw_button(game, btn, "MAIN MENU", hover == 6);
	put_string(game, WIDTH / 2 - 22 * FONT_W,
		box[1] + box[3] - 14,
		"UP/DOWN select   LEFT/RIGHT adjust   ESC/O resume");
}
