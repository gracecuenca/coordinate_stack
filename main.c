#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "matrix.h"
#include "parser.h"

int main(int argc, char **argv) {

  screen s;
  struct matrix * edges;
  struct matrix * polygons;
  struct matrix * transform;

  edges = new_matrix(4, 4);
  transform = new_matrix(4, 4);
  polygons = new_matrix(4, 4);

  if ( argc == 2 )
    parse_file( argv[1], transform, edges, polygons, s );
  else
    parse_file( "stdin", transform, edges, polygons, s );

  free_matrix( edges );
  free_matrix( transform );
  free_matrix( polygons );

}
