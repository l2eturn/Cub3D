#include "cub3d.h"

static int	load_weapon_frames(t_weapon *w, const char *prefix)
{
	char	path[64];
	int		i;
	int		n;

	w->frame_count = 0;
	i = 0;
	while (i < WEAPON_FRAMES)
	{
		n = 0;
		while (prefix[n] && n < 50)
		{
			path[n] = prefix[n];
			n++;
		}
		path[n++] = '0' + i;
		path[n++] = '.';
		path[n++] = 'p';
		path[n++] = 'n';
		path[n++] = 'g';
		path[n] = 0;
		if (load_png_tex(&w->frames[i], path))
			return (i == 0 ? 1 : 0);
		w->frame_count++;
		i++;
	}
	return (0);
}

int	load_gun_tex(t_game *game)
{
	game->weapons[WEP_MACHETE].name = "MACHETE";
	game->weapons[WEP_MACHETE].range = 1.8;
	game->weapons[WEP_MACHETE].radius = 0.7;
	game->weapons[WEP_MACHETE].uses_ammo = 0;
	game->weapons[WEP_MACHETE].locked = 0;
	if (load_weapon_frames(&game->weapons[WEP_MACHETE],
			"./resource/textures/weapons/machete_"))
		return (1);
	game->weapons[WEP_PISTOL].name = "PISTOL";
	game->weapons[WEP_PISTOL].range = 10.0;
	game->weapons[WEP_PISTOL].radius = 0.4;
	game->weapons[WEP_PISTOL].uses_ammo = 1;
	game->weapons[WEP_PISTOL].locked = 1;
	if (load_weapon_frames(&game->weapons[WEP_PISTOL],
			"./resource/textures/weapons/pistol_"))
		return (1);
	game->weapons[WEP_BOLTER].name = "BOLTER";
	game->weapons[WEP_BOLTER].range = 12.0;
	game->weapons[WEP_BOLTER].radius = 0.6;
	game->weapons[WEP_BOLTER].uses_ammo = 1;
	game->weapons[WEP_BOLTER].locked = 1;
	if (load_weapon_frames(&game->weapons[WEP_BOLTER],
			"./resource/textures/weapons/bolter_"))
		return (1);
	game->active_weapon = WEP_MACHETE;
	game->weapon_anim_timer = 0;
	game->weapon_anim_frame = 0;
	return (0);
}

void	switch_weapon(t_game *game, int idx)
{
	if (idx < 0 || idx >= MAX_WEAPONS)
		return ;
	if (game->weapons[idx].locked)
		return ;
	game->active_weapon = idx;
	game->weapon_anim_timer = 0;
	game->weapon_anim_frame = 0;
}

void	update_weapon_anim(t_game *game)
{
	int	f;

	if (game->weapon_anim_timer > 0)
	{
		game->weapon_anim_timer--;
		f = (WEAPON_ANIM_TICKS - game->weapon_anim_timer)
			* WEAPON_FRAMES / WEAPON_ANIM_TICKS;
		if (f >= WEAPON_FRAMES)
			f = 0;
		if (f < 0)
			f = 0;
		game->weapon_anim_frame = f;
	}
	else
		game->weapon_anim_frame = 0;
}

static void	bob_offset(t_game *game, int *bx, int *by)
{
	double	t;
	double	amp;
	int		rec;

	t = game->anim_tick * 0.10;
	amp = (game->player.move_fb || game->player.move_lr) ? 3.5 : 1.0;
	if (game->player.sprint)
		amp *= 1.7;
	*bx = (int)(sin(t * 0.9) * amp);
	*by = (int)(fabs(cos(t)) * amp);
	rec = game->player.recoil;
	if (rec > 0)
	{
		*by += rec * 4;
		*bx += rec * 2;
	}
}

/*
** Render weapon scaled 2x at bottom-center.
** Source frame ~290x160 → drawn 580x320 with nearest-neighbor scaling.
*/
static void	draw_weapon_scaled(t_game *game, t_img *t, int bx, int by)
{
	int	sw;
	int	sh;
	int	ox;
	int	oy;
	int	x;
	int	y;
	int	color;
	int	tx;
	int	ty;

	sw = t->width * 2;
	sh = t->height * 2;
	ox = WIDTH / 2 - sw / 2 + bx;
	oy = HEIGHT - sh + by;
	y = 0;
	while (y < sh)
	{
		x = 0;
		while (x < sw)
		{
			tx = x / 2;
			ty = y / 2;
			color = get_pixel(t, tx, ty);
			if ((color & 0xFF000000) == 0xFF000000)
				put_pixel(&game->mlx.screen, ox + x, oy + y, color);
			x++;
		}
		y++;
	}
}

void	render_gun(t_game *game)
{
	t_weapon	*w;
	t_img		*frame;
	int			bx;
	int			by;

	w = &game->weapons[game->active_weapon];
	if (w->frame_count == 0)
		return ;
	frame = &w->frames[game->weapon_anim_frame % w->frame_count];
	if (!frame->tex)
		frame = &w->frames[0];
	bob_offset(game, &bx, &by);
	draw_weapon_scaled(game, frame, bx, by);
}
