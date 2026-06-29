#include "cub3d.h"

#define DIALOG_LINES   5
#define DIALOG_HOLD    280

static const char	*g_lines[DIALOG_LINES] = {
	"...wake up traveler. you are in the abyss.",
	"ghosts wander these halls. they hunger.",
	"find the warden NPC and talk - press E when close.",
	"the warden will give you a gun and reveal the map.",
	"silence them all. then escape."
};

void	dialog_start(t_game *game)
{
	game->dialog_index = 0;
	game->dialog_timer = DIALOG_HOLD;
}

void	dialog_update(t_game *game)
{
	if (game->dialog_index >= DIALOG_LINES)
		return ;
	if (game->dialog_timer > 0)
	{
		game->dialog_timer--;
		return ;
	}
	game->dialog_index++;
	game->dialog_timer = DIALOG_HOLD;
}

static void	fill_rect(t_img *img, int xy[4], int color)
{
	int	dx;
	int	dy;

	dy = 0;
	while (dy < xy[3])
	{
		dx = 0;
		while (dx < xy[2])
		{
			put_pixel(img, xy[0] + dx, xy[1] + dy, color);
			dx++;
		}
		dy++;
	}
}

static void	draw_frame(t_img *img, int xy[4], int color)
{
	int	i;

	i = 0;
	while (i < xy[2])
	{
		put_pixel(img, xy[0] + i, xy[1], color);
		put_pixel(img, xy[0] + i, xy[1] + xy[3] - 1, color);
		i++;
	}
	i = 0;
	while (i < xy[3])
	{
		put_pixel(img, xy[0], xy[1] + i, color);
		put_pixel(img, xy[0] + xy[2] - 1, xy[1] + i, color);
		i++;
	}
}

void	dialog_render(t_game *game)
{
	int		panel[4];
	int		cx;
	int		y;
	char	*line;

	if (game->dialog_index >= DIALOG_LINES)
		return ;
	cx = WIDTH / 2;
	y = HEIGHT - 150;
	panel[0] = cx - 400;
	panel[1] = y - 10;
	panel[2] = 800;
	panel[3] = 80;
	fill_rect(&game->mlx.screen, panel, 0x080604);
	draw_frame(&game->mlx.screen, panel, 0x886622);
	put_string(game, cx - 4 * FONT_W, y, "[ INTRO ]");
	line = (char *)g_lines[game->dialog_index];
	put_string(game, cx - (ft_strlen(line) * FONT_W) / 2, y + 28, line);
	put_string(game, panel[0] + panel[2] - 130,
		panel[1] + panel[3] - 22, "[skip: space]");
}
