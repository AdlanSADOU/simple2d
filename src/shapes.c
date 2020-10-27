// shapes.c

#include "../include/simple2d.h"


/*
 * Rotate a point around a given point
 * Params:
 *   p      The point to rotate
 *   angle  The angle in degrees
 *   rx     The x coordinate to rotate around
 *   ry     The y coordinate to rotate around
 */
S2D_GL_Point S2D_RotatePoint(S2D_GL_Point p, GLfloat angle, GLfloat rx, GLfloat ry) {

  // Convert from degrees to radians
  angle = angle * M_PI / 180.0;

  // Get the sine and cosine of the angle
  GLfloat sa = sin(angle);
  GLfloat ca = cos(angle);

  // Translate point to origin
  p.x -= rx;
  p.y -= ry;

  // Rotate point
  GLfloat xnew = p.x * ca - p.y * sa;
  GLfloat ynew = p.x * sa + p.y * ca;

  // Translate point back
  p.x = xnew + rx;
  p.y = ynew + ry;

  return p;
}


/*
 * Get the point to be rotated around given a position in a rectangle
 */
S2D_GL_Point S2D_GetRectRotationPoint(int x, int y, int w, int h, int position) {

  S2D_GL_Point p;

  switch (position) {
    case S2D_CENTER:
      p.x = x + (w / 2.0);
      p.y = y + (h / 2.0);
      break;
    case S2D_TOP_LEFT:
      p.x = x;
      p.y = y;
      break;
    case S2D_TOP_RIGHT:
      p.x = x + w;
      p.y = y;
      break;
    case S2D_BOTTOM_LEFT:
      p.x = x;
      p.y = y + h;
      break;
    case S2D_BOTTOM_RIGHT:
      p.x = x + w;
      p.y = y + h;
      break;
  }

  return p;
}


/*
 * Draw a triangle
 */
void S2D_DrawTriangle(GLfloat x1, GLfloat y1,
                      GLfloat r1, GLfloat g1, GLfloat b1, GLfloat a1,
                      GLfloat x2, GLfloat y2,
                      GLfloat r2, GLfloat g2, GLfloat b2, GLfloat a2,
                      GLfloat x3, GLfloat y3,
                      GLfloat r3, GLfloat g3, GLfloat b3, GLfloat a3) {

  S2D_GL_DrawTriangle(x1, y1, r1, g1, b1, a1,
                      x2, y2, r2, g2, b2, a2,
                      x3, y3, r3, g3, b3, a3);
}


/*
 * Draw a quad, using two triangles
 */
void S2D_DrawQuad(GLfloat x1, GLfloat y1,
                  GLfloat r1, GLfloat g1, GLfloat b1, GLfloat a1,
                  GLfloat x2, GLfloat y2,
                  GLfloat r2, GLfloat g2, GLfloat b2, GLfloat a2,
                  GLfloat x3, GLfloat y3,
                  GLfloat r3, GLfloat g3, GLfloat b3, GLfloat a3,
                  GLfloat x4, GLfloat y4,
                  GLfloat r4, GLfloat g4, GLfloat b4, GLfloat a4) {

  S2D_GL_DrawTriangle(x1, y1, r1, g1, b1, a1,
                      x2, y2, r2, g2, b2, a2,
                      x3, y3, r3, g3, b3, a3);

  S2D_GL_DrawTriangle(x3, y3, r3, g3, b3, a3,
                      x4, y4, r4, g4, b4, a4,
                      x1, y1, r1, g1, b1, a1);
};


/*
 * Draw a rectangle, using two triangles. Simplified version of quad
 */
void S2D_DrawRect_XYWH(GLfloat x, GLfloat y, GLfloat width, GLfloat height, S2D_Color color, bool filled) {
  GLfloat x1, y1, r1, g1, b1, a1,
          x2, y2, r2, g2, b2, a2,
          x3, y3, r3, g3, b3, a3,
          x4, y4, r4, g4, b4, a4;

  x1 = x,         y1 = y;
  x2 = x + width, y2 = y;
  x3 = x + width, y3 = y + height;
  x4 = x,         y4 = y + height;

  S2D_GL_DrawTriangle(x1, y1, color.r, color.g, color.b, color.a,
                      x2, y2, color.r, color.g, color.b, color.a,
                      x3, y3, color.r, color.g, color.b, color.a);

  S2D_GL_DrawTriangle(x3, y3, color.r, color.g, color.b, color.a,
                      x4, y4, color.r, color.g, color.b, color.a,
                      x1, y1, color.r, color.g, color.b, color.a);
};


/*
 * Draw a rectangle, using two triangles. Simplified version of quad
 */
void S2D_DrawRect(S2D_FRect rect, S2D_Color color, bool filled) {
  GLfloat x1, y1, r1, g1, b1, a1,
          x2, y2, r2, g2, b2, a2,
          x3, y3, r3, g3, b3, a3,
          x4, y4, r4, g4, b4, a4;

  x1 = rect.x,                y1 = rect.y;
  x2 = rect.x + rect.width,   y2 = rect.y;
  x3 = rect.x + rect.width,   y3 = rect.y + rect.height;
  x4 = rect.x,                y4 = rect.y + rect.height;

  S2D_GL_DrawTriangle(x1, y1, color.r, color.g, color.b, color.a,
                      x2, y2, color.r, color.g, color.b, color.a,
                      x3, y3, color.r, color.g, color.b, color.a);

  S2D_GL_DrawTriangle(x3, y3, color.r, color.g, color.b, color.a,
                      x4, y4, color.r, color.g, color.b, color.a,
                      x1, y1, color.r, color.g, color.b, color.a);
};


/*
 * Draw a line from a quad
 */
void S2D_DrawLine(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2,
                  GLfloat width,
                  GLfloat r1, GLfloat g1, GLfloat b1, GLfloat a1,
                  GLfloat r2, GLfloat g2, GLfloat b2, GLfloat a2,
                  GLfloat r3, GLfloat g3, GLfloat b3, GLfloat a3,
                  GLfloat r4, GLfloat g4, GLfloat b4, GLfloat a4) {

  double length = sqrt(powf(x1 - x2, 2) + powf(y1 - y2, 2));
  double x = ((x2 - x1) / length) * width / 2;
  double y = ((y2 - y1) / length) * width / 2;

  S2D_DrawQuad(
    x1 - y, y1 + x, r1, g1, b1, a1,
    x1 + y, y1 - x, r2, g2, b2, a2,
    x2 + y, y2 - x, r3, g3, b3, a3,
    x2 - y, y2 + x, r4, g4, b4, a4
  );
};


/*
 * Draw a circle from triangles
 */
void S2D_DrawCircle(GLfloat x, GLfloat y, GLfloat radius, int sectors,
                    GLfloat r, GLfloat g, GLfloat b, GLfloat a) {

  double angle = 2 * M_PI / sectors;

  for (int i = 0; i < sectors; i++) {

    GLfloat x1 = x + radius * cos(i * angle);
    GLfloat y1 = y + radius * sin(i * angle);

    GLfloat x2 = x + radius * cos((i - 1) * angle);
    GLfloat y2 = y + radius * sin((i - 1) * angle);

    S2D_GL_DrawTriangle( x,  y, r, g, b, a,
                        x1, y1, r, g, b, a,
                        x2, y2, r, g, b, a);
  }
}
