#include "cub3d.h"

/*
** Apply rotation matrix to dir and plane by `angle`.
** plane MUST rotate with dir to preserve FOV consistency.
*/
static void	rotate_view(t_player *p, double angle)
{
	double	old_dir_x;
	double	old_plane_x;

	old_dir_x = p->dir_x;
	p->dir_x = p->dir_x * cos(angle) - p->dir_y * sin(angle);
	p->dir_y = old_dir_x * sin(angle) + p->dir_y * cos(angle);
	old_plane_x = p->plane_x;
	p->plane_x = p->plane_x * cos(angle) - p->plane_y * sin(angle);
	p->plane_y = old_plane_x * sin(angle) + p->plane_y * cos(angle);
}

/*
** Poll mouse position each frame, derive delta from screen center,
** rotate view, warp cursor back to center. MLX42 disables OS cursor
** with MLX_MOUSE_DISABLED — virtual position keeps growing past edges.
*/
void	update_mouse_look(t_game *game)
{
	int32_t	mx;
	int32_t	my;
	int		delta;

	if (game->state != STATE_PLAYING)
		return ;
	mlx_get_mouse_pos(game->mlx.mlx, &mx, &my);
	delta = (int)mx - WIDTH / 2;
	if (delta != 0 && abs(delta) < WIDTH)
		rotate_view(&game->player, delta * game->sens_level * 0.0001);
	mlx_set_mouse_pos(game->mlx.mlx, WIDTH / 2, HEIGHT / 2);
	(void)my;
}
