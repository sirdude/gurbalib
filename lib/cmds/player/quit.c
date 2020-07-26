inherit M_COMMAND;

string *usage(void) {
   string *lines;

   lines = ({ "Usage: quit [-h] [-r]" });
   lines += ({ "" });
   lines += ({ "This is how you leave the game." });
   lines += ({ "Note: You will keep your money, but you will drop all " +
      "of your items " });
   lines += ({ "when you quit." });
   lines += ({ "" });
   lines += ({ "Options:" });
   lines += ({ "\t-h\tHelp, this usage message." });
   lines += ({ "\t-r\tRemove your account.  WARNING this is not reversible." });
   lines += ({ "Examples:" });
   lines += ({ "\tquit" });
   lines += ({ "\tquit -r" });

   lines += get_alsos();

   return lines;
}

void setup_alsos() {
   add_also("player", "attack");
   add_also("player", "consider");
   add_also("player", "cast");
   add_also("player", "eat");
   add_also("player", "enter");
   add_also("player", "follow");
   add_also("player", "go");
   add_also("player", "pray");
   add_also("player", "wimpy");

   add_also("wiz", "aload");
   add_also("wiz", "forcequit");
}

void verify_remove(string str) {
   if (str == "Y" || str == "y") {
      USER_D->delete_user(this_player()->query_name());
   } else {
      write("Removal cancelled.\n");
   }
}

static void main(string str) {
   string follower, *followers;
   object obj;
   int x, max;

   if (!alsos) {
      setup_alsos();
   }

   if (!empty_str(str)) {
      if (str == "-r") {
         write("WARNING!!! this is not reversible.\n");
         write("Are you sure you really want to remove your player? (Y|N)\n");
         this_player()->input_to_object(this_object(), "verify_remove");
         return;
      }
      this_player()->more(usage());
      return;
   }

   if (this_player()->is_resting()) {
      this_player()->rest();
   }
   follower = this_player()->query_follower();
   if (follower != "") {
      write("You stop following " + follower + ".\n");
      this_environment()->tell_room(this_player(),
         this_player()->query_Name() + " stops following " +
         capitalize(follower) + ".\n");
      obj = USER_D->find_player(follower);
      obj->remove_follower(this_player()->query_name());
      this_player()->set_follower("");
   }
   followers = this_player()->query_followers();
   if (followers) {
     max = sizeof(followers);
     for (x=0; x< max; x++) {
        obj = USER_D->find_player(followers[x]);
        if (obj) {
          followers[x]->message("You stop following " +
            this_player()->query_Name() + " they quit.\n");
          followers[x]->query_environment()->tell_room(followers[x],
            followers[x]->query_Name() + " stops following " +
            this_player()->query_Name() + ".\n",
            this_player());
        } else {
           this_player()->remove_follower(followers[x]);
	}
     }
   }

   this_player()->clear_followers();

   this_player()->do_quit();
}
