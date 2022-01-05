#include "../domain.h"

inherit "/std/monster";
inherit "/std/modules/m_triggers";

#define INTERVAL 8
static int count;
string *a_str, *na_str;

void setup() {
   set_name("harry");
   add_id("fjant");
   set_gender("male");
   add_adj("affectionate");
   set_short("Harry the affectionate");
   set_long("Harry has an agreeable look.");
   set_race("human");
   set_level(3);
   set_hit_skill("combat/unarmed");
   set_skill("combat/unarmed", 50);
   set_skill("combat/defense", 100);
   count = 0;

   add_pattern("%s picks up %s.", "say Why did $1 take $2?");
   add_pattern("%s drops %s.", "say Why did $1 drop $2?");

   /* These need to be checked */
   add_pattern("shut up", "say WHy do you want me to shut up?");
   add_pattern("Shut up", "say WHy do you want me to shut up?");
   add_pattern("%s sells", "say WHy did you sell $1?");
   add_pattern("%s attack", "say Why do you attack that $1?");
   add_pattern("%s left the game", "say Why did $1 quit the game?");
   add_pattern("is now level", "say How does it feel to now be level $2?");
   add_pattern("smiles", "smiles");
   add_pattern("happily", "smiles");
   add_pattern("%s arrives", "say Hello $1, nice to see you!");
   add_pattern("%s enters", "say Hello $1, nice to see you!");
   add_pattern("%s appears", "say Hello $1, nice to see you!");
   add_pattern("%s says:", "test_say");
   add_pattern("%s tells you:", "test_say");
   add_pattern("%s leaves %s", "go $2");
   add_pattern("%s gives you %s", "gives");
   add_pattern("%s gives you %s", "gives");
   add_pattern("%s falls to the ground...dead", "monster_died");

   a_str = ({
      "say Don't hit me!",
      "say That hurt!",
      "say Help, someone!",
      "say Why can't you go bullying elsewhere?",
      "say Aooooo!",
      "say I hate bashers!\n",
      "say Bastard\n",
      "say You big brute!\n"            
   });

   na_str = ({
      "say What are you waiting for?",
      "say Hello there!",
      "say I don't like winter.",
      "say I don't like snow.",
      "say I don't like rain.",
      "say Who are you?",
      "say Why do you look like that?",
      "say What are you doing here?",
      "say Nice weather, isn't it?",
      "smile"
   });
}

void notify(string str) {
   query_environment()->tell_room(this_object(), str);
}

private int check_verb(string str, string verb) {
   return sscanf(str, verb + "%*s") == 1;
}

private int is_harry(string str) {
   return lowercase(str) == "harry";
}

private int contains(string needle, string hay) {
   return sscanf(hay, "%*s" + needle + "%*s") == 2;
}

void handle_say(string str) {
   string a, b, message;

   sscanf(str, "%s %s", a, b);
   if (is_harry(a)) {
      return;
   }
   if (sscanf(str, "%s says: %s\n", a, b) != 2) {
      return;
   }

   str = lowercase(b);

   if (contains("hello", str) || contains("hi", str) ||
      contains("hello everybody", str)) {
      message = "say Pleased to meet you!";
   }
   if (contains("shut up", str)) {
      message = "say Why do you want me to shut up ?";
   }
   if (contains("stay here", str) || contains("not follow", str) ||
      contains("get lost", str)) {
      message = "say Ok then.";
   }
   if (!message) {
      message = "say Why do you say '" + str + "'???";
   }
   respond(message);
}

private void follow(string str) {
   string who, where;

   if (sscanf(str, "%s leaves %s.\n", who, where) == 2) {
      if (!is_harry(who)) {
         respond("go " + where);
      }
   }
}

private void gives(string str) {
   string who, what;
   int rand;
   object obj, next_obj, who_obj;

   if (sscanf(str, "%s gives the %s to you.", who, what) != 2) {
      return;
   }

   if ((what == "firebreather" || what == "special" ||
         what == "beer" || what == "bottle") && this_object()->present(what)) {
      rand = random(4);

      if (rand == 0) {
         if (random(10) > 6) {
            respond("sigh");
            respond("say I guess you're gonna kill me now.");
            respond("drop all");
            respond("go west");
         }
      }
      if (rand == 1) {
         respond("drink " + what);
      }
      if (rand == 2) {
         respond("drop " + what);
      }
      if (rand == 3) {
         obj = this_object()->present(what);
         if (obj) {
            respond("give " + what + " to " + who);
         }
      }
   } else if (what == "corpse") {
      respond("say HEY, bury your corpses yourself, asshole.");
      obj = this_object()->present(what);
      if (obj) {
         respond("give corpse to " + who);
      }
   } else {
      if (this_object()->present(what)) {
         who_obj = this_object()->query_environment()->present(who);

		 if (who_obj->query_gender() == "male") {
            respond("say Thank you very much, sir.");
		 } else if (who_obj->query_gender() == "female") {
            respond("say Thank you very much, ma'am.");
		 } else {
            respond("say Thank you very much.");
		 }
      }
   }
}

void monster_died() {
   object *objs;
   int num, i, max;

   objs = this_object()->query_envrionment()->query_inventory(); 
   max = sizeof(objs);
   for (i = 0; i < max; i++) {
      if (call_other(objs[i], "id") == "bottle") {
	      destruct_object(objs[i]);
		 num = 1;
      }
   }
   if (num) {
      notify("There is a crushing sound of bottles breaking, as the body falls.\n");
   }
}

void do_extra_actions() {
   count++;

   if (count >= INTERVAL) {
      if (is_fighting()) {
         respond(a_str[random(sizeof(a_str))]);
      } else {
         respond(na_str[random(sizeof(na_str))]);
      }
      count = 0;
   }
}
