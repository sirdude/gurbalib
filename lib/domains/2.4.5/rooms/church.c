#include "../domain.h"

inherit "/std/room";
inherit "/domains/2.4.5/lib/el";

object el;
int lamp_is_lit;

int query_floor() {
   return 2;
}

void setup(void) {
   add_area("2.4.5");

   set_short("Village church");

   add_exit("south", DIR + "/rooms/vill_green.c");

   add_action("press_button", "press");
   add_action("press_button", "push");

   add_item("clock", "The clock appears to be broken, it is stuck at: 1:26");
   add_item("pit", "In the middle of the church is a deep pit.  IT was used " +
      "for sacrifice in the old times, but nowadays it is only " +
      "left for tourists to look at.");
   add_item("lamp", "#look_lamp");

   el = get_el();
   if (el) {
      if (el->query_location() == query_floor()) {
         el_arrives();
      } else {
         el_leaves();
      }
   }
}

void lamp_on() {
   lamp_is_lit = 1;
}

void lamp_off() {
   lamp_is_lit = 0;
}

string look_lamp() {
   if (lamp_is_lit == 1) {
      return "The lamp beside the elevator is lit.\n";
   }
   return "There is a lamp beside the elevator which is currently unlit.\n";
}

string query_long() {
   string str;

   str = "You are in the local village church.  There is a huge " +
      "pit in the center, and a door in the west wall.  " +
      "There is a button beside the door.  This church has the service " +
      "of reviving ghosts.  Dead people come to the church and pray.  " +
      "there is a clock on the wall.  There is an exit to the south.  ";

   str += look_lamp();

   return str;
}
