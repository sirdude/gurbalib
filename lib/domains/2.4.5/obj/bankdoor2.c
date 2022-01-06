inherit "/std/lockable_door";

void setup(void) {
   set_id("door");
   set_adj("super-thick");
   set_short("A super-thick door");
   set_long("The door looks very strong.");
   mudlib_setup(({ "east", "bankvalutdoor" }));
   add_block("east");
   set_lock_code("bankvault01");
}
