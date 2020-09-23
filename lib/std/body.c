inherit ob "/std/object";
inherit "/std/body/experience";
inherit "/std/body/quests";
inherit "/std/body/size";
inherit "/std/body/money";
inherit "/std/body/stats";
inherit hp "/std/body/hit_points";
inherit hb "/std/body/heart_beat";
inherit "/std/body/armor";      /* Added by Fudge */
inherit "/std/body/weapon";     /* Added by Fudge */
inherit com "/std/combat";
inherit "/std/body/gender";
inherit "/std/body/race";
inherit "/std/body/snoop";      /* Added by Fudge */
inherit "/std/body/follow";

static object possessing;    /* The object this player/monster is possessing */
static object possessor;     /* who is possessing this object */
string living_name;

int is_player(void) {  /* Player objects over write this one... */
   return 0;
}

int is_living(void) {
   return 1;
}

string query_name(void) {
   if ((!living_name) || (living_name=="") || (living_name=="mudlib")) {
      return nil;
   }

   return living_name;
}

string query_Name(void) {
   if ((!living_name) || (living_name=="") || (living_name=="mudlib")) {
      return nil;
   }

   return capitalize(living_name);
}

string query_name_proofed(string prefix) {
   string result;

   argcheck(prefix, 1, "string");

   if (nilp(prefix)) {
      prefix="the";
   }

   result=query_name();

   if (!result) {
      result= prefix + " " + query_id();
   }

   return result;
}

int is_possessed(void) {
   if (possessor) {
      return 1;
   }
   return 0;
}

void set_possessor(object ob) {
   possessor = ob;
}

object query_possessor(void) {
   return possessor;
}

int is_possessing(void) {
   if (possessing) {
      return 1;
   }
   return 0;
}

object query_possessing(void) {
   return possessing;
}

void set_possessing(object ob) {
   possessing = ob;
}

void set_name(string name) {
   living_name = name;
   set_id(name);
}

void create(void) {
   ob::create();
   hp::create();
   hb::create();
   com::create();
   equipment = ([]);
   living_name = "mudlib";
   wielding = ({ });
   clear_followers();
}

