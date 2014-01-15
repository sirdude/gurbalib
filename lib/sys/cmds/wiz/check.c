inherit M_COMMAND;

int warn, error;

void usage() {
   string *lines;

   lines = ({ "Usage: check [-h] FILENAME" });
   lines += ({ " " });
   lines += ({ "Check the supplied file, FILENAME." });
   lines += ({ "If FILENAME is a directory check all files in that directory." 
      });
   lines += ({ " " });
   lines += ({ "Options:" });
   lines += ({ "\t-h\tHelp, this usage message." });
   lines += ({ "See also:" });

   if (query_admin(this_player())) {
      lines += ({ "\tcheckmud, clean, clone, dest, eval, graph, rebuild, " +
         "update, warmboot" });
   } else {
      lines += ({ "\tclean, clone, dest, eval, graph, rebuild, " +
         "update" });
   }

   this_player()->more(lines);
}

void warn(string str) {
   write("Warning: " + str);
   warn+=1;
}

void error(string str) {
   write("Warning: " + str);
   error+=1;
}

string add_dotc(string input) {
   string str;

   str = input;
   if (!str || (str == "")) return "";

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
   if (file_exists(path)) return path;
   path = add_dotc(path);
   return path;
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
   string *indices;
   string exit, filename, exitfile;
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

   if (c > 0) return 0;
   return 1;
}

void do_standard_checks(object obj) {
   string tmp, tmp2;
   int x;

   tmp = obj->query_short();
   if (!tmp || (tmp == "")) {
      warn("Object has no short description.\n");
   }

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

   tmp = obj->query_long();
   if (!tmp || (tmp == "")) warn("Monster has no long description.\n");

}


int check_functions(object obj, mixed funs) {
   int x, c;

   if (!funs)
      return -1;
      
   x = sizeof(funs) - 1;
   
   if(x < 0)
      return -1;

   c = 0;
   
   while(x > -1) {
      write("Checking Function: "+obj->query_object_command( funs[x] )+"\n"); 
      if(!function_object( obj->query_object_command( funs[x] ), obj ) ) {
         warn("Warning: Function " + obj->query_object_command( funs[x] ) + " not defined in: " +
         obj->file_name() + "\n");
         c = c + 1;
         }
    x--;
    }
   if (c > 0) return 0;
   return 1;
}


void do_room_check(object obj) {
   mixed myexits;
   int x;

   write("Doing room check\n");

   write("Checking exits:\n");
   myexits = obj->query_exits();
   x = check_exits(obj,myexits);

   if (x == -1) {
      write("\tNo visible exits.\n");
   } else if (x) {
      write("\tOk.\n");
   } else {
      write("\tFail.\n");
   }

   write("Checking hidden exits: ");
   myexits = obj->query_hidden_exits();
   x = check_exits(obj,myexits);

   if (x == -1) {
      write("\tNo hidden exits.\n");
   } else if (x) {
      write("\tOk.\n");
   } else {
      write("\tFail.\n");
   }

   write("Checking room commands: ");
   myexits = obj->query_object_commands();
   x = check_functions(obj,myexits);

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

   write("Doing monster check\n");

   tmp = obj->query_name();
   if (!tmp || (tmp == "")) warn("Monster has no name.\n");

   if (obj->is_gettable()) warn("Living object is gettable.\n");

   x = obj->query_level();
   if (x < 1) warn("Monster has no level.\n");
}

void do_object_check(object obj) {
   string tmpstr;
   string *functions;
   int x;

   write("Doing object check\n");

   if (obj->is_gettable() && (obj->query_weight() < 1)) {
      warn("Object gettable and weight < 1\n");
   }
   if (obj->is_gettable() && (obj->query_value() < 1)) {
      warn("Object gettable and value < 1\n");
   }
   if (!obj->is_gettable() && (obj->query_value() > 1)) {
      warn("Object ungettable and value > 1\n");
   }

   functions = obj->query_object_commands();
   write("Checking object functions.\n");
   x = check_functions(obj,functions);

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

   if (empty_str(str)) {
      usage();
      return;
   } else if (sscanf(str, "-%s", str)) {
      usage();
      return;
   }

   files = explode(str," ");
   if (!files) files = ({ str });
   max = sizeof(files);
   for(x=0;x<max;x++) {
      warn =0;
      error = 0;
      do_check(files[x]);
      write("Errors: " + error + " Warnings: " + warn + "\n");
   }
}
