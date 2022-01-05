#include "../domain.h"

inherit "/std/room";

int location, destination, moving_time, time_to_close_door;
string open;

void set_floor(int x) {
   location = x;

   tell_room(this_object(), "The elevator starts moving.\n");

   switch (location) {
      case 0:
         open = "closed";
         remove_exit("east");
         break;
      case 1:
         open = "open";
         remove_exit("east");
         add_exit("east", DIR + "/rooms/wiz_hall.c");
		 call_other(DIR + "/rooms/wiz_hall", "el_arrives");
		 time_to_close_door = 3;
         break;
      case 2:
         open = "open";
         remove_exit("east");
         add_exit("east", DIR + "/rooms/church.c");
		 call_other(DIR + "/rooms/church", "el_arrives");
		 time_to_close_door = 3;
         break;
      case 3:
         open = "open";
         remove_exit("east");
         add_exit("east", DIR + "/rooms/attic.c");
		 call_other(DIR + "/rooms/attic", "el_arrives");
		 time_to_close_door = 3;
         break;
   }
   tell_room(this_object(), "The elevator stops and the doors open.\n");
}

int query_location() {
   return location;
}

void setup(void) {
   add_area("2.4.5");

   set_short("Elevator");

   destination = 2;
   set_floor(2);
   moving_time = 0;
   add_action("press_button", "press");
   add_action("press_button", "push");
}

string query_long() {
   string str;

   str = "You are in the elevator.  On the wall are three buttons, " +
      "numbered 1 to 3.\nCurrently the number " + destination + 
      " button is lit.  The door to the east is " + open + "\n";

   return str;
}

int press_button(string str) {
   object elev;
   int num;

   if (str == "1" || str == "2" || str == "3") {
      sscanf(str, "%d", num);
      if (location == num) {
	     write("You are already at level " + num + ".\n");
	  } else {
	     destination = num;
         write("You press button " + num + ".\n");
         this_object()->tell_room(this_player(), this_player()->query_Name() +
            " presses button " + num + ".\n");
		 if (destination > location) {
		    moving_time = destination - location;
		    say("The elevator jerks upward.\n");
		 } else if (location > destination) {
		    moving_time = location - destination;
		    say("The elevator starts moving down.\n");
		 }
		 if ((destination == 1) || (location == 1)) {
		    moving_time += 10;
		 }
		 moving_time += 1;
         EVENT_D->subscribe_event("heart_beat");

      }
   } else {
      write("Which button do you want to press? (1, 2, 3)?\n");
   }
   return 1;
}

void call_elevator(int button) {
   destination = button;
   if (destination == location) {
      return;
   }
   destination = button;
   if (destination > location) {
      moving_time = destination - location;
      say("The elevator jerks upward.\n");
   } else if (location > destination) {
      moving_time = location - destination;
      say("The elevator starts moving down.\n");
   }
   if ((destination == 1) || (location == 1)) {
      moving_time += 10;
   }
}

void event_heart_beat() {
   if (time_to_close_door >0) {
      time_to_close_door -= 1;
      if (time_to_close_door == 0) {
	     if (location == 1) {
            call_other(DIR + "/rooms/wiz_hall", "el_leavess");
		 } else if (location == 2) {
            call_other(DIR + "/rooms/church", "el_leaves");
		 } else if (location == 3) {
            call_other(DIR + "/rooms/attic", "el_leaves");
		 }
         say("The door swings shut.\n");
      }
   }
   if (moving_time <= 0) {
      return;
   }
   moving_time -= 1;
   if (moving_time > 0) {
      say("The elevator continues.\n");
      return;
   }
   EVENT_D->unsubscribe_event("heart_beat");
   set_floor(destination);
}
