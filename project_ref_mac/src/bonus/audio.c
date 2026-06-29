#include "cub3d.h"

static int	g_music_volume = 30;

int	music_get_volume(void)
{
	return (g_music_volume);
}

/*
** macOS: afplay replaces ffplay (Linux). afplay has no -loop flag, so
** we wrap it in a shell while-loop subshell. Volume is 0.0-1.0 (not
** 0-100), so g_music_volume is scaled: vol/100 → "0.XX" or "1.00".
**
** music_stop uses pkill -f to reliably kill afplay even if the parent
** subshell exits before the child finishes.
*/
static void	build_music_cmd(char *cmd, int vol)
{
	int		i;
	char	*p1;
	char	*p2;

	p1 = "(while :; do afplay -v ";
	p2 = " ./resource/horror_game_music.mp3; done)"
		" >/dev/null 2>&1 & echo $! > /tmp/cub3d_music.pid";
	i = 0;
	while (*p1)
		cmd[i++] = *p1++;
	if (vol >= 100)
	{
		cmd[i++] = '1';
		cmd[i++] = '.';
		cmd[i++] = '0';
		cmd[i++] = '0';
	}
	else
	{
		cmd[i++] = '0';
		cmd[i++] = '.';
		cmd[i++] = '0' + (vol / 10);
		cmd[i++] = '0' + (vol % 10);
	}
	while (*p2)
		cmd[i++] = *p2++;
	cmd[i] = 0;
}

void	music_start(void)
{
	char	cmd[256];

	build_music_cmd(cmd, g_music_volume);
	(void)!system(cmd);
}

void	music_stop(void)
{
	(void)!system("pkill -f 'afplay.*horror_game_music' 2>/dev/null;"
		" rm -f /tmp/cub3d_music.pid");
}

void	music_set_volume(int vol)
{
	if (vol < 0)
		vol = 0;
	if (vol > 100)
		vol = 100;
	g_music_volume = vol;
	music_stop();
	music_start();
}

/*
** Ambient scare: random growl when player has been alive for a while.
** Triggered roughly every 600-1200 frames (10-20s at 60fps) — pure
** atmosphere, no damage. Uses anim_tick as deterministic-ish RNG.
*/
void	ambient_scare_tick(t_game *game)
{
	int	period;
	int	roll;

	if (game->player.dead || game->state != STATE_PLAYING)
		return ;
	period = 600 + ((game->anim_tick * 37) % 600);
	if (game->anim_tick % period != 0)
		return ;
	roll = (game->anim_tick * 53) % 3;
	if (roll == 0)
		snd_growl();
	else if (roll == 1)
		snd_door();
	game->flicker_timer = 18;
}

/*
** Flickering torchlight: light_mult oscillates 0.65–1.00 with random
** dips when flicker_timer is non-zero (triggered by ambient scares or
** damage). Multiplied into the fog brightness in render_wall.c and
** render_flat.c so the whole world dims/brightens together.
**
** Render order: must be called BEFORE render_world so the wall+floor
** sample current_light(game) consistently within one frame.
*/
void	update_lighting(t_game *game)
{
	double	base;
	double	wobble;
	double	dip;

	base = 0.92;
	wobble = sin(game->anim_tick * 0.21) * 0.04
		+ sin(game->anim_tick * 0.07) * 0.03;
	dip = 0.0;
	if (game->flicker_timer > 0)
	{
		dip = ((game->anim_tick * 31) % 30) / 30.0 * 0.25;
		game->flicker_timer--;
	}
	game->light_mult = base + wobble - dip;
	if (game->light_mult < 0.55)
		game->light_mult = 0.55;
	if (game->light_mult > 1.0)
		game->light_mult = 1.0;
}

double	current_light(t_game *game)
{
	if (game->light_mult <= 0.0)
		return (1.0);
	return (game->light_mult);
}
