#include "cub3d.h"

static int	load_jscare_frame(t_img *t, int idx)
{
	char	path[48];
	int		n;

	n = 0;
	while ("./resource/textures/jumpscare_"[n])
	{
		path[n] = "./resource/textures/jumpscare_"[n];
		n++;
	}
	path[n++] = '0' + idx;
	path[n++] = '.';
	path[n++] = 'p';
	path[n++] = 'n';
	path[n++] = 'g';
	path[n] = 0;
	return (load_png_tex(t, path));
}

void	init_jumpscare(t_game *game)
{
	int	i;

	game->jscare.active = 0;
	game->jscare.frame = 0;
	game->jscare.cooldown = 0;
	game->jscare_frame_count = 0;
	i = 0;
	while (i < JSCARE_ANIM_N)
	{
		if (load_jscare_frame(&game->jscare_frames[i], i) == 0)
			game->jscare_frame_count++;
		else
			break ;
		i++;
	}
}

/*
** Triggered exclusively by combat.c on death transition.
** Once finished, stays inactive forever this run — the death screen
** takes over from here.
*/
void	update_jumpscare(t_game *game)
{
	if (!game->jscare.active)
		return ;
	game->jscare.frame++;
	if (game->jscare.frame >= JSCARE_FRAMES)
	{
		game->jscare.active = 0;
		game->jscare.frame = 0;
	}
}

void	trigger_death_jumpscare(t_game *game)
{
	game->jscare.active = 1;
	game->jscare.frame = 0;
	game->player.shake = 20;
	game->player.dmg_flash = 40;
}

static t_img	*pick_frame(t_game *game)
{
	int	idx;

	if (game->jscare_frame_count <= 0)
		return (&game->sp_tex[SP_JSCARE]);
	idx = (game->jscare.frame * game->jscare_frame_count) / JSCARE_FRAMES;
	if (idx >= game->jscare_frame_count)
		idx = game->jscare_frame_count - 1;
	if (idx < 0)
		idx = 0;
	if (game->jscare_frames[idx].tex)
		return (&game->jscare_frames[idx]);
	return (&game->sp_tex[SP_JSCARE]);
}

/*
** Full-screen jumpscare: starts at 40% screen height, scales up to
** ~140% over JSCARE_FRAMES. Animation frame from enderman gif cycles
** through extracted frames. Slight red tint as horror amplifier.
*/
void	render_jumpscare(t_game *game)
{
	t_img	*tex;
	int		size_w;
	int		size_h;
	int		ox;
	int		oy;
	int		x;
	int		y;
	int		tx;
	int		ty;
	int		color;
	double	scale;

	if (!game->jscare.active)
		return ;
	tex = pick_frame(game);
	if (!tex || !tex->tex)
		return ;
	scale = (double)game->jscare.frame / (double)JSCARE_FRAMES;
	if (scale > 1.0)
		scale = 1.0;
	size_h = (int)(HEIGHT * (0.4 + scale * 1.0));
	size_w = (int)((double)size_h * tex->width / (double)tex->height);
	ox = WIDTH / 2 - size_w / 2;
	oy = HEIGHT / 2 - size_h / 2;
	y = 0;
	while (y < size_h)
	{
		x = 0;
		while (x < size_w)
		{
			tx = x * tex->width / size_w;
			ty = y * tex->height / size_h;
			color = get_pixel(tex, tx, ty);
			if ((color & 0xFF000000) == 0xFF000000)
				put_pixel(&game->mlx.screen, ox + x, oy + y, color);
			x++;
		}
		y++;
	}
}
