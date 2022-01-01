int level, experience;

int ready_for_next_level(void) {
   int tmpexp;

   tmpexp = level * level * 2000;

   if (experience > tmpexp) {
      return 1;
   }
   return 0;
}

string query_level_title(int level) {
   switch (level) {
      case 1:
         return "the uninitiated.\n";
         break;
      case 2:
         return "the newbie.\n";
         break;
      case 3:
         return "is getting the hang of things.\n";
         break;
      case 4:
         return "the mediocre.\n";
         break;
      case 5:
         return "the Sub-average.\n";
         break;
      case 6:
         return "the Average.\n";
         break;
      case 7:
         return "the Strong.\n";
         break;
      case 8:
         return "the Great.\n";
         break;
      case 9:
         if (this_object()->query_race() == "male") {
            return "the Baron.\n";
         } else {
            return "the Baroness.\n";
         }
         break;
      case 10:
         return "the Titan.\n";
         break;
      case 11:
         return "the Conquerer.\n";
         break;
      case 12:
         return "the Famous.\n";
         break;
      case 13:
         return "the Awe-inspiring.\n";
         break;
      case 14:
         return "the Battle Hardened.\n";
         break;
      case 15:
         return "the More than Adequate.\n";
         break;
      case 16:
         if (this_object()->query_race() == "male") {
            return "the Grand Baron.\n";
         } else {
            return "the Grand Baroness.\n";
         }
         break;
      case 17:
         return "the Great Titan.\n";
         break;
      case 18:
         return "the Mighty Conquerer.\n";
         break;
      case 19:
         return "the High and Mighty.";
         break;
      default:
         return "the Grand Wizard!!!";
         break;
   }
}

void increase_level(void) {
   level++;
   this_object()->set_max_hp((level *
      this_object()->query_base_stat("str")) + 20);
   this_object()->set_max_mana((level *
      this_object()->query_base_stat("wis")) + 20);
   this_object()->set_max_end((level *
      this_object()->query_base_stat("con")) + 20);
   write("Congratulations, you just achieved level: " + level + "\n");
   this_object()->set_title("$N " + query_level_title(level));
}

void increase_expr(int expr) {
   if (expr < 0) {
      expr = -expr;
   }
   experience += expr;
   if (experience < 0) {
      experience = 0;
   }
   if (ready_for_next_level()) {
      increase_level();
      write("Congratulations, you just went up a level...\n");
   }
}

void decrease_expr(int expr) {
   if (expr > 0) {
      expr = expr * -1;
   }
   experience -= expr;
   if (experience < 0) {
      experience = 0;
   }
}

void set_expr(int expr) {
   experience = expr;
   if (experience < 0) {
      experience = 0;
   }
}

int query_expr(void) {
   return (experience);
}

int query_level(void) {
   return (level);
}
