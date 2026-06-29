#include "cub3d.h"

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

static void	draw_health_bar(t_game *game)
{
	int	bg[4];
	int	bar[4];
	int	fill;

	bg[0] = MINIMAP_PAD;
	bg[1] = HEIGHT - 22 - MINIMAP_PAD;
	bg[2] = 240;
	bg[3] = 22;
	fill_rect(&game->mlx.screen, bg, 0x110000);
	draw_frame(&game->mlx.screen, bg, 0x551111);
	fill = (int)(bg[2] * (game->player.health / HP_MAX));
	if (fill > 2)
	{
		bar[0] = bg[0] + 2;
		bar[1] = bg[1] + 2;
		bar[2] = fill - 4;
		bar[3] = bg[3] - 4;
		fill_rect(&game->mlx.screen, bar, 0xAA1111);
		bar[3] = (bg[3] - 4) / 3;
		fill_rect(&game->mlx.screen, bar, 0xDD3322);
	}
}

static void	draw_stamina_bar(t_game *game)
{
	int	bg[4];
	int	bar[4];
	int	fill;
	int	color;

	bg[0] = MINIMAP_PAD;
	bg[1] = HEIGHT - 22 - MINIMAP_PAD - 14;
	bg[2] = 240;
	bg[3] = 8;
	fill_rect(&game->mlx.screen, bg, 0x001020);
	draw_frame(&game->mlx.screen, bg, 0x224477);
	fill = (int)(bg[2] * (game->player.stamina / STAMINA_MAX));
	if (fill > 2)
	{
		bar[0] = bg[0] + 2;
		bar[1] = bg[1] + 2;
		bar[2] = fill - 4;
		bar[3] = bg[3] - 4;
		if (game->player.stamina < SPRINT_MIN)
			color = 0x884400;
		else
			color = 0x3388CC;
		fill_rect(&game->mlx.screen, bar, color);
	}
}

static void	draw_crosshair(t_game *game)
{
	int	cx;
	int	cy;
	int	i;
	int	color;
	int	glow;

	cx = WIDTH / 2;
	cy = HEIGHT / 2;
	color = game->player.aim_on_target ? 0xFF3322 : 0xDDDDDD;
	glow = game->player.aim_on_target ? 0x661111 : 0x444444;
	i = -8;
	while (i <= 8)
	{
		if (i < -2 || i > 2)
		{
			put_pixel(&game->mlx.screen, cx + i, cy - 1, glow);
			put_pixel(&game->mlx.screen, cx + i, cy + 1, glow);
			put_pixel(&game->mlx.screen, cx - 1, cy + i, glow);
			put_pixel(&game->mlx.screen, cx + 1, cy + i, glow);
		}
		i++;
	}
	i = -7;
	while (i <= 7)
	{
		if (i < -1 || i > 1)
		{
			put_pixel(&game->mlx.screen, cx + i, cy, color);
			put_pixel(&game->mlx.screen, cx, cy + i, color);
		}
		i++;
	}
	put_pixel(&game->mlx.screen, cx, cy, color);
}

static int	blend_red(int orig, double a)
{
	int	r;
	int	g;
	int	b;

	r = (int)(((orig >> 16) & 0xFF) * (1 - a) + 220 * a);
	g = (int)(((orig >> 8) & 0xFF) * (1 - a));
	b = (int)((orig & 0xFF) * (1 - a));
	return ((r << 16) | (g << 8) | b);
}

static void	draw_damage_flash(t_game *game)
{
	int		x;
	int		y;
	int		orig;
	double	a;

	if (game->player.dmg_flash <= 0)
		return ;
	a = game->player.dmg_flash / 24.0;
	if (a > 0.5)
		a = 0.5;
	y = 0;
	while (y < HEIGHT)
	{
		x = 0;
		while (x < WIDTH)
		{
			orig = get_pixel(&game->mlx.screen, x, y);
			put_pixel(&game->mlx.screen, x, y, blend_red(orig, a));
			x += 2;
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

static void	copy_str(char *dst, const char *src, int max)
{
	int	i;

	i = 0;
	while (src[i] && i < max - 1)
	{
		dst[i] = src[i];
		i++;
	}
	dst[i] = 0;
}

/*
** Info panel top-left above health/stamina bars.
** Shows active weapon (with [N] hotkey), ammo, kills.
*/
static void	draw_info_panel(t_game *game)
{
	int			panel[4];
	t_weapon	*w;
	char		buf[8];
	char		line[24];
	int			px;
	int			py;

	panel[0] = MINIMAP_PAD;
	panel[1] = HEIGHT - 22 - MINIMAP_PAD - 14 - 8 - 100;
	panel[2] = 240;
	panel[3] = 100;
	fill_rect(&game->mlx.screen, panel, 0x0a0a14);
	draw_frame(&game->mlx.screen, panel, 0x444477);
	w = &game->weapons[game->active_weapon];
	px = panel[0] + 12;
	py = panel[1] + 12;
	line[0] = '[';
	line[1] = '1' + game->active_weapon;
	line[2] = ']';
	line[3] = ' ';
	copy_str(line + 4, w->name ? w->name : "?", 18);
	put_string(game, px, py, line);
	copy_str(line, "AMMO: ", 24);
	itoa3(game->player.ammo, buf);
	line[6] = buf[0];
	line[7] = buf[1];
	line[8] = buf[2];
	line[9] = 0;
	put_string(game, px, py + 30, line);
	copy_str(line, "KILLS: ", 24);
	itoa3(game->player.kills, buf);
	line[7] = buf[0];
	line[8] = buf[1];
	line[9] = buf[2];
	line[10] = 0;
	put_string(game, px, py + 60, line);
}

static void	draw_overlay_screen(t_game *game, int color)
{
	int	rect[4];

	rect[0] = 0;
	rect[1] = 0;
	rect[2] = WIDTH;
	rect[3] = HEIGHT;
	fill_rect(&game->mlx.screen, rect, color);
}

static void	draw_centered_panel(t_game *game, int border)
{
	int	panel[4];

	panel[0] = WIDTH / 2 - 240;
	panel[1] = HEIGHT / 2 - 90;
	panel[2] = 480;
	panel[3] = 180;
	fill_rect(&game->mlx.screen, panel, 0x080404);
	draw_frame(&game->mlx.screen, panel, border);
}

static void	draw_death_screen(t_game *game)
{
	char	buf[8];
	char	line[32];
	int		cx;
	int		i;

	if (!game->player.dead || game->jscare.active)
		return ;
	draw_centered_panel(game, 0xAA2222);
	cx = WIDTH / 2;
	put_string(game, text_cx("YOU ARE DEAD", cx),
		HEIGHT / 2 - 60, "YOU ARE DEAD");
	itoa3(game->player.kills, buf);
	i = 0;
	while (i < 15)
	{
		line[i] = "ghosts killed: "[i];
		i++;
	}
	line[15] = buf[0];
	line[16] = buf[1];
	line[17] = buf[2];
	line[18] = 0;
	put_string(game, text_cx(line, cx), HEIGHT / 2 - 20, line);
	put_string(game, text_cx("press R to restart", cx),
		HEIGHT / 2 + 20, "press R to restart");
	put_string(game, text_cx("press ESC to quit", cx),
		HEIGHT / 2 + 50, "press ESC to quit");
}

static void	draw_win_screen(t_game *game)
{
	int	cx;

	if (!game->won)
		return ;
	draw_centered_panel(game, 0x22AA44);
	cx = WIDTH / 2;
	put_string(game, text_cx("ALL GHOSTS PURGED", cx),
		HEIGHT / 2 - 60, "ALL GHOSTS PURGED");
	put_string(game, text_cx("the abyss is silent again", cx),
		HEIGHT / 2 - 20, "the abyss is silent again");
	put_string(game, text_cx("press R to replay", cx),
		HEIGHT / 2 + 20, "press R to replay");
	put_string(game, text_cx("press ESC to quit", cx),
		HEIGHT / 2 + 50, "press ESC to quit");
}

static void	draw_heartbeat(t_game *game)
{
	double	t;
	double	a;
	int		alpha;

	if (game->player.health >= 30 || game->player.dead)
		return ;
	t = game->anim_tick * 0.10;
	a = (sin(t) + 1.0) * 0.5;
	alpha = (int)(a * 80);
	game->player.dmg_flash = alpha / 4;
}

void	render_hud(t_game *game)
{
	if (game->player.dead && !game->jscare.active)
		draw_overlay_screen(game, 0x200000);
	else if (game->won)
		draw_overlay_screen(game, 0x002010);
	else if (!game->player.dead)
	{
		draw_heartbeat(game);
		draw_damage_flash(game);
	}
	if (!game->jscare.active)
	{
		draw_health_bar(game);
		draw_stamina_bar(game);
		draw_crosshair(game);
		draw_info_panel(game);
	}
}

static void	draw_notif(t_game *game)
{
	int	panel[4];
	int	cx;
	int	y;

	if (game->notif_timer <= 0)
		return ;
	y = HEIGHT / 2 + 60 - (90 - game->notif_timer) / 3;
	cx = WIDTH / 2;
	panel[0] = cx - 110;
	panel[1] = y - 8;
	panel[2] = 220;
	panel[3] = FONT_H + 16;
	fill_rect(&game->mlx.screen, panel, 0x002211);
	draw_frame(&game->mlx.screen, panel, 0x44AA88);
	put_string(game, text_cx(game->notif_text, cx), y, game->notif_text);
	game->notif_timer--;
}

void	render_text_overlay(t_game *game)
{
	draw_notif(game);
	dialog_render(game);
	render_npc_dialog(game);
	draw_death_screen(game);
	draw_win_screen(game);
}
