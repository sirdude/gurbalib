#include "../domain.h"
inherit "/std/room";

int door_closed;
int locked;
string key;

void setup(void) {
   add_area("2.4.5");

   set_short("The bank");
   set_long("You are in the bank.  To the east is a low counter.  The " +
      "counter is covered with heavy iron bars.  On the wall beside " +
      "the counter, a door leads further east");

   add_exit("west", DIR + "/rooms/narr_alley.c");
   add_exit("east", "#enter_door");

   add_item("counter", "There is a sign on the counter that says\n" +
      "CLOSED FOR RECONSTRUCTION.\n");

   add_item("door", "#look_door");

   set_objects (DIR + "/monsters/bankguard.c");
   door_closed = 1;
   locked = 1;
   key = "gurba bank key";
}

/* XXX Need to attach open/close,lock,unlock to a user... */

string look_door() {
   if (!door_closed) {
      return "The door is open.";
   }
   return "The door is closed.";
}

int open_door() {
   if (locked) {
      write("The door is locked.\n");
      tell_room(this_player(), this_player()->query_Name() + 
         " fiddles with the locked door.\n");
   } else if (door_closed) {
      door_closed = 0;
      write("You open the door.\n");
      tell_room(this_player(), this_player()->query_Name() + 
         " opens the door.\n");
   } else {
      write("The door is already open.\n");
      tell_room(this_player(), this_player()->query_Name() + 
         " fiddles with the open door.\n");
   }
   return 1;
}

int close_door() {
   if (door_closed) {
      write("The door is already closed.\n");
      tell_room(this_player(), this_player()->query_Name() + 
         " fiddles with the closed door.\n");
   } else {
      door_closed = 1;
      write("You close the door.\n");
      tell_room(this_player(), this_player()->query_Name() + 
         " closes door.\n");
   }
   return 1;
}

int has_key(string str) {
   object obj;

   obj = this_player()->present(str);

   if (obj) {
      return 1;
   }
   return 0;
}

int unlock_door() {
   if (!locked) {
      write("The door is already unlocked.\n");
      tell_room(this_player(), this_player()->query_Name() + 
         " fiddles with the door.\n");
   } else if (has_key(key)) {
      write("You unlock the door.\n");
      tell_room(this_player(), this_player()->query_Name() + 
         " unlocks the door.\n");
      locked = 0;
   } else {
      write("You can't find the key for that door.\n");
      tell_room(this_player(), this_player()->query_Name() + 
         " fiddles with the door.\n");
   }
}

int lock_door() {
   if (locked) {
      write("The door is already locked.\n");
      tell_room(this_player(), this_player()->query_Name() + 
         " fiddles with the door.\n");
   } else if (!door_closed) {
      write("The door is open, you can't lock it when it's open.\n");
      tell_room(this_player(), this_player()->query_Name() + 
         " fiddles with the door.\n");
   } else if (has_key(key)) {
      write("You lock the door.\n");
      tell_room(this_player(), this_player()->query_Name() + 
         " locks the door.\n");
      locked = 1;
   } else {
      write("You can't find the key for that door.\n");
      tell_room(this_player(), this_player()->query_Name() + 
         " fiddles with the door.\n");
   }
}

int enter_door() {
   object obj;
   string file;

   if (!door_closed) {

      file = DIR + "/rooms/bankroom.c";
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
      write("You enter the door.\n");
      this_player()->query_environment()->tell_room(this_player(),
        this_player()->query_Name() + " enters the door.");
      if (this_player()->move(obj)) {
         this_player()->query_environment()->tell_room(this_player(),
            this_player()->query_Name() + " enters.\n");
         this_player()->do_look(this_player()->query_environment());
      } else {
         write("Error the door seems to be broken.\n");
      }	

      return 1;
   } else {
      write("The door is closed.  Maybe you should open it first.\n");
      tell_room(this_player(), this_player()->query_Name() + 
         " walks right into the closed door, with a SMACK.\n");
   }
   return 1;
}

