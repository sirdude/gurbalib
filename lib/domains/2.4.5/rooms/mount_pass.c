#include "../domain.h"

inherit "/std/room";

void setup(void) {
   add_area("2.4.5");

   set_short("Mountain pass");
   set_long("You are in a pass going into the mountain with a steep " +
     "slope upwards to the north.  However, the path is barred.  There " +
     "is a tunnel entrance to the north.  It might be possible to climb " +
     "up, though.");

   add_exit("north", DIR + "/rooms/mine/tunnel.c");
   add_exit("south", DIR + "/rooms/plain11.c");
   add_exit("up", "#do_climb"); 
   add_action("do_climb", "climb");
}

int do_climb(string str) {
   object obj;
   string file;

   file = DIR + "/rooms/ravine.c";
   if (!(obj = find_object(file))) {
      catch {
         obj = compile_object(file);
         obj->setup();
         obj->setup_mudlib();
      } : {
         write("Could not load :" + file);
         return 1;
      }
   }
   write("You climb up the hill.\n");
   this_player()->query_environment()->tell_room(this_player(),
     this_player()->query_Name() + " climbs up the ravine.");
   if (this_player()->move(obj)) {
      this_player()->query_environment()->tell_room(this_player(),
         this_player()->query_Name() + " enters.\n");
      this_player()->do_look(this_player()->query_environment());
   } else {
      write("Error climbing up the hill.\n");
   }	
   return 1;
}
