void usage() {
   string *lines;

   lines = ({ "Usage: where [-h] [player]" });
   lines += ({ " " });
   lines += ({ "Locates the given [player] and prints out their location." });
   lines += ({ "If [player] is omitted it prints out everyones location." });
   lines += ({ " " });
   lines += ({ "Options:" });
   lines += ({ "\t-h\tHelp, this usage message." });
   lines += ({ "Examples:" });
   linse += ({ "\twhere" });
   linse += ({ "\twhere sirdude" });
   lines += ({ "See also:" });
   lines += ({ "\tgoto, home, summon" });

   this_player()->more(lines);
}

void main(string str) {
   int i;
   object *usrs;
   object usr;

   if (!str || (str == "")) {
      usrs = USER_D->query_players();

      for (i = 0; i < sizeof(usrs); i++) {
	 write(usrs[i]->query_Name() + " " + 
            usrs[i]->query_environment()->query_brief() + " " +
	    usrs[i]->query_environment()->base_name() + "\n");
      }
   } else {
      if (sscanf(str, "-%s", str)) {
	 usage();
      } else if (usr = USER_D->find_player(str)) {
	 write(usr->query_Name() + " " +
	    usr->query_environment()->query_brief() + " " +
	    usr->query_environment()->base_name() + "\n");
      } else {
	 write("Who are you looking for?  I can't find: " + str + "!\n");
      }
   }
}
