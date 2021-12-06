inherit M_COMMAND;

int warn, error;
string *syspath;

string *usage(void) {
   string *lines;

   lines = ({ "Usage: check [-h] FILENAME" });
   lines += ({ " " });
   lines += ({ "Check the supplied file, FILENAME." });
   lines += ({ "If FILENAME is a directory check all files in that directory." 
      });
   lines += ({ " " });
   lines += ({ "Options:" });
   lines += ({ "\t-h\tHelp, this usage message." });
   lines += ({ "Examples:" });
   lines += ({ "\tcheck /domains/required/rooms/void.c" });
   lines += ({ "\tcheck /domains/required/objects/small_rock.c" });

   lines += get_alsos();

   return lines;
}

void setup_syspath() {
   syspath = ({ "/sys/cmds/admin/",
      "/sys/cmds/wiz/",
      "/cmds/player/",
      "/cmds/guild/fighter/",
      "/cmds/spells/",
      "/cmds/monster/" });
}

void setup_alsos() {
   add_also("wiz", "clean");
   add_also("wiz", "clone");
   add_also("wiz", "dest");
   add_also("wiz", "eval");
   add_also("wiz", "graph");
   add_also("wiz", "rebuild");
   add_also("wiz", "update");

   add_also("admin", "checkmud");
   add_also("admin", "warmboot");
}

void warn(string str) {
   write("Warning: " + str);
   warn += 1;
}

void error(string str) {
   write("Warning: " + str);
   error += 1;
}

string add_dotc(string input) {
   string str;

   str = input;
   if (!str || (str == "")) { 
      return "";
   }

   if (strlen(str) > 2) {
      if ((str[strlen(str) - 2] == '.') && (str[strlen(str) - 1] == 'c')) {
         /* were good do nothing... */
      } else {
         str = str + ".c";
      }
   }
   return str;
}

string get_what(string str) {
   string path;

   path = this_player()->query_env("cwd");
   path = normalize_path(str, path);

   if (file_exists(path)) {
      return path;
   }

   path = add_dotc(path);
   return path;
}

void check_a_spell(string filename) {
   object obj;
   string *functionlist;
   int x, max;

   write("Check spell: " + filename + "\n");

   obj = compile_object(filename);

   if (!obj) {
      error("Unable to load command: filename\n");
      return;
   }

   functionlist = ({ "usage" });
   functionlist += ({ "do_spell" });

   max = sizeof(functionlist);
   for (x = 0; x < max; x++) {
      if (!function_object(functionlist[x], obj)) {
         warn(obj->file_name() + ": " + functionlist[x] + " undefined.\n");
      }
   }
}

void check_a_command(string filename) {
   object obj, tobj;
   string *functionlist, *talsos, *fname, tf, atype;
   int x, max;
   
   write("Check command: " + filename + "\n");

   obj = compile_object(filename);

   if (!obj) {
      error("Unable to load command: filename\n");
      return;
   }

   fname = path_file(filename);

   functionlist = ({ "usage" });
   functionlist += ({ "setup_alsos" });
   
   max = sizeof(functionlist);
   for (x = 0; x < max; x++) {
      if (!function_object(functionlist[x], obj)) {
         warn(obj->file_name() + ": " + functionlist[x] + " undefined.\n");
      }
   }

   obj->setup_alsos();
   talsos = obj->query_alsos();
   max = sizeof(talsos);


   for (x = 0; x < max; x++) {
      tf = talsos[x] + ".c";
      if (fname[1] == tf) {
         warn(obj->file_name() + ": references itself in also.\n");
      } else {
         tobj = COMMAND_D->find_command(talsos[x], "", syspath);

         if (!tobj) {
            warn(obj->file_name() + ": references unknown also " + talsos[x] +
               "\n");
         } else {
/*            tobj->setup_alsos(); */
            if (!member_array(talsos[x], tobj->query_alsos())) {
               warn(obj->file_name() + ": also " + talsos[x] + 
                  " not back referenced.\n");
            } else {
               atype = tobj->get_also_type(talsos[x]);
               if ((atype == "player") && file_exists("/cmds/player/" + talsos[x] + ".c")) {
               }else if ((atype == "wiz") && file_exists("/sys/cmds/wiz/" + talsos[x] + ".c")) {
               } else if ((atype == "admin") && file_exists("/sys/cmds/admin/" + talsos[x] + ".c")) {
               } else if ((atype == "monster") && file_exists("/cmds/monster/" + talsos[x] + ".c")) {
               } else {
                  warn(obj->file_name() + " has wrong type \"" + atype + 
                     "\" for also: " + talsos[x] + "\n");
               }
            }
         }
      }
   }
}

void check_a_daemon(string filename) {
   object obj;

   write("Check daemon: " + filename + "\n");

   obj = compile_object(filename);

   if (!obj) {
      error("Unable to load command: filename\n");
      return;
   }
}

void check_a_library(string filename) {
   object obj;

   write("Check library: " + filename + "\n");

   if (!compile_library(filename)) {
      error("Unable to load command: filename\n");
      return;
   }
}

void check_remote_exit(string room, string exit, string filename) {
   object obj;
   string tmp, myexit;

   if (file_exists(room)) {

      obj = find_object(room);

      if (!obj) {
         obj = compile_object(room);
         obj->setup();
         obj->setup_mudlib();
      }
   }

   if (!obj) {
      error("Unable to load room : " + room + "\n");
      return;
   }

  myexit = invert_exit(exit);
  tmp = obj->query_exit(myexit);
  tmp = add_dotc(tmp);

  if (!tmp || (tmp == "")) {
     warn("No reverse exit: " + obj->base_name() + 
        "(" + myexit + ")\n");
  } else if (tmp != filename) {
     warn("Reverse exit " + obj->base_name() + ":(" + myexit + ")\n" +
        "Points at: " + tmp + " not " + filename + ".\n");
  }
}

int check_exits(object obj, mapping myexits) {
   string exit, filename, exitfile, *indices;
   int x, len, c;

   if (!myexits) {
      return -1;
   }

   c = 0;
   if (indices = map_indices(myexits)) {

      x = sizeof(indices) -1;
      if (x < 0) {
         return -1;
      }
      while (x > -1) {
         exitfile = myexits[indices[x]];
         exitfile = add_dotc(exitfile);

         write("Checking exit: " + indices[x] + " " + exitfile + "\n");

         if (!file_exists(exitfile)) {
            if (myexits[exitfile]) {
               warn("Exit: " + indices[x] + ":" + 
                  myexits[exitfile] + " does not exist.\n");
               c = c + 1;
            } else {
               warn("Exit: " + indices[x] + ": does not exist.\n"); 
               c = c + 1;
            }
         } else if (invert_exit(indices[x]) != "unknown") {
            filename = obj->base_name();
            filename = add_dotc(filename);

            check_remote_exit(exitfile, indices[x], filename);

         } else {
            warn("Nonstandard exit: " + exitfile + " no further checks.\n");
               c = c + 1;
         }
         x = x - 1;
      }
   }

   if (c > 0) {
      return 0;
   }
   return 1;
}

void do_standard_checks(object obj) {
   string tmp, tmp2;
   int x;

   tmp = obj->query_short();
   if (!tmp || (tmp == "")) {
      warn("Object has no short description.\n");
   } else {
      tmp2 = capitalize(tmp);

      if (tmp2 != tmp) {
         warn("Object short not capitalized.\n");
      }

      x = strlen(tmp) -1;
      if (x < 1) {
         warn("Object short too short : \'" + obj->query_short() + "\'\n");
      } else if ((tmp[x] == '.') || (tmp[x] == '?') || (tmp[x] == '!')) {
         warn("Object short ends with punctuation.\n");
      }
   }

   tmp = obj->query_long();
   if (!tmp || (tmp == "")) {
      warn("Object has no long description.\n");
   }
}

int check_functions(object obj, mixed funs) {
   int x, c;

   if (!funs) {
      return -1;
   }
      
   x = sizeof(funs) - 1;
   
   if (x < 0) {
      return -1;
   }

   c = 0;
   
   while (x > -1) {
      write("Checking Function: " + obj->query_action( funs[x] ) + "\n"); 
      if (!function_object( obj->query_action( funs[x] ), obj ) ) {
         warn("Warning: Function " + obj->query_action( funs[x] ) + 
            " not defined in: " + obj->file_name() + "\n");
         c = c + 1;
      }
      x--;
    }
   if (c > 0) {
      return 0;
   }
   return 1;
}

void do_room_check(object obj) {
   mixed myexits;
   int x;

   write("Doing room check: " + obj->file_name() + "\n");

   write("Checking exits:\n");
   myexits = obj->query_exits();
   x = check_exits(obj, myexits);

   if (x == -1) {
      write("\tNo visible exits.\n");
   } else if (x) {
      write("\tOk.\n");
   } else {
      write("\tFail.\n");
   }

   write("Checking hidden exits: ");
   myexits = obj->query_hidden_exits();
   x = check_exits(obj, myexits);

   if (x == -1) {
      write("\tNo hidden exits.\n");
   } else if (x) {
      write("\tOk.\n");
   } else {
      write("\tFail.\n");
   }

   write("Checking room commands: ");
   myexits = obj->query_actions();
   x = check_functions(obj, myexits);

   if (x == -1) {
      write("\tNo room commands.\n");
   } else if (x) {
      write("\tOk.\n");
   } else {
      write("\tFail.\n");
   }
}

void do_monster_check(object obj) {
   string tmp;
   int x;

   write("Doing monster check: " + obj->file_name() + "\n");

   tmp = obj->query_name();
   if (!tmp || (tmp == "") || (tmp == "mudlib")) {
      warn("Monster has no name.\n");
   }

   if (obj->is_gettable()) {
      warn("Living object is gettable.\n");
   }

   x = obj->query_level();
   if (x < 1) {
      warn("Monster has no level.\n");
   }

   if (obj->query_spell_message()) {
      x = obj->query_spell_chance();
      if (x < 1) {
         warn("spell chance < 1\n");
      }
      if (x > 99) {
         warn("spell chance > 99\n");
      }
   }
}

void do_money_check(object obj) {
   if (obj->query_value() == 0) {
      warn("This money has no value.\n");
   } else if (obj->query_value() < 0) {
      error("This money has - value.\n");
   }

   if (obj->query_weight() > 0) {
      warn("Money should not have a weight.\n");
   } else if (obj->query_weight() < 0) {
      error("Objects should not have - weight.\n");
   }
}

void do_object_check(object obj) {
   string tmpstr, *functions;
   int x;

   write("Doing object check: " + obj->file_name() + "\n");

   if (obj->is_money()) {
      do_money_check(obj);
      return;
   }

   if (obj->is_gettable()) {
      if (obj->query_weight() < 1) {
         warn("Object gettable and weight < 1.\n");
      }
      if (obj->query_value() < 1) {
         warn("Object gettable and value < 1.\n");
      }
      if (obj->is_wearable() || obj->is_wieldable()) {
         x = obj->query_size();
         if ((x > 6) || (x < 1)) {
            warn("Object size = " + x + "; should be 0 < size < 7.\n");
         }
      } else {
         x = obj->query_size();
         if (x > 0) {
            warn("Object size = " + x + " this object is not wearable or " +
               "wieldable.\n");
         }
      }
   } else {
      if (obj->query_value() > 1) {
         warn("Object ungettable and value > 1.\n");
      }
      if (obj->is_wearable()) {
         warn("Object ungettable and wearable.\n");
      }
      if (obj->is_wieldable()) {
         warn("Object ungettable and wieldable.\n");
      }
   }

   functions = obj->query_actions();
   write("Checking object functions.\n");
   x = check_functions(obj, functions);

   if (x == -1) {
      write("\tNo object functions.\n");
   } else if (x) {
      write("\tOk.\n");
   } else {
      write("\tFail.\n");
   }
}

void do_check(string str) {
   string what;
   object obj;

   what = get_what(str);
   if (file_exists(what) == -1) {
      error("Checking directories unsupported currently: " + what + "\n");
   } else {
      if (file_exists(what) == 1) {
         write("Looking at file: " + what + "\n");

         if (COMMAND_D->file_is_spell(what)) {
            check_a_spell(what);
            return;
         } else if (COMMAND_D->file_is_command(what)) {
            check_a_command(what);
            return;
         } else if (INIT_D->file_is_daemon(what)) {
            check_a_daemon(what);
            return;
         } else if (COMPILER_D->test_inheritable(what)) {
            check_a_library(what);
            return;
         }

         obj = compile_object(what);

         obj->setup();
         obj->setup_mudlib();

         if (obj) {
            do_standard_checks(obj);

            if (obj->is_room()) {
               do_room_check(obj);
            } else if (obj->is_living()) {
               do_monster_check(obj);
            } else {
               do_object_check(obj);
            }
         } else {
            error("Unable to compile: " + str + "\n");
         }
      } else {
            error("File does not exist: " + str + "\n");
      }
   }
}

static void main(string str) {
   string *files;
   int x, max;

   if (!alsos) {
      setup_alsos();
   }

   if (!syspath) {
      setup_syspath();
   }

   if (empty_str(str)) {
      this_player()->more(usage());
      return;
   } else if (sscanf(str, "-%s", str)) {
      this_player()->more(usage());
      return;
   }

   files = explode(str, " ");

   if (!files) {
      files = ({ str });
   }

   max = sizeof(files);

   for (x = 0; x < max; x++) {
      warn = 0;
      error = 0;
      do_check(files[x]);
      write("Errors: " + error + " Warnings: " + warn + "\n");
   }
}
