#include "../domain.h"

inherit "/std/monster";
inherit "/std/modules/m_triggers";

int count;
int buying_beer;
#define INTERVAL 5

void setup() {
   buying_beer = 0;
   set_name("beggar");
   set_gender( "male" );
   set_short("A beggar");
   set_long("A really filthy looking poor beggar.");
   set_race("human");
   set_level(3);
   set_hit_skill("combat/unarmed");
   set_skill("combat/unarmed", 50);
   set_skill("combat/defense", 100);
}

void do_extra_actions() {
   count = count + 1;

   if (buying_beer == 0 && count >= INTERVAL) {
      switch(random(3)) {
         case 0:
           respond("say Please, give money to a poor beggar!");
           break;
         case 1:
           respond("say Why can't I find any money?");
           break;
         default:
           respond("say Two coins please.");
           break;
      }
      count = 0;
   }

/*
 * respond() uses a callout so we have to string them together
 * with call_out() that is greater than the delay of respond().
 * I think.. :)
 */
   if (buying_beer == 0 && this_object()->query_total_money() > 12) {
      buying_beer = 1;
      call_out("go_east", 5);
   }
}

static void go_east() {
   respond("go east");
   call_out("buy_beer", 5);
}

static void buy_beer() {
   respond("buy beer");
   call_out("drink_beer", 5);
}

static void drink_beer() {
   respond("drink beer");
   call_out("go_west", 5);
}

static void go_west() {
   respond("go west");
   buying_beer = 0;
}

void outside_message(string str) {
   int x;
   str = ANSI_D->strip_colors(str);
   x = strlen(str) -3;
   
   str = str[..x];

   if (is_fighting() && random(100) < 20) {
      respond("say Why do you do this to me?");
   }
}
