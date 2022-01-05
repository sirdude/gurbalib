#include "../domain.h"

inherit "/std/room";

void setup(void) {
   add_area("2.4.5");

   set_short("Room 2");
   set_long("This is room 2.  There is an exit to the west.");

   add_exit("west", DIR + "/rooms/test.c");
}

