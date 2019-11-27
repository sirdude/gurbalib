static object *targets;
static int fighting;
static object target;
int kills;
mapping killed_by;
object killer;
int wimpy;
int wimpy_hp;
string wimpy_dir;

#define FIGHTING_TIMEOUT 300
/* Amount of Endurance required to Attack */
#define ATTACK_COST 1

/* Uncomment this to get extra info in combat */
/* #define DEBUG_COMBAT 1 */

void create(void) {
   targets = ({ });
   fighting = 0;
   wimpy = 0;
   wimpy_hp = 0;
   wimpy_dir = "";
}

int is_fighting(void) {
   return fighting;
}

object query_killer(void) {
   return killer;
}

mapping query_killed_by(void) {
   if (!mappingp(killed_by)) {
      killed_by = ([]);
   }
   return killed_by;
}

void increment_kills(void) {
   kills++;
}

int query_kills(void) {
   return kills;
}

void add_killed_by(object who, int t) {
   string str;

   if (!mappingp(killed_by)) {
      killed_by = ([]);
   }

   killed_by[t] = who->file_name();
#ifdef DO_STATS
   str = "killed: " + this_object()->file_name() + " by " +
   who->query_Name() + "(" + who->file_name() + "):" + t + "\n";
   LOG_D->write_log("stats.raw", str);
#endif
}

int query_killed(void) {
   if (!mappingp(killed_by)) {
      killed_by = ([]);
   }
   return sizeof(map_indices(killed_by));
}

void set_wimpy(int w) {
   wimpy = w;
}

int query_wimpy() {
   return wimpy;
}

void set_wimpy_dir(string where) {
   wimpy_dir = where;
}

string query_wimpy_dir() {
   return wimpy_dir;
}

void set_wimpy_hp(int w) {
   wimpy_hp = w;
}

int query_wimpy_hp() {
   return wimpy_hp;
}

void halt_fight(void) {
   fighting = 0;
   targets = ({ });
}

int run_away(void) {
   string dir, error;
   string *exits;
   int x, y;

   if (!empty_str(wimpy_dir) &&
      this_object()->query_environment()->query_exit(wimpy_dir)) {

      write("You attempt to run " + wimpy_dir + ".\n");
      error = this_object()->this_environment()->body_exit(this_object(),
         wimpy_dir);
      if (error) {
         write(error);
      } else {
         return 1;
      }
   } else if (!empty_str(wimpy_dir) &&
      this_object()->query_environment()->query_hidden_exit(wimpy_dir)) {

      write("You attempt to run " + wimpy_dir + ".\n");
      error = this_object()->this_environment()->body_exit(this_object(),
         wimpy_dir);
      if (error) {
         write(error);
      } else {
         return 1;
      }
   } else {
      exits = this_object()->query_environment()->query_exit_indices();

      if (exits) {
         x = sizeof(exits);
         if (x < 1) {
            write("You attempt to run away but can not find " + "an exit.\n");
         } else {
            if (x > 1) {
               y = random(x);
            } else {
               y = 0;
            }
            this_object()->message("RUNAWAY : " + exits[y] + "\n");
            write("You attempt to run " + exits[y] + ".\n");
            error = this_object()->this_environment()->body_exit(this_object(),
               exits[y]);
            if (error) {
               write(error);
            } else {
               return 1;
            }
         }
      } else {
         write("You attempt to run away but can not find " + "an exit.\n");
      }
   }
   return 0;
}

void receive_damage(object who, int dam) {
   int x, when;

   this_object()->message("%^RED%^You took " + dam + " damage from " +
      who->query_id() + ".%^RESET%^");

   if (this_object()->query_hp() <= dam) {
      x = this_object()->query_max_hp();
      killer = who;
      when = time();
      this_object()->add_killed_by(killer, when);
      this_object()->halt_fight();
      who->halt_fight();
      who->increment_kills();
      who->increase_expr(x);
      /* make sure they are dead */
      this_object()->decrease_hp(x);
   }
   this_object()->decrease_hp(dam);
}

void damage_target(int dam, object who) {
   int target_hp;

   target_hp = who->query_hp();

   /* award expr for damage inflicted to target */
   if (dam > target_hp) {
      this_object()->increase_expr(target_hp);
   } else {
      this_object()->increase_expr(dam);
   }

   /* damage target */
   who->receive_damage(this_object(), dam);
}

int query_defense(void) {
   int me, i, max, skill;
   object *armor;

   skill = (this_object()->query_skill("combat/defense") / 50);
   if (skill > 0) {
      /* if skill is greater than zero use random */
      me = random(skill);
   } else {
      /* if skill is 0 then prevent random(0) from */
      /* using the full range */
      me = random(1);
   }

   me += this_object()->query_statbonus("dex");
   armor = this_object()->query_equipment();

   max = sizeof(armor);
   for (i = 0; i < max; i++) {
      if (armor[i]->is_armor()) {
         me += armor[i]->query_ac();
      }
   }

   armor = this_object()->query_wielded();
   max = sizeof(armor);
   for (i = 0; i < max; i++) {
      if (armor[i]->is_armor()) {
         me += armor[i]->query_ac();
      }
   }

   return me;
}

int do_swing(int me) {
   int opponent, me_roll, opponent_roll;

   if (me < 3) {
      me = 3;
   }

   me_roll = random(me + 1);

   opponent = target->query_defense();
   opponent_roll = random(opponent + 1);

#ifdef DEBUG_COMBAT
   this_object()->message("Roll [%^RED%^" + me_roll + "%^RESET%^/%^GREEN%^" +
      me + "%^RESET%^  vs %^RED%^" + opponent_roll + "%^RESET%^/%^GREEN%^" +
      opponent + "%^RESET%^]");
#endif

   if (me_roll > opponent_roll) {
      return 1;
   }
   return 0;
}

object get_target(object targ) {
   int i, max;

   if (targ) {
      if (targ->is_dead()) {
         targets -= ({ targ });
         if (sizeof(targets) == 0) {
            fighting = 0;
         }
      } else if (targ->query_environment() ==
         this_object()->query_environment()) {

         fighting = FIGHTING_TIMEOUT;
         return targ;
      }
   }
   max = sizeof(targets);
   for (i = 0; i < max; i++) {
      if (targets[i] && targets[i]->is_dead()) {
         targets -= ({targets[i]});
         i--;
         max--;                 /* shorten up our array.... */

         if (sizeof(targets) == 0) {
            fighting = 0;
            i = max;
         }
      }
      if (i < sizeof(targets) &&
         targets[i] && (targets[i]->query_environment() ==
         this_object()->query_environment())) {

         fighting = FIGHTING_TIMEOUT;
         return targets[i];
      }
   }
   fighting = fighting - 1;
   if (fighting < 1) {
      targets = ({ });
   }
   return nil;
}

int damage_hook(object victim, object weapon, int damage) {
   int extra_damage;

   extra_damage = damage;
   if (!nilp(weapon) && victim->is_vulnerable(weapon->query_materials())) {
      extra_damage += (damage / 3);
   }

   return extra_damage;
}

int after_damage_hook(object aggressor, object weapon, int damage) {
   return damage;
}

private void handle_performance_enhancement_expires() {
   if (this_object()->has_performance_enhancement()) {
      this_object()->decrease_performance_enhancement(1);
      if (!this_object()->has_performance_enhancement()) {
         this_object()->message("You no longer feel like you could " +
            "fight forever.");
      }
   }
}

void attack_with(string skill, object weapon, object target) {
   int me, tmp, damage;

   if ( (wimpy == 1) &&
      (wimpy_hp > this_object()->query_hp())) {

      run_away();
      return;
   }

   me = this_object()->query_end();
   if (me < ATTACK_COST) {
      this_object()->message("You are too tired to attack.\n");
      return;
   } else {
      handle_performance_enhancement_expires();
      if (!this_object()->has_performance_enhancement()) {
         this_object()->decrease_end(ATTACK_COST);
      }
   }

   if (!weapon) {
      me = (this_object()->query_skill("combat/unarmed") / 50) +
         this_object()->query_statbonus("str");
   } else {
      me = (this_object()->query_skill(weapon->query_weapon_skill()) / 50) +
         this_object()->query_statbonus("str") + weapon->query_hit_bonus();
   }

   if (do_swing(me) == 1) {
      if (!weapon) {
         damage = random(3) + this_object()->query_statbonus("str");
         tmp = this_object()->query_skill("combat/unarmed") +
            this_object()->query_skill("combat/unarmed") / 2;
         if (tmp <= target->query_skill("combat/defense")) {
            this_object()->learn_skill(this_object()->query_hit_skill());
#ifdef DEBUG_COMBAT
            this_object()->message("Learn: hit_skill, " +
               this_object()->query_skill("combat/unarmed"));
#endif
         }

         damage = damage_hook(target, nil, damage);
         damage = target->after_damage_hook(this_object(), nil, damage);

         if (damage == 0) {
            this_object()->targeted_action("$N $v" +
               this_object()->query_hit_string() + " $T, but $vdo no " +
               "damage!", target);
         } else {
            this_object()->targeted_action("$N $v" +
               this_object()->query_hit_string() + " $T.", target);
         }
      } else {
         damage = this_object()->query_statbonus("str") +
            weapon->query_weapon_damage();
         tmp = this_object()->query_skill(weapon->query_weapon_skill()) +
            this_object()->query_skill(weapon->query_weapon_skill()) / 2;
         if (tmp <= target->query_skill("combat/defense")) {
            this_object()->learn_skill(weapon->query_weapon_skill());
#ifdef DEBUG_COMBAT
            this_object()->message("Learn: hit_skill, " +
               this_object()->query_skill(weapon->query_weapon_skill()));
#endif
         }

         damage = damage_hook(target, weapon, damage);
         damage = target->after_damage_hook(this_object(), weapon, damage);

         if (damage == 0) {
            this_object()->targeted_action("$N " +
               "$v" + weapon->query_weapon_action() + " $T with a " +
               weapon->query_id() + ", but $vdo no damage!", target);
         } else {
            this_object()->targeted_action("$N " +
               "$v" + weapon->query_weapon_action() + " $T with a " +
               weapon->query_id() + ".", target);
         }
      }

      this_object()->damage_target(damage, target);
   } else {
      string miss;

      if (weapon) {
         miss = weapon->query_weapon_miss();
      }

      if (!miss) {
         this_object()->targeted_action("$N $vmiss $T.", target);
      } else {
         this_object()->targeted_action("$N " + miss + " $T.", target);
      }

      if (!weapon) {
         tmp = this_object()->query_skill("combat/unarmed") +
         this_object()->query_skill("combat/unarmed") / 2;
      } else {
         tmp = this_object()->query_skill(weapon->query_weapon_skill()) +
         this_object()->query_skill(weapon->query_weapon_skill()) / 2;
      }
      if (target->query_skill("combat/defense") <= tmp) {
         target->learn_skill("combat/defense");
#ifdef DEBUG_COMBAT
         target->message("Learn: defense, " +
            this_object()->query_skill("combat/defense"));
#endif
      }
   }
}

void cast_spell(object target) {
   int damage;
   string message;

   damage = random(this_object()->query_spell_damage()) + 1;

   message = this_object()->query_spell_message();
   if (!message || (message == "")) {
      message = "Casts an unamed spell at $t.";
   }
   message = replace_string(message, "$t", target->query_name());

   target->query_environment()->tell_room(this_object(), message);

   this_object()->damage_target(damage, target);
}

string get_color(int x, int maxx) {
   int y, z;

   z = (maxx / 3) + 1;
   y = maxx - z;

   if (x > y) {
      return "%^GREEN%^";
   } else if (x <= z) {
      return "%^RED%^";
   } else {
      return "%^YELLOW%^";
   }
}

string get_status(object thing) {
   int tmp, tmpmax;
   string col, line;

   if (!objectp(thing)) {
      return "";
   }

   tmp = thing->query_hp();
   tmpmax = thing->query_max_hp();
   col = get_color(tmp, tmpmax);
   line = col + "HP[" + tmp + "/" + tmpmax + "]%^RESET%^";

   tmp = thing->query_mana();
   tmpmax = thing->query_max_mana();
   col = get_color(tmp, tmpmax);
   line += " " + col + "MANA[" + tmp + "/" + tmpmax + "]%^RESET%^";

   tmp = thing->query_end();
   tmpmax = thing->query_max_end();
   col = get_color(tmp, tmpmax);
   line += " " + col + "END[" + tmp + "/" + tmpmax + "]%^RESET%^";
   line += "\tEXP: " + thing->query_expr();

   return line;
}

void do_fight(void) {
   int i, max, x;
   object *weapons;
   string line;

   target = get_target(target);

   if (target) {
      if (!this_object()->is_player()) {
         x = this_object()->query_spell_chance();

         if (x && (random(100) <= x)) {
            cast_spell(target);
         }
      }

      weapons = this_object()->query_wielded();
      max = sizeof(weapons);

      if (max == 0) {
         this_object()->attack_with("combat/unarmed", nil, target);
      } else {
         for (i = 0; i < max; i++) {
            if (!weapons[i]->query_offensive()) {
               continue;
            }
            this_object()->attack_with(weapons[i]->query_weapon_skill(),
               weapons[i], target);
         }
      }

      /* Need to check target again, because target may have died. */
      line = get_status(this_object());

#ifdef DEBUG_COMBAT
      line += " " + get_status(target);
#endif

      this_object()->message(line);
   }
}

void attacked_by(object who) {
   if (!targets) {
      targets = ({ });
   }
   targets += ({ who });
   target = who;
   fighting = FIGHTING_TIMEOUT;
}

void attack(object who) {
   if (!targets) {
      targets = ({ });
   }
   if (who->is_dead()) {
      return;
   }

   targets += ({ who });
   fighting = FIGHTING_TIMEOUT;
   target = who;
   who->attacked_by(this_object());
   do_fight();
}

string *summarise_killers(void) {
   string retval, killer_name, *killers, *lines;
   mapping killer_count, killer_map;
   object tmp_killer;
   int i, dim, done, *killed_times;

   killer_map = this_object()->query_killed_by();
   killed_times = map_indices(killer_map);
   killer_count = ([ ]);
   killers = ({ });

   done = 0;
   lines = ({ "Summary of your killers:" });
   lines += ({ "------------------------" });

   for (i = 0, dim = sizeof(killed_times); i < dim; i++) {
      tmp_killer = clone_object(killer_map[killed_times[i]]);
      tmp_killer->setup();
      killer_name = tmp_killer->query_Name();
      destruct_object(tmp_killer);

      if (!killer_count[killer_name]) {
         killer_count[killer_name] = 1;
      } else {
         killer_count[killer_name]++;
      }

      done = 1;
   }

   if (!done) {
      lines += ({ "So far you have been spared, count yourself lucky.\n" });
   } else {

      killers = map_indices(killer_count);
      dim = sizeof(killers);
      for (i = 0; i < dim; i++) {
         lines += ({ killers[i] + " killed you " +
            killer_count[killers[i]] + " time(s).\n" });
      }
   }

   return lines;
}
