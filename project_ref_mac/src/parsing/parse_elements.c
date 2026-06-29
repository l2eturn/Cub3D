#include "cub3d.h"

static int	parse_texture(t_game *game, char *line, int wall_type, int side)
{
	char	*path;

	if (game->tex_path[wall_type][side])
		return (printf("Error\nDuplicate texture identifier\n"), 1);
	path = ft_strtrim(line, " \t\n");
	if (!path || ft_strlen(path) == 0)
		return (free(path), printf("Error\nEmpty texture path\n"), 1);
	game->tex_path[wall_type][side] = path;
	return (0);
}

static int	validate_rgb(int r, int g, int b, char **parts)
{
	if (!parts[0] || !parts[1] || !parts[2] || parts[3])
	{
		printf("Error\nInvalid RGB format (need R,G,B)\n");
		return (1);
	}
	if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
	{
		printf("Error\nRGB values must be in range [0, 255]\n");
		return (1);
	}
	return (0);
}

static int	parse_color(t_game *game, char *line, int is_floor)
{
	char	**parts;
	int		r;
	int		g;
	int		b;

	parts = ft_split(line, ',');
	if (!parts)
		return (printf("Error\nColor parse failed\n"), 1);
	r = ft_atoi(parts[0]);
	g = ft_atoi(parts[1]);
	b = ft_atoi(parts[2]);
	if (validate_rgb(r, g, b, parts))
		return (ft_free_split(parts), 1);
	ft_free_split(parts);
	if (is_floor)
		game->floor_color = (r << 16) | (g << 8) | b;
	else
		game->ceil_color = (r << 16) | (g << 8) | b;
	return (0);
}

/*
** Identifier format:
**   NO  ./path.xpm     → wall type 0 (digit '1'), north side
**   SO2 ./path.xpm     → wall type 1 (digit '2'), south side
**   WE3 ./path.xpm     → wall type 2 (digit '3'), west side
**   EA4 ./path.xpm     → wall type 3 (digit '4'), east side
** Returns wall type (0-3) or -1 if not a wall identifier.
*/
static int	match_side(char *line, int *side, int *offset)
{
	if (ft_strncmp(line, "NO", 2) == 0)
		return (*side = NO, *offset = 2, 0);
	if (ft_strncmp(line, "SO", 2) == 0)
		return (*side = SO, *offset = 2, 0);
	if (ft_strncmp(line, "WE", 2) == 0)
		return (*side = WE, *offset = 2, 0);
	if (ft_strncmp(line, "EA", 2) == 0)
		return (*side = EA, *offset = 2, 0);
	return (1);
}

static int	parse_wall_id(t_game *game, char *line)
{
	int	side;
	int	wall_type;
	int	off;

	if (match_side(line, &side, &off))
		return (-1);
	if (line[off] >= '2' && line[off] <= '0' + MAX_WALL_TYPES)
	{
		wall_type = line[off] - '1';
		off++;
	}
	else
		wall_type = 0;
	if (line[off] != ' ' && line[off] != '\t')
		return (-1);
	return (parse_texture(game, line + off + 1, wall_type, side));
}

int	parse_element_line(t_game *game, char *line)
{
	char	*val;
	int		r;

	r = parse_wall_id(game, line);
	if (r == 0)
		return (0);
	if (r == 1)
		return (1);
	if (ft_strncmp(line, "F ", 2) == 0)
	{
		val = ft_strtrim(line + 2, " \t\n");
		if (parse_color(game, val, 1))
			return (free(val), 1);
		return (free(val), 0);
	}
	if (ft_strncmp(line, "C ", 2) == 0)
	{
		val = ft_strtrim(line + 2, " \t\n");
		if (parse_color(game, val, 0))
			return (free(val), 1);
		return (free(val), 0);
	}
	return (printf("Error\nUnknown identifier: %s\n", line), 1);
}

static int	is_empty_line(char *line)
{
	int	i;

	i = 0;
	while (line[i] && line[i] != '\n')
	{
		if (!ft_isspace(line[i]))
			return (0);
		i++;
	}
	return (1);
}

static int	is_map_line(char *line)
{
	int	i;

	i = 0;
	while (line[i] == ' ' || line[i] == '\t')
		i++;
	return (line[i] == '1' || line[i] == '0' || line[i] == ' ');
}

int	parse_elements(t_game *game, int fd, char **first_map_line)
{
	char	*line;

	while ((line = get_next_line(fd)))
	{
		if (is_empty_line(line))
		{
			free(line);
			continue ;
		}
		if (is_map_line(line))
		{
			*first_map_line = line;
			return (0);
		}
		if (parse_element_line(game, line))
			return (free(line), 1);
		free(line);
	}
	return (0);
}
