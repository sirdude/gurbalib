#include "../domain.h"
inherit "/std/room";

void setup(void) {
   add_area("2.4.5");

   set_short("The bank");
   set_long("You are in the bank.  To the east is a low counter.  The " +
      "counter is covered with heavy iron bars.  On the wall beside " +
      "the counter, a door leads further east");

   add_exit("west", DIR + "/rooms/narr_alley.c");
   add_exit("east", DIR + "/rooms/bankroom.c");

   add_item("counter", "There is a sign on the counter that says\n" +
      "CLOSED FOR RECONSTRUCTION.\n");

   set_objects (DIR + "/monsters/bankguard.c",
      DIR + "/obj/bankdoor2.c");
}

