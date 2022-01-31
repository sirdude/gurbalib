#include "../domain.h"

inherit "/std/object";

void setup(void) {
   set_id("gear");
   add_id("climbing gear");
   add_adj("climbing");
   set_short("A set of climbing gear");
   set_long("A set of high quality, professional climbing gear. " +
      "This set will be useful to you when trying to climb cliffs " +
      "and some other sheer surfaces.");
   set_gettable(1);
   set_weight(2);
   set_value(200);
}
