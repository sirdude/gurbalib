#include "../../domain.h"

inherit "/std/room";

void setup(void) {
   add_area("mine");

   set_short("Big rock");
   set_long("There is a giant rock in the center of the room.  Instead of " +
      "digging through it they just carved out enough room to wiggle " +
      "around the thing.");

   set_exits(([
      "south" : DIR + "/rooms/mine/passages.c",
      "east" : DIR + "/rooms/mine/mineshaft.c",
      "west" : DIR + "/rooms/mine/cavein.c",
   ]));

   set_objects(DIR + "/obj/bigrock.c",
      DIR + "/obj/irondoor2.c");
}

