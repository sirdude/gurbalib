/* inheritable for commands */

mapping alsos;

static void main( string arg );

static nomask string query_command(void) {
  return get_otlvar( "command" );
}

nomask void _main(string arg, string cmd) {
  if (previous_program() == COMMAND_D) {
    set_otlvar("command", cmd);
    main(arg);
  }
}

string *query_alsos() {
   if (!alsos) {
      return ({ });
   }

   return map_indices(alsos);
}

string get_also_type(string value) {
   if (alsos && member_map(value, alsos)) {
       return alsos[value];
   }

   return "";
}

string *get_alsos() {
   string *lines, *values, tmp;
   int i, sz, done, tmplen, width;

   lines = ({ "See also:" });

   done = 0;
   width = this_player()->query_width() - 8;

   if (alsos) {
      values = map_indices(alsos);
      sz = sizeof(values);
 
      for (i = 0; i < sz; i++) {
         if (alsos[values[i]] == "player" ||
            (alsos[values[i]] == "wiz" &&
            query_wizard(this_player())) ||
            (alsos[values[i]] == "admin" &&
            query_admin(this_player()))) { 

            if (!done) {
               tmp = "\t" + values[i];
               done = 1;
            } else {
               tmplen = strlen(tmp) + strlen(values[i]) + 2;
               if (tmplen < width) {
                  tmp += ", " + values[i]; 
               } else {
                  lines += ({ tmp });
                  tmp = "\t" + values[i];
               }
            }
         } else {
            write("Error: invalid also type: " + alsos[values[i]] + 
               "for entry: " + values[i] + "\n");
         }
      }
      lines += ({ tmp });
   }

   return lines;
}

int add_also(string type, string value) {
   if (!alsos) {
      alsos = ([ ]);
   }

   if (!member_map(value, alsos)) {
      alsos[value] = type;
      return 1;
   }

   return 0;
}
