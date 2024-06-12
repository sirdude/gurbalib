#include <privileges.h>

#define AUTH_DATA "/sys/daemons/auth_data"
#define CACHE_INTERVAL 300
#define AUTH_DATA_DIR "/sys/daemons/data/users"

static mapping users, cache, sessions, wizards;
static object  spare;
static int     auto_admin, handle;
int data_version;

static void secure(void) {
   string name;

   name = previous_program(1);
   if ((name != USER_OB) && (name != this_program())
      && (name != "/sys/daemons/ftp_session") 
      && (name != "/cmds/player/quit") 
      && !require_priv("system")) {
      error("Access denied: " + name + "\n");
   }
}

static void save_me(void) {
   unguarded("save_object", "/sys/daemons/data/user_d.o");
}

static void restore_me(void) {
   unguarded("restore_object", "/sys/daemons/data/user_d.o");
}

int logout(string name) {
   object ob;

   secure();

   if (!sessions[name] || !sizeof(sessions[name])) {
      sessions[name] = nil;
      ob = cache[name];

      if (ob) {
         if (!spare) {
            spare = ob;
         } else {
            destruct_object(ob);
         }
         cache[name] = nil;
         return 1;
      } else {
         return 0;
      }
   }

   return 0;
}

object find_user(string name) {
   return users[name];
}

/* If there are any data objects (clones) around, destruct them.  */
static void cleanup(void) {
   object c, n;

   c = find_object(AUTH_DATA);

   if (!c) {
      return;
   }

   c = c->next_clone();

   while (c) {
      n = c->next_clone();
      if (c == spare) {
         spare = nil;
      }
      destruct_object(c);
      c = n;
   }
}

static object get_data_ob(string name) {
   object ob;

   ob = cache[name];

   if (ob) {
      return ob;
   }

   if (spare) {
      ob = spare;
      spare = nil;
   } else {
      ob = clone_object(AUTH_DATA);
   }

   if (ob->load(name)) {
      cache[name] = ob;
      return ob;
   } else {
      if (!spare) {
         spare = ob;
      } else {
         destruct_object(ob);
      }
   }
}

static void clean_cache(void) {
   int i, sz;
   string *names;

   handle = 0;
   names = map_indices(cache);

   sz = sizeof(names);
   for (i = 0; i < sz; i++) {
      if (!USER_D->query_sessions(names[i])) {
         logout(names[i]);
      }
   }

   handle = call_out("clean_cache", CACHE_INTERVAL);
}

int set_password(string name, string secret) {
   object data;

   secure();

   data = get_data_ob(name);

   if (!data) {
      return 0;
   }
   data->set_pass(name, secret);
   save_me();
}

int reset_password(string who, string passwd) {
   object obj;

   if (!query_admin(this_player())) {
      return 0;
   }

   obj = USER_D->find_player(who);
   if (obj) {
      write("That user is currently logged in you cannot change their " +
         "password.\n");
   }

   obj = get_data_ob(who);

   if (!obj) {
      return 0;
   }

   obj->set_pass(who, passwd);
   obj->save_me();

   return 1;
}

int login(string name, string secret) {
   object data;

   secure();

   data = get_data_ob(name);

   if (!data) {
      return 0;
   }

   return data->test(name, secret);
}

int user_exists(string name) {
   return unguarded("file_exists", AUTH_DATA_DIR + "/" + name + ".o") == 1;
}

static int _new_user(string name, string secret, object u) {
   object ob;

   if (spare) {
      ob = spare;
      spare = nil;
   } else {
      ob = clone_object(AUTH_DATA);
   }

   cache[name] = ob;
   ob->set_name(name);
   ob->set_pass(name, secret);

   if (auto_admin) {
      u->set_auto_admin(1);
      auto_admin = 0;
#ifdef ALL_USERS_WIZ
   } else if (ALL_USERS_WIZ) {
      u->set_auto_wiz(1);
#endif
   } else {
      ob->set_type(PLAYER_L);
   }

   return 1;
}

int new_user(string name, string secret) {
   int result;

   secure();
   unguarded("_new_user", name, secret, previous_object());

   return 1;
}

int _delete_user(string name) {
   object u, p;

   if (!require_priv("system")) {
      error("Access denied.");
      return 0;
   }

   filter_array(DOMAIN_D->query_domains(), "remove_domain_member", DOMAIN_D,
      name);

   if (u = find_user(name)) {
      p = u->query_player();
      destruct_object(u);
      if (p) {
         destruct_object(p);
      }
   }

   if (sessions[name]) {
      int i, sz;

      sz = sizeof(sessions[name]);
      for (i = 0; i < sz; i++) {
         if (sessions[name][i]) {
            destruct_object(sessions[name][i]);
         }
      }
      sessions[name] = nil;
   }

   if (cache[name]) {
      if (!spare) {
         spare = cache[name];
      } else {
         destruct_object(cache[name]);
      }
      cache[name] = nil;
   }

   LAST_D->remove_entry(name);

   unguarded("remove_file", AUTH_DATA_DIR + "/" + name + ".o");
   unguarded("remove_file", "/data/players/" + name + ".o");

   return 1;
}

int delete_user(string name) {
   secure();

   return unguarded("_delete_user", name);
}

static void destructing(void) {
   cleanup();
}

void user_online(string name, object user) {
   secure();

   if (user<-USER_OB) {
      users[name] = user;
      if (user->query_auto_admin() ) {
         unguarded("make_admin", name);
      } else if (user->query_auto_wiz() ) {
         unguarded("make_wizard", name);
      }
   }

   if (!sessions[name]) {
      sessions[name] = ({ });
   } else {
      sessions[name] -= ({ nil });
   }
   sessions[name] |= ({ user });
}

void user_offline(string name, object user) {
   secure();

   if (user<-USER_OB) {
      users[name] = nil;
   }
   if (sessions[name]) {
      sessions[name] -= ({ user, nil });
   }
   unguarded("logout", name);
}

object *query_users(void) {
   return map_values(users);
}

string *query_user_names(void) {
   return map_indices(users);
}

object find_player(string name) {
   object ob;

   ob = find_user(lowercase(name));

   if (ob) {
      return ob->query_player();
   }

   return ob;
}

object *query_players(void) {
   object *usr, *players;
   int i, sz;

   usr = query_users();

   players = ({ });

   sz = sizeof(usr);
   for (i = 0; i < sz; i++) {
      if (usr[i]->query_name() != "who") {
         players += ({ usr[i]->query_player() });
      }
   }
   return players;
}

object *query_wizards(void) {
   object *usr, *wizards;
   int i, sz;

   usr = query_users();

   wizards = ({ });

   sz = sizeof(usr);
   for (i = 0; i < sz; i++) {
      if (query_wizard(usr[i]->query_player()->query_name()) > 0) {
         wizards += ({ usr[i]->query_player() });
      }
   }

   return wizards;
}

int valid_player_name(string str) {
   return !catch(parse_string("valid=/[a-zA-Z][a-zA-Z0-9]+/ name: valid", str));
}

int player_exists(string str) {
   return unguarded("file_exists", "/data/players/" + str + ".o") > 0;
}

string *list_all_users(void) {
   string name, *files, *names;
   int x, i;

   names = ({ });
   files = unguarded( "get_dir", "/data/players/*.o")[0];

   rlimits(MAX_DEPTH; -1) {
      for (i = sizeof(files) - 1; i >= 0; i--) {
         x = strlen(files[i]) - 3;
         name = files[i][..x];
         names += ({ name });
      }
   }

   return names;
}

void print_finger_info(object player, object player2, int cloned) {
   string linkd;

   player->message("%^BLUE%^Name:%^RESET%^ " + player2->query_Name() + "\n");
   player->message("%^BLUE%^Title:%^RESET%^ " + player2->query_title() + "\n");

   if (query_admin(player2->query_name()) == 1) {
      player->message("%^BLUE%^Status: %^RESET%^Administrator\n");
   } else if (query_wizard(player2->query_name()) == 1) {
      player->message("%^BLUE%^Status: %^RESET%^Wizard\n");
   } else {
      player->message("%^BLUE%^Status: %^RESET%^Player\n");
   }
   player->message("%^BLUE%^Age:%^RESET%^ " + player2->query_age() + "\n");

   if (query_wizard(player)) {
      player->message("%^BLUE%^Description:%^RESET%^ " +
         player2->query_long() + "\n");
   }

   if (query_admin(player)) {
      if (player2->query_realname()) {
         player->message("%^BLUE%^Real name: %^RESET%^" +
            player2->query_realname() + "\n");
      } else {
         player->message("%^BLUE%^Real name: %^RESET%^\n");
      }
      if (player2->query_email()) {
         player->message("%^BLUE%^Email address: %^RESET%^" +
            player2->query_email() + "\n");
      } else {
         player->message("%^BLUE%^Email address: %^RESET%^\n");
      }
   }

   if (cloned == 1) {
      player->message("%^BLUE%^Last login: %^RESET%^" +
         ctime(player2->query_last_login()));
   } else {
      
      if (LINKDEAD_D->is_linkdead(player2) ) {
         linkd = " %^BOLD%^%^YELLOW%^[link-dead]%^RESET%^";
      } else {
         linkd = "";
      }

      player->message("%^BLUE%^Last login: %^RESET%^Now " + linkd + "\n");
      if (player2->query_idle() > 60) {
         player->message("%^BLUE%^Idle: %^RESET%^" +
            fmt_time(player2->query_idle()) + "\n");
      }
   }
}

/* Player is requesting finger information on player: name
   So lets tell them....  return 0 if not found. 1 if telling player info...
*/
void finger(object player, string name) {
   object obj;

   obj = find_player(name);
   if (!obj) {

      obj = get_data_ob(name);

      if (obj) {
         obj = clone_object(PLAYER_OB);
         obj->set_name(name);
         obj->restore_me();

         print_finger_info(player, obj, 1);
         destruct_object(obj);
      }
   } else {
      print_finger_info(player, obj, 0);
   }
}

string print_email_info(object player, object obj, string type) {
   string str;
   int doit;

   doit = 0;
   if (type == "wizards") {
      if (query_wizard(obj)) {
         doit = 1;
      } else {
         doit = 0;
      }
   } else if (type == "admins") {
      if (query_admin(obj)) {
         doit = 1;
      } else {
         doit = 0;
      }
   } else {
      doit = 1;
   }

   if (doit) {
      if (obj->query_realname()) {
         str = obj->query_realname();
      } else {
         str = "";
      }
      if (obj->query_name()) {
         str += "(" + obj->query_name() + ") <" ;
      } else {
         str += "() <";
      }
      if (obj->query_email()) {
         str += obj->query_email() + ">";
      } else {
         str += ">";
      }
   }

   return str;
}

string get_email_info(object player, string name, string type) {
   object obj;
   string stuff;

   obj = find_player(name);

   if (!obj) {
      obj = clone_object(PLAYER_OB);
      obj->set_name(name);
      obj->restore_me();

      stuff = print_email_info(player, obj, type);
      destruct_object(obj);
   } else {
      stuff = print_email_info(player, obj, type);
   }

   return stuff;
}

int restore_privs(string name) {
   object *ses;
   int i, sz;

   if (sessions[name]) {
      ses = sessions[name] - ({ nil });
      if (!sizeof(ses)) {
         ses = nil;
      } else {
         sz = sizeof(ses);
         for (i = 0; i < sz; i++) {
            ses[i]->restore_privs();
         }
      }
      sessions[name] = ses;
      return 1;
   }
}

int set_type(string name, int priv) {
   object ob;

   secure();

   ob = get_data_ob(name);

   if (ob) {
      ob->set_type(priv);
      restore_privs(name);
      return 1;
   }
}

int query_type(string name) {
   object ob;

   ob = get_data_ob(name);

   if (ob) {
      return ob->query_type();
   }

   return 0;
}

static string get_player_name(object p) {
   if (p<-PLAYER_OB) {
      return p->query_name();
   }

   return "";
}

object **query_all_online(void) {
   object *admin, *mortal, *wizard;

   ({ wizard, mortal }) = split_array(players(), "query_wizard");
   ({ admin, wizard }) = split_array(wizard, "query_admin");

   return ({ admin, wizard, mortal });
}

string **query_all_online_names(void) {
   mixed *a, *m, *w;

   ({ a, w, m }) = query_all_online();

   if (sizeof(a)) {
      a = map_array(a, "get_player_name");
   }

   if (sizeof(w)) {
      w = map_array(w, "get_player_name");
   }

   if (sizeof(m)) {
      m = map_array(m, "get_player_name");
   }

   return ({ a, w, m });
}

mixed query_sessions(varargs string name) {
   if (!name) {
      return sessions[..];
   } else if (sessions[name]) {
      return sessions[name] - ({ nil });
   }
}

static void convert_users(void) {
   string *names, n;
   int i, sz;
   object c;

   names = unguarded( "get_dir", "/data/players/*.o")[0];
   c = clone_object(AUTH_DATA);

   rlimits(MAX_DEPTH; -1) {
      sz = sizeof(names);
      for (i = 0; i < sz; i++) {
         catch {
            if (data_version < 2) {
               sscanf(names[i], "%s.o", n);
               console_msg("Moving user " + n + " from secure_d to user_d\n");
               set_type(n, SECURE_D->query_priv(n));
            }
         } : {
            console_msg("WARNING: " + caught_error() + " while converting " + 
               n + "\n");
         }
      }
   }
   destruct_object(c);

   data_version = 2;

   if (!catch(save_me())) {
      SECURE_D->user_d_v2();
   }
}

void create_homedir(string wiz) {
   string path, prev;

   if (!require_priv("system")) {
      prev = previous_object()->base_name();
      error("Access denied: " + prev + "\n");
   }

   path = WIZ_DIR + "/" + wiz + "/";

   if (file_exists(path) == 0) {
      make_dir(path);
      make_dir(path + "rooms/");
      make_dir(path + "logs/");
      copy(DOMAINS_DIR + "/required/rooms/workroom.c",
         path + "rooms/workroom.c");
   }
}

void delete_homedir(string wiz) {
   string path, prev;

   if (!require_priv("system")) {
      prev = previous_object()->base_name();
      error("Access denied: " + prev + "\n");
   }

   path = WIZ_DIR + "/" + wiz + "/";

   if (file_exists(path) == 0) {
      error("No such directory: " + path + "\n");
   } else {
      if (remove_dir(path)) {
         write("Ok.\n");
      } else {
         write("Failed to remove: " + path + "\n");
      }
   }
}

void make_mortal(string name) {
   object player;
   string prev;

   prev = previous_object()->base_name();
   if ((prev != "/sys/cmds/admin/promote") &&
      (prev != this_object()->base_name())) {

      LOG_D->write_log("cheating", "Player: " + this_player()->query_Name() +
         " was trying to make_mortal(" + name + ") with this object " +
         prev + "\n");
      error("Hey! No cheating!\n" + prev + " != /sys/cmds/admin/promote\n");
   }

   if (!require_priv("system")) {
      error("Access denied: " + prev + "\n");
   }

   name = lowercase(name);
   if (user_exists(name)) {
      set_type(name, PLAYER_L);
      player = find_player(name);
      if (!player) {
         /* Player not active now, load him in and add his paths. */
         player = clone_object(PLAYER_OB);
         player->set_name(name);
         player->restore_me();
         player->remove_channel("dgd");
         player->remove_cmd_path("/sys/cmds/admin");
         player->remove_cmd_path("/sys/cmds/wiz");
         player->save_me();
         destruct_object(player);
      } else {
         player->remove_channel("dgd");
         player->remove_cmd_path("/sys/cmds/admin");
         player->remove_cmd_path("/sys/cmds/wiz");
         player->save_me();
         player->message(this_player()->query_Name() +
            " has promoted you to a mortal.");
      }
      filter_array(DOMAIN_D->query_domains(), "remove_domain_member",
         DOMAIN_D, name);
      write(capitalize(name) + " has been made a mortal.");
      save_me();
   } else {
      write("No such player.\n");
   }
}

void make_wizard(string name) {
   object player;
   string prev;

   prev = previous_object()->base_name();
   if (prev != "/sys/cmds/admin/promote" &&
      prev != this_object()->base_name()) {

      LOG_D->write_log("cheating", "Player: " + this_player()->query_Name() +
         " was trying to make_wizard(" + name + ") with this object " +
         prev + "\n");
      error("Hey! No cheating!\n" + prev + " != /sys/cmds/admin/promote\n");
   }

   if (!require_priv("system")) {
      error("Access denied: " + prev + "\n");
   }

   name = lowercase(name);
   if (user_exists(name)) {
      set_type(name, WIZ_L);
      player = find_player(name);
      if (!player) {
         /* Player not active now, load him in and add his paths. */
         player = clone_object(PLAYER_OB);
         player->set_name(name);
         player->restore_me();
         player->remove_channel("dgd");
         player->remove_cmd_path("/sys/cmds/admin");
         player->add_cmd_path("/sys/cmds/wiz");
         player->save_me();
         destruct_object(player);
      } else {
         player->remove_channel("dgd");
         player->remove_cmd_path("/sys/cmds/admin");
         player->add_cmd_path("/sys/cmds/wiz");
         player->save_me();

         if (player != this_player()) {
            player->message(this_player()->query_Name() +
               " has promoted you to a wizard.");
         }
      }
      unguarded("create_homedir", name);
      write(capitalize(name) + " has been made a wizard.");
      save_me();
   } else {
      write("No such player.\n");
   }
}

void make_admin(string name) {
   object player;
   string prev;

   prev = previous_object()->base_name();
   if (prev != "/sys/cmds/admin/promote" &&
      prev != this_object()->base_name()) {

      LOG_D->write_log("cheating", "Player: " + this_player()->query_Name() +
         " was trying to make_admin(" + name + ") with this object " +
         prev + "\n");
      error("Hey! No cheating!\n" + prev + " != /sys/cmds/admin/promote\n");
   }

   if (!require_priv("system")) {
      error("Access denied: " + prev + "\n");
   }

   name = lowercase(name);
   if (user_exists(name)) {
      set_type(name, ADMIN_L);
      player = find_player(name);
      if (!player) {
         /* Player not active now, load him in and add his paths. */
         player = clone_object(PLAYER_OB);
         player->set_name(name);
         player->restore_me();
         player->add_cmd_path("/sys/cmds/wiz");
         player->add_cmd_path("/sys/cmds/admin");
         player->add_channel("dgd");
         player->save_me();
         destruct_object(player);
      } else {
         player->add_cmd_path("/sys/cmds/wiz");
         player->add_cmd_path("/sys/cmds/admin");
         player->add_channel("dgd");
         player->save_me();
         if (player != this_player()) {
            player->message(this_player()->query_Name() +
               " has promoted you to an admin.");
         }
      }
      unguarded("create_homedir", name);
      write(capitalize(name) + " has been made an admin.");
      save_me();
   } else {
      write("No such player.\n");
   }
}

mapping query_cache(void) {
   return cache;
}

string *list_players(int long_flag) {
   string idle, *lines;
   int i, max, hidden, nump;
   object *usr;
   mixed idletime;

   usr = query_players();
   nump = sizeof(usr);

   lines = ({ MUD_NAME + " currently has " + nump + " users online." });

   max = sizeof(usr);
   for (i = 0; i < max; i++) {
      string line;

      line = usr[i]->query_title();

      if (usr[i]->query_hidden() == 1) {
         hidden = 1;
         line += " %^BOLD%^%^RED%^(hidden)%^RESET%^";
      } else {
         hidden = 0;
      }

      if (query_admin(usr[i])) {
         line += " %^BOLD%^%^BLUE%^(Admin)%^RESET%^";
      } else if (query_wizard(usr[i])) {
         line += " %^CYAN%^(Wizard)%^RESET%^";
      }

      if (LINKDEAD_D->is_linkdead(usr[i]) ) {
         line += " %^BOLD%^%^YELLOW%^[link-dead]%^RESET%^";
      }

      idletime = fmt_time(usr[i]->query_idle());
      if (idletime == "") {
         idle = "";
      } else {
         idle = "  (idle " + idletime + ")";
      }
      line += idle;

      if (long_flag == 1) {
         if (usr[i]->query_environment()) {
            lines += ({ line + "\n\t" + usr[i]->query_name() + "'s Location: " +
               usr[i]->query_environment()->query_short() });
         } else {
                lines += ({ line + "\n\t" + usr[i]->query_name() });
         }
      } else {
         if (!hidden) {
            lines += ({ line });
         }
      }
   }

   return lines;
}

static void create(void) {
   object *u;
   int i, sz;

   cache = ([]);
   sessions = ([]);
   wizards = ([ ]);
   restore_me();

   if (sizeof(unguarded("get_dir", AUTH_DATA_DIR + "/*.o")[0]) == 0) {
      console_msg("enabling auto_admin\n");
      auto_admin = 1;
   }

   if (data_version <2) {
      call_out("convert_users", 0);
   }

   users = ([]);

   u = users();

   for (i = 0, sz = sizeof(u); i < sz; i++) {
      string name; 
      object uob;

      if (u[i]<-USER_OB) { 
         if (!u[i]->online()) {
            /* will go online itself */
            continue;
         }
         uob = u[i];
      } else if (u[i]<-CONNECTION) {
         uob = u[i]->query_user();
      }

      if (uob) {
         name = uob->query_name();
         if (name && name != "who") {
            if (uob<-USER_OB) {
               users[name] = uob;
            }

            if (!sessions[name]) {
               sessions[name] = ({ });
            }
            sessions[name] |= ({ uob });
         }
      }
   }

   if (!handle) {
      handle = call_out("clean_cache", CACHE_INTERVAL);
   }
   cleanup();
}

void upgraded(void) {
   create();
}
