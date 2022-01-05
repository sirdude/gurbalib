#include "../domain.h"

inherit "/std/room";

string east_door_open, name;
int a, amiga_present, amiga_power;

int do_reset(string str) {
   amiga_present = 0;
   amiga_power = 0;
   a = 0;
   tell_room(nil, "The lights in the room flicker!\n");
   return 1;
}

void setup(void) {
   add_area("2.4.5");

   set_short("Computer room");

   add_exit("south", DIR + "/rooms/wiz_hall.c");
   add_exit("east", DIR + "/rooms/rum2.c");
   add_action("sesame", "sesame");
   add_action("power", "power");
   add_action("test", "test");
   add_action("do_reset", "reset");
   add_action("do_hit", "hit");

   add_item("amiga", "#do_computer");
   add_item("computer", "#do_computer");

   set_objects (DIR + "/obj/jacket.c",
      DIR + "/obj/bag.c");
   a = 0;
   amiga_present = 0;
   amiga_power = 0;
}

string do_computer() {
   if (amiga_present) {
      if (amiga_power) {
         return "A powered amiga is here.";
      } else {
         return "An amiga is here, it's screen is dark.";
      }
   } else {
      return "You do not see a computer here.";
   }
}

int do_hit(string str) {
   if (amiga_present) {
      if ((str == "amiga") || (str == "computer")) {
         write("You hit the amiga.");
		 tell_room(this_player(),
		    this_player()->query_Name() + " hits the amiga HARD!");
		 return 1;
      }
   }
   write("Hit what?");
   return 1;
}

string query_long() {
   string str;

   str = "You are in an old computer room.";
   if (amiga_present) {
      if (amiga_power) {
         str += "  A powered amiga is here.\n";
      } else {
         str += "  An amiga is here.\n";
      }
   }
   return str;
}

int sesame(string str) {
   if (amiga_present) {
      write("The amiga is already here!\n");
   } else {
      write("An amiga materializes!\n");
      amiga_present = 1;
   }
   return 1;
}

int power(string str) {
   if (!amiga_present) {
      return 0;
   }
   if (amiga_power) {
      write("The screen dims.\n");
      amiga_power = 0;
      return 1;
   }
   write("The screen lights up.\n");
   amiga_power = 1;
   return 1;
}

int fac(int n) {
   if (n <= 0) {
      return 1;
   }
   return n * fac(n-1);
}

int test(string str) {
   if (amiga_power) {
      a = a + 1;
      write("Fac " + a + " is " + fac(a) + "\n");
      return 1;
   }
   return 0;
}

