#include "cub3d.h"

/*
** Simple GNL using a static leftover string per fd.
** Returns one line including '\n' (if present), or NULL at EOF.
*/

static char	*g_leftover[4096];

static char	*extract_line(char **leftover)
{
	char	*nl;
	char	*line;
	char	*new_left;

	nl = ft_strchr(*leftover, '\n');
	if (!nl)
	{
		line = ft_strdup(*leftover);
		free(*leftover);
		*leftover = NULL;
		return (line);
	}
	line = ft_strndup(*leftover, (int)(nl - *leftover) + 1);
	new_left = ft_strdup(nl + 1);
	free(*leftover);
	*leftover = new_left;
	return (line);
}

static int	append_buf(char **leftover, char *buf)
{
	char	*tmp;

	if (!*leftover)
	{
		*leftover = ft_strdup(buf);
		return (*leftover != NULL);
	}
	tmp = ft_strjoin(*leftover, buf);
	free(*leftover);
	*leftover = tmp;
	return (*leftover != NULL);
}

char	*get_next_line(int fd)
{
	char	buf[BUFFER_SIZE + 1];
	int		r;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	while (!g_leftover[fd] || !ft_strchr(g_leftover[fd], '\n'))
	{
		r = read(fd, buf, BUFFER_SIZE);
		if (r <= 0)
			break ;
		buf[r] = '\0';
		if (!append_buf(&g_leftover[fd], buf))
			return (NULL);
	}
	if (!g_leftover[fd] || g_leftover[fd][0] == '\0')
	{
		free(g_leftover[fd]);
		g_leftover[fd] = NULL;
		return (NULL);
	}
	return (extract_line(&g_leftover[fd]));
}
