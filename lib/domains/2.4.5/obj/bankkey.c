#include "../domain.h"

inherit "/std/object";

void setup(void) {
   set_id("gurba bank key");
   add_id("key");
   set_short("A bank key");
   set_long("A large well worn key.");
   set_gettable(1);

   set_value(8);
   set_weight(2);
}

string query_key_code() {
   return "bankvault01";       
}
