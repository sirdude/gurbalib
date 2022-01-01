#include "../../../../domain.h"

string teleport_destination;

nomask int set_destination_cmd(string str) {
   object target;

   if (empty_str(str) || lowercase(str) != "situ") {
      write("Syntax: memorator situ\n");
      return 1;
   }
   target = this_player()->query_environment();
   if (!target<-"/std/room") {
      write("You are not in a valid destination.\n");
      return 1;
   }
   if (!target->ok_to_teleport()) {
      write("This environment does not allow teleportation into.");
      return 1;
   }
   teleport_destination = target->file_name();
   write("You set the wizard's hat teleporation destination to this room.");
   return 1;
}

static nomask object get_teleport_destination(void) {
   object obj;

   if (teleport_destination) {
      obj = find_object(teleport_destination);
      if (!obj) {
         obj = compile_object(teleport_destination);
      }
   }
   return obj;
}

nomask int teleport_cmd(string str) {
   object obj;
   int enough_mana;

   if (empty_str(str) || lowercase(str) != "jump") {
      write("Syntax: hoppla jump");
      return 1;
   }
   TELL_ROOM(this_player(),
      this_player()->query_Name() + " says: hoppla jump!");
   enough_mana = this_player()->query_mana() > WIZHAT_COST;
   if (!enough_mana) {
      write("Your wizard's hat fizzles as you " +
         "don't have enough mana for that action.");
      return 1;
   }
   obj = get_teleport_destination();
   if (obj->file_name() == this_player()->query_environment()->file_name()) {
      write("You are already at the teleport destination. Nothing happens.");
      return 1;
   }
   if (!nilp(obj)) {
      TELL_ROOM(this_player(), this_player()->query_Name() + " disappears " +
         "in a puff of smoke.");
      write("*poof*");
      this_player()->set_mana(this_player()->query_mana() - WIZHAT_COST);
      this_player()->move(obj);
      this_player()->do_look(this_player()->query_environment());
      TELL_ROOM(this_player(), this_player()->query_Name() + " appears in a " +
         "puff of smoke.");
      return 1;
   }
   write("Teleport destination has not yet been set.");
   return 1;
}
