#include <type.h>

inherit cont "/std/container";

static mapping exits, hidden_exits, areas, items;
static int last_exit, weather, light, pk, nokill;

string dark_msg;

void setup(void);

int is_room(void) {
   return 1;
}

void set_light(int flag) {
   light = flag;
}

private int object_is_lighted(object obj) {
   object *objs;
   int i, dim, l;

   if (obj->is_living()) {
      objs = obj->query_inventory();
	  dim = sizeof(objs);
      for (i = 0; i < dim; i++) {
         if (objs[i]->query_lit()) {
            return 1;
         }
      }
   }

   if (obj->query_lit()) {
      return 1;
   }

   return 0;
}

int is_dark(void) {
   object* objs;
   int x;

   if (light) {
      return 0;
   }

   objs = query_inventory();
   for (x = sizeof(objs) -1; x >= 0; x--) {
      if (object_is_lighted(objs[x])) {
         return 0;
      }
   }

   return 1;
}

string query_dark_msg(void) {
   if (!dark_msg) {
      return "It is too dark to see here.\n";
   }
   return dark_msg;
}

void set_dark_msg(string str) {
   dark_msg = str;
}

void set_weather(int flag) {
   weather = flag;
}

int query_weather(void) {
   return weather;
}

void set_pk(int flag) {
   pk = flag;
}

int query_pk(void) {
  if (!pk) {
    return 0;
  }
  return pk;
}

void set_nokill(int flag) {
   nokill = flag;
}

int query_nokill(void) {
  if (!nokill) {
    return 0;
  }
  return nokill;
}

void add_area(string str) {
   areas += ([str:1]);
}

int query_in_area(string str) {
   if (areas[str]) {
      return 1;
   } else {
      return 0;
   }
}

string *query_areas(void) {
   return map_indices(areas);
}

void set_exits(mapping ex) {
   exits = ex;
}

void set_hidden_exits(mapping ex) {
   hidden_exits = ex;
}

void add_exit(string direction, string file) {
   exits += ([direction:file]);
}

void remove_exit(string direction) {
   exits[direction] = nil;
}

void add_hidden_exit(string direction, string file) {
   hidden_exits += ([direction:file]);
}

void remove_hidden_exit(string direction) {
   hidden_exits[direction] = nil;
}

mapping query_exits(void) {
   return exits;
}

mapping query_hidden_exits(void) {
   return hidden_exits;
}

string *query_exit_indices(void) {
   return map_indices(exits);
}

string *query_hidden_exit_indices(void) {
   return map_indices(hidden_exits);
}

string query_exit(string direction) {
   return exits[direction];
}

string query_hidden_exit(string direction) {
   return hidden_exits[direction];
}

/* Attempts to find the actual room object attached to a given exit.
  Will compile it if necessary.
  Cerihan - 3/13/09
*/
object query_exit_room(string direction) {
   string exit;
   object qroom;

   exit = query_exit(direction);
   if (!exit) {
      return nil;
   }

   qroom = find_object(exit);
   if (!qroom) {
      qroom = compile_object(exit);
   }

   return qroom;
}

string query_desc(varargs int brief) {
   string text;
   int count, size;
   object *inventory;
   mapping compacted_inv;

   if (!brief) {
      brief = 0;
   }

   text = "";

   if (is_dark()) {
      if (this_player()->query_race_object()->has_darkvision()) {
      } else if (query_wizard(this_player())) {
      } else {
         return query_dark_msg();
      }
   }

   text += "%^ROOM_NAME%^\t" + query_short() + "%^RESET%^\n";
   text += "%^ROOM_DESC%^" + query_long() + "%^RESET%^\n";
   text += "%^ROOM_EXIT%^[ Obvious exits: ";

   if (!exits || map_sizeof(exits) == 0) {
      text += "none ";
   } else {
      for (count = 0; count < map_sizeof(exits); count++) {
         text += map_indices(exits)[count] + " ";
      }
   }
   text += "]%^RESET%^\n";

   inventory = query_inventory();

   /* There is something in the room */
   if (inventory && sizeof(inventory) > 1) {
      string desc;

      compacted_inv = ([]);

      text += "\nYou see:\n";

      for (count = 0; count < sizeof(inventory); count++) {
         if (!inventory[count]->is_living()) {
            desc = "  %^OBJ_BRIEF%^" + capitalize(
               inventory[count]->query_short()) + "%^RESET%^";
         } else {
            mixed x;
            string pc, rest;

            if (inventory[count] == this_player()) {
               continue;
            }
            x = inventory[count]->query_idle();

            if (inventory[count]->is_player()) {
               pc = "%^PLAYER%^";
            } else {
               pc = "%^NPC_FRIENDLY%^";
            }

            if (inventory[count]->is_resting()) {
               rest = "(Resting)";
            } else {
               rest = "";
            }

            if (x && x > 60) {
               desc = "  " + pc + capitalize(inventory[count]->query_short()) +
                  " [idle" + fmt_time(inventory[count]->query_idle()) +
                  "]  " + rest + "%^RESET%^";
            } else {
               desc = "  " + pc + capitalize(inventory[count]->query_short()) +
                  rest + "%^RESET%^";
            }
         }
#ifdef LONG_ROOM_INV
         text += desc + "\n";
      }
#else
         if (!compacted_inv[desc]) {
            compacted_inv[desc] = ({ });
         }
         compacted_inv[desc] += ({ inventory[count] });
      }

      if (compacted_inv && map_sizeof(compacted_inv)) {
         string *keys;
         int amount;

         keys = map_indices(compacted_inv);

         for (count = 0, size = sizeof(keys); count < size; count++) {
            amount = sizeof(compacted_inv[keys[count]]);

            if (amount == 1) {
               text += keys[count];
            } else {
               text += keys[count] + " [x" + amount + "]";
            }
            text += "\n";
         }
      }
#endif
   }
   return text;
}

void set_items(string id, varargs mixed args ...) {
   string description;
   int i;

   if (!items) {
      items = ([]);
   }

   description = args[sizeof(args) - 1];
   items[id] = description;

   if (sizeof(args) > 1) {
      for (i = 0; i < sizeof(args) - 1; i++) {
         items[args[i]] = description;
      }
   }
}

void add_item(string id, varargs mixed args ...) {
   string description;
   int i;

   if (!items) {
      items = ([]);
   }

   description = args[sizeof(args) - 1];
   items[id] = description;

   if (sizeof(args) > 1) {
      for (i = 0; i < sizeof(args) - 1; i++) {
         items[args[i]] = description;
      }
   }
}

void remove_item(string id) {
   mapping temp_desc;
   int size, i;
   string description, *indices, *values;

   if (!items) {
      items = ([]);
   }

   temp_desc = ([]);
   description = items[id];
   size = map_sizeof(items);
   values = map_values(items);
   indices = map_indices(items);

   for (i = 0; i < size; i++) {
      if (description != values[i]) {
         temp_desc[indices[i]] = values[i];
      }
   }
   items = ([]);
   items = temp_desc;
}

string *query_item_ids(void) {
   if (!items) {
      items = ([]);
   }
   return map_indices(items);
}

string *query_items(void) {
   if (!items) {
      items = ([]);
   }
   return map_indices(items);
}

string query_item(string item) {
   if (!items) {
      items = ([]);
   }
   if (items[item][0..0] == "#") {
      return call_other(this_object(), items[item][1..]);
   }

   return items[item];
}

/*-------------------------------------------------------------------
  sends a message to the room, designed specificly for non-living
  objects to communicate with the room
*/
void message_room(object originator, string str) {
   int i, sz;
   object *inventory;

   inventory = query_inventory();

   if (!inventory && sizeof(inventory) > 0) {
      return;
   }
   for (i = 0, sz = sizeof(inventory); i < sz; i++) {
      if (!inventory[i]) {
    continue;
      }
      if (originator != inventory[i]) {
    if (inventory[i]->is_living()) {
       inventory[i]->message(str);
    }
    inventory[i]->outside_message(str);
      }
   }
}

void tell_room(object originator, string str, varargs mixed obj ...) {
   int i;
   object *inventory;

   inventory = query_inventory();

   if (!inventory && sizeof(inventory) > 0) {
      return;
   }
   for (i = 0; i < sizeof(inventory); i++) {
      if (!inventory[i]) {
         continue;
      }
      if (originator != inventory[i] && member_array(inventory[i], obj) == -1) {
         if (inventory[i]->is_living() &&
            (!originator ||
            !inventory[i]->query_ignored(originator->query_name()))) {

            if (previous_object()->base_name() == "/cmds/player/say") {
               inventory[i]->message(capitalize(str), 1);
            } else {
               inventory[i]->message(capitalize(str));
            }
         }
         inventory[i]->outside_message(capitalize(str));
      }
   }
}

int check_block_object(object obj, string dir, object who) {
   if (obj->is_blocking(dir) && obj->do_block(who)) {
      obj->other_action(obj, obj->query_block_action(), who, dir);
      return 1;
   }
   return 0;
}

string body_exit(object who, string dir) {
   int i, max;
   string error, lname, aname;
   object room, *inventory;

   if (!query_exit(dir) && !query_hidden_exit(dir)) {
      write("You can't go " + dir + ".\n");
      return nil;
   }

   lname = who->query_Name();
   aname = lname;
   if (!lname) {
      /* Generic-named NPCs */
      lname = "The " + who->query_id();
      aname = "A " + who->query_id();
   }

   inventory = query_inventory();

   if (query_exit(dir) || query_hidden_exit(dir)) {
      if (check_block_object(this_object(), dir, who)) {
         return nil;
      }
   }

   /* there is a normal exit */
   if (query_exit(dir)) {
      max = sizeof(inventory);

      for (i = 0; i < max; i++) {
         if (check_block_object(inventory[i], dir, who)) {
             return nil;
         }
      }

      if (query_exit(dir)[0] == '#') {
         call_other(this_object(), query_exit(dir)[1..]);
         return nil;
      }

      event("body_leave", who);
      tell_room(who, lname + " leaves " + dir + ".\n");
      error = catch(who->move(query_exit(dir)));
      if (who->is_player()) {
    last_exit = time();
      }
   } else if (query_hidden_exit(dir)) {
      max = sizeof(inventory);
      for (i = 0; i < max; i++) {
         if (check_block_object(inventory[i], dir, who)) {
             return nil;
         }
      }

      if (query_hidden_exit(dir)[0] == '#') {
         call_other(this_object(), query_exit(dir)[1..]);
         return nil;
      }

      event("body_leave", who);
      tell_room(who, lname + " leaves " + dir + ".\n");
      error = catch(who->move(query_hidden_exit(dir)));
      if (who->is_player()) {
         last_exit = time();
      }
   }

   if (error) {
      if (query_wizard(who) == 1) {
         return "\nConstruction blocks your path.\n" + "Error: " + error;
      } else {
         return "\nConstruction blocks your path.\n";
      }
   } else {
      string *followers;
      int x, max;
      object obj;

      followers = this_player()->query_followers();
      max = sizeof(followers);
      for (x=0; x< max; x++) {
         obj = USER_D->find_player(followers[x]);

         if (obj) {
            obj->query_environment()->body_exit(obj, dir);
            if (obj->query_environment() !=
               this_player()->query_environemnt()) {
               write("You lost " + obj->query_Name() +
                  " they stop following you.\n");
               obj->message("You lost " + this_player()->query_Name() +
                  " you stop following them.\n");
               this_object()->query_environment()->tell_room(obj,
                  obj->query_Name() + " looses " +
                  this_player()->query_Name() + "they stop following them.\n",
                  this_player());
  
               obj->query_environment()->tell_room(obj,
                  obj->query_Name() + " looses " +
                  this_player()->query_Name() + "they stop following them.\n",
                  this_player());
            }
         }
      }
   }

   if (who->is_player() || who->is_possessed()) {
      call_other(this_player(), "do_look", this_environment());
   }

   room = who->query_environment();
   room->event("body_enter", who);
   room->tell_room(who, aname + " enters.\n");
   return nil;
}

void destruct(void) {
   move_or_destruct_inventory();

   EVENT_D->unsubscribe_event("clean_up");
   ::destruct();
}

void event_clean_up(void) {
   int i, max;
   object *inventory;

   if (time() - last_exit < 60 * 15) {
      return;
   }

   inventory = query_inventory();
   max = sizeof(inventory);
   for (i = 0; i < max; i++) {
      if (!inventory[i]) {
         continue;
      }
      if (inventory[i]->is_player() || inventory[i]->is_possessed() ||
         inventory[i]->no_cleanup()) {
         return;
      }
   }

   destruct();
}

void upgraded(void) {
   ::upgraded();
   /*
    * special for rooms, ::upgraded() will only call
    * setup in clones, while blueprints of rooms are often used
    * and will also require setup.
    */

   if (clone_num() == 0) {
      setup();
   }
}

void create(void) {
   cont::create();
   add_event("body_enter");
   add_event("body_leave");
   add_event("body_look");
   add_event("body_look_at");
   add_event("room_message");
   if (!WORLD_PERSIST) {
     EVENT_D->subscribe_event("clean_up");
   }
   exits = ([]);
   hidden_exits = ([]);
   areas = ([]);
   items = ([]);
   last_exit = 0;
   light = 1;
   setup();
}
