#ifndef HCAPTCHA_H
#define HCAPTCHA_H

#include <civue/ui.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* an hcaptcha challenge over a rect of a civue frame, headless like civue's  */
/* widgets: a call each frame it is wanted answers with its state and the app */
/* paints what is around it, a card, a spinner or an error. the challenge is  */
/* hcaptcha's own page in a civue_webview, so macos and windows for now.      */
/* everything here runs on the main thread.                                   */
/* ========================================================================== */

typedef enum : uint8_t {
  // the page loads, or hcaptcha decides whether to ask anything.
  HCAPTCHA_LOADING,
  // the challenge is on the screen.
  HCAPTCHA_OPEN,
  // passed, with or without a challenge shown.
  HCAPTCHA_SOLVED,
  // the user closed the challenge.
  HCAPTCHA_CANCELLED,
  // the challenge waited too long, or the token did after about two minutes.
  HCAPTCHA_EXPIRED,
  HCAPTCHA_FAILED,
} hcaptcha_status;

typedef struct {
  // the site's key.
  const char *sitekey;
  // the site the key belongs to, like "https://example.com": the page
  // believes it is on it.
  const char *origin;
  // hcaptcha enterprise's request data, which must be passed when the site
  // gives one. none when null.
  const char *rqdata;
  // "dark" or "light", hcaptcha's own choice when null.
  const char *theme;
  // the language of the challenge, like "fr", the system's when null.
  const char *language;
  // the system webview's when null.
  const char *user_agent;
} hcaptcha_options;

typedef struct {
  hcaptcha_status status;
  // the response to send the site while HCAPTCHA_SOLVED, the hcaptcha's own
  // until its status changes.
  const char *token;
  // what went wrong while HCAPTCHA_FAILED, hcaptcha's error code or the
  // webview's words.
  const char *error;
} hcaptcha_state;

typedef struct hcaptcha hcaptcha;

// starts loading the challenge of `o`, whose strings are copied. hcaptcha
// runs invisible: it asks nothing of a user it trusts, and the status goes
// from loading to solved.
[[nodiscard, gnu::returns_nonnull]]
hcaptcha *hcaptcha_new(const hcaptcha_options *o);
void hcaptcha_free(hcaptcha *h);
// shows the challenge over `r` for this frame while it loads or is open,
// centered in it, the rest of `r` left as the app painted it. made for a
// rect around 400 by 600 points. a status that changes wakes the window, and
// loading fails after 15 seconds shown.
hcaptcha_state hcaptcha_show(civue_ui *ui, hcaptcha *h, civue_rect r);
// the same over the open node of <civue/layout.h>.
hcaptcha_state hcaptcha_node(civue_ui *ui, hcaptcha *h);
// asks again, for a token used or expired, or after a failure.
void hcaptcha_reset(hcaptcha *h);

#ifdef __cplusplus
}
#endif

#endif
