#include "cub3d.h"

static char	map_get(t_map *map, int x, int y)
{
	if (y < 0 || y >= map->height)
		return (' ');
	if (x < 0 || x >= (int)ft_strlen(map->grid[y]))
		return (' ');
	return (map->grid[y][x]);
}

/*
** Correct flood fill order:
** 1. OOB first → prevents visited[] UB, identifies open maps
** 2. Already visited → skip
** 3. Mark visited
** 4. Wall ('1') → safe boundary, stop (return 0)
** 5. Space/null → open map (return 1 = error)
** 6. Walkable → recurse to all 4 neighbors
**
** Previous bug: boundary check (x<=0) came BEFORE wall check (c=='1'),
** so a wall cell at x=0 falsely triggered "open map" error.
*/
static int	flood_fill(t_map *map, int **visited, int x, int y)
{
	char	c;

	if (x < 0 || x >= map->width || y < 0 || y >= map->height)
		return (1);
	if (visited[y][x])
		return (0);
	visited[y][x] = 1;
	c = map_get(map, x, y);
	if (c >= '1' && c <= '0' + MAX_WALL_TYPES)
		return (0);
	if (c == ' ' || c == '\0')
		return (1);
	if (c == 'D')
		return (0);
	if (flood_fill(map, visited, x + 1, y))
		return (1);
	if (flood_fill(map, visited, x - 1, y))
		return (1);
	if (flood_fill(map, visited, x, y + 1))
		return (1);
	return (flood_fill(map, visited, x, y - 1));
}

static int	**alloc_visited(t_map *map)
{
	int	**v;
	int	y;

	v = malloc(map->height * sizeof(int *));
	if (!v)
		return (NULL);
	y = 0;
	while (y < map->height)
	{
		v[y] = malloc(map->width * sizeof(int));
		if (!v[y])
			return (NULL);
		memset(v[y], 0, map->width * sizeof(int));
		y++;
	}
	return (v);
}

static void	free_visited(int **v, int height)
{
	int	y;

	y = 0;
	while (y < height)
		free(v[y++]);
	free(v);
}

static int	find_player(t_map *map, int *px, int *py)
{
	int		x;
	int		y;
	char	c;
	int		found;

	found = 0;
	y = 0;
	while (y < map->height)
	{
		x = 0;
		while (x < (int)ft_strlen(map->grid[y]))
		{
			c = map->grid[y][x];
			if (c == 'N' || c == 'S' || c == 'E' || c == 'W')
			{
				if (found++)
					return (printf("Error\nMultiple players\n"), -1);
				*px = x;
				*py = y;
			}
			x++;
		}
		y++;
	}
	if (!found)
		return (printf("Error\nNo player found\n"), -1);
	return (0);
}

int	validate_map(t_game *game)
{
	int		**visited;
	int		px;
	int		py;
	int		result;

	if (find_player(&game->map, &px, &py) < 0)
		return (1);
	visited = alloc_visited(&game->map);
	if (!visited)
		return (1);
	result = flood_fill(&game->map, visited, px, py);
	free_visited(visited, game->map.height);
	if (result)
		return (printf("Error\nMap is not closed/surrounded by walls\n"), 1);
	return (0);
}
