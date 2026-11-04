/* tllist: the typed linked list macros used the way fcft and foot use them --
 * push back/front, iterate in order, remove while iterating, pop, length --
 * with known contents after each step. Header only: building and running this
 * checks the installed tllist.h. */
#include "common.h"
#include <tllist.h>

int main(void) {
  QT_BEGIN("tllist");
  tll(int) l = tll_init();
  QT_CHECK(tll_length(l) == 0, "a new list is empty");
  for (int i = 1; i <= 5; i++) tll_push_back(l, i * 10);   /* 10 20 30 40 50 */
  tll_push_front(l, 5);                                     /* 5 10 20 30 40 50 */
  QT_CHECK(tll_length(l) == 6, "six elements after five push_back and one push_front");
  QT_CHECK(tll_front(l) == 5 && tll_back(l) == 50, "front is 5, back is 50");
  int sum = 0, order_ok = 1, prev = -1;
  tll_foreach(l, it) { sum += it->item; if (it->item <= prev) order_ok = 0; prev = it->item; }
  QT_CHECK(sum == 155 && order_ok, "foreach visits 5..50 in order (sum 155)");
  tll_foreach(l, it) if (it->item % 20 == 0) tll_remove(l, it);   /* drop 20, 40 */
  QT_CHECK(tll_length(l) == 4, "remove while iterating drops 20 and 40");
  sum = 0; tll_foreach(l, it) sum += it->item;
  QT_CHECK(sum == 95, "remaining 5 10 30 50 sum to 95");
  int a = tll_pop_front(l), b = tll_pop_back(l);
  QT_CHECK(a == 5 && b == 50 && tll_length(l) == 2, "pop_front 5, pop_back 50, two left");
  tll_rforeach(l, it) { QT_CHECK(it->item == 30, "rforeach starts at the back (30)"); break; }
  tll_free(l);
  QT_CHECK(tll_length(l) == 0, "free empties the list");
  QT_END();
}
