#include "../domain.h"

inherit "/std/room";
inherit "/domains/2.4.5/lib/el";

object el;
int lamp_is_lit;

int query_floor() {
   return 1;
}

void setup(void) {
   add_area("2.4.5");

   set_short("Wizards hall");

   add_action("press_button", "press");
   add_action("press_button", "push");
   add_exit("north", DIR + "/rooms/test.c");

   set_objects (DIR + "/monsters/leo.c");

   el = get_el();
   if (el) {
      if (el->query_location() == query_floor()) {
         tell_room(this_object(), "The lamp goes out.\n");
         el_arrives();
      } else {
         el_leaves();
      }
   }
}

string query_long() {
   string str;

   str = "You are in the hall of wizards.  There is a door to " +
      "the west, with a button next to it, and a shimmering field to " +
      "the north.\n";

   if (lamp_is_lit) {
     str += "There is a lit lamp beside the elevator.\n";
   } else {
      str += "There is a lamp beside the elevator.\n";
   }

   return str;
}

void lamp_on() {
   lamp_is_lit = 1;
}

void lamp_off() {
   lamp_is_lit = 0;
}

