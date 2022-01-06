#include "../domain.h"

inherit "/std/monster";
inherit "/std/modules/m_triggers";

#define INTERVAL 8
int count;

void setup() {
   set_name("leo");
   set_gender( "male" );
   set_short("Leo the Archwizard");
   set_long("Leo the Archwizard.");
   set_race("human");
   set_level(40);
   set_hit_skill("combat/unarmed");
   set_skill("combat/unarmed", 90);
   set_skill("combat/defense", 200);

   set_spell_chance(50);
   set_spell_damage(30);
   set_spell_message("Leo casts a blazing fireball at $t.\n");
}

void do_extra_actions() {
   count = count + 1;
   if (count >= INTERVAL) {
      respond("smile");
      count = 0;
   }
}

void give_castle(string who) {
   object player;

   player = this_environment()->present(who);
   if (player) {
      if (query_wizard(player)) {
	     /* originally the mudlib had castles, were not doing that though so here we are...
		    If you are interested in that please look at the original mudlib */
         this_environment()->tell_room(this_object(), "Leo says: I seem to " +
            "have missplaced my portable castles, please come back later.");
      } else {
         this_environment()->tell_room(this_object(), "Leo says: sorry " +
            player->query_Name() + " you need to be a wizard to get a castle.");
      }
   }
}

void outside_message(string str) {
   string who, what, where;
   object sword, player;

   str = ANSI_D->strip_colors(str);
   str = lowercase(str);

   if (sscanf(str, "%s gives %s to you.", who, what) == 2) {
       player = this_environment()->present(who);
       sword = this_object()->present("orc slayer");

       if (sword) {
          destruct_object(sword);
          if (!player->is_completed_quest("Orc Slayer")) {
             player->add_completed_quest("Orc Slayer");
             player->increase_expr(200);
             player->message("Congratulations!!! You have just completed the " +
                "Orc Slayer quest.\n");
          }
       }
   } else if (sscanf(str, "%s says: %s", who, what) == 2) {
      if (strstr(what[0..3], "hello") == 0 || 
          strstr(what[0..1], "hi") == 0) {
          player = this_environment()->present(who);

          this_object()->respond("say Welcome " + who + ".");
          give_castle(who);
      }
   } else if (sscanf(str, "%s says:%scastle%s", who, what, where) == 3) {
      give_castle(who);
   }
}
