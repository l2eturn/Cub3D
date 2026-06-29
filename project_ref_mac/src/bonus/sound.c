#include "cub3d.h"
#include <time.h>

/*
** Channel-based debounce inspired by DOOM's sndserv: 8 channels, each
** tracks when it last fired so a rapid-fire trigger (shoot spam,
** footstep storm) does not start overlapping aplay processes that
** all play the same sample at full volume.
**
** SND_COOLDOWN_MS is the minimum gap between two plays of the same
** key. New plays for distinct keys still go through immediately.
*/
# define SND_CHANNELS    8
# define SND_COOLDOWN_MS 80

static long	now_ms(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000L + ts.tv_nsec / 1000000L);
}

static int	channel_blocked(const char *key)
{
	static const char	*last_key[SND_CHANNELS];
	static long			last_ms[SND_CHANNELS];
	long				t;
	int					i;

	t = now_ms();
	i = 0;
	while (i < SND_CHANNELS)
	{
		if (last_key[i] == key && t - last_ms[i] < SND_COOLDOWN_MS)
			return (1);
		i++;
	}
	i = 0;
	while (i < SND_CHANNELS && last_key[i] != NULL && last_key[i] != key)
		i++;
	if (i == SND_CHANNELS)
		i = (int)(t % SND_CHANNELS);
	last_key[i] = key;
	last_ms[i] = t;
	return (0);
}

/*
** macOS: afplay replaces aplay (Linux). afplay is built into macOS,
** plays WAV/MP3 natively, quiet by default — no -q flag needed.
**
** Audio note: system() is NOT 42-Norm safe. Reference only.
*/
static void	play_sound(char *wav_path)
{
	if (channel_blocked(wav_path))
		return ;

	char	cmd[128];
	int		i;
	char	*prefix;
	char	*suffix;

	i = 0;
	prefix = "afplay ";
	while (*prefix && i < 120)
		cmd[i++] = *prefix++;
	while (*wav_path && i < 120)
		cmd[i++] = *wav_path++;
	suffix = " >/dev/null 2>&1 &";
	while (*suffix && i < 127)
		cmd[i++] = *suffix++;
	cmd[i] = 0;
	(void)!system(cmd);
}

void	snd_shoot(void)
{
	play_sound("./resource/sounds/shoot.wav");
}

void	snd_hit(void)
{
	play_sound("./resource/sounds/hit.wav");
}

void	snd_pickup(void)
{
	play_sound("./resource/sounds/pickup.wav");
}

void	snd_hurt(void)
{
	play_sound("./resource/sounds/hurt.wav");
}

void	snd_door(void)
{
	play_sound("./resource/sounds/door.wav");
}

void	snd_death(void)
{
	play_sound("./resource/sounds/death.wav");
}

void	snd_footstep(void)
{
	play_sound("./resource/sounds/footstep.wav");
}

void	snd_growl(void)
{
	play_sound("./resource/sounds/growl.wav");
}
