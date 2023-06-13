// the snake is queue, so snake.tail is actually head and vice versa

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
    TAIL,
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
} SnakeNode;

typedef struct
{
    int8_t dirs;
    uint16_t len;
    SnakeNode *head;
    SnakeNode *tail;
} Snake;

static int8_t row_size = 9;
static int8_t col_size = 12;

volatile static Snake snake;
volatile static Playground **playground;
static lv_obj_t *coord_label;
static lv_obj_t *score_label;
static lv_obj_t *dir_btns;

#define GROUND_COLOR lv_color_make(31, 30, 51)
#define SNAKE_COLOR lv_color_make(0, 255, 0)
#define FOOD_COLOR lv_color_make(255, 255, 0)
#define HEAD_COLOR lv_color_make(152, 251, 152)

int mod(int x, int y)
{
    int t = x - ((x / y) * y);
    if (t < 0)
        t += y;
    return t;
}

static void init_score_label()
{
    score_label = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(score_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(score_label, 150);
    lv_obj_set_style_text_align(score_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(score_label, LV_ALIGN_TOP_LEFT, -40, 0);
}

static void draw_score_label()
{
    lv_label_set_text_fmt(score_label, "Score: %d", snake.len - 1);
}

static void init_coord_label()
{
    coord_label = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(coord_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(coord_label, 150);
    lv_obj_set_style_text_align(coord_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(coord_label, LV_ALIGN_TOP_RIGHT, 40, 0);
}

static void draw_coord_label()
{
    lv_label_set_text_fmt(coord_label, "x:%u y: %u", snake.head->pos.x, snake.head->pos.y);
}

static bool collision_body(Coord cur_coord)
{
    return (playground[cur_coord.y][cur_coord.x].state == BODY);
}

static bool collision_food(Coord cur_coord)
{
    return (playground[cur_coord.y][cur_coord.x].state == FOOD);
}

static void generate_food()
{
    // generate the food tha does not have the food and head
    Coord food_coord;
    food_coord.x = rand() % col_size;
    food_coord.y = rand() % row_size;

    while (playground[food_coord.y][food_coord.x].state != EMPTY)
    {
        food_coord.x = rand() % col_size;
        food_coord.y = rand() % row_size;
    }

    playground[food_coord.y][food_coord.x].state = FOOD;
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
            lv_obj_set_style_border_width(playground[i][j].rect, 0, LV_PART_MAIN);
            lv_obj_set_style_outline_width(playground[i][j].rect, 0, LV_PART_MAIN);
            lv_obj_clear_flag(playground[i][j].rect, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_radius(playground[i][j].rect, 0, LV_PART_MAIN);
        }
    }
    playground[row_size / 2][col_size / 2].state = HEAD;
}

static void push_snake(Coord pos)
{
    if (snake.len > 0)
    {
        Coord old_pos = snake.tail->pos;
        playground[old_pos.y][old_pos.x].state = BODY;
    }
    Coord new_pos = pos;
    playground[new_pos.y][new_pos.x].state = HEAD;

    SnakeNode *new_node = (SnakeNode *)malloc(sizeof(SnakeNode));
    new_node->pos.x = pos.x;
    new_node->pos.y = pos.y;

    if (snake.len == 0)
    {
        snake.head = new_node;
        snake.tail = new_node;
    }
    else
    {
        snake.tail->next = new_node;
        snake.tail = new_node;
    }

    snake.len++;
}

static void pop_snake()
{
    if (snake.len == 0)
        return;

    SnakeNode *temp = snake.head;
    Coord old_pos = temp->pos;
    snake.head = temp->next;

    if (snake.head == NULL)
        snake.tail = NULL;

    playground[old_pos.y][old_pos.x].state = EMPTY;

    free(temp);

    snake.len--;
}

static void init_snake()
{
    Coord pos;
    pos.x = col_size / 2;
    pos.y = row_size / 2;

    snake.dirs = RIGHT;
    snake.head = NULL;
    snake.tail = NULL;
    snake.len = 0;

    push_snake(pos);
    playground[pos.y][pos.x].state = HEAD;
}

static void snake_move()
{
    Coord cur_coord = snake.tail->pos;
    int8_t next_x = cur_coord.x;
    int8_t next_y = cur_coord.y;

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
    Coord new_coord;
    new_coord.x = next_x;
    new_coord.y = next_y;

    if (!collision_food(new_coord))
        pop_snake();
    else
        generate_food();

    if (collision_body(new_coord)) // touch the body, game over, reset game state
    {
        // pop all snake node from queue
        while (snake.len > 0)
            pop_snake();
        init_snake();
        for (uint8_t i = 0; i < row_size; i++)
        {
            for (uint8_t j = 0; j < col_size; j++)
                playground[i][j].state = EMPTY;
        }
        generate_food();
        return; // this is kinda hacky, but it works
    }
    
    push_snake(new_coord);
}

static void draw_playground()
{
    for (uint8_t i = 0; i < row_size; i++)
    {
        for (uint8_t j = 0; j < col_size; j++)
        {
            switch (playground[i][j].state)
            {
            case HEAD:
                lv_obj_set_style_bg_color(playground[i][j].rect, HEAD_COLOR, LV_PART_MAIN);
                break;
            case BODY:
                lv_obj_set_style_bg_color(playground[i][j].rect, SNAKE_COLOR, LV_PART_MAIN);
                break;
            case FOOD:
                lv_obj_set_style_bg_color(playground[i][j].rect, FOOD_COLOR, LV_PART_MAIN);
                break;
            case EMPTY:
                lv_obj_set_style_bg_color(playground[i][j].rect, GROUND_COLOR, LV_PART_MAIN);
                break;
            }
        }
    }
}

static void game_timer_cb(lv_timer_t *timer)
{
    snake_move();

    draw_coord_label();
    draw_score_label();
    draw_playground();
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
            temp_dirs = UP;
        else if (strcmp(txt, "Right") == 0)
            temp_dirs = RIGHT;
        else if (strcmp(txt, "Down") == 0)
            temp_dirs = DOWN;
        else if (strcmp(txt, "Left") == 0)
            temp_dirs = LEFT;

        if (snake.len <= 1)
        {
            snake.dirs = temp_dirs;
            return;
        }

        if (snake.dirs == -temp_dirs)
            return;

        snake.dirs = temp_dirs;
    }
}

static const char *dir_btn_map[] = {"Up", "\n",
                                    "Left", "Right", "\n",
                                    "Down", ""};

void snake_game(void)
{
    srand(time(0));

    init_coord_label();
    init_snake();
    init_playground();
    init_score_label();
    generate_food();
    draw_playground();

    dir_btns = lv_btnmatrix_create(lv_scr_act());
    lv_btnmatrix_set_map(dir_btns, dir_btn_map);
    lv_obj_align(dir_btns, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_size(dir_btns, 220, 100);
    lv_obj_add_event_cb(dir_btns, dirs_btn_handler, LV_EVENT_ALL, NULL);

    // create a timer to render the snake
    lv_timer_t *game_timer = lv_timer_create(game_timer_cb, 500, NULL);
}