#include "cub3d.h"

static const char	*g_sp_paths[SP_TEX_N] = {
	"./resource/textures/ghost.png",
	"./resource/textures/healthkit.png",
	"./resource/textures/ammo.png",
	"./resource/textures/exit.png",
	"./resource/textures/npc.png",
	"./resource/textures/jumpscare.png",
	"./resource/textures/troop.png",
	"./resource/textures/caco.png"
};

static int	classify(char c)
{
	if (c == 'G')
		return (SP_GHOST);
	if (c == 'H')
		return (SP_HEAL);
	if (c == 'A')
		return (SP_AMMO);
	if (c == 'X')
		return (SP_EXIT);
	if (c == 'I')
		return (SP_TROOP);
	if (c == 'V')
		return (SP_CACO);
	return (-1);
}

void	init_sprites(t_game *game)
{
	int		x;
	int		y;
	int		type;

	y = 0;
	while (y < game->map.height && game->sprite_count < MAX_SPRITES)
	{
		x = 0;
		while (x < (int)ft_strlen(game->map.grid[y]))
		{
			type = classify(game->map.grid[y][x]);
			if (type >= 0)
			{
				game->sprites[game->sprite_count].x = x + 0.5;
				game->sprites[game->sprite_count].y = y + 0.5;
				game->sprites[game->sprite_count].orig_x = x + 0.5;
				game->sprites[game->sprite_count].orig_y = y + 0.5;
				game->sprites[game->sprite_count].dist = 0;
				game->sprites[game->sprite_count].hp = SPRITE_HP;
				game->sprites[game->sprite_count].alive = 1;
				game->sprites[game->sprite_count].type = type;
				game->sprite_count++;
				game->map.grid[y][x] = '0';
			}
			x++;
		}
		y++;
	}
}

static int	load_one(t_game *game, int idx)
{
	game->sp_path[idx] = ft_strdup(g_sp_paths[idx]);
	if (!game->sp_path[idx])
		return (1);
	if (load_png_tex(&game->sp_tex[idx], game->sp_path[idx]))
	{
		printf("Warning: sprite texture missing: %s\n", g_sp_paths[idx]);
		game->sp_tex[idx].tex = NULL;
		game->sp_tex[idx].width = 0;
		game->sp_tex[idx].height = 0;
	}
	return (0);
}

int	load_sp_tex(t_game *game)
{
	int	i;

	i = 0;
	while (i < SP_TEX_N)
	{
		if (load_one(game, i))
			return (1);
		i++;
	}
	return (0);
}

static void	compute_dists(t_game *game)
{
	int		i;
	double	dx;
	double	dy;

	i = 0;
	while (i < game->sprite_count)
	{
		if (game->sprites[i].alive)
		{
			dx = game->sprites[i].x - game->player.pos_x;
			dy = game->sprites[i].y - game->player.pos_y;
			game->sprites[i].dist = dx * dx + dy * dy;
		}
		else
			game->sprites[i].dist = -1;
		i++;
	}
}

static void	sort_sprites(t_game *game)
{
	t_sprite	tmp;
	int			i;
	int			j;

	compute_dists(game);
	i = 0;
	while (i < game->sprite_count - 1)
	{
		j = i + 1;
		while (j < game->sprite_count)
		{
			if (game->sprites[j].dist > game->sprites[i].dist)
			{
				tmp = game->sprites[i];
				game->sprites[i] = game->sprites[j];
				game->sprites[j] = tmp;
			}
			j++;
		}
		i++;
	}
}

static double	sprite_fog(double sq_dist)
{
	double	d;
	double	f;

	d = sqrt(sq_dist);
	f = 1.0 - (d - FOG_START) / (FOG_END - FOG_START);
	if (f > 1.0)
		f = 1.0;
	if (f < 0.10)
		f = 0.10;
	return (f);
}

static int	pick_brightness(t_sprite *sp, t_game *game)
{
	if (sp->type == SP_GHOST)
		return (180 + (int)(40.0 * sin(game->anim_tick * 0.15)));
	if (sp->type == SP_EXIT)
		return (255 + (int)(60.0 * sin(game->anim_tick * 0.10)));
	return (230 + (int)(20.0 * sin(game->anim_tick * 0.08 + sp->x)));
}

static int	tint(int color, t_sprite *sp, t_game *game)
{
	int	bright;
	int	rgb[3];

	bright = pick_brightness(sp, game);
	bright = (int)(bright * sprite_fog(sp->dist));
	if (bright > 320)
		bright = 320;
	rgb[0] = (((color >> 16) & 0xFF) * bright) / 255;
	rgb[1] = (((color >> 8) & 0xFF) * bright) / 255;
	rgb[2] = ((color & 0xFF) * bright) / 255;
	if (rgb[0] > 255)
		rgb[0] = 255;
	if (rgb[1] > 255)
		rgb[1] = 255;
	if (rgb[2] > 255)
		rgb[2] = 255;
	return ((rgb[0] << 16) | (rgb[1] << 8) | rgb[2]);
}

/*
** Wolf3D-inspired vertical strip scaler. Fixed-point Q16 step avoids
** double-accumulator drift over tall sprites, and pre-clipping the
** y range to [0, HEIGHT-1] before the loop removes the per-pixel
** bounds check (WL_SCALE.C did the same by patching scaler jump
** tables at runtime — we just use a clipped int range).
*/
static void	draw_sprite_strip(t_game *game, t_sprite *sp,
	int xy[4], int tex_x)
{
	t_img	*tex;
	int		step_fp;
	int		tpos_fp;
	int		y;
	int		y_end;
	int		color;

	tex = &game->sp_tex[sp->type];
	if (!tex->tex || xy[3] <= xy[1])
		return ;
	step_fp = ((int)tex->height << 16) / (xy[3] - xy[1]);
	y = xy[1];
	tpos_fp = 0;
	if (y < 0)
	{
		tpos_fp = -y * step_fp;
		y = 0;
	}
	y_end = xy[3];
	if (y_end > HEIGHT)
		y_end = HEIGHT;
	while (y < y_end)
	{
		color = get_pixel(tex, tex_x, (tpos_fp >> 16) % tex->height);
		if ((color & 0xFF000000) == 0xFF000000 && (color & 0xFFFFFF) != 0)
			put_pixel(&game->mlx.screen, xy[0], y, tint(color, sp, game));
		tpos_fp += step_fp;
		y++;
	}
}

static void	project_one(t_game *game, t_sprite *sp, double inv_det)
{
	double	tx;
	double	ty;
	double	tr[2];
	int		bounds[4];
	int		xy[4];
	int		stripe;
	int		tex_x;
	int		w;

	if (!sp->alive)
		return ;
	tx = sp->x - game->player.pos_x;
	ty = sp->y - game->player.pos_y;
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
		tex_x = (int)((stripe - bounds[1]) * game->sp_tex[sp->type].width / w);
		if (tr[1] < game->zbuf[stripe])
		{
			xy[0] = stripe;
			draw_sprite_strip(game, sp, xy, tex_x);
		}
		stripe++;
	}
}

void	render_sprites(t_game *game)
{
	int		i;
	double	inv_det;

	sort_sprites(game);
	inv_det = 1.0 / (game->player.plane_x * game->player.dir_y
			- game->player.dir_x * game->player.plane_y);
	i = 0;
	while (i < game->sprite_count)
	{
		project_one(game, &game->sprites[i], inv_det);
		i++;
	}
}
