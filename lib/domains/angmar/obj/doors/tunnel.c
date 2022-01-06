#include "../../domain.h"

inherit "/std/lockable_door";

void setup(void) {
   set_id("door");
   set_adj("strong");
   set_short("A strong door");
   set_long("The door looks very strong.");
   door_setup("south", "misty-mountain-door");
   add_block("south");
   set_lock_code("auxd");
}
