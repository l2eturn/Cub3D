#include "cub3d.h"

#define HS_PATH "./highscore.txt"

int	load_high_score(void)
{
	int		fd;
	char	buf[16];
	int		n;
	int		score;
	int		i;

	fd = open(HS_PATH, O_RDONLY);
	if (fd < 0)
		return (0);
	n = read(fd, buf, 15);
	close(fd);
	if (n <= 0)
		return (0);
	buf[n] = 0;
	score = 0;
	i = 0;
	while (i < n && buf[i] >= '0' && buf[i] <= '9')
	{
		score = score * 10 + (buf[i] - '0');
		i++;
	}
	return (score);
}

static void	int_to_buf(int n, char *buf, int *len)
{
	char	tmp[12];
	int		i;
	int		j;

	if (n == 0)
	{
		buf[0] = '0';
		buf[1] = '\n';
		*len = 2;
		return ;
	}
	i = 0;
	while (n > 0 && i < 11)
	{
		tmp[i++] = '0' + (n % 10);
		n /= 10;
	}
	j = 0;
	while (i > 0)
		buf[j++] = tmp[--i];
	buf[j++] = '\n';
	*len = j;
}

void	save_high_score(int score)
{
	int		fd;
	char	buf[16];
	int		len;

	fd = open(HS_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
		return ;
	int_to_buf(score, buf, &len);
	(void)!write(fd, buf, len);
	close(fd);
}
