#include "cub3d.h"

/*
** Combined screen offset: damage-shake (jagged 2D from anim_tick hash)
** + walking head-bob (smooth lissajous: fast-bob vertical, slower
** lateral sway). Bob amplitude scales with sprint state for that
** "running breathless" feel.
*/
static void	shake_offset(t_game *game, int *sx, int *sy)
{
	int		s;
	double	amp;
	double	t;

	*sx = 0;
	*sy = 0;
	s = game->player.shake;
	if (s > 0)
	{
		*sx = ((game->anim_tick * 73) % (s * 2 + 1)) - s;
		*sy = ((game->anim_tick * 137 + 11) % (s * 2 + 1)) - s;
	}
	if (game->player.move_fb || game->player.move_lr)
	{
		amp = game->player.sprint ? 6.0 : 3.0;
		t = game->anim_tick * 0.22;
		*sy -= (int)(fabs(sin(t)) * amp);
		*sx += (int)(sin(t * 0.5) * amp * 0.7);
	}
}

static void	render_world(t_game *game)
{
	t_ray	ray;
	int		x;

	x = 0;
	while (x < WIDTH)
	{
		init_ray(&ray, &game->player, x);
		init_step(&ray, &game->player);
		perform_dda(&ray, game);
		calc_wall_data(&ray);
		game->zbuf[x] = ray.perp_wall_dist;
		draw_flat(&game->mlx.screen, x, 0, ray.draw_start - 1,
			game->ceil_color);
		draw_column(game, &ray, x);
		draw_flat(&game->mlx.screen, x, ray.draw_end + 1, HEIGHT - 1,
			game->floor_color);
		x++;
	}
}

void	render_frame(void *param)
{
	t_game	*game;
	int		sx;
	int		sy;

	game = (t_game *)param;
	clear_strings(game);
	if (game->state == STATE_MENU)
	{
		render_menu(game);
		return ;
	}
	if (game->state == STATE_CHEAT)
	{
		render_cheat(game);
		game->anim_tick++;
		return ;
	}
	if (game->state == STATE_SETTINGS)
	{
		render_settings(game);
		game->anim_tick++;
		return ;
	}
	update_mouse_look(game);
	update_player(game);
	update_ghosts(game);
	update_pickups(game);
	update_aim(game);
	update_weapon_anim(game);
	update_doors(game);
	update_lighting(game);
	ambient_scare_tick(game);
	dialog_update(game);
	update_npc(game);
	update_jumpscare(game);
	render_world(game);
	render_sprites(game);
	render_npc(game);
	apply_vignette(game);
	render_gun(game);
	if (game->npc.minimap_unlocked)
		render_minimap(game);
	render_hud(game);
	render_jumpscare(game);
	update_combat(game);
	shake_offset(game, &sx, &sy);
	if (game->mlx.screen.img && game->mlx.screen.img->instances)
	{
		game->mlx.screen.img->instances[0].x = sx;
		game->mlx.screen.img->instances[0].y = sy;
	}
	render_text_overlay(game);
	game->anim_tick++;
}
