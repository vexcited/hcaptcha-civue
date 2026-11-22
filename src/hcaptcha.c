#include <hcaptcha/hcaptcha.h>

#include "page.h"

#include <civue/layout.h>
#include <civue/webview.h>
#include <civue/window.h>

#include <stdlib.h>
#include <string.h>

constexpr double LOAD_TIMEOUT = 15;

struct hcaptcha {
  civue_webview *web;
  char *html, *origin;
  hcaptcha_status status;
  char *token, *error;
  // hcaptcha's widget is rendered: the page can be asked again as it is.
  bool ready;
  // when a frame first showed the page loading, 0 before any did.
  double since;
};

static void turn(hcaptcha *h, hcaptcha_status status, const char *text) {
  free(h->token);
  free(h->error);
  h->token = nullptr;
  h->error = nullptr;

  if (status == HCAPTCHA_SOLVED) {
    h->token = strdup(text);
  }
  else if (status == HCAPTCHA_FAILED) {
    h->error = strdup(text && *text ? text : "error");
  }

  h->status = status;
  civue_window_wake();
}

static bool live(const hcaptcha *h) {
  return h->status == HCAPTCHA_LOADING || h->status == HCAPTCHA_OPEN;
}

static const char *after(const char *text, const char *prefix) {
  size_t n = strlen(prefix);

  return strncmp(text, prefix, n) ? nullptr : text + n;
}

static void on_message(void *ctx, const char *text) {
  hcaptcha *h = ctx;
  const char *said;

  if (!strcmp(text, "ready")) {
    h->ready = true;
  }
  else if ((said = after(text, "token:"))) {
    turn(h, HCAPTCHA_SOLVED, said);
  }
  else if (!strcmp(text, "expired")) {
    if (live(h) || h->status == HCAPTCHA_SOLVED) {
      turn(h, HCAPTCHA_EXPIRED, nullptr);
    }
  }
  else if (!live(h)) {
    // hcaptcha tells a closed or expired challenge twice, as an event then
    // as an error: the second is no news.
  }
  else if (!strcmp(text, "open")) {
    turn(h, HCAPTCHA_OPEN, nullptr);
  }
  else if (!strcmp(text, "closed") || !strcmp(text, "error:challenge-closed")) {
    turn(h, HCAPTCHA_CANCELLED, nullptr);
  }
  else if (!strcmp(text, "error:challenge-expired")) {
    turn(h, HCAPTCHA_EXPIRED, nullptr);
  }
  else if ((said = after(text, "error:"))) {
    turn(h, HCAPTCHA_FAILED, said);
  }
}

static void on_loaded(void *ctx, const char *error) {
  hcaptcha *h = ctx;

  if (error && live(h)) {
    turn(h, HCAPTCHA_FAILED, error);
  }
}

static hcaptcha_state state_of(const hcaptcha *h) {
  return (hcaptcha_state){h->status, h->token, h->error};
}

hcaptcha *hcaptcha_new(const hcaptcha_options *o) {
  hcaptcha *h = calloc(1, sizeof *h);

  if (!h) {
    abort();
  }

  if (!o->sitekey || !o->origin) {
    h->status = HCAPTCHA_FAILED;
    h->error = strdup("no sitekey or origin");
    return h;
  }

  h->html = hcaptcha__page(o);
  h->origin = strdup(o->origin);
  h->web = civue_webview_new(&(civue_webview_options){
    .ctx = h,
    .message = on_message,
    .loaded = on_loaded,
    .user_agent = o->user_agent,
    .transparent = true,
  });

  if (!h->web) {
    h->status = HCAPTCHA_FAILED;
    h->error = strdup("no webview on this platform");
    return h;
  }

  civue_webview_load_html(h->web, h->html, h->origin);
  return h;
}

void hcaptcha_free(hcaptcha *h) {
  if (!h) {
    return;
  }

  civue_webview_free(h->web);
  free(h->html);
  free(h->origin);
  free(h->token);
  free(h->error);
  free(h);
}

hcaptcha_state hcaptcha_show(civue_ui *ui, hcaptcha *h, civue_rect r) {
  if (h->status == HCAPTCHA_LOADING && !h->ready) {
    double now = civue_now();

    if (!h->since) {
      h->since = now;
    }

    double left = h->since + LOAD_TIMEOUT - now;

    if (left <= 0) {
      turn(h, HCAPTCHA_FAILED, "loading timeout");
    }
    else {
      civue_request_repaint_after(ui, left);
    }
  }

  // shown while it loads too, clear: webkit holds back the timers of a page
  // that is not on the screen, and hcaptcha's with them.
  if (live(h)) {
    civue_webview_show(ui, h->web, r);
  }

  return state_of(h);
}

hcaptcha_state hcaptcha_node(civue_ui *ui, hcaptcha *h) {
  return hcaptcha_show(ui, h, civue_node_rect(ui));
}

void hcaptcha_reset(hcaptcha *h) {
  if (!h->web) {
    return;
  }

  // a page that failed is loaded anew, hcaptcha's script with it.
  bool again = h->ready && h->status != HCAPTCHA_FAILED;

  turn(h, HCAPTCHA_LOADING, nullptr);
  h->since = 0;

  if (again) {
    civue_webview_eval(h->web, "again();");
  }
  else {
    h->ready = false;
    civue_webview_load_html(h->web, h->html, h->origin);
  }
}
