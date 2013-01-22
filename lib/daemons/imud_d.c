/*
  A very basic Intermud-3 daemon for those MUDs running on the DGD driver
  with network support.  It is not meant to run out of the box, but rather
  to show how to receive and send data over the Intermud network using
  DGD's network capabilities.

  Originally created by Jason Cone (Qi@CornerStone)
*/

#include <status.h>
#include <limits.h>
#include <ports.h>

inherit M_CONNECTION;
inherit M_SERIALIZE;

#ifndef RECONNECT_INTERVAL
#define RECONNECT_INTERVAL 30
#endif

#ifndef KEEPALIVE_INTERVAL
#define KEEPALIVE_INTERVAL 60
#endif

#define DATA_VERSION 5
#define MAX_ERRCOUNT 3

/*
 * Define this to use the routerlist provided by the I3 router.
 * Note that this does not work on the *yatmim/*i4/*wpr routers,
 * but it does for example on the WOTF test routers. Enabling this
 * makes the client follow the I3 specification, but will break
 * automatic switching of routers on the public I3 network.
 */
#undef BELIEVE_ROUTERLIST

mixed *mpRouterList;
static private mapping aServices;
static int connected;
int password;
mapping chanlist;
int chanlist_id;
mapping mudlist;
int mudlist_id;
int enabled;
int current_router;
int last_good_router;
int data_version;
string default_router;

static string buffer;
static int packet_len;
static int errcount;
static int pingtime;
static int keepalive_handle;
static int reconnect_handle;

void create(void);

void IMUDLOG(string str) {
   LOG_D->write_log("imud", ctime(time()) + " : " + str);
}

static void save_me(void) {
   unguarded("save_object", "/daemons/data/imud_d.o");
}

static void restore_me(void) {
   unguarded("restore_object", "/daemons/data/imud_d.o");
}

private void write_imud_stream(string sType, mixed sTargetMUD,
   mixed sTargetUser, mixed * mpMessage) {
   mixed sOrigUser;
   string sSend;
   int nSize;
   mixed *mpPacket;

   if (!stringp(sTargetMUD)) {
      sTargetMUD = 0;
   }

   if (!stringp(sTargetUser)) {
      sTargetUser = 0;
   }

   if (this_user()) {
      if (this_user()->query_player()) {
	 sOrigUser = this_user()->query_player()->query_name();
      } else {
	 sOrigUser = 0;
      }
   }

   /*  The first 6 entries into the outgoing array will always be the
      same. 
    */

   mpPacket = ( {
	 sType,			/*The packet type.  Should correspond to one of the services */
	 5,			/*Always 5. */
	 IMUD_NAME,		/*Name of the MUD as it should be known over the I-3 network */
	 sOrigUser,		/*Name of the local user involved in this transaction */
	 sTargetMUD,		/*Name of the target MUD */
	 sTargetUser,		/*Name of the target user on the target MUD */
      }
   ) + mpMessage;		/*Service-specific information */

/*
  First, I convert the array to a string.  I am currently using a daemon
  to do this though I may eventually make it an afun.  Anyway, since I'm
  passing a 1 to the function via to_string(mpPacket, 1) that's telling
  the daemon to leave out the "|<number of elements>" string in the
  array/mapping translation as it is unique to DGD values.  I then get
  the size of the string and add 1 for the null character that is appended
  to the end.
*/

   nSize = strlen(sSend = serialize("MUDMODE", mpPacket)) + 1;

   /*  Initialize the string so I can reference specific bytes */
   sSend = "    " + sSend + " ";

   /*  Convert the length of the packet to a byte string */
   sSend[0] = (nSize & 0xff000000) >> 24;
   sSend[1] = (nSize & 0xff0000) >> 16;
   sSend[2] = (nSize & 0xff00) >> 8;
   sSend[3] = nSize & 0xff;

   /*  Append the null character */
   sSend[strlen(sSend) - 1] = 0;

   /*  Send it to the router */
   if (catch(send_message(sSend)))
      IMUDLOG("There was an error writing the data to the Intermud connection");
}

/*
  **NOTE**
  The following group of functions allow for easier sending of data over
  the I-3 network according to the nature of the action.
  The logic for this setup was borrowed from the LIMA MUDlib.
*/

/*  Use this function to send a packet to the router */
static void send_to_router(string sType, mixed * mpMessage) {
   write_imud_stream(sType, mpRouterList[current_router][0], 0, mpMessage);
}

/*  Use this function to send a packet to a certain MUD */
static void send_to_mud(string sType, string sMudName, mixed * mpMessage) {
   write_imud_stream(sType, sMudName, 0, mpMessage);
}

/*  Use this function to send a packet to a certain MUD -and- direct */
/*  the action to a specific user. */
static void send_to_user(string sType, string sMudName, string sUserName,
   mixed * mpMessage) {
   write_imud_stream(sType, sMudName, sUserName, mpMessage);
}

/*  Use this function to send a packet to all the MUDs connected to the */
/*  I-3 network. */
static void send_to_all(string sType, mixed * mpMessage) {
   write_imud_stream(sType, 0, 0, mpMessage);
}

void do_channel_m(string chan, string what) {

   send_to_all("channel-m", ( { chan, this_player()->query_Name(), what } ) );
}

void do_channel_e(string chan, string what) {

   send_to_all("channel-e", ( { chan, this_player()->query_Name(), what } ) );
}

void do_tell(string who, string where, string what) {
   string s;

   if (catch(s = this_player()->query_Name()))
      s = object_name(this_object());	/* if not a player sending the tell, identify ourself */

   send_to_user("tell", where, who, ( { s, what } ) );
}

void do_who(string where) {
   send_to_mud("who-req", where, ( { } ) );
}

void do_locate(string who) {
   send_to_all("locate-req", ( { who } ) );
}

/*
  When a whole packet has been received, call the associated function
  for the given function.  If none exists, send an error packet back
  to the sender saying that we don't support that service.
*/
private void handle_router_read(mixed * mpMessage) {
/*
  If you don't have support for this service, return an error.  You
  shouldn't receive packets for unsupported services if you didn't
  define them in your startup-request-3 packet.
  Don't respond to broadcasts with an error.
*/

   if (!mpMessage || sizeof(mpMessage) == 0) {
      IMUDLOG("Empty packet!");
      return;
   }

   if (!aServices[mpMessage[0]]) {
      if (mpMessage[4] != 0) {
	 write_imud_stream("error", mpMessage[2], mpMessage[3], ( {
	       "unk-type",
               "Packet-type '" + mpMessage[0] + "' is not defined",
		  mpMessage
         } ) );
      }
      return;
   }

   switch (mpMessage[0]) {
      case "auth-mud-req":
      case "oob-req":
      case "who-req":
      case "ping":
      case "ping-req":
      case "chan-who-req":
      case "chan-user-req":
	 if (mpMessage[4] == 0) {
	    write_imud_stream("error", mpMessage[2], mpMessage[3], ( {
		  "bad-pkt",
		     "Broadcasted " + mpMessage[0] +
		     " packets are not accepted here.", mpMessage
            }) );
	    IMUDLOG("Denied broadcast " + mpMessage[0] + " from " +
	       mpMessage[3] + "@" + mpMessage[2] + "\n");
	    return;
	 }
	 break;
      default:
	 /* allow the packet to pass through */
	 break;
   }

/*
  This call assumes that you've either #include'ed or inherited the
  various services into this object.  If not, you'll have to modify
  this call to reflect your setup.
*/
   call_other(this_object(), aServices[mpMessage[0]],
      mpMessage[2], mpMessage[3], mpMessage[5], mpMessage[6..]);
}

void rcv_locate_reply(string origmud, mixed origuser, string destuser,
   mixed rest) {
   object p;

   p = USER_D->find_player(destuser);
   if (p) {
      p->message("%^ORANGE%^[locate]%^RESET%^ " + rest[1] +
	 " has been found on " + rest[0] +
	 " (idle" + format_idle_time(rest[2]) + ").\n");
   }
}

void rcv_locate_req(string origmud, mixed origuser, mixed destuser, mixed rest) {
   object u;

   if (!stringp(rest[0]))
      return;
   u = USER_D->find_player(lowercase(rest[0]));
   if (u) {
      send_to_user("locate-reply", origmud, origuser, ( {
	    IMUD_NAME, u->query_Name(), u->query_idle(), 0
      } ) );
   }
}

void rcv_who_reply(string origmud, mixed origuser, mixed destuser, mixed rest) {
   object p;
   int i;

   if (!stringp(destuser))
      return;

   p = USER_D->find_player(destuser);
   if (p) {
      p->message("[" + origmud + "] " + sizeof(rest[0]) +
	 " users connected.\n");
      p->message("-----------------------------------------------------------------------------\n");
      for (i = 0; i < sizeof(rest[0]); i++) {
	 p->message(rest[0][i][0] + " " + format_idle_time(rest[0][i][1]) +
	    " " + rest[0][i][2] + "\n");
      }
      p->message("-----------------------------------------------------------------------------\n");
   }
}

void rcv_who_req(string origmud, mixed origuser, mixed destuser, mixed rest) {

   mixed *msg;
   object *p;
   int i;

   msg = ( {  } );

   p = USER_D->query_users();

   for (i = 0; i < sizeof(p); i++) {
      msg += ( {
	    ( {
		  p[i]->query_player()->query_Name(),
		  p[i]->query_player()->query_idle(),
		  p[i]->query_player()->query_title()
	     } ) 
      } );
   }
   send_to_user("who-reply", origmud, origuser, ( { msg } ) );
   IMUDLOG("Replied to a who-request by " + origuser + "@" + origmud + ".\n");
}

void rcv_channel_t(string origmud, mixed origuser, mixed destuser, mixed rest) {
   CHANNEL_D->chan_imud_rcv_targetted(rest[0], rest[5], origmud, rest[6],
      rest[1], rest[3]);
}

void rcv_channel_e(string origmud, mixed origuser, mixed destuser, mixed rest) {
   CHANNEL_D->chan_imud_rcv_emote(rest[0], rest[1], origmud, rest[2]);
}

void rcv_channel_m(string origmud, mixed origuser, mixed destuser, mixed rest) {
   CHANNEL_D->chan_imud_rcv_say(rest[0], rest[1], origmud, rest[2]);
}

void rcv_tell(string origmud, mixed origuser, mixed destuser, mixed rest) {
   object user;

   IMUDLOG("Got tell from " + origuser + "@" + origmud + " to " + destuser +
      ":" + rest[1] + ".\n");

   if (!destuser)
      return;
   user = USER_D->find_player(destuser);
   if (user && !user->query_ignored(origuser + "@" + origmud)) {
      user->set_last_tell(origuser + "@" + origmud);
      user->message("%^PLAYER%^" + rest[0] + "@" + origmud +
	 "%^RESET%^ tells you: %^TELL_FROM%^" + rest[1] + "%^RESET%^");
   } else {
      write_imud_stream("error", origmud, origuser, ( {
	    "unk-user",
	       "You can't seem to reach " + destuser + " on " + IMUD_NAME, 0
      } ) );
   }
}

void rcv_chanlist_reply(string origmud, mixed origuser, mixed destuser,
   mixed rest) {
   string *chans;
   int i;

   if (!chanlist) {
      chanlist = ([]);
   }

   chanlist_id = rest[0];

   /*  chans = map_indices( chanlist );
      for( i = 0; i < sizeof( chans ); i++ ) {
      if( rest[1][chans[i]] == 0 ) {
      chanlist[chans[i]] = 0;
      }
      }
    */
   chans = map_indices(rest[1]);
   for (i = 0; i < sizeof(chans); i++) {
      if (rest[1][chans[i]] == 0) {
	 chanlist[chans[i]] = nil;
	 IMUDLOG("Removed channel: " + chans[i] + "\n");
      } else {
	 chanlist[chans[i]] = rest[1][chans[i]];
	 IMUDLOG("Added channel: " + chans[i] + "\n");
	 send_to_router("channel-listen", ( { chans[i], 1 } ) );
      }
   }
   save_me();
}

void rcv_mudlist(string origmud, mixed origuser, mixed destuser, mixed rest) {
   string *muds;
   int i;

   if (!mudlist) {
      mudlist = ([]);
   }

   mudlist_id = rest[0];

/*  muds = map_indices( mudlist ); */

   /*  for( i = 0; i < sizeof( muds ); i++ ) {
      if( rest[1][muds[i]] == 0 ) {
      mudlist[muds[i]] = 0;
      }
      }
    */
   muds = map_indices(rest[1]);

   for (i = 0; i < sizeof(muds); i++) {
      if (rest[1][muds[i]] == 0) {
	 IMUDLOG("Removing mud: " + muds[i] + ", " + (map_sizeof(mudlist) - 1) +
	    " muds total\n");
	 mudlist[muds[i]] = nil;
      } else {
	 if (!mudlist[muds[i]]) {
	    IMUDLOG("Adding mud " + muds[i] + ", " + (map_sizeof(mudlist) + 1) +
	       " muds total\n");
	 } else {
	    IMUDLOG("Changing mud " + muds[i] + "\n");
	 }
	 mudlist[muds[i]] = rest[1][muds[i]];
      }
   }
   save_me();
}

void rcv_startup_reply(string origmud, mixed origuser, mixed destuser,
   mixed rest) {

#ifdef BELIEVE_ROUTERLIST
   int i;
   string rname;

   rname = mpRouterList[current_router][0];
   mpRouterList = rest[0];

#endif

   if (connected) {
      IMUDLOG("Router requested disconnect.\n");

#ifdef BELIEVE_ROUTERLIST

      errcount = 0;
      current_router = 0;
      mudlist = ([]);
      chanlist = ([]);
      mudlist_id = 0;
      chanlist_id = 0;

#endif

      disconnect();
      connected = 0;
   } else {
      connected = 1;

#ifdef BELIEVE_ROUTERLIST

      current_router = -1;

      for (i = 0; i < sizeof(mpRouterList); i++) {
	 if (mpRouterList[i][0] == rname) {
	    current_router = i;
	 }
      }
      if (current_router < 0) {
	 disconnect();
	 current_router++;
	 error("Could not find the current router in the received routerlist. Maybe you have BELIEVE_ROUTERLIST defined on a router that doesn't support this properly");
      }
#endif
      last_good_router = current_router;
      password = rest[1];

      save_me();
      IMUDLOG("I3 startup reply received.\n");
   }
}

void rcv_error(string origmud, mixed origuser, mixed destuser, mixed rest) {
   object user;

   IMUDLOG("Got an error packet! " + (rest[0] ? rest[0] : "<none>") + " : " +
      (rest[1] ? rest[1] : "<none>") + "\n");
   if (stringp(destuser)) {
      user = USER_D->find_player(destuser);
      if (!user)
	 return;

      user->message("%^RED%^Intermud error " + (rest[0] ? ("'" + rest[0] +
	       "'") : "<no error>") + " received from " + origmud +
	 "%^RESET%^\n" + "%^CYAN%^" + (rest[1] ? rest[1] : "<no message>") +
	 "%^RESET%^\n");
   }
}

void rcv_auth_mud_req(string origmud, mixed origuser, mixed destuser,
   mixed rest) {
   if (origmud == IMUD_NAME) {
      pingtime = time();
      /* IMUDLOG("keepalive ok\n");  */
   }
}

mapping query_mudlist(void) {
   return (mudlist);
}

mapping query_chanlist(void) {
   return (chanlist);
}

static string sBuffer;

/*
  This is where we parse the incoming data from the socket connection.
  The basic logic flow is this:

    * Append the data to the buffer string until a null character at
      the end of an array is reached (denoted end of packet).

    * Because it's possible to receive more than one packet in a stream,
      I keep an array of packets parsed on the "})\0" sequence (unique
      to the end of a packet).

    * Strip the first 4 bytes (packet length info) of each complete
      packet and append the trailing "})" that was lost in the explode()
      operation.

    * Convert each packet to a value using the convert daemon and pass
      the resulting value to handle_router_read() for evaluation.

  I started out trying to use the packet length information to decide
  when I had received the whole packet, but it made for a mess and
  nearly tripled the amount of code needed to parse the string.  It
  didn't work consistently, either.  So until I'm shown that this method
  won't work, I'm going to keep on using it (parsing on the null
  character). :)

  Well, it did break, so here is a new implementation, based on the one from
  Way of the Force. It collects a complete packet before trying anything.

  Aidil
*/

void receive_message(string str) {
   /*
    * If we had no input, return here;
    */
   if (str == nil) {
      return;
   }

   /*
    * Initialize the buffer if it doesn't exist yet.
    */
   if (!buffer) {
      buffer = "";
      packet_len = 0;		/* We had no data so reset this as well */
   }

   /*
    * Decode the packets, note that sending each seperate packet to the
    * user object is handled by send_data which runs with its own
    * rlimits
    */
   rlimits(2048; -1) {
      string packet;
      int size;

      /*
       * Make sure the combined input doesn't exceed our max string size.
       * This also serves to resync the connection after a while in case
       * we failed to detect the end of the previous package.
       * Buffer and expected packet size are reset here.
       */
      if (strlen(buffer) + strlen(str) > MAX_STRING_SIZE) {
	 buffer = "";
	 packet_len = 0;
	 errcount++;
	 return;
      }

      /*
       * add the input data to our receive buffer.
       */
      buffer += str;
      size = strlen(buffer);
      /* Do we have the 4 bytes for the packet size and possibly the entire packet? */
      while (size >= packet_len + 4) {
	 packet_len =
	    buffer[3] + (buffer[2] << 8) + (buffer[1] << 16) +
	    (buffer[0] << 24);

	 /* Do we really have a complete packet? */
	 if (packet_len + 4 <= size) {
	    /*
	     * extract packet from the buffer, excluding the size and
	     * trailing 0
	     */
	    packet = buffer[4..packet_len + 2];
	    /*
	     * remove current packet from the buffer
	     * note that we skip the trailing zero
	     * from the previous packet here.
	     */
	    if (size > packet_len + 4) {
	       buffer = buffer[packet_len + 4..];
	    } else {
	       buffer = "";
	    }
	    /*
	     * reset size to the current length of the buffer
	     */
	    size = strlen(buffer);
	    /*
	     * set packet length to zero, will be set to something more useful during next
	     * iteration of this loop, provided there are at least 4 bytes in the buffer.
	     */
	    packet_len = 0;
	    /*
	     * restore the variable in the packet and send it to the user object.
	     */
	    rlimits(2048; -1) {
	       mixed data;
	       string rc;

	       rc = catch(data = deserialize("MUDMODE", packet));
	       if (!rc) {
		  errcount = 0;
		  handle_router_read(data);
	       } else {
		  errcount++;
	       }
	    }
	 }
      }
   }
}

/*
  This is called when the connection is lost, closed, or when the daemon
  itself is destructed.  Possible code that could go in here could
  include, but is not limited to, automatic reconnecting, save current
  settings, notify a group that may be logged in, log event to a file,
  etc.
*/
int close(varargs int force) {
   connected = 0;
   IMUDLOG("Connection closed.\n");
   if (enabled && !reconnect_handle)
      reconnect_handle = call_out("reconnect", RECONNECT_INTERVAL);
   event("i3_connection",
      "I3 connection to " + mpRouterList[current_router][0] + " lost");
   return connected == 0;
}

void use_default_router() {
   int i, sz;

   current_router = 0;

   if (default_router != "*") {
      for (i = 0, sz = sizeof(mpRouterList); i < sz; i++) {
	 if (default_router == mpRouterList[i][0]) {
	    current_router = i;
	    break;
	 }
      }
   }

   if (current_router != last_good_router) {
      mudlist = ([]);
      chanlist = ([]);
      mudlist_id = 0;
      chanlist_id = 0;
   }

   save_me();
}

void use_next_router() {
   current_router++;

   if (current_router >= sizeof(mpRouterList)) {
      current_router = 0;
   }

   if (current_router != last_good_router) {
      mudlist = ([]);
      chanlist = ([]);
      mudlist_id = 0;
      chanlist_id = 0;
   }
   save_me();
}

void reconnect(void) {
   string *spAddress;

   reconnect_handle = 0;

   if (!enabled)
      return;

   if (query_connection()) {
      DRIVER->
	 message("reconnect called but we still have a connection object!\n");
      destruct_object(query_connection());
   }

   if (errcount > MAX_ERRCOUNT) {
      use_next_router();
   }

   IMUDLOG("Router: " + mpRouterList[current_router][1] + "\n");

   if (connected == 0) {
      spAddress = explode(mpRouterList[current_router][1], " ");
      IMUDLOG("Reconnecting to " + spAddress[0] + " " + spAddress[1] + "\n");
      connect(spAddress[0], (int)spAddress[1]);
   }
}

int query_mudlist_id(void) {
   return (mudlist_id);
}
int query_chanlist_id(void) {
   return (chanlist_id);
}

void keepalive(void) {
   keepalive_handle = call_out("keepalive", KEEPALIVE_INTERVAL);

   /* IMUDLOG("keepalive\n"); */
   if (pingtime && (time() - pingtime >= 180)) {
      if (query_connection()) {
	 IMUDLOG("Keepalive timeout (last keepalive received at " +
	    ctime(pingtime) + "), disconnecting\n");
	 disconnect();
      } else {
	 connected = 0;
	 IMUDLOG("Connection lost.\n");
	 if (enabled && !reconnect_handle)
	    reconnect_handle = call_out("reconnect", RECONNECT_INTERVAL);
      }
   } else {
      /* send an auth packet to ourselves once a minute to try to keep us connected. */
      /* IMUDLOG("Last keepalive received at "+ctime(pingtime)+".\n"); */
      send_to_mud("auth-mud-req", IMUD_NAME, ( { 0 } ) );
   }
}

void open() {
   pingtime = time();
   errcount = 0;

   send_to_router("startup-req-3", ( {
	    password,		/* Password - Send 0 if you're new */
	    query_mudlist_id(),	/* Mudlist ID - Send 0 if you're new */
	    query_chanlist_id(),	/* Channel List ID - Send 0 if you're new */
	    LOGIN_PORT,		/* What port people use to connect to your MUD */
	    I3_TCP_PORT,	/* The MUD's designated port for Imud TCP comm */
	    I3_UDP_PORT,	/* The MUD's designated port for Imud UDP/OOB comm */
	    LIB_NAME + " " + LIB_VERSION,	/*Name of current lib */
	    LIB_NAME,		/*Name of base lib */
	    status()[ST_VERSION],	/*Driver version */
	    "LPMud",		/*MUD Type */
	    "Mudlib Development",	/*MUD Status */
	    ADMIN_EMAIL,	/*Email address of the MUD's admin */
	    /*The following is a list of SUPPORTED services with appropriate values.
	       Information regarding the various services can be found in the I-3
	       specification.
	     */
 (["auth": 1, "channel":1,
		  /*      "emoteto" : 1, */
		  /*      "file"    : 1, */
		  /*      "finger"  : 1, */
 "ftp": FTP_PORT, "locate":1,
		  /*      "mail"    : 1, */
 "tell": 1, "who":1,
		  /*      "ucache"  : 1, */
	       ]),
	    /*This is a list of custom services that are support by your MUD, but
	       not by the I-3 protocol.
	     */
	 ([]),}));

   sBuffer = "";
   IMUDLOG("Connected to " + mpRouterList[current_router][0] + " : " +
      mpRouterList[current_router][1] + "\n");
   if (!keepalive_handle) {
      keepalive_handle = call_out("keepalive", KEEPALIVE_INTERVAL);
   }
   if (reconnect_handle) {
      remove_call_out(reconnect_handle);
      reconnect_handle = 0;
   }
   event("i3_connection",
      "I3 connection to " + mpRouterList[current_router][0] + " (" +
      mpRouterList[current_router][1] + ") is now up.");
}

/*
  create() is called when the daemon is first loaded (as is the case
  with every object).  Things of note that take place here:

    * Router list is given a default address and name

    * The Services mapping is initialized to reflect supported
      call-back functions for the respective services.

    * The connection to the Intermud router is initialized.
*/
void create(void) {
   string *spAddress;
   int old_router;

   if (IMUD_NAME == "GurbaLib") {
      "/kernel/sys/driver"->
	 message("Please edit kernel/include/mudname.h and change the MUD_NAME and IMUD_NAME defines\n");
      return;
   }

   /*
    * Make sure the mudmode parser/generator is loaded
    *
    */
   if (!find_object("/daemons/serialize/mudmode")) {
      compile_object("/daemons/serialize/mudmode");
   }

   add_event("i3_connection");

   if (find_object(CHANNEL_D)) {
      find_object(CHANNEL_D)->resubscribe();
   }
   password = 0;
   connected = 0;
   enabled = 1;
   mpRouterList = ( {
	 ( { "*wpr", "195.242.99.94 8080" } ), 
         ( { "*i4", "204.209.44.3 8080" } )
   } );

   mudlist = ([]);
   chanlist = ([]);
   mudlist_id = 0;
   chanlist_id = 0;

/*
    I went ahead and included this segment to show how I set up the packet-
    type/function relationships.  Each key is the string that is sent
    in each packet and the values are the names of the actual functions
    that are called when packets are received.

    You should only define those functions that you have coded support
    for, though, as defining unsupported ones will lead to router/mud
    miscommunications.
*/

   restore_me();

   if (!default_router) {
      default_router = mpRouterList[0][0];
      current_router = 0;
      mudlist = ([]);
      chanlist = ([]);
      mudlist_id = 0;
      chanlist_id = 0;
   }

   use_default_router();

   aServices = ([
      "tell" : "rcv_tell",
      "emoteto" : "rcv_emoteto",
      "who-req" : "rcv_who_req",
      "who-reply" : "rcv_who_reply",
      "finger-req" : "rcv_finger_req",
      "finger-reply" : "rcv_finger_reply", 
      "locate-req" : "rcv_locate_req",
      "locate-reply" : "rcv_locate_reply",
      "chanlist-reply" : "rcv_chanlist_reply",
      "chan-who-req" : "rcv_chan_who_req",
      "chan-who-reply" : "rcv_chan_who_reply",
      "channel-m" : "rcv_channel_m",
      "channel-e" : "rcv_channel_e",
      "channel-t": "rcv_channel_t",
      "chan-user-req" : "rcv_chan_user_req",
      "chan-user-reply" : "rcv_chan_user_reply",
      "auth-mud-req" : "rcv_auth_mud_req",
      "auth-mud-reply" : "rcv_auth_mud_reply",
      "ucache-update" : "rcv_ucache_update",
      "mudlist" : "rcv_mudlist",
      "startup-reply" : "rcv_startup_reply",
      "error" : "rcv_error",
      "oob-req" : "rcv_oob_req",
   ] );

   spAddress = explode(mpRouterList[current_router][1], " ");
   connect(spAddress[0], (int)spAddress[1]);
}

void receive_error(string err) {
   errcount++;
   if (!reconnect_handle) {
      reconnect_handle = call_out("reconnect", RECONNECT_INTERVAL);
   }
}

#ifndef SYS_NETWORKING
static void unconnected(int refused)
{
   receive_error((refused) ? "Connection refused" : "Connection failed");
}
#endif

int query_connected() {
   return connected;
}

void upgraded() {
   if (data_version != DATA_VERSION) {
      if (data_version < 5) {
	 mpRouterList = ( {
	       ( { "*wpr", "195.242.99.94 8080" } ), 
               ( { "*i4", "204.209.44.3 8080" } )
         } );
      }

      if (data_version < 2) {
	 remove_event("i3_connect");
	 remove_event("i3_disconnect");
	 add_event("i3_connection");
      }

      if (data_version < 1) {
	 enabled = 1;
      }

      data_version = DATA_VERSION;
      call_out("save_me", 2);
   }

   if (!find_object("/daemons/serialize/mudmode")) {
      compile_object("/daemons/serialize/mudmode");
   }
}

void enable_i3() {
   enabled = 1;
   if (!connected)
      reconnect();
}

void disable_i3() {
   enabled = 0;

   if (connected) {
      disconnect();
   }
   if (query_connection()) {
      destruct_object(query_connection());
   }
}

int query_enabled() {
   return enabled;
}

void destructing() {
   ::destructing();
}

int query_pingtime() {
   return pingtime;
}

void close_connection() {
   if (query_connection()) {
      disconnect();
   }
}

void set_default_router(string str) {
   default_router = str;
   if (query_connection()) {
      disconnect();
   }

   errcount = 0;
   use_default_router();
}

string query_default_router() {
   return default_router;
}

string query_current_router_name() {
   return mpRouterList[current_router][0];
}

string query_current_router_ip() {
   return mpRouterList[current_router][1];
}

void switch_router() {
   if (query_connection()) {
      disconnect();
   }

   use_next_router();
}
