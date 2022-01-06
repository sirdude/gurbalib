#include "lockable.h"

static int lock_state;
static string lock_id;
static string lock_code;

int is_lockable(void) {
   return TRUE;
}

int query_lock_state(void) {
   return lock_state;
}

int is_locked(void) {
   return query_lock_state() == M_LOCKABLE_LOCKED;
}

string query_unlocked_desc(void) {
   return "%^GREEN%^[unlocked]%^RESET%^";
}

string query_locked_desc(void) {
   return "%^RED%^[locked]%^RESET%^";
}

string query_short_lock_append(void) {
   switch (query_lock_state()) {
      case M_LOCKABLE_LOCKED:
         return " " + query_locked_desc();
      case M_LOCKABLE_UNLOCKED:
         return " " + query_unlocked_desc();
      default:
         return "";
   }
}

string *query_lock_codes(void) {
   return ({ "skeleton", lock_code });
}

string query_lock_id(void) {
   return lock_id;
}

void set_lock_state(int lock_state) {
   if (lock_state != M_LOCKABLE_UNLOCKED && lock_state != M_LOCKABLE_LOCKED) {
      lock_state = M_LOCKABLE_DISABLED;
   }

   ::lock_state = lock_state;
}

void set_lock_code(string lock_code) {
   ::lock_code = lock_code;
}

void set_lock_id(string lock_id) {
   ::lock_id = lock_id;
}

void update_lock_state(int lock_state) {
   set_lock_state(lock_state);

   switch (lock_state) {
      case M_LOCKABLE_LOCKED:
      case M_LOCKABLE_UNLOCKED:
         break;
      default:
         this_object()->simple_action("$N $vbecome inoperative.");
         set_lock_state(M_LOCKABLE_DISABLED);
         break;
   }
}

int has_correct_key(object who) {
   object *inv;
   int i, sz;

   inv = who->query_inventory();
   sz = sizeof(inv);
   for (i = 0; i < sz; i++) {
      if (member_array(inv[i]->query_key_code(), this_object()->query_lock_codes()) > -1) {
         return 1;
      }
   }

   return 0;
}

int can_unlock(object who) {
   return query_lock_state() == M_LOCKABLE_LOCKED && has_correct_key(who) ? TRUE : FALSE;
}

int can_lock(object who) {
   return query_lock_state() == M_LOCKABLE_UNLOCKED && has_correct_key(who) ? TRUE : FALSE;
}

string query_cannot_unlock_message(object who, varargs int from_do_unlock) {
   return "$N cannot seem to figure out the lock to $o.";
}

string query_cannot_lock_message(object who, varargs int from_do_lock) {
   return "$N cannot seem to figure out the lock to $o.";
}

string query_lock_unlocks(object who, varargs int from_do_unlock) {
   return "$N $vunlock $o.";
}

string query_lock_locks(object who, varargs int from_do_unlock) {
   return "$N $vlock $o.";
}

void disable_lockable(void) {
   update_lock_state(M_LOCKABLE_DISABLED);
}

void do_on_lock(object who) {

}

void do_on_unlock(object who) {

}

int do_unlock(object who) {
   if (query_lock_state() == M_LOCKABLE_DISABLED) {
      return 0;
   }

   if (!who) {
      if (query_lock_state() == M_LOCKABLE_LOCKED) {
         update_lock_state(M_LOCKABLE_UNLOCKED);
         this_object()->simple_action("$N $vunlock $o.");
      }
      return 1;
   }

   if (query_lock_state() == M_LOCKABLE_UNLOCKED) {
      who->targeted_action("$N $vtry to unlock $o but it is already unlocked.", nil, this_object());
      return 1;
   }

   if (!can_unlock(who)) {
      who->targeted_action(query_cannot_unlock_message(who, TRUE), nil, this_object());
      return 1;
   }

   who->targeted_action(query_lock_unlocks(who, TRUE), nil, this_object());
   update_lock_state(M_LOCKABLE_UNLOCKED);
   do_on_unlock(who);

   return 1;
}

int do_lock(object who) {
   if (query_lock_state() == M_LOCKABLE_DISABLED) {
      return 0;
   }

   if (!who) {
      if (query_lock_state() == M_LOCKABLE_UNLOCKED) {
         update_lock_state(M_LOCKABLE_LOCKED);
         this_object()->simple_action("$N $vlock $o.");
         return 1;
      }
   }

   if (query_lock_state() == M_LOCKABLE_LOCKED) {
      who->targeted_action("$N $vtry to lock $o but it is already locked.", nil, this_object());
      return 1;
   }

   if (!can_lock(who)) {
      who->targeted_action(query_cannot_lock_message(who, TRUE), nil, this_object());
      return 1;
   }

   who->targeted_action(query_lock_locks(who, TRUE), nil, this_object());
   update_lock_state(M_LOCKABLE_LOCKED);
   do_on_lock(who);

   return 1;
}
