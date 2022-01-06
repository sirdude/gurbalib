inherit "/std/object";

void setup(void) {
   set_id("rusty key");
   add_id("key");
   set_short("A rusty old key");
   set_long("A large well worn key.");
   set_gettable(1);

   set_value(5);
   set_weight(2);
}

string query_key_code(void) {
	return "Bingo#";
}
