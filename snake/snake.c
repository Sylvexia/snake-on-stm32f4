#include "snake.h"

#define BG_WIDTH 240
#define BG_HEIGHT 180

enum Direction
{
    UP,
    RIGHT,
    DOWN,
    LEFT,
    IDLE
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
    uint8_t x;
    uint8_t y;
} Coord;

typedef struct
{
    lv_draw_rect_dsc_t dsc;
    lv_area_t *area;
} Rect;

typedef struct
{
    uint8_t state;
    lv_obj_t *rect;
} Playground;

typedef struct
{
    Coord head_pos;
    struct SnakeNode *next;
} SnakeNode;

typedef struct
{
    uint16_t dirs;
    uint16_t len;
    SnakeNode *head;
} Snake;

static uint8_t row_size = 6;
static uint8_t col_size = 8;

static Snake snake;
volatile static Playground **playground;
static lv_obj_t *dir_label;
static lv_obj_t *dir_btns;

static void draw_dir_label()
{
    switch (snake.dirs)
    {
    case UP:
        lv_label_set_text(dir_label, "UP");
        break;
    case RIGHT:
        lv_label_set_text(dir_label, "RIGHT");
        break;
    case DOWN:
        lv_label_set_text(dir_label, "DOWN");
        break;
    case LEFT:
        lv_label_set_text(dir_label, "LEFT");
        break;
    case IDLE:
        lv_label_set_text(dir_label, "IDLE");
        break;
    }
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
            lv_obj_set_style_bg_color(playground[i][j].rect, lv_color_make(255, 163, 26), LV_PART_MAIN);
        }
    }
    playground[row_size / 2][col_size / 2].state = HEAD;
}

static void draw_playground()
{
}

static void game_timer_cb(lv_timer_t *timer)
{
    // render snake
    draw_dir_label();
    // lv_obj_set_style_bg_color(playground[1][1].rect, lv_color_make(0, 0, 0), LV_PART_MAIN);
    // lv_obj_invalidate(playground[1][1].rect);
}

static void dirs_btn_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED)
    {
        uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        const char *txt = lv_btnmatrix_get_btn_text(obj, id);

        if (strcmp(txt, "Up") == 0)
        {
            snake.dirs = UP;
        }
        else if (strcmp(txt, "Right") == 0)
        {
            snake.dirs = RIGHT;
        }
        else if (strcmp(txt, "Down") == 0)
        {
            snake.dirs = DOWN;
        }
        else if (strcmp(txt, "Left") == 0)
        {
            snake.dirs = LEFT;
        }
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

    init_playground();

    dir_btns = lv_btnmatrix_create(lv_scr_act());
    lv_btnmatrix_set_map(dir_btns, dir_btn_map);
    lv_obj_align(dir_btns, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_size(dir_btns, 220, 100);
    lv_obj_add_event_cb(dir_btns, dirs_btn_handler, LV_EVENT_ALL, NULL);

    // create a timer to render the snake
    lv_timer_t *game_timer = lv_timer_create(game_timer_cb, 33, NULL);
}
