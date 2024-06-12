#define VALID_STATS ({ "str", "dex", "con", "int", "wis", "cha" })

mapping base_stats;
mapping bonus_stats;

string query_valid_base_stat(string statname) {
   if (base_stats[statname]) {
      return statname;
   }
}

int query_stat(string statname) {
   int base, bonus;

   if (!statname) {
      return 0;
   }
   if (!base_stats) {
      base_stats = ([]);
   }
   if (!bonus_stats) {
      bonus_stats = ([]);
   }
   if (base_stats[statname]) {
      base = base_stats[statname];
   } else {
      base = 0;
   }
   if (bonus_stats[statname]) {
      bonus = bonus_stats[statname];
   } else {
      bonus = 0;
   }
   return (base + bonus);
}

int query_base_stat(string statname) {
   if (!statname) {
      return 0;
   }
   if (!base_stats) {
      base_stats = ([]);
   }

   if (base_stats[statname]) {
      return base_stats[statname];
   }
   return 0;
}

int query_bonus_stat(string statname) {
   if (!statname) {
      return 0;
   }
   if (!bonus_stats) {
      bonus_stats = ([]);
   }
   if (bonus_stats[statname]) {
      return bonus_stats[statname];
   }

   return 0;
}

void trim_base_stat(string statname) {
   int max, min;

   max = this_object()->query_race_object()->query_base_stat_maximum(statname);
   min = this_object()->query_race_object()->query_base_stat_minimum(statname);

   if (query_base_stat(statname) < min) {
      base_stats[statname] = min;
   } else if (query_base_stat(statname) > max) {
      base_stats[statname] = max;
   }
}

void trim_bonus_stat(string statname) {
   int max, min;

   max = 10;
   min = 0;

   if (query_bonus_stat(statname) < min) {
      bonus_stats[statname] = min;
   } else if (query_bonus_stat(statname) > max) {
      bonus_stats[statname] = max;
   }
}

int add_base_stat(string statname, int amt) {
   if (!base_stats) {
      base_stats = ([]);
   }
   if (member_array(statname, VALID_STATS) == -1) {
      return 0;
   }
   if (!base_stats[statname]) {
      base_stats[statname] = amt;
   } else {
      base_stats[statname] += amt;
   }
   trim_base_stat(statname);
   return 1;
}

int add_timed_bonus_stat(string statname, int amt, int time) {
   add_base_stat(statname, amt);
   call_out("add_base_stat", time, -amt);
   return 1;
}

int add_bonus_stat(string statname, int amt) {
   if (!bonus_stats) {
      bonus_stats = ([]);
   }
   if (member_array(statname, VALID_STATS) == -1) {
      return 0;
   }
   if (!bonus_stats[statname]) {
      bonus_stats[statname] = amt;
   } else {
      bonus_stats[statname] += amt;
   }
   trim_bonus_stat(statname);
   return 1;
}

int set_base_stat(string statname, int amt) {
   if (!base_stats) {
      base_stats = ([]);
   }
   if (member_array(statname, VALID_STATS) == -1) {
      return 0;
   }
   base_stats[statname] = amt;
   trim_base_stat(statname);
   return 1;
}

int set_bonus_stat(string statname, int amt) {
   if (!bonus_stats) {
      bonus_stats = ([]);
   }
   if (member_array(statname, VALID_STATS) == -1) {
      return 0;
   }
   bonus_stats[statname] = amt;
   trim_bonus_stat(statname);
   return 1;
}

mixed *query_all_stats(void) {
   mixed *ret;
   string *stat_names;
   int i;

   stat_names = map_indices(base_stats);
   ret = ({ });
   for (i = 0; i < sizeof(stat_names); i++) {
      ret += ({ ({ stat_names[i], base_stats[stat_names[i]],
         bonus_stats[stat_names[i]]}) });
   }
   return ret;
}

int query_statbonus(string name) {
   int stat;

   if (member_array(name, VALID_STATS) == -1) {
      return 0;
   }

   stat = query_stat(name);

   if (stat > 1 && stat < 9) {
      return 1;
   } else if (stat > 8 && stat < 14) {
      return 2;
   } else if (stat > 13 && stat < 18) {
      return 3;
   } else if (stat > 17 && stat < 20) {
      return 4;
   } else if (stat > 19) {
      return 5;
   }
   return 0;
}

void initialize_base_stats(void) {
   int i, sOK, sum, *s;
   object race;

   race = this_object()->query_race_object();

   sOK = FALSE;

   while (!sOK) {

      s = ({ 0, 0, 0, 0, 0, 0 });
      i = 0;
      for (i = 0; i < 6; i++) {
         s[i] = random(race->query_base_stat_maximum(VALID_STATS[i]) -
            race->query_base_stat_minimum(VALID_STATS[i]) + 1) +
            race->query_base_stat_minimum(VALID_STATS[i]);
      }

      if (!race->is_monster_race()) {
         sum = 0;
         for (i = 0; i < 6; i++) {
            sum += s[i];
         }

         if (sum > 75) {
            sOK = 1;
         }
      } else {
         sOK = 1;
      }
   }

   for (i = 0; i < 6; i++) {
      this_object()->set_base_stat(VALID_STATS[i], s[i] + 
         race->query_stat_mod(VALID_STATS[i]));
   }
}

void initialize_bonus_stats(void) {
   int i;

   for (i = 0; i < 6; i++) {
      this_object()->set_bonus_stat(VALID_STATS[i], 0);
   }
}

void initialize_stat_dependant_variables(void) {
   int tmp, stat;

   tmp = this_object()->query_base_stat("con");
   stat = 20 + tmp;
   this_object()->set_max_hp(stat);

   tmp = this_object()->query_base_stat("wis");
   stat = 20 + tmp;
   this_object()->set_max_mana(stat);

   tmp = this_object()->query_base_stat("con") +
      this_object()->query_base_stat("dex");
   stat = 20 + tmp;
   this_object()->set_max_end(stat);
}

void initialize_stats(void) {
   this_object()->initialize_base_stats();
   this_object()->initialize_bonus_stats();
   initialize_stat_dependant_variables();
}
