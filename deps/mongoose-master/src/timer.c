// Copyright (c) Cesanta Software Limited
// All rights reserved

#include "timer.h"
#include "arch.h"

void mg_timer_init(struct mg_timer **head, struct mg_timer *t, uint64_t ms,
                   unsigned flags, void (*fn)(void *), void *arg) {
  struct mg_timer tmp = {ms, 0U, 0U, flags, fn, arg, *head};
  *t = tmp;
  *head = t;
  if (flags & MG_TIMER_RUN_NOW) fn(arg);
}

void mg_timer_free(struct mg_timer **head, struct mg_timer *t) {
  while (*head && *head != t) head = &(*head)->next;
  if (*head) *head = t->next;
}

void mg_timer_poll(struct mg_timer **head, uint64_t now_ms) {
  // If time goes back (wrapped around), reset timers
  struct mg_timer *t, *tmp;
  for (t = *head; t != NULL; t = tmp) {
    tmp = t->next;
    if (t->prev_ms > now_ms) t->expire = 0;  // Handle time wrap
    t->prev_ms = now_ms;
    if (t->expire == 0) t->expire = now_ms + t->period_ms;
    if (t->expire > now_ms) continue;
    t->fn(t->arg);
    // Try to tick timers with the given period as accurate as possible,
    // even if this polling function is called with some random period.
    t->expire = now_ms - t->expire > t->period_ms ? now_ms + t->period_ms
                                                  : t->expire + t->period_ms;
    if (!(t->flags & MG_TIMER_REPEAT)) mg_timer_free(head, t);
  }
}
