# hcaptcha-civue

*A captcha and a rect.* An hCaptcha challenge for [civue](../ciko-civue) apps in C23, headless like civue's widgets: a call each frame it is wanted answers with its state, and the app paints the card, the spinner and the error around it.

```c
#include <civue/dsl.h>
#include <hcaptcha/hcaptcha.h>

hcaptcha *captcha = hcaptcha_new(&(hcaptcha_options){        // the page starts loading
  .sitekey = "10000000-ffff-ffff-ffff-000000000001",         // hcaptcha's test key, passes unasked
  .origin = "https://example.com",                           // the site the key belongs to
  .theme = "dark",
});

static void frame(void *ctx, civue_ui *ui) {
  div(w(px(400)), h(px(600)), rounded(lg), bg(rgb(0x1e1f22))) {   // the app's own card
    hcaptcha_state s = hcaptcha_node(ui, captcha);           // the challenge over the node, while there is one

    if (s.status == HCAPTCHA_SOLVED) send(s.token);          // the hcaptcha's own until its status changes
    if (s.status == HCAPTCHA_FAILED) text(s.error);
  }
}
```

`examples/demo` is a dark card with a button that asks, the challenge in a well under it and the token once it has one. It is a CMake project of its own, taking this checkout where an app would fetch it.

## How it works

- **hCaptcha's own page.** The challenge is a page built in C and shown in a `civue_webview`, loaded as if it were on `origin`, which is what hCaptcha checks the key against. It is the page of [react-native-hcaptcha](https://github.com/hCaptcha/react-native-hcaptcha) cut down: `api.js` with `render=explicit`, an invisible widget rendered then executed at once, and every callback posted to the app. Every value given goes into the page as escaped json.
- **Invisible.** hCaptcha asks nothing of a user it trusts, the status going from `HCAPTCHA_LOADING` to `HCAPTCHA_SOLVED` with nothing shown, and opens a challenge for the others, `HCAPTCHA_OPEN`. The page is clear and hCaptcha centers the challenge in it, so the app's card shows all around, and the sheet hCaptcha dims the page with is clear too. A click outside the challenge closes it, as on the web.
- **A state a frame.** `hcaptcha_show(ui, h, r)` shows the page over `r` while it loads or is open and answers with the status, the token while solved and the error while failed. `hcaptcha_node` does it over the open node. A status that changes wakes the window, so the app has nothing to poll, and a frame that does not call them hides the page.
- **Ends.** The user closing the challenge is `HCAPTCHA_CANCELLED`. A challenge left too long, or a token kept past the two minutes hCaptcha gives it, is `HCAPTCHA_EXPIRED`. A script that does not come within 15 seconds, a page that does not load or an error of hCaptcha's is `HCAPTCHA_FAILED` with its reason. `hcaptcha_reset` asks again from any of them.
- **Links leave.** hCaptcha's links, its terms and its privacy policy, open in the system's browser.

## Discord

Discord answers a request it wants checked with a 400 holding `captcha_sitekey`, and at times `captcha_rqdata`, `captcha_rqtoken` and `captcha_session_id`.

```c
hcaptcha *captcha = hcaptcha_new(&(hcaptcha_options){
  .sitekey = challenge.captcha_sitekey,     // never the same, taken from each answer
  .origin = "https://discord.com",
  .rqdata = challenge.captcha_rqdata,       // must be passed when it is there
  .theme = "dark",
});
```

The token then goes back with the request in `X-Captcha-Key`, with `X-Captcha-Rqtoken` and `X-Captcha-Session-Id` when the answer had them. `should_serve_invisible` needs nothing here, the widget is always invisible.

## Building

It depends on civue, taken from `../ciko-civue` unless a parent project already has `civue::civue`, and civue's cilo comes from `../ciko-cilo` when that checkout is there. C23 with the GNU extensions, clang 19 or later or Apple clang 21, CMake 3.29 and Ninja.

```sh
cmake -S . -B build -G Ninja
ninja -C build
build/examples/demo/demo
build/examples/demo/demo --open                      # asks at once, without the button
build/examples/demo/demo --open <sitekey> <origin>   # another site's key
```

```cmake
include(FetchContent)
FetchContent_Declare(hcaptcha-civue SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../hcaptcha-civue)
FetchContent_MakeAvailable(hcaptcha-civue)
target_link_libraries(app PRIVATE hcaptcha::civue)
```

## Limits

- macOS only for now, as civue's webviews are: on Linux `hcaptcha_new` answers a captcha that is `HCAPTCHA_FAILED` at once.
- The challenge is a view of the system's over the frame, so nothing the app draws goes over it. It is made for a rect around 400 by 600 points and hCaptcha sizes it to what it gets.
- The sheet behind a challenge is found by how hCaptcha styles it. Should hCaptcha change that, the page dims again and nothing else breaks.
- Cookies are kept by WebKit under the app's name, between runs, as hCaptcha expects of a browser.
