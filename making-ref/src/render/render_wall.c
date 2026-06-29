#include "cub3d.h"

/*
** Choose texture based on which face was hit.
** side == 0 (EW wall): ray dir in X determines EA or WE face.
** side == 1 (NS wall): ray dir in Y determines NO or SO face.
*/
static t_img	*select_texture(t_game *game, t_ray *ray)
{
	int	t;

	if (ray->hit_type == HIT_DOOR)
	{
		if (ray->side == 0)
		{
			if (ray->ray_dir_x > 0)
				return (&game->door_tex[EA]);
			return (&game->door_tex[WE]);
		}
		if (ray->ray_dir_y > 0)
			return (&game->door_tex[SO]);
		return (&game->door_tex[NO]);
	}
	t = ray->hit_type;
	if (t < 0 || t >= MAX_WALL_TYPES)
		t = 0;
	if (ray->side == 0)
	{
		if (ray->ray_dir_x > 0)
			return (&game->tex[t][EA]);
		return (&game->tex[t][WE]);
	}
	if (ray->ray_dir_y > 0)
		return (&game->tex[t][SO]);
	return (&game->tex[t][NO]);
}

/*
** Compute wall_x: exact position on the wall face [0.0, 1.0].
** Mirror rule follows Lodev: flip tex_x for rays going negative on the
** axis that defines the wall face, otherwise text on textures comes out
** reversed on half the wall faces.
*/
static void	calc_tex_x(t_ray *ray, t_player *p, t_img *tex)
{
	if (ray->side == 0)
		ray->wall_x = p->pos_y + ray->perp_wall_dist * ray->ray_dir_y;
	else
		ray->wall_x = p->pos_x + ray->perp_wall_dist * ray->ray_dir_x;
	ray->wall_x -= floor(ray->wall_x);
	ray->tex_x = (int)(ray->wall_x * (double)tex->width);
	if (ray->side == 0 && ray->ray_dir_x < 0)
		ray->tex_x = tex->width - ray->tex_x - 1;
	if (ray->side == 1 && ray->ray_dir_y > 0)
		ray->tex_x = tex->width - ray->tex_x - 1;
}

/*
** DOOM colormap pattern in truecolor: fog factor depends only on
** distance + light, both of which are constant for the whole column.
** Compute once (fixed-point Q8), then the inner pixel loop reduces to
** integer multiplies on each RGB channel.
*/
static int	fog_factor_q8(double dist, double light)
{
	double	f;

	f = 1.0 - (dist - FOG_START) / (FOG_END - FOG_START);
	if (f > 1.0)
		f = 1.0;
	if (f < FOG_FLOOR)
		f = FOG_FLOOR;
	f *= light;
	return ((int)(f * 256.0));
}

static int	shade(int color, int fog_q8)
{
	int	r;
	int	g;
	int	b;

	r = (((color >> 16) & 0xFF) * fog_q8) >> 8;
	g = (((color >> 8) & 0xFF) * fog_q8) >> 8;
	b = ((color & 0xFF) * fog_q8) >> 8;
	return ((r << 16) | (g << 8) | b);
}

/*
** Door slide offset: how far the door has slid up, in screen pixels.
** progress in [0,1] = closed -> open. We shift the visible top down by
** that fraction of line_height so the slab appears to retract upward
** out of the cell. The texture step gets matched to the *new* visible
** height so the full door image stays mapped onto the visible region
** (otherwise a near-camera door would only show a centered crop, which
** is what made the texture look like only its upper portion was used).
*/
static int	door_slide(t_game *game, t_ray *ray)
{
	t_door	*d;

	if (ray->hit_type != HIT_DOOR)
		return (0);
	d = door_at(game, ray->map_x, ray->map_y);
	if (!d || d->timer <= 0)
		return (0);
	return ((int)((double)ray->line_height
			* (double)d->timer / (double)DOOR_ANIM_MAX));
}

/*
** Draw one vertical wall strip using texture and fog.
** tex_pos steps through texture rows proportionally to wall height.
** For doors: full texture maps onto the (post-slide) visible region.
*/
void	draw_column(t_game *game, t_ray *ray, int x)
{
	t_img	*tex;
	double	step;
	double	tex_pos;
	int		y;
	int		slide;
	int		top;
	int		fog_q8;
	int		visible_h;

	tex = select_texture(game, ray);
	calc_tex_x(ray, &game->player, tex);
	slide = door_slide(game, ray);
	top = ray->draw_start + slide;
	if (slide > 0)
		draw_flat(&game->mlx.screen, x, ray->draw_start, top - 1,
			game->ceil_color);
	if (top > ray->draw_end)
		return ;
	if (ray->hit_type == HIT_DOOR)
	{
		visible_h = ray->draw_end - top + 1;
		step = (double)tex->height / (double)visible_h;
		tex_pos = 0.0;
	}
	else
	{
		step = (double)tex->height / (double)ray->line_height;
		tex_pos = (top - HEIGHT / 2 + ray->line_height / 2) * step;
	}
	fog_q8 = fog_factor_q8(ray->perp_wall_dist, current_light(game));
	y = top;
	while (y <= ray->draw_end)
	{
		put_pixel(&game->mlx.screen, x, y, shade(
				get_pixel(tex, ray->tex_x, (int)tex_pos % tex->height), fog_q8));
		tex_pos += step;
		y++;
	}
}
