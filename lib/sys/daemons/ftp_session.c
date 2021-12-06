#define CHUNK_SIZE 8192

#ifndef DISABLE_FTP

inherit M_CONNECTION;
inherit "/sys/lib/runas";

private mapping cmds;
int priv, binary, filesize, where, closing, connected, timeout_handle;
string file_name, chunk, cwd, name, store_file_name;
object connection;

void ftp_cmd_list(string str);

void ftp_connection_wait(void) {
   if (!connection) {
      send_message("425 Can't open data connection\n");
      return;
   }

   if (connection->is_connected() == 0) {
      call_out("ftp_connection_wait", 1);
      return;
   }
   send_message("200 PORT command successful.\n");
}

void restore_privs(void) {
   string cpriv;

   if (name == "guest") {
      priv = 0;
      cpriv = "nobody";
   } else {
      cpriv = name;

      if (query_wizard(name)) {
         int i, sz;
         string *dn;

         priv = 1;
         cpriv = "wizard:" + cpriv;
         if (query_admin(name)) {
            cpriv = "system:" + cpriv;
         }

         dn = DOMAIN_D->query_domains();

         sz = sizeof(dn);
         for (i = 0; i < sz; i++) {
            if (DOMAIN_D->query_domain_member(dn[i], name)) {
               cpriv += ":" + dn[i];
            }
         }
      } else {
         priv = 0;
      }
   }
   run_as(cpriv);
}

void open(void) {
   send_message("220-GurbaLib FTP daemon v0.01 ready.\n");
   send_message("220 Use your mud name as login.\n");
   cmds = ([
      "user" : "ftp_cmd_user",
      "pass" : "ftp_cmd_pass",
      "retr" : "ftp_cmd_retr",
      "stor" : "ftp_cmd_stor",
      "nlst" : "ftp_cmd_nlst",
      "list" : "ftp_cmd_list",
      "pwd" : "ftp_cmd_pwd",
      "cdup" : "ftp_cmd_cdup",
      "cwd" : "ftp_cmd_cwd",
      "quit" : "ftp_cmd_quit",
      "type" : "ftp_cmd_type",
      "mkd" : "ftp_cmd_mkd",
      "port" : "ftp_cmd_port",
      "noop" : "ftp_cmd_noop",
      "dele" : "ftp_cmd_dele",
      "syst" : "ftp_cmd_syst",
   ]);
   connected = 0;
   name = "";
   timeout_handle = call_out("login_timeout", 120);
}

void close(varargs int flag) {
   closing = 1;
   destruct_object(this_object());
}

string query_name(void) {
   return name;
}

void FTPLOG(string str) {
   LOG_D->write_log("ftpd", ctime(time()) + " : " + str);
}

static string normalize_path(string file, string dir) {
   string path;

   path = ::normalize_path(file, dir);

   if ((priv != 0) || (path == "/pub") || (sscanf(path, "/pub/%*s") == 1)) {
      return path;
   } else {
      return nil;
   }
}

void ftp_cmd_user(string arg) {
   arg = lowercase(arg);
   if (connected) {
      send_message("530 User " + arg + " access denied.\n");
      return;
   }
   name = arg;

#ifndef DISABLE_ANON_FTP
   if (name == "ftp" || name == "anonymous") {
      send_message("331 Guest login ok, send your complete e-mail " +
         "address as password.\n");
      return;
   }
#endif

   send_message("331 Password required for " + arg + ".\n");
   return;
}

void ftp_cmd_pass(string arg) {
   if (!name || name == "") {
      send_message("530 Login with USER first.\n");
      return;
   }
#ifndef DISABLE_ANON_FTP
   if (name == "ftp" || name == "anonymous") {
      send_message("230 guest login ok, access restrictions apply.\n");
      name = "guest";
      connected = 1;
      restore_privs();
      cwd = "/pub/";
      FTPLOG("Anonymous login (" + arg + ")\n");
      return;
   }
#endif

   if (!USER_D->login(name, arg)) {
      send_message("530 Login incorrect.\n");
      disconnect();
      return;
   }
   send_message("230 User " + name + " logged in.\n");
   FTPLOG(name + " logged in.\n");
   connected = 1;

   restore_privs();

   cwd = "/pub/";
   USER_D->user_online(name, this_object());
}

void ftp_cmd_retr(string str) {
   string p;

   if (!str) {
      send_message("550 No file selected.\n");
      connection->terminate();
      return;
   }

   p = str;

   str = normalize_path(str, cwd);
   if (!str || str == "" || !valid_read(str)) {
      send_message("550 " + p + ": Permission denied.\n");
      connection->terminate();
      return;
   }

   if (file_exists(str) <= 0) {
      send_message("550 " + str + ": No such file.\n");
      connection->terminate();
      return;
   }

   chunk = read_file(str, 0, CHUNK_SIZE);

   filesize = file_size(str);
   file_name = str;

   if (binary == 0) {
      where = strlen(chunk);
      chunk = implode(explode(chunk, "\n"), "\r\n");
      send_message("150 Opening ASCII mode data connection for " + str + " (" +
         filesize + " bytes).\n");
   } else {
      where = strlen(chunk);
      send_message("150 Opening binary mode data connection for " + str + " (" +
         filesize + " bytes).\n");
   }

   if (filesize < CHUNK_SIZE) {
      connection->set_callback("ftp_write");
      connection->send_data(chunk);
   } else {
      connection->set_callback("ftp_retr");
      connection->send_data(chunk);
   }

   FTPLOG(name + " GOT " + str + ".\n");
}

void ftp_cmd_stor(string arg) {
   string path, dir, *dirs;

   if (priv == 0) {
      send_message("550 Permission denied.\n");
      return;
   }

   path = normalize_path(arg, cwd);
   if (!path || path == "" || !valid_write(path)) {
      send_message("550 Permission denied.\n");
      return;
   }

   store_file_name = path;

   dirs = explode(path, "/");
   dir = implode(dirs[..sizeof(dirs) - 2], "/");
   if (strlen(dir) > 2) {
      dir = dir[..strlen(dir) - 1];
   }
   if (file_exists(dir) != -1) {
      send_message("553 No such directory to STOR into. (" + dir + ")\n");
      return;
   }

   if (!connection) {
      send_message("426 Connection closed, transfer aborted");
      return;
   }

   if (file_exists(store_file_name)) {
      remove_file(store_file_name);
   }

   connection->set_read_callback("ftp_stor");

   if (binary == 0) {
      send_message("150 Opening ASCII mode data connection for " + arg + ".\n");
   } else {
      send_message("150 Opening binary mode data connection for " + arg +
         ".\n");
   }
   where = 0;
   FTPLOG(name + " PUT " + store_file_name + ".\n");
}

void ftp_cmd_nlst(string str) {
   mixed *files, *objects;
   string *names, timestr, size, dirlist;
   int *sizes, *times, long, ancient, i, j, sz, max, len, rows, time;

   if (str == "-l") {
      ftp_cmd_list("./");
      return;
   }

   if (!str) {
      str = ".";
   } else if (sscanf(str, "-%s", str) != 0) {
      long = 1;
      if (str == "l") {
         str = ".";
      } else if (sscanf(str, "l %s", str) == 0) {
         return;
      }
   }

   str = normalize_path(str, cwd);
   if (!str || str == "" || !valid_read(str)) {
      send_message("550 Permission denied.\n");
      connection->terminate();
      return;
   }

   files = get_dir(str);

   if (!files) {
      send_message("550 " + str + ": No such file or directory.\n");
      connection->terminate();
      return;
   }

   files = get_dir(str + "/*");
   if (!files) {
      send_message("550 " + str + ": Permission denied.\n");
      connection->terminate();
      return;
   }

   names = files[0];
   sz = sizeof(names);
   if (sz == 0) {
      send_message("550 No files found.\n");
      connection->terminate();
      return;
   }
   sizes = files[1];
   times = files[2];

   dirlist = "";

   for (i = 0; i < sizeof(names); i++) {
      dirlist += names[i] + "\r\n";
   }
   send_message("150 Opening ASCII mode data connection for file list.\n");
   connection->set_callback("ftp_write");
   connection->send_data(dirlist);
}

string ftp_myctime(int nTime) {

   string zDate, zWeekday, zMon, zTime, zYear, zThis_year, zDay;
   int nDay;

   zDate = ctime(nTime);
   sscanf(zDate, "%s %s %d %s %s", zWeekday, zMon, nDay, zTime, zYear);
   zThis_year = ctime(time())[20..];

   zDay = "";
   zDay += nDay;
   if (strlen(zDay) == 1) {
      zDay = " " + zDay;
   }

   if (zYear == zThis_year) {
      return (zMon + " " + zDay + " " + zTime[0..4]);
   } else {
      return (zMon + " " + zDay + "  " + zYear);
   }
}

void ftp_cmd_list(string str) {
   mixed *files, *objects;
   string *names, timestr, size, dirlist;
   int *sizes, *times, long, ancient, i, j, sz, max, len, rows, time;

   if (!str) {
      str = ".";
   } else if (sscanf(str, "-%s", str) != 0) {
      long = 1;
      if (str == "l") {
         str = ".";
      } else if (sscanf(str, "l %s", str) == 0) {
         return;
      }
   }

   str = normalize_path(str, cwd);
   if (!str || str == "" || !valid_read(str)) {
      send_message("550 Permission denied.\n");
      connection->terminate();
      return;
   }

   files = get_dir(str);

   if (!files) {
      send_message("550 " + str + ": No such file or directory.\n");
      connection->terminate();
      return;
   }

   files = get_dir(str + "/*");
   if (!files) {
      send_message("550 " + str + ": Permission denied.\n");
      connection->terminate();
      return;
   }

   names = files[0];
   sz = sizeof(names);
   if (sz == 0) {
      send_message("550 No files found.\n");
      connection->terminate();
      return;
   }
   sizes = files[1];
   times = files[2];

   dirlist = "";

   for (i = 0; i < sizeof(names); i++) {
      if (sizes[i] < 0) {
         /* We're dealing with a directory */
         dirlist += "drwxr-xr-x   1 gurba    gurba        1024 " +
            ftp_myctime(times[i]) + " " + names[i] + "\r\n";
      } else {
         /* We're dealing with a file */
         dirlist += "-rw-r--r--   1 gurba    gurba ";
         size = "                    " + sizes[i];
         size = size[strlen(size) - 11..];
         dirlist += size + " " + ftp_myctime(times[i]) + " " + names[i] +
            "\r\n";
      }

   }
   send_message("150 Opening ASCII mode data connection for /bin/ls.\n");
   connection->set_callback("ftp_write");
   connection->send_data(dirlist);
}

void ftp_cmd_pwd(string arg) {
   send_message("257 \"" + cwd + "\" is current directory.\n");
}

void ftp_cmd_cwd(string arg) {
   arg = normalize_path(arg, cwd);

   if (!arg || arg == "" || !valid_read(arg)) {
      send_message("550 Permission denied.\n");
      return;
   }

   if (strlen(arg) > 1 && arg[strlen(arg) - 1] == '/') {
      arg = arg[..strlen(arg) - 2];
   }

   if (file_exists(arg) != -1) {
      send_message("550 " + arg + ": No such file or directory.\n");
      return;
   }
   cwd = arg;
   send_message("250 CWD command successful.\n");
}

void ftp_cmd_cdup(string arg) {
   ftp_cmd_cwd("..");
}

void ftp_cmd_quit(string arg) {
   send_message("221 Goodbye.\n");
   FTPLOG(name + " quit.\n");
   USER_D->user_offline(name, this_object());
}

void ftp_cmd_type(string arg) {
   switch (arg) {
      case "a":
      case "A":
         binary = 0;
         send_message("200 Type set to A.\n");
         return;
      case "i":
      case "I":
         binary = 1;
         send_message("200 Type set to I.\n");
         return;
      default:
         send_message("550 Unknown file type.\n");
         return;
   }
}

void ftp_cmd_mkd(string arg) {
   string file;

   file = normalize_path(arg, cwd);
   if (!file || file == "" || priv == 0 || !valid_write(file)) {
      send_message("550 Permission denied.\n");
      return;
   }

   if (file_exists(file) == 0) {
      if (make_dir(file) == 0) {
         send_message("550 Unable to create directory.\n");
         return;
      } else {
         send_message("257 MKD command successful.\n");
         return;
      }
   } else {
      send_message("550 File or dir already exists.\n");
   }
}

void ftp_cmd_port(string arg) {
   string ip, *tmp;
   int port;

   tmp = explode(arg, ",");

   if (sizeof(tmp) != 6) {
      send_message("500 PORT " + arg + " not understood.\n");
      return;
   }

   ip = implode(tmp[0..3], ".");
   port = (str2val(tmp[4]) << 8) + (str2val(tmp[5]));

   if (connection) {
      connection->terminate();
   }

   connection = clone_object("/sys/daemons/ftp_data");
   connection->start_connection(ip, port, binary);
   ftp_connection_wait();
}

void ftp_cmd_noop(string arg) {
}

void ftp_cmd_dele(string arg) {
   string file;

   file = normalize_path(arg, cwd);
   if (!file || file == "" || priv == 0 || !valid_write(file)) {
      send_message("550 " + arg + ": Permission denied.\n");
      return;
   }

   if (file_exists(file) == -1) {
      if (remove_dir(file) == 0) {
         send_message("550 " + arg + ": Not empty.\n");
      } else {
         send_message("250 DELE command successful.\n");
      }
   } else if (file_exists(file) != 0) {
      if (remove_file(file) == 0) {
         send_message("550 " + arg + ": Unable to DELE.\n");
      } else {
         send_message("250 DELE command successful.\n");
      }
   } else {
      send_message("550 " + arg + ": Not found.\n");
   }
}

void ftp_cmd_syst(string arg) {
   send_message("215 UNIX Mud name: Gurba\n");
}

void ftp_write(void) {
   send_message("226 Transfer complete.\n");
   connection->terminate();
}

void ftp_retr(void) {
   string *converted;

   if (where < filesize) {
      chunk = read_file(file_name, where, CHUNK_SIZE);
   }

   if (binary == 0 && chunk != "") {
      if (chunk[strlen(chunk) - 1] == '\n') {
         chunk += " ";
      }
      if (chunk[0] == '\n') {
         chunk = " " + chunk;
      }
      converted = explode(chunk, "\n");
      chunk = implode(converted, "\r\n");
   }

   if ((where + CHUNK_SIZE) > filesize) {
      connection->set_callback("ftp_write");
   }
   connection->send_data(chunk);
   where += CHUNK_SIZE;
}

void ftp_stor(string str) {
   string *lines;

   if (binary == 0) {
      lines = explode(str, "\r");
      str = implode(lines, "");
   }

   write_file(store_file_name, str);
}

void receive_message(string message) {
   string cmd, arg, func;

   if (message != "" && strlen(message) >= 2) {
      if (message[strlen(message) - 1] == '\n') {
         message = message[..strlen(message) - 2];
      }
   }

   if (message != "" && strlen(message) >= 2) {
      if (message[strlen(message) - 1] == '\r') {
         message = message[..strlen(message) - 2];
      }
   }

   arg = "";
   if (sscanf(message, "%s %s", cmd, arg) != 2) {
      cmd = message;
   }

   cmd = lowercase(cmd);

   if (connected == 0) {
      /* Only allow these commands if not connected */
      switch (cmd) {
         case "user":
            ftp_cmd_user(arg);
            return;
         case "pass":
            ftp_cmd_pass(arg);
            return;
         case "quit":
            ftp_cmd_quit(arg);
            return;
         case "noop":
            ftp_cmd_noop(arg);
            return;
         default:
            send_message("503 Log in with USER first.\n");
            return;
      }
   }
   func = cmds[cmd];
   if (!func) {
      /* Log command so we know what clients are trying to use */
      send_message("502 Unknown command " + cmd + "\n");
      return;
   }

   if ((call_other(this_object(), func, arg))) {
      FTPLOG(name + " caused a FAILURE with command '" + message + "'.\n");
      send_message("550 Unknown failure.  Please report what you were doing " +
         "to the mud admin.\n");
   }

   return;
}

void login_timeout(void) {
   if (!connected) {
      send_message("220 Timed out.\n");
      disconnect();
      destruct_object(this_object());
   }
}

void destructing(void) {
   if (!closing) {
      disconnect();
   }
}
#endif
