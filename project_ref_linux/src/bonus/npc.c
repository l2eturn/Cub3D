#include "cub3d.h"

void	init_npc(t_game *game)
{
	game->npc.x = game->player.spawn_x + game->player.spawn_dir_x * 2.0;
	game->npc.y = game->player.spawn_y + game->player.spawn_dir_y * 2.0;
	game->npc.spawned = 1;
	game->npc.dialog_done = 0;
	game->npc.dialog_active = 0;
	game->npc.gun_unlocked = 0;
	game->npc.minimap_unlocked = 0;
}

void	update_npc(t_game *game)
{
	(void)game;
}

/*
** Project NPC sprite using same logic as render_sprites, but separate
** to keep z-buffer untouched and skip the sprite-typed pulse tinting.
*/
static void	draw_npc_strip(t_game *game, int xy[4], int tex_x)
{
	t_img	*tex;
	double	step;
	double	tpos;
	int		y;
	int		color;

	tex = &game->sp_tex[SP_NPC];
	if (!tex->tex || xy[3] <= xy[1])
		return ;
	step = (double)tex->height / (double)(xy[3] - xy[1]);
	tpos = 0;
	y = xy[1];
	while (y < xy[3] && y < HEIGHT)
	{
		if (y >= 0 && xy[0] >= 0 && xy[0] < WIDTH)
		{
			color = get_pixel(tex, tex_x, (int)tpos % tex->height);
			if ((color & 0xFF000000) == 0xFF000000 && (color & 0xFFFFFF) != 0)
				put_pixel(&game->mlx.screen, xy[0], y, color);
		}
		tpos += step;
		y++;
	}
}

void	render_npc(t_game *game)
{
	double	tx;
	double	ty;
	double	tr[2];
	double	inv_det;
	int		bounds[4];
	int		xy[4];
	int		stripe;
	int		tex_x;
	int		w;

	if (!game->npc.spawned || !game->sp_tex[SP_NPC].tex)
		return ;
	tx = game->npc.x - game->player.pos_x;
	ty = game->npc.y - game->player.pos_y;
	inv_det = 1.0 / (game->player.plane_x * game->player.dir_y
			- game->player.dir_x * game->player.plane_y);
	tr[0] = inv_det * (game->player.dir_y * tx - game->player.dir_x * ty);
	tr[1] = inv_det * (-game->player.plane_y * tx + game->player.plane_x * ty);
	if (tr[1] <= 0)
		return ;
	bounds[0] = (int)((WIDTH / 2) * (1 + tr[0] / tr[1]));
	w = (int)(HEIGHT / tr[1]);
	xy[1] = HEIGHT / 2 - w / 2;
	xy[3] = HEIGHT / 2 + w / 2;
	bounds[1] = bounds[0] - w / 2;
	bounds[2] = bounds[0] + w / 2;
	stripe = (bounds[1] < 0) ? 0 : bounds[1];
	while (stripe < bounds[2] && stripe < WIDTH)
	{
		tex_x = (int)((stripe - bounds[1])
				* game->sp_tex[SP_NPC].width / w);
		if (tr[1] < game->zbuf[stripe])
		{
			xy[0] = stripe;
			draw_npc_strip(game, xy, tex_x);
		}
		stripe++;
	}
}

void	npc_interact(t_game *game)
{
	double	dx;
	double	dy;
	double	d2;

	if (!game->npc.spawned || game->npc.dialog_done)
		return ;
	dx = game->npc.x - game->player.pos_x;
	dy = game->npc.y - game->player.pos_y;
	d2 = dx * dx + dy * dy;
	if (d2 > NPC_INTERACT_RADIUS * NPC_INTERACT_RADIUS)
		return ;
	if (!game->npc.dialog_active)
	{
		game->npc.dialog_active = 1;
		game->dialog_index = 0;
		return ;
	}
	game->dialog_index++;
	if (game->dialog_index >= 3)
	{
		game->npc.dialog_active = 0;
		game->npc.dialog_done = 1;
		game->npc.gun_unlocked = 1;
		game->npc.minimap_unlocked = 1;
		game->weapons[WEP_PISTOL].locked = 0;
		game->weapons[WEP_BOLTER].locked = 0;
		memcpy(game->notif_text, "GUNS UNLOCKED", 14);
		game->notif_timer = 120;
	}
}

static void	dlg_fill(t_img *img, int xy[4], int color)
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

static void	dlg_frame(t_img *img, int xy[4], int color)
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

void	render_npc_dialog(t_game *game)
{
	const char	*lines[3];
	int			panel[4];
	int			cx;
	int			y;
	const char	*line;

	if (!game->npc.dialog_active)
		return ;
	lines[0] = "...you found me. take this gun.";
	lines[1] = "and here, a map of the halls.";
	lines[2] = "now go. silence them all.";
	cx = WIDTH / 2;
	y = HEIGHT - 150;
	panel[0] = cx - 400;
	panel[1] = y - 10;
	panel[2] = 800;
	panel[3] = 80;
	dlg_fill(&game->mlx.screen, panel, 0x081208);
	dlg_frame(&game->mlx.screen, panel, 0x44AA66);
	put_string(game, cx - 7 * FONT_W, y, "[ THE WARDEN ]");
	if (game->dialog_index >= 0 && game->dialog_index < 3)
	{
		line = lines[game->dialog_index];
		put_string(game, cx - (ft_strlen(line) * FONT_W) / 2,
			y + 28, (char *)line);
	}
	put_string(game, panel[0] + panel[2] - 100,
		panel[1] + panel[3] - 22, "[next: E]");
}
