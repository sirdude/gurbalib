#ifndef DISABLE_FTP

inherit M_CONNECTION;
inherit "/sys/lib/runas";

#include <ports.h>

object "/sys/daemons/ftp_session" prev;
string callback;
string read_callback;
int connected;

static void secure(void) {
   if (previous_program() != this_program() && 
      !previous_object()<-"/sys/daemons/ftp_session") {
      error("Permission denied!");
   }
}

void FTPLOG(string str) {
   LOG_D->write_log("ftpd", ctime(time()) + " : " + str);
}

void message_done(void) {
   if (prev) {
      call_other(prev, callback);
   }
}

static void create(void) {
   callback = "ftp_write";
   connected = 0;
   run_as("network");
}

void set_callback(string str) {
   callback = str;
}

void open(varargs int port) {
   connected = 1;
}

int is_connected(void) {
   return connected;
}

void close(varargs int force) {
   if (read_callback == "ftp_stor") {
      call_other(prev, "ftp_write");
   }
   connected = 0;
   destruct_object(this_object());
}

void start_connection(string ip, int port, int type) {
   secure();
   connect(ip, port);
   /* unguarded("connect", ip, port); */
}

void send_data(mixed data) {
   secure();
   prev = previous_object();
   send_message(data);
}

void set_read_callback(string func) {
   secure();
   prev = previous_object();
   read_callback = func;
}

void receive_message(string str) {
   call_other(prev, read_callback, str);
}

void terminate(void) {
   secure();
   unguarded("disconnect");
}

static void destructing(void) {
   if (connected && query_connection()) {
      query_connection()->set_mode(MODE_DISCONNECT);
   }
}

#endif
