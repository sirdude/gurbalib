#include "../domain.h"

inherit "/std/object";

void setup(void) {
   set_id("rubble");
   set_short("A pile of rubble");
   set_long("A large pile of rubble strewn about the room.  It looks " +
      "like it will take quite a lot of time to clean this mess up.");
   add_action("do_search", "search");
}

int do_search(string str) {
   object obj;

   if (str != "rubble") {
      return 0;
   }

   if (this_player()->present("rusty key")) ||
      this_player()->is_completed_quest("AbandonedMine")) {
      write("You find nothing.\n");
      this_player()->query_environment()->tell_room(this_player(),
         this_player()->query_Name() +
         " digs through the rubble.\n");
	   obj = clone_object(DIR + "/obj/ironkey.c");
	   obj->move(this_player());
   }
   return 1;
}
