#include "../domain.h"
inherit "/std/room";

void setup(void) {
   add_area("2.4.5");

   set_short("The adventures guild");
   set_long("You can change your title here with the 'advance' command.  " +
      "There is an opening to the south, and some shimmering " +
	  "blue light in the doorway.  " +
      "There is also a small note nailed to the wall.");
   add_exit("north", DIR + "/rooms/vill_road2.c");
   add_exit("south", "/domains/required/rooms/bug_room.c");

   set_objects(DIR + "/obj/book.c");

   add_item("chain", "The chain is secured to the wall.\n");
   add_item("note", "If your looking for adventure, Retrieve the Orc " +
      "slayer from the evil orc shaman, and give it to Leo.");
   add_action("do_title", "advance");
}

/* If you look at the original, there was a bunch of stuff here for
   leveling up, however were doing it differently so does not apply.
   You may want to look at the old code for more info... 
   Instead we let you set your title.
*/

/* XXX Should expand this and make this a guild like the fighters guild. */

int do_title (string str) {
   int level;

   level = this_player()->query_level();
   switch (level) {
      case 1:
	  	this_player()->set_title("$N the simple novice");
		write("You are awarded the title: the simple novice.");
		break;
      case 2:
	  	this_player()->set_title("$N the low rank novice");
		write("You are awarded the title: the low rank novice.");
		break;
      case 3:
	  	this_player()->set_title("$N the medium beginner");
		write("You are awarded the title: the medium beginner.");
		break;
      case 4:
	  	this_player()->set_title("$N the small adventurer");
		write("You are awarded the title: the small adventurer.");
		break;
      case 5:
	  	this_player()->set_title("$N the experienced adventurer");
		write("You are awarded the title: the experienced adventurer.");
		break;
      case 6:
	  	this_player()->set_title("$N the expert adventurer");
		write("You are awarded the title: the expert adventurer.");
		break;
      case 7:
	  	this_player()->set_title("$N the great adventurer");
		write("You are awarded the title: the great adventurer.");
		break;
      case 8:
	  	this_player()->set_title("$N the myrmidon");
		write("You are awarded the title: the myrmidon.");
		break;
      case 9:
	  	this_player()->set_title("$N the champion");
		write("You are awarded the title: the champion.");
		break;
      case 10:
	  	this_player()->set_title("$N the great champion");
		write("You are awarded the title: the great champion.");
		break;
      case 11:
	  	this_player()->set_title("$N the apprentice magician");
		write("You are awarded the title: the apprentice magician.");
		break;
      case 12:
	  	this_player()->set_title("$N the journeyman magician");
		write("You are awarded the title: the journeyman magician.");
		break;
      case 13:
	  	this_player()->set_title("$N the master magician");
		write("You are awarded the title: the master magician.");
		break;
      case 14:
	  	this_player()->set_title("$N the lesser sorcerer");
		write("You are awarded the title: the lesser sorcerer.");
		break;
      case 15:
	  	this_player()->set_title("$N the sorcerer");
		write("You are awarded the title: the sorcerer.");
		break;
      case 16:
	  	this_player()->set_title("$N the master sorcerer");
		write("You are awarded the title: the master sorcerer.");
		break;
      case 17:
	  	this_player()->set_title("$N the necromancer");
		write("You are awarded the title: the necromancer.");
		break;
      case 18:
	  	this_player()->set_title("$N the great necromancer");
		write("You are awarded the title: the great necromancer.");
		break;
      case 19:
	  	this_player()->set_title("$N the apprentice Wizard");
		write("You are awarded the title: the apprentice Wizard.");
		break;
      case 20:
	  	this_player()->set_title("$N the Wizard");
		write("You are awarded the title: the Wizard.");
		break;
	  default:
		write("You are unable to set your title at this time.");
		break;
   }
   say(this_player()->query_Name() + " selects a title.");
   return 1;
}

