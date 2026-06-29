#include "cub3d.h"

/*
** Step 1: Compute ray direction from camera_x in [-1, 1].
** delta_dist = how far ray travels between grid lines.
** fabs(1/0) = infinity → safe: that axis never gets a DDA step.
*/
void	init_ray(t_ray *ray, t_player *p, int x)
{
	ray->camera_x = 2.0 * x / (double)WIDTH - 1.0;
	ray->ray_dir_x = p->dir_x + p->plane_x * ray->camera_x;
	ray->ray_dir_y = p->dir_y + p->plane_y * ray->camera_x;
	ray->map_x = (int)p->pos_x;
	ray->map_y = (int)p->pos_y;
	ray->delta_dist_x = fabs(1.0 / ray->ray_dir_x);
	ray->delta_dist_y = fabs(1.0 / ray->ray_dir_y);
	ray->hit = 0;
	ray->hit_type = 0;
}

/*
** Step 2: Set step direction and initial side_dist.
** side_dist = distance from player pos to first grid boundary.
*/
void	init_step(t_ray *ray, t_player *p)
{
	if (ray->ray_dir_x < 0)
	{
		ray->step_x = -1;
		ray->side_dist_x = (p->pos_x - ray->map_x) * ray->delta_dist_x;
	}
	else
	{
		ray->step_x = 1;
		ray->side_dist_x = (ray->map_x + 1.0 - p->pos_x) * ray->delta_dist_x;
	}
	if (ray->ray_dir_y < 0)
	{
		ray->step_y = -1;
		ray->side_dist_y = (p->pos_y - ray->map_y) * ray->delta_dist_y;
	}
	else
	{
		ray->step_y = 1;
		ray->side_dist_y = (ray->map_y + 1.0 - p->pos_y) * ray->delta_dist_y;
	}
}

/*
** Step 3: DDA loop — advance to nearest grid boundary until wall hit.
** side == 0 → hit an East/West wall face (crossed a vertical gridline).
** side == 1 → hit a North/South wall face (crossed a horizontal gridline).
**
** 'D' cells: skip when the door is fully open, so the ray sees the
** geometry behind the doorway. Otherwise treat as a wall hit and let
** the renderer animate the slide based on door->timer.
*/
void	perform_dda(t_ray *ray, t_game *game)
{
	t_map	*map;
	t_door	*d;
	int		row_len;
	char	cell;

	map = &game->map;
	while (!ray->hit)
	{
		if (ray->side_dist_x < ray->side_dist_y)
		{
			ray->side_dist_x += ray->delta_dist_x;
			ray->map_x += ray->step_x;
			ray->side = 0;
		}
		else
		{
			ray->side_dist_y += ray->delta_dist_y;
			ray->map_y += ray->step_y;
			ray->side = 1;
		}
		if (ray->map_y < 0 || ray->map_y >= map->height)
			break ;
		row_len = ft_strlen(map->grid[ray->map_y]);
		if (ray->map_x < 0 || ray->map_x >= row_len)
			break ;
		cell = map->grid[ray->map_y][ray->map_x];
		if (cell >= '1' && cell <= '0' + MAX_WALL_TYPES)
		{
			ray->hit_type = cell - '1';
			ray->hit = 1;
		}
		else if (cell == 'D')
		{
			d = door_at(game, ray->map_x, ray->map_y);
			if (d && d->state == DOOR_OPEN)
				continue ;
			ray->hit_type = HIT_DOOR;
			ray->hit = 1;
		}
	}
}

/*
** Step 4: Compute perpendicular distance (prevents fisheye),
** wall column height, draw bounds, and wall_x (texture coordinate).
**
** perp_wall_dist ≠ Euclidean distance — it's the distance projected
** onto the camera plane, which gives a correct non-distorted view.
*/
void	calc_wall_data(t_ray *ray)
{
	if (ray->side == 0)
		ray->perp_wall_dist = ray->side_dist_x - ray->delta_dist_x;
	else
		ray->perp_wall_dist = ray->side_dist_y - ray->delta_dist_y;
	ray->line_height = (int)(HEIGHT / ray->perp_wall_dist);
	ray->draw_start = HEIGHT / 2 - ray->line_height / 2;
	if (ray->draw_start < 0)
		ray->draw_start = 0;
	ray->draw_end = HEIGHT / 2 + ray->line_height / 2;
	if (ray->draw_end >= HEIGHT)
		ray->draw_end = HEIGHT - 1;
}
