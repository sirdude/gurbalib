/*
 * System and game init
 *
 * Reads /sys/data/init
 * This file should contain a list of objects 
 * that have to be loaded at startup.
 */

#define INIT_LIST "/sys/data/init"

/* Returns a default init list. */
private string *default_init() {
   string *objects;

   objects = ( { SCHEDULE_D, CHANNEL_D, TIME_D, USER_D, IMUD_D } );

#ifdef SYS_NETWORKING
   objects += ( {
       TELNET_D,
#ifndef DISABLE_FTP
        FTP_D,
#endif
   } );
#endif

   objects += ( {
      OBJECT_D,
      RACE_D,
      ANSI_D,
      SKILL_D,
      MONEY_D,
      BANISH_D, 
      SITEBAN_D, 
      GUILD_D, 
      LANGUAGE_D, 
      PARSE_D, 
      STARTING_ROOM
   } );
   return objects;
}

/*
 * Get the list of required objects.
 * If it exists, add the contents of the init list file
 * (/sys/data/init) to it.
 * Load all listed objects.
 *
 */
static void create() {
   string *objects;
   int count;
   int size;

   objects = default_init();

   if (file_exists(INIT_LIST)) {
      objects += explode(read_file(INIT_LIST), "\n");
   }

   if (!objects) {
      return;
   }

   rlimits(MAX_DEPTH; -1) {
      for (count = 0, size = sizeof(objects); count < size; count++) {
	 object ob;

	 if (!(ob = find_object(objects[count]))) {
	    rlimits(MAX_DEPTH; MAX_TICKS) {
	       call_other(objects[count], "???");
	    }
	 }
      }
   }
}
