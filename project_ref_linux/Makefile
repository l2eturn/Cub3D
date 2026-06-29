NAME     = cub3D
CC       = cc
FLAGS    = -Wall -Wextra -Werror -O2
RM       = rm -f

MLX_DIR  = ../MLX42
MLX_LIB  = $(MLX_DIR)/build/libmlx42.a
MLX_INC  = -I$(MLX_DIR)/include
MLX_LINK = $(MLX_LIB) -ldl -lglfw -pthread -lm

INC      = -Iincludes $(MLX_INC)

SRCS     = src/main.c \
           src/parsing/parse_file.c \
           src/parsing/parse_elements.c \
           src/parsing/parse_map.c \
           src/parsing/validate_map.c \
           src/mlx/mlx_init.c \
           src/mlx/mlx_utils.c \
           src/mlx/mlx_text.c \
           src/textures/texture_load.c \
           src/player/player.c \
           src/events/events.c \
           src/raycast/raycast.c \
           src/render/render_frame.c \
           src/render/render_wall.c \
           src/render/render_flat.c \
           src/cleanup/cleanup.c \
           src/utils/utils.c \
           src/utils/gnl.c \
           src/bonus/door.c \
           src/bonus/minimap.c \
           src/bonus/mouse.c \
           src/bonus/sprite.c \
           src/bonus/gun.c \
           src/bonus/hud.c \
           src/bonus/combat.c \
           src/bonus/vignette.c \
           src/bonus/menu.c \
           src/bonus/ghost_ai.c \
           src/bonus/pickups.c \
           src/bonus/restart.c \
           src/bonus/sound.c \
           src/bonus/highscore.c \
           src/bonus/dialog.c \
           src/bonus/npc.c \
           src/bonus/jumpscare.c \
           src/bonus/cheat.c \
           src/bonus/audio.c \
           src/bonus/settings.c

OBJS     = $(SRCS:.c=.o)
HEADERS  = includes/cub3d.h

all: $(MLX_LIB) $(NAME)

$(MLX_LIB):
	@cd $(MLX_DIR) && cmake -B build && cmake --build build -j

$(NAME): $(OBJS)
	$(CC) $(FLAGS) $(OBJS) $(MLX_LINK) -o $(NAME)

%.o: %.c $(HEADERS)
	$(CC) $(FLAGS) $(INC) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

bonus: all

.PHONY: all clean fclean re bonus
