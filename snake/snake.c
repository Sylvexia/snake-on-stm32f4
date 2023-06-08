#include "snake.h"

#define BG_WIDTH 240
#define BG_HEIGHT 180

enum Direction
{
    UP = (int8_t)1,
    RIGHT = (int8_t)2,
    DOWN = (int8_t)-1,
    LEFT = (int8_t)-2,
    IDLE = (int8_t)0
};

enum GridState
{
    EMPTY,
    HEAD,
    BODY,
    FOOD
};

typedef struct
{
    int8_t x;
    int8_t y;
} Coord;

typedef struct
{
    lv_draw_rect_dsc_t dsc;
    lv_area_t *area;
} Rect;

typedef struct
{
    int8_t state;
    lv_obj_t *rect;
} Playground;

typedef struct
{
    Coord pos;
    struct SnakeNode *next;
    struct SnakeNode *prev;
} SnakeNode;

typedef struct
{
    int8_t dirs;
    uint16_t len;
    SnakeNode *head;
    SnakeNode *tail;
} Snake;

static int8_t row_size = 6;
static int8_t col_size = 8;

volatile static Snake snake;
volatile static Playground **playground;
static lv_obj_t *dir_label;
static lv_obj_t *dir_btns;

#define GROUND_COLOR lv_color_make(31, 30, 51)
#define SNAKE_COLOR lv_color_make(0, 255, 0)

int mod(int x, int y)
{
    int t = x - ((x / y) * y);
    if (t < 0)
        t += y;
    return t;
}

static void draw_dir_label()
{
    // switch (snake.dirs)
    // {
    // case UP:
    //     lv_label_set_text(dir_label, "UP");
    //     break;
    // case RIGHT:
    //     lv_label_set_text(dir_label, "RIGHT");
    //     break;
    // case DOWN:
    //     lv_label_set_text(dir_label, "DOWN");
    //     break;
    // case LEFT:
    //     lv_label_set_text(dir_label, "LEFT");
    //     break;
    // case IDLE:
    //     lv_label_set_text(dir_label, "IDLE");
    //     break;
    // }

    char buf[32] = "";
    sprintf(buf, "x:%u y: %u", snake.head->pos.x, snake.head->pos.y);

    lv_label_set_text(dir_label, buf);
}

static void init_playground()
{
    uint16_t grid_width = BG_WIDTH / col_size;
    uint16_t grid_height = BG_HEIGHT / row_size;

    uint16_t y_offset = 20;

    playground = (Playground **)malloc(sizeof(Playground *) * row_size);
    for (uint8_t i = 0; i < row_size; i++)
    {
        playground[i] = (Playground *)malloc(sizeof(Playground) * col_size);
        for (uint8_t j = 0; j < col_size; j++)
        {
            playground[i][j].state = EMPTY;
            playground[i][j].rect = lv_obj_create(lv_scr_act());
            lv_obj_set_size(playground[i][j].rect, grid_width, grid_height);
            lv_obj_set_pos(playground[i][j].rect, j * grid_width, y_offset + i * grid_height);
            lv_obj_set_style_radius(playground[i][j].rect, 0, LV_PART_MAIN);
            lv_obj_set_style_bg_color(playground[i][j].rect, GROUND_COLOR, LV_PART_MAIN);
        }
    }
    playground[row_size / 2][col_size / 2].state = HEAD;
}

static void push_front_snake(Coord pos)
{
    SnakeNode *new_node = (SnakeNode *)malloc(sizeof(SnakeNode));
    new_node->pos.x = pos.x;
    new_node->pos.y = pos.y;
    new_node->prev = NULL;

    if (snake.len == 0)
    {
        snake.head = snake.tail = new_node;
        new_node->next = NULL;
    }
    else
    {
        new_node->next = snake.head;
        snake.head->prev = new_node;
        snake.head = new_node;
    }

    snake.len++;
}

static void pop_back_snake()
{
    if (snake.len == 0)
        return;

    SnakeNode *temp = snake.tail;
    snake.tail = snake.tail->prev;

    if (snake.tail = NULL)
        snake.head = NULL;
    else
        snake.tail->next = NULL;

    free(temp);

    snake.len--;
}

static void init_snake()
{
    snake.dirs = IDLE;
    snake.head = NULL;
    snake.tail = NULL;
    snake.len = 0;

    Coord pos;
    pos.x = col_size / 2;
    pos.y = row_size / 2;
    push_front_snake(pos);
}

static void snake_move()
{
    Coord cur = snake.head->pos;
    int8_t next_x = cur.x;
    int8_t next_y = cur.y;

    switch (snake.dirs)
    {
    case UP:
        next_y--;
        break;
    case RIGHT:
        next_x++;
        break;
    case DOWN:
        next_y++;
        break;
    case LEFT:
        next_x--;
        break;
    }

    next_x = mod(next_x, col_size);
    next_y = mod(next_y, row_size);

    // update snake
    Coord new;
    new.x = next_x;
    new.y = next_y;

    pop_back_snake();
    push_front_snake(new);
}

static void draw_playground()
{
    for (uint8_t i = 0; i < row_size; i++)
    {
        for (uint8_t j = 0; j < col_size; j++)
        {
            lv_obj_set_style_bg_color(playground[i][j].rect, GROUND_COLOR, LV_PART_MAIN);
        }
    }

    SnakeNode *cur = snake.head;
    while (cur != NULL)
    {
        lv_obj_set_style_bg_color(playground[cur->pos.y][cur->pos.x].rect, SNAKE_COLOR, LV_PART_MAIN);
        cur = cur->next;
    }
}

static void game_timer_cb(lv_timer_t *timer)
{
    snake_move();

    draw_dir_label();
    // draw_playground();
}

static void dirs_btn_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        const char *txt = lv_btnmatrix_get_btn_text(obj, id);
        int8_t temp_dirs = -10;

        if (strcmp(txt, "Up") == 0)
        {
            temp_dirs = UP;
        }
        else if (strcmp(txt, "Right") == 0)
        {
            temp_dirs = RIGHT;
        }
        else if (strcmp(txt, "Down") == 0)
        {
            temp_dirs = DOWN;
        }
        else if (strcmp(txt, "Left") == 0)
        {
            temp_dirs = LEFT;
        }

        if (snake.len <= 1)
        {
            snake.dirs = temp_dirs;
            return;
        }

        if (snake.dirs == -temp_dirs)
        {
            return;
        }

        snake.dirs = temp_dirs;
    }
}

static const char *dir_btn_map[] = {"Up", "\n",
                                    "Left", "Right", "\n",
                                    "Down", ""};

void snake_game(void)
{
    dir_label = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(dir_label, LV_LABEL_LONG_WRAP);
    lv_label_set_recolor(dir_label, true);
    lv_obj_set_width(dir_label, 150);
    lv_label_set_text(dir_label, "IDLE");
    lv_obj_set_style_text_align(dir_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(dir_label, LV_ALIGN_TOP_MID, 0, 0);

    init_snake();
    init_playground();

    dir_btns = lv_btnmatrix_create(lv_scr_act());
    lv_btnmatrix_set_map(dir_btns, dir_btn_map);
    lv_obj_align(dir_btns, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_size(dir_btns, 220, 100);
    lv_obj_add_event_cb(dir_btns, dirs_btn_handler, LV_EVENT_ALL, NULL);

    // create a timer to render the snake
    lv_timer_t *game_timer = lv_timer_create(game_timer_cb, 500, NULL);
}
