#include <tonc_core.h>
#include <tonc_tte.h>
#include <tonc_math.h>

#include "graphic_utils.h"

const Rect FULL_SCREENBLOCK_RECT = { 0, 0, SE_ROW_LEN - 1, SE_COL_LEN - 1};

// Clips a rect of screenblock entries to a specified rect
// The bounding rect is not required to be within screenblock boundaries
static void clip_se_rect_to_bounding_rect(Rect* rect, const Rect* bounding_rect)
{
    rect->right = min(rect->right, bounding_rect->right);
    rect->bottom = min(rect->bottom, bounding_rect->bottom);
    rect->left = max(rect->left, bounding_rect->left);
    rect->top = max(rect->top, bounding_rect->top);
}

// Can be unstaticed if needed
// Clips a rect of screenblock entries to screenblock boundaries
static void clip_se_rect_to_screenblock(Rect* rect)
{
    clip_se_rect_to_bounding_rect(rect, &FULL_SCREENBLOCK_RECT);
}

u16 main_bg_se_get_tile(BG_POINT pos)
{
    return se_mem[MAIN_BG_SBB][pos.x + SE_ROW_LEN * pos.y];
}

// Clips a rect of screenblock entries to be within one step of 
// screenblock boundaries vertically depending on direction.
static void clip_se_rect_within_step_of_full_screen_vert(Rect* se_rect, int direction)
{
    Rect bounding_rect = FULL_SCREENBLOCK_RECT;
    if (direction == SE_UP)
    {
        bounding_rect.top += 1;
    }
    else if (direction == SE_DOWN)
    {
        bounding_rect.bottom -= 1;
    }
    
    clip_se_rect_to_bounding_rect(se_rect, &bounding_rect);
}

void main_bg_se_clear_rect(Rect se_rect)
{
    if (se_rect.left > se_rect.right)
        return;
    // Clip to avoid screenblock overflow
    clip_se_rect_to_screenblock(&se_rect);

    for (int y = se_rect.top; y < se_rect.bottom; y++)
    {
        memset16(&(se_mem[MAIN_BG_SBB][se_rect.left + SE_ROW_LEN * y]), 0x0000, rect_width(&se_rect));
    }
}


// Internal static function to merge implementation of move/copy functions.
static void main_bg_se_copy_or_move_rect_1_tile_vert(Rect se_rect, int direction, bool move)
{
    if (se_rect.left > se_rect.right
        || (direction != SE_UP && direction != SE_DOWN))
    {
        return;
    }

    // Clip to avoid read/write overflow of the screenblock
    clip_se_rect_within_step_of_full_screen_vert(&se_rect, direction);

    int start = (direction == SE_UP) ? se_rect.top : se_rect.bottom;
    int end = (direction == SE_UP) ? se_rect.bottom : se_rect.top;

    for (int y = start; y != end - direction; y -= direction)
    {
        memcpy16(&se_mem[MAIN_BG_SBB][se_rect.left + SE_ROW_LEN * (y + direction)],
                 &se_mem[MAIN_BG_SBB][se_rect.left + SE_ROW_LEN * y],
                 rect_width(&se_rect));
    }

    if (move)
    {
        memset16(&se_mem[MAIN_BG_SBB][se_rect.left + SE_ROW_LEN * (end)], 0x0000, rect_width(&se_rect));
    }
}

void main_bg_se_copy_rect_1_tile_vert(Rect se_rect, int direction)
{
    main_bg_se_copy_or_move_rect_1_tile_vert(se_rect, direction, false);
}

void main_bg_se_move_rect_1_tile_vert(Rect se_rect, int direction)
{
    main_bg_se_copy_or_move_rect_1_tile_vert(se_rect, direction, true);
}

void main_bg_se_copy_rect(Rect se_rect, BG_POINT pos)
{
    if (se_rect.left > se_rect.right || se_rect.top > se_rect.bottom)
        return;

    // Clip to avoid screenblock overflow
    clip_se_rect_to_screenblock(&se_rect);

    int width = rect_width(&se_rect);
    int height = rect_height(&se_rect);
    u16 tile_map[height][width];

    // Copy the rect to the tile map
    for (int sy = 0; sy < height; sy++)
    {
        for (int sx = 0; sx < width; sx++)
        {
            BG_POINT pt;
            pt.x = se_rect.left + sx;
            pt.y = se_rect.top + sy;
            tile_map[sy][sx] = main_bg_se_get_tile(pt);
        }
    }

    for (int sy = 0; sy < height; sy++)
    {
        memcpy16(&se_mem[MAIN_BG_SBB][pos.x + SE_ROW_LEN * (pos.y + sy)],
                 &tile_map[sy][0],
                 width);
    }
}

void main_bg_se_copy_tile_to_rect(u16 tile, Rect se_rect)
{
    if (se_rect.left > se_rect.right || se_rect.top > se_rect.bottom)
        return;

    // Clip to avoid screenblock overflow
    clip_se_rect_to_screenblock(&se_rect);

    int width = rect_width(&se_rect);
    int height = rect_height(&se_rect);

    for (int sy = 0; sy < height; sy++)
    {
        memset16(&se_mem[MAIN_BG_SBB][se_rect.left + SE_ROW_LEN * (se_rect.top + sy)], tile, width);
    }
}

void tte_erase_rect_wrapper(Rect rect)
{
    tte_erase_rect(rect.left, rect.top, rect.right, rect.bottom);
}

