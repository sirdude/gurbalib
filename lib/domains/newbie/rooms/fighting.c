inherit ROOM;

void setup( void ) {
  add_area( "newbie" );

  set_brief( "%^MAGENTA%^newbie arena.%^RESET%^" );
  set_long( "You are in a small tunnel.  It was a tight squeeze to get in " +
     "here.  There is a large flower in the middle of the room.");

  add_item( "flower", "A Large flower sits in the middle of the room.  " +
     "Perhapse you should pick it.");

  add_room_command("pick", "do_pick");

  set_exits( ([ 
	       "north" : "/domains/newbie/rooms/equipment",
	       "south" : "/domains/newbie/rooms/quest",
  ]) );
}

void do_pick(string arg) {
   if (arg == "flower") {
      object obj;
 
      write("As you reach for the flower, a giant bunny rabbit appears, " +
         "blocking your way.\n");
      this_player()->query_environment()->tell_room(this_player(),
         capitalize(this_player()->query_name()) + 
         " reaches for the flower in the center of the room.\n" +
         "All of a sudden a giant bunny rabbit appears blocking their path.\n");
      
      obj = clone_object("/domains/newbie/monsters/bunny.c");
      obj->setup();
      obj->move(this_player()->query_environment());
      obj->attack(this_player());
   } else {
      write("Pick what?\n");
   }
}

