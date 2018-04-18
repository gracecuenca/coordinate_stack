#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "matrix.h"
#include "parser.h"
#include "stack.h"

/*
To implement a relative coordinate system... system, add/modify your current parser so it has the following behavior
translate/rotate/scale
create a translation/rotation/scale matrix
multiply the current top of the cs stack by it
The ordering of multiplication is important here.
box/sphere/torus
add a box/sphere/torus to a temporary polygon matrix
multiply it by the current top of the cs stack
draw it to the screen
clear the polygon matrix
line/curve/circle
add a line to a temporary edge matrix
multiply it by the current top
draw it to the screen (note a line is not a solid, so avoid draw_polygons)
save
save the screen with the provided file name
display
show the image
Also note that the ident, apply and clear commands no longer have any use
*/
/*======== void parse_file () ==========
Inputs:   char * filename
          struct matrix * transform,
          struct matrix * edges,
          struct matrix * polygons,
          screen s
Returns:

Goes through the file named filename and performs all of the actions listed in that file.
The file follows the following format:
     Every command is a single character that takes up a line
     Any command that requires arguments must have those arguments in the second line.
     The commands are as follows:

     push: push a copy of the curent top of the coordinate system stack to the stack

     pop: remove the current top of the coordinate system stack

     All the shape commands work as follows:
        1) Add the shape to a temporary matrix
        2) Multiply that matrix by the current top of the coordinate system stack
        3) Draw the shape to the screen
        4) Clear the temporary matrix

     sphere: add a sphere -
             takes 4 arguemnts (cx, cy, cz, r)
     torus: add a torus to the polygon matrix -
            takes 5 arguemnts (cx, cy, cz, r1, r2)
     box: add a rectangular prism -
          takes 6 arguemnts (x, y, z, width, height, depth)
     circle: add a circle -
             takes 4 arguments (cx, cy, cz, r)
     hermite: add a hermite curve -
              takes 8 arguments (x0, y0, x1, y1, rx0, ry0, rx1, ry1)
     bezier: add a bezier curve -
             takes 8 arguments (x0, y0, x1, y1, x2, y2, x3, y3)
     line: add a line to the edge matrix -
           takes 6 arguemnts (x0, y0, z0, x1, y1, z1)

     scale: create a scale matrix,
            then multiply the current top of the coordinate system stack -
            takes 3 arguments (sx, sy, sz)
     translate: create a translation matrix,
                then multiply the current top of the coordinate system stack -
                takes 3 arguments (tx, ty, tz)
     rotate: create a rotation matrix,
             then multiply the transform matrix by the translation matrix -
             takes 2 arguments (axis, theta) axis should be x, y or z

     display: display the screen

     save: save the screen to a file -
           takes 1 argument (file name)

    quit: end parsing

See the file script for an example of the file format

IMPORTANT MATH NOTE:
the trig functions int math.h use radian mesure, but us normal
humans use degrees, so the file will contain degrees for rotations,
be sure to conver those degrees to radians (M_PI is the constant
for PI)
====================*/
void parse_file ( char * filename,
                  struct matrix * transform,
                  struct matrix * edges,
                  struct matrix * polygons,
                  screen s) {

  FILE *f;
  char line[255];
  clear_screen(s);
  struct stack * stack = new_stack();

  color c;
  c.red = 0;
  c.green = 0;
  c.blue = 0;

  if ( strcmp(filename, "stdin") == 0 )
    f = stdin;
  else
    f = fopen(filename, "r");

  while ( fgets(line, sizeof(line), f) != NULL ) {
    line[strlen(line)-1]='\0';
    //printf(":%s:\n",line);

    double xvals[4];
    double yvals[4];
    double zvals[4];
    struct matrix *tmp;
    double r, r1;
    double theta;
    char axis;
    int type;
    int step = 100;
    int step_3d = 10;

    struct matrix * temp;

    if( strncmp(line, "push", strlen(line)) == 0 ){
      push(stack);
    }//push

    else if( strncmp(line, "pop", strlen(line)) == 0 ){
      pop(stack);
    }//pop

    else if ( strncmp(line, "box", strlen(line)) == 0 ) {
      fgets(line, sizeof(line), f);
      //printf("BOX\t%s", line);

      sscanf(line, "%lf %lf %lf %lf %lf %lf",
             xvals, yvals, zvals,
             xvals+1, yvals+1, zvals+1);

             temp = new_matrix(4,4);

      add_box(temp, xvals[0], yvals[0], zvals[0],
              xvals[1], yvals[1], zvals[1]);

              matrix_mult(stack->data[stack->top], temp);
              draw_polygons(temp, s, c);
              free_matrix(temp);
    }//end of box

    else if ( strncmp(line, "sphere", strlen(line)) == 0 ) {
      fgets(line, sizeof(line), f);
      //printf("SPHERE\t%s", line);

      sscanf(line, "%lf %lf %lf %lf",
             xvals, yvals, zvals, &r);

             temp = new_matrix(4,4);

      add_sphere( temp, xvals[0], yvals[0], zvals[0], r, step_3d);
      matrix_mult(stack->data[stack->top], temp);
      draw_polygons(temp, s, c);
      free_matrix(temp);
    }//end of sphere

    else if ( strncmp(line, "torus", strlen(line)) == 0 ) {
      fgets(line, sizeof(line), f);
      //printf("torus\t%s", line);

      sscanf(line, "%lf %lf %lf %lf %lf",
             xvals, yvals, zvals, &r, &r1);
             temp = new_matrix(4,4);
      add_torus(temp, xvals[0], yvals[0], zvals[0], r, r1, step_3d);
      matrix_mult(stack->data[stack->top], temp);
      draw_polygons(temp, s,c);
      free_matrix(temp);
    }//end of torus

    else if ( strncmp(line, "circle", strlen(line)) == 0 ) {
      fgets(line, sizeof(line), f);
      //printf("CIRCLE\t%s", line);

      sscanf(line, "%lf %lf %lf %lf",
             xvals, yvals, zvals, &r);
             temp = new_matrix(4,4);
      add_circle( edges, xvals[0], yvals[0], zvals[0], r, step);
      matrix_mult(stack->data[stack->top], temp);
      draw_lines(temp, s,c);
      free_matrix(temp);
    }//end of circle

    else if ( strncmp(line, "hermite", strlen(line)) == 0 ||
              strncmp(line, "bezier", strlen(line)) == 0 ) {
      if (strncmp(line, "hermite", strlen(line)) == 0 )
        type = HERMITE;
      else
        type = BEZIER;
      fgets(line, sizeof(line), f);
      //printf("CURVE\t%s", line);

      sscanf(line, "%lf %lf %lf %lf %lf %lf %lf %lf",
             xvals, yvals, xvals+1, yvals+1,
             xvals+2, yvals+2, xvals+3, yvals+3);
      /* printf("%lf %lf %lf %lf %lf %lf %lf %lf\n", */
      /* 	     xvals[0], yvals[0], */
      /* 	     xvals[1], yvals[1], */
      /* 	     xvals[2], yvals[2], */
      /* 	     xvals[3], yvals[3]); */

      //printf("%d\n", type);
      temp = new_matrix(4,4);
      add_curve( edges, xvals[0], yvals[0], xvals[1], yvals[1],
                 xvals[2], yvals[2], xvals[3], yvals[3], step, type);
                 matrix_mult(stack->data[stack->top], temp);
                 draw_lines(temp, s,c);
                 free_matrix(temp);
    }//end of curve

    else if ( strncmp(line, "line", strlen(line)) == 0 ) {
      fgets(line, sizeof(line), f);
      //printf("LINE\t%s", line);

      sscanf(line, "%lf %lf %lf %lf %lf %lf",
             xvals, yvals, zvals,
             xvals+1, yvals+1, zvals+1);
      /*printf("%lf %lf %lf %lf %lf %lf",
        xvals[0], yvals[0], zvals[0],
        xvals[1], yvals[1], zvals[1]) */
        temp = new_matrix(4,4);
      add_edge(edges, xvals[0], yvals[0], zvals[0],
               xvals[1], yvals[1], zvals[1]);
               matrix_mult(stack->data[stack->top], temp);
               draw_lines(temp, s, c);
               free_matrix(temp);
    }//end line

    else if ( strncmp(line, "scale", strlen(line)) == 0 ) {
      fgets(line, sizeof(line), f);
      //printf("SCALE\t%s", line);
      sscanf(line, "%lf %lf %lf",
             xvals, yvals, zvals);
      /* printf("%lf %lf %lf\n", */
      /* 	xvals[0], yvals[0], zvals[0]); */
      tmp = make_scale( xvals[0], yvals[0], zvals[0]);
      matrix_mult(stack->data[stack->top], tmp);
      copy_matrix(tmp, stack->data[stack->top]);
    }//end scale

    else if ( strncmp(line, "move", strlen(line)) == 0 ) {
      fgets(line, sizeof(line), f);
      //printf("MOVE\t%s", line);
      sscanf(line, "%lf %lf %lf",
             xvals, yvals, zvals);
      /* printf("%lf %lf %lf\n", */
      /* 	xvals[0], yvals[0], zvals[0]); */
      tmp = make_translate( xvals[0], yvals[0], zvals[0]);
      matrix_mult(stack->data[stack->top], tmp);
      copy_matrix(tmp, stack->data[stack->top]);
    }//end translate

    else if ( strncmp(line, "rotate", strlen(line)) == 0 ) {
      fgets(line, sizeof(line), f);
      //printf("Rotate\t%s", line);
      sscanf(line, "%c %lf",
             &axis, &theta);
      /* printf("%c %lf\n", */
      /* 	axis, theta); */
      theta = theta * (M_PI / 180);
      if ( axis == 'x' )
        tmp = make_rotX( theta );
      else if ( axis == 'y' )
        tmp = make_rotY( theta );
      else
        tmp = make_rotZ( theta );

      matrix_mult(stack->data[stack->top], tmp);
      copy_matrix(tmp, stack->data[stack->top]);
    }//end rotate

    else if ( strncmp(line, "clear", strlen(line)) == 0 ) {
      //printf("clear\t%s", line);
      edges->lastcol = 0;
    }//end clear

    else if ( strncmp(line, "ident", strlen(line)) == 0 ) {
      //printf("IDENT\t%s", line);
      ident(transform);
    }//end ident

    else if ( strncmp(line, "apply", strlen(line)) == 0 ) {
      //printf("APPLY\t%s", line);
      matrix_mult(transform, edges);
    }//end apply

    else if ( strncmp(line, "display", strlen(line)) == 0 ) {
      //printf("DISPLAY\t%s", line);
      display( s );
    }//end display

    else if ( strncmp(line, "save", strlen(line)) == 0 ) {
      fgets(line, sizeof(line), f);
      *strchr(line, '\n') = 0;
      //printf("SAVE\t%s\n", line);
      save_extension(s, line);
    }//end save
  }

}
