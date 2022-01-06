inherit "/std/lockable_door";

void setup(void) {
   set_id("door");
   set_adj("sturdy");
   set_short("A sturdy iron door");
   set_long("This thing is solid, your not getting through it without " +
      "the key.  What kind of thing needs a heavy door like this?");
   door_setup("east", "ironminedoor");
   add_block("east");
   set_lock_code("Bingo#");
}
