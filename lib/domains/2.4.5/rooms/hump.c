#include "../domain.h"

inherit "/std/room";

void setup(void) {
   object ob;
   string name;
   add_area("2.4.5");

   set_short("Humpbacked bridge");
   set_long("An old humpbacked bridge.");
   add_item("bridge", "It's old and in need of repair.");

   add_exit("east", DIR + "/rooms/vill_green.c");
   add_exit("west", DIR + "/rooms/wild1.c");

   set_objects (DIR + "/obj/torch.c");

   name = "/domains/required/objects/coin.c";
   if (!present(name)) {
      ob = clone_object(name);
      ob->set_amount(10);
      ob->move(object_name(this_object()));
      ob->setup();
   }
}
