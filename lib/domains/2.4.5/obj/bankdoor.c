inherit "/std/lockable_door";

void setup(void) {
   set_id("door");
   set_adj("super-thick");
   set_short("A super-thick door");
   set_long("The door looks very strong.");
   mudlib_setup(({ "west", "bankvalutdoor" }));
   add_block("west");
   set_lock_code("bankvault01");
}
