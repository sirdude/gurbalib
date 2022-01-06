#include "lockable.h"
inherit "/std/door";
inherit "/std/modules/m_lockable";

void create(void) {
   ::create();
   update_lock_state(M_LOCKABLE_LOCKED);
}

string query_cannot_unlock_message(object who, varargs int from_do_unlock) {
   return "$N $vtry to unlock $o but $vneed a key.";
}

string query_cannot_lock_message(object who, varargs int from_do_lock) {
   return "$N $vtry to unlock $o but $vneed a key.";
}

string query_cannot_open_message(object who) {
   return "$N $vtry to open $o but it's locked.";
}

int can_open(object who) {
   return ::can_open(who) && query_lock_state() != M_LOCKABLE_LOCKED;
}

void do_on_lock(object who) {
   if (!sibling) {
      refresh_sibling();
   }

   if (sibling) {
      sibling->set_lock_state(this_object()->query_lock_state());
      sibling->simple_action("$N $vlock.");
   }
}

void do_on_unlock(object who) {
   if (!sibling) {
      refresh_sibling();
   }

   if (sibling) {
      sibling->set_lock_state(this_object()->query_lock_state());
      sibling->simple_action("$N $vunlock.");
   }
}
