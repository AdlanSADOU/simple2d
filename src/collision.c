// collision.c

#include "../include/simple2d.h"

/*
 * Check if there is collision between two rectangles
 */
bool S2D_Intersect_FRect(S2D_FRect rect1, S2D_FRect rect2)
{
    return (rect1.x < rect2.x      + rect2.width  &&
            rect1.x + rect1.width  > rect2.x      &&
            rect1.y < rect2.y      + rect2.height &&
            rect1.y + rect1.height > rect2.y
            );
}

/*
 * Check if a rectangle contains a point (ie. if mouse pointer inside a rectangle)
 */
bool S2D_Intersect_Point_FRect(S2D_FRect rect, int point_x, int point_y)
{
    bool topLeft     = (point_x > rect.x);
    bool bottomLeft  = (point_y > rect.y);
    bool topRight    = (point_x < (rect.x + rect.width));
    bool bottomRight = (point_y < (rect.y + rect.height));
    int i = 1;
    return (topLeft && topRight && bottomLeft && bottomRight);
}
