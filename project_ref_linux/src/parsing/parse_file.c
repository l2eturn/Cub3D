#include "cub3d.h"

static int	is_map_char(char c)
{
	return (c == '1' || c == '0' || c == ' '
		|| c == 'N' || c == 'S' || c == 'E' || c == 'W');
}

static int	is_map_start(char *line)
{
	int	i;

	i = 0;
	while (line[i] && line[i] != '\n')
	{
		if (!is_map_char(line[i]))
			return (0);
		i++;
	}
	if (i == 0)
		return (0);
	return (1);
}

static int	all_elements_set(t_game *game)
{
	int	side;

	side = 0;
	while (side < 4)
	{
		if (!game->tex_path[0][side])
		{
			printf("Error\nMissing texture identifier (wall type 1)\n");
			return (0);
		}
		side++;
	}
	if (game->floor_color == -1 || game->ceil_color == -1)
	{
		printf("Error\nMissing floor or ceiling color\n");
		return (0);
	}
	return (1);
}

/*
** Open the .cub file, parse elements until the map starts,
** then hand off the first map line to parse_map().
*/
int	parse_cub(t_game *game, char *path)
{
	int		fd;
	char	*line;
	char	*first_map;

	game->floor_color = -1;
	game->ceil_color = -1;
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (printf("Error\nCannot open file: %s\n", path), 1);
	first_map = NULL;
	if (parse_elements(game, fd, &first_map))
		return (close(fd), 1);
	if (!all_elements_set(game))
		return (close(fd), free(first_map), 1);
	if (!first_map)
	{
		line = get_next_line(fd);
		while (line && !is_map_start(line))
		{
			free(line);
			line = get_next_line(fd);
		}
		first_map = line;
	}
	if (parse_map(game, fd, first_map))
		return (close(fd), 1);
	close(fd);
	return (validate_map(game));
}
