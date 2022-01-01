#include "../../../domain.h"

inherit "/std/room";

string player_name;
string *myitems;

void restore_me(void) {
   object obj;
   int i, max;

   if (!unguarded("restore_object", DIR + "/data/lockers/" +
      player_name + "/locker.o")) {
      write("Error while restoring " + DIR + "/data/lockers/" +
         player_name + "/locker.o");
      write("Please notify the administration.");
      return;
   }
   max = sizeof(myitems);
   for (i = 0; i < max; i++) {
      obj = clone_object(myitems[i]);
      unguarded("restore_object", DIR + "/data/lockers/" + player_name + 
         "/" + i + ".o");
   }
   set_short(capitalize(player_name) + "'s locker room");
}

void clean_up(void) {
   int x, y, max;
   object *objs;
   myitems = ({});

   objs = this_object()->query_inventory();
   if (objs) {
      max = sizeof(objs);
   } else {
      max = 0;
   }
   max = max - 1;

   for (x = max; x >= 0; x--) {
      if (objs[x]->is_living()) {
         objs[x]->move(VOID);
      } else {
         objs[x]->destruct();
      }
      destruct();
   }
}

void save_me(void) {
   int x, y, max;
   object *objs;
   myitems = ({});

   unguarded("save_object", DIR + "/data/lockers/" + player_name + "/locker.o");

   objs = this_object()->query_inventory();
   if (objs) {
      max = sizeof(objs);
   } else {
      max = 0;
   }
   y = 0;
   for (x = 0; x < max; x++) {
      if (objs[x]->is_living()) {
      } else {
         myitems += ({ objs[x]->base_name() });
         unguarded("save_object", DIR + "/data/lockers/" + player_name + 
            "/" + y + ".o");
         y = y + 1;
      }
   }

   unguarded("save_object", DIR + "/data/lockers/" + player_name + "/locker.o");
}

void set_player_name(string str) {
   player_name = str;
   set_short(capitalize(player_name) + "'s locker room");
}

void setup(void) {
   set_short(" An unknown locker room");
   set_long("You can store your stuff here.");

   add_action("do_save", "save");

   set_exits(([
      "west" : DIR + "/guilds/fighter/rooms/storage.c",
   ]));
}

/* On exit need to save_me and destroy this object... XXX */
int do_save(string str) {
   object obj;
   string file;

   file = DIR + "/guilds/fighter/rooms/storage.c";

   if (!(obj = find_object(file))) {
      catch {
         obj = compile_object(file);
         obj->setup();
         obj->setup_mudlib();
      } : {
         write("Could not load " + str);
      }
   }

   if (this_player()->move(obj)) {
      /* XXX Need to move this stuff to other move's like summon/goto */
      this_object()->event("body_leave", this_player());
      tell_room(this_player(), this_player()->query_Name() + " leaves east.\n");
      obj->tell_room(this_player(), this_player()->query_Name() + " enters.\n");
      this_player()->do_look(this_player()->query_environment());
   } else {
      write("Error going there...\n");
   }

   save_me();
   clean_up();
}
