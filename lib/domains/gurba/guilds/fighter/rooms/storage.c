#include "../../../domain.h"

inherit "/std/room";

void setup(void) {
   set_short("%^BLUE%^Fighters Guild Locker Rooms%^RESET%^");
   set_long("There are a bunch of lockers here.  Hey there is yours, to " +
      "the east.");

   set_exits(([
      "south" : DIR + "/guilds/fighter/rooms/main",
      "east" : "#go_locker",
   ]));

}

void go_locker(void) {
   string locker;
   object obj;

   locker = DIR + "/data/lockers/" + this_player()->query_name() + "/locker";

   if (file_exists(locker + ".o")) {
      obj = clone_object(DIR + "/guilds/fighter/rooms/locker.c");

      if (!obj) {
         write("Error: Lockers are messed up.  Please talk to an admin.");
         return;
      }

      obj->setup();
      obj->set_player_name(this_player()->query_name());
      obj->restore_me();

      if (!obj) {
         write("Error: Your locker is messed up.  Please talk to an admin.");
         return;
      }
   } else {
      SECURE_D->make_lockerdir("gurba", this_player()->query_name());
      obj = clone_object(DIR + "/guilds/fighter/rooms/locker.c");
      obj->setup();
      obj->set_player_name(this_player()->query_name());
      obj->save_me();
   }

   this_player()->query_environment()->tell_room(this_player(),
      this_player()->query_Name() + " leaves east.\n");
   if (this_player()->move(obj)) {
      obj->tell_room(this_player(), this_player()->query_Name() + " enters.\n");
	  this_player()->do_look(this_player()->query_environment());
   } else {
      write("Error going there...\n");
   }
}
