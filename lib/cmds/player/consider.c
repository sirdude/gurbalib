inherit M_COMMAND;

string *usage(void) {
   string *lines;

   lines = ({ "Usage: con [-h] [OBJECT]" });
   lines += ({ "" });
   lines += ({ "OBJ offends you, you are deciding if you can kill it." });
   lines += ({ "" });
   lines += ({ "Options:" });
   lines += ({ "\t-h\tHelp, this usage message." });
   lines += ({ "Examples:" });
   lines += ({ "\tconsider rat" });

   lines += get_alsos();

   return lines;
}

void setup_alsos() {
   add_also("player", "cast");
   add_also("player", "eat");
   add_also("player", "enter");
   add_also("player", "follow");
   add_also("player", "go");
   add_also("player", "pray");
   add_also("player", "quit");
   add_also("player", "wimpy");
}

static void main(string str) {
   object obj;
   int level, leveldelta, yourlevel;

   if (!alsos) {
      setup_alsos();
   }

   if (!str && (str != "")) {
      this_player()->more(usage());
      return;
   }

   if (sscanf(str, "-%s", str)) {
      this_player()->more(usage());
      return;
   }

   if (this_player()->is_dead()) {
      write("You can not do that when your are not among the living.\n");
      return;
   }

   obj = this_player()->query_environment()->present(lowercase(str));
   if (!obj) {
      write("Consider what?\n");
      return;
   }

   if (!obj->is_living() || obj->is_dead()) {
      write("You can not consider attacking that.\n");
      return;
   }

   if (obj == this_player()) {
      write("You can not consider attacking yourself.\n");
      return;
   }

   level = obj->query_level();
   /*write("Level is: " + level + "\n");*/

   yourlevel = this_player()->query_level();
   leveldelta = (level-yourlevel);
   /*write("Level difference is " + leveldelta + "\n");*/

  if(leveldelta < 0){
    write("It will be an easy kill.\n");
  }else if(leveldelta == 0){
    write("Perfect match!\n");
  }else if(leveldelta > 0 && leveldelta <=3){
    write("It is a little stronger than you.\n");
  }else if(leveldelta > 3 && leveldelta <=5){
    write("It is a fair deal stronger than you.\n");
  }else if (leveldelta > 5){
    write("It is MUCHO stronger than you. How much, you don't know. You are fairly certain it will kill you though.\n");
  }

   return;

}
