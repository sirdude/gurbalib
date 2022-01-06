#include "../domain.h"

inherit "/std/room";

void setup(void) {
   add_area("2.4.5");

   set_short("Back room of the bank");
   set_long("You are in the back room of the bank.");
   add_exit("west", DIR + "/rooms/bank.c");

   set_objects(DIR + "/obj/bankdoor.c");

}

