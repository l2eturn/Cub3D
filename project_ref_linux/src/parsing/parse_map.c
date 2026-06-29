#include "cub3d.h"

static int	valid_map_chars(char *line)
{
	int	i;

	i = 0;
	while (line[i] && line[i] != '\n')
	{
		if (line[i] != '0' && line[i] != '1' && line[i] != ' '
			&& line[i] != 'N' && line[i] != 'S'
			&& line[i] != 'E' && line[i] != 'W'
			&& line[i] != 'D' && line[i] != 'G'
			&& line[i] != 'H' && line[i] != 'A'
			&& line[i] != '2' && line[i] != '3'
			&& line[i] != '4' && line[i] != 'X'
			&& line[i] != 'I' && line[i] != 'V')
			return (0);
		i++;
	}
	return (1);
}

static char	*strip_newline(char *line)
{
	int	len;
	int	i;

	len = ft_strlen(line);
	i = 0;
	while (i < len)
	{
		if (line[i] == '\n')
		{
			line[i] = '\0';
			break ;
		}
		i++;
	}
	return (line);
}

static int	grow_grid(t_game *game, char *row)
{
	char	**new_grid;
	int		i;
	int		len;

	new_grid = malloc((game->map.height + 2) * sizeof(char *));
	if (!new_grid)
		return (1);
	i = 0;
	while (i < game->map.height)
	{
		new_grid[i] = game->map.grid[i];
		i++;
	}
	new_grid[i] = row;
	new_grid[i + 1] = NULL;
	free(game->map.grid);
	game->map.grid = new_grid;
	len = ft_strlen(row);
	if (len > game->map.width)
		game->map.width = len;
	game->map.height++;
	return (0);
}

/*
** Read and store all map lines starting from first_line.
** Empty line after map has started = error.
*/
int	parse_map(t_game *game, int fd, char *first_line)
{
	char	*line;
	char	*clean;
	int		started;

	started = 0;
	game->map.grid = malloc(sizeof(char *));
	if (!game->map.grid)
		return (1);
	game->map.grid[0] = NULL;
	line = first_line;
	while (line)
	{
		if (!valid_map_chars(line))
			return (free(line), printf("Error\nInvalid map character\n"), 1);
		clean = strip_newline(ft_strdup(line));
		free(line);
		started = 1;
		if (grow_grid(game, clean))
			return (free(clean), 1);
		line = get_next_line(fd);
		if (line && started)
		{
			if (ft_strlen(ft_strtrim(line, " \t\n")) == 0)
				return (free(line), printf("Error\nEmpty line in map\n"), 1);
		}
	}
	if (game->map.height == 0)
		return (printf("Error\nNo map found\n"), 1);
	return (0);
}
