#include "../../domain.h"

inherit "/std/room";

void setup(void) {
   add_area("2.4.5");

   set_short("Link to the mainland");
   set_long("You are standing on the shore of the isle of the Magi.  " +
      "The shore of the island continues east and southwest from here " +
      "To the south, a hill rises up to the ancient ruins of the Tower " +
      "of Arcanarton, the archmage who used to live on this island.  " +
      "A magical bridge now stands on the ruins of the old stone bridge to " +
      "the northwest.");

   add_exit("south", DIR + "/rooms/south/sislnd13.c");
   add_exit("east", DIR + "/rooms/south/sislnd2.c");
   add_exit("northwest", "#go_northwest");
   add_exit("northeast", DIR + "/rooms/south/sislnd12.c");
}

int go_northwest(void) {
   object obj;
   string file;

   file = DIR + "/rooms/south/sshore26.c";
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
   write("Trusting in your faith, you step onto the magical bridge " +
      "and move across to the Isle of the Magi.\n");
   this_player()->query_environment()->tell_room(this_player(),
     this_player()->query_Name() + " walks across a faintly glowing bridge.");
   if (this_player()->move(obj)) {
      this_player()->query_environment()->tell_room(this_player(),
         this_player()->query_Name() + " enters.\n");
      this_player()->do_look(this_player()->query_environment());
   } else {
      write("Error going across the bridge.\n");
   }	
   return 1;
}
