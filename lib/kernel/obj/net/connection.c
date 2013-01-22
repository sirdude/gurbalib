/*
 * GurbaLib tcp connection object
 * Inspired by the DGD kernel lib and the Way of the Force kernel
 *
 * Feb 2009, Aidil@Way of the Force
 *
 */

#include <status.h>
#include <ports.h>

private object user;		/* our user object */
private int opened;
private int mode;		/* connection mode */
private int blocked;		/* connection blocked? */
private string buffer;		/* buffered output string */
private string protocol;	/* telnet or tcp */
int closing;

static void DEBUG(string str) {
#ifdef DEBUG_CONNECTION
   LOG_D->write_log("network",
      ctime(time())[4..] + " : " + dump_value(this_object()) + " : " + str +
      "\n");
#endif
}

static void _receive_error(mixed * tls, string err);

void set_user(object u) {
   if (previous_program() == M_CONNECTION) {
      user = u;
   }
}

object query_user() {
   return user;
}

void create() {
   buffer = nil;
   protocol = nil;
   user = nil;
   blocked = 0;
   mode = MODE_ECHO;
   DEBUG("Created new connection object");
}

void set_mode(int m) {
   if (m != mode && m != MODE_NOCHANGE) {
      if (m == MODE_DISCONNECT) {
	 if (sizeof(users() & ( { this_object() } ) )) {
#ifdef SYS_NETWORKING
	    catch {
	       close_user();
	    }
	    call_out("remove_me", 0);
#else
	    destruct_object(this_object());
#endif
	 } else {
	    destruct_object(this_object());
	 }
      } else if (m >= MODE_UNBLOCK) {
	 if (m - MODE_UNBLOCK != blocked) {
	    block_input(blocked = m - MODE_UNBLOCK);
	 }
      } else {
	 if (blocked) {
	    block_input(blocked = FALSE);
	 }
	 mode = m;
      }
   }
}

int query_mode() {
   return (blocked) ? MODE_BLOCK : mode;
}

void set_protocol(string proto) {
   if (protocol) {
      return;
   } else {
      switch (proto) {
	 case "telnet":
	    protocol = proto;
	    set_mode(MODE_UNBLOCK | MODE_ECHO);
	    break;
	 case "tcp":
	    protocol = proto;
	    set_mode(MODE_UNBLOCK | MODE_RAW);
	    break;
	 default:
	    error("Unknown protocol");
	    break;
      }
   }
}

static void _open(mixed * tls) {
   opened = 1;
   if (user)
      user->open();
   DEBUG("Connection established");
}

void open() {
   _open(DRIVER->new_tls());
}

static void _close(mixed * tls, varargs int force) {
   if (closing++) {
      /* error("recursive call to close()"); */
   }

   DEBUG("Connection closed");

   rlimits(MAX_DEPTH; MAX_TICKS) {
      if (user) {
	 user->close(force);
      }
      if (!force) {
	 call_out("remove_me", 0);
      }
   }
   opened = 0;
}

static void remove_me() {
   destruct_object(this_object());
}

static void close(varargs int force) {
   _close(DRIVER->new_tls(), force);
}

void connect(string ip, int port, varargs string proto) {
   if (previous_object() == user) {
      if (!proto)
	 proto = "tcp";
#ifndef SYS_NETWORKING
      if (proto != "tcp")
	 error("Unsupported protocol");
#endif

      set_protocol(proto);

      if (protocol) {
	 DEBUG("Making outbound " + protocol + " connection to : " + ip + ", " +
	    port);
	 catch {
#ifdef SYS_NETWORKING
	    ::connect(ip, port, proto);
#else
	    ::connect(ip, port);
#endif
	 } : {
	    _receive_error(nil, caught_error());
	 }
      }
   }
}

int message(string str) {
   int len;

   if (previous_object() != user) {
      error("Illegal call to connection object");
   }

   buffer = nil;
   len = send_message(str);
   if (len != strlen(str)) {
      buffer = str[len..];
      return 0;
   } else {
      return 1;
   }
}

static void _message_done(mixed * tls) {
   if (buffer) {
      send_message(buffer);
      buffer = nil;
   } else if (user) {
      set_mode(MODE_NOCHANGE);
   }
   rlimits(MAX_DEPTH; MAX_TICKS) {
      if (user)
	 user->message_done();
   }
}

static void message_done() {
   _message_done(DRIVER->new_tls());
}

static void _receive_error(mixed * tls, string err) {
   DEBUG(err);
   if (user) {
      catch {
	 user->receive_error(err);
      } : {
      }
   }

   if (!opened) {
      destruct_object(this_object());
   }
}

static void receive_error(string err) {
   _receive_error(DRIVER->new_tls(), err);
}

#ifndef SYS_NETWORKING

static void unconnected(int refused) {
   _receive_error(DRIVER->new_tls(),
		  (refused) ? "Connection refused" : "Connection failed");
}

#endif

static void _receive_message(mixed * tls, string str) {
   if (user) {
      rlimits(MAX_DEPTH; MAX_TICKS) {
	 user->receive_message(str);
      }
   } else {
      /*
       * Our user is gone, we have no reason to be here either.
       */
      destruct_object(this_object());
   }
}

static void receive_message(string str) {
   _receive_message(DRIVER->new_tls(), str);
}

static void destructing() {
   DEBUG("Destructing connection object");
}
