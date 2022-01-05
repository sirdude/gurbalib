#define ELFILE "/domains/2.4.5/rooms/elevator.c"

object get_el() {
   object obj;
   obj = find_object(ELFILE);
   if (!obj) {
      if (obj = compile_object(ELFILE)) {
         obj->setup();
         obj->setup_mudlib();
      }
   }

   return obj;
}

void el_arrives() {
   this_object()->tell_room(nil, "Ding!  The door to the west opens.");
   this_object()->add_exit("west", ELFILE);
   this_object()->remove_item("door");
   this_object()->add_item("door", "The door is open.");
   this_object()->lamp_off();
}

void el_leaves() {
   this_object()->tell_room(nil, "The door to the west closes.");
   this_object()->remove_exit("west");
   this_object()->remove_item("door");
   this_object()->add_item("door", "The door is closed.");
}

int press_button(string str) {
   object elev;

   if (str == "button") {
      write("You press the button.\n");
      this_object()->tell_room(this_player(), this_player()->query_Name() + 
         " presses the button.\n");
      this_object()->tell_room(this_object(),
         "The lamp on the desk lights up.\n");

      this_object()->lamp_on();

      elev = get_el();
      call_other(elev, "call_elevator", this_object()->query_floor());
      return 1;
   }
   return 0;
}
