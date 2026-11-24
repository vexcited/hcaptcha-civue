An hCaptcha challenge for [Civue](https://code.vexcited.com/ciko/civue) apps.

[hCaptcha](https://www.hcaptcha.com) is a drop-replacement for reCAPTCHA that protects user privacy, rewards websites, and helps companies get their data labeled.

The challenge is hCaptcha's own page in a `civue_webview`, so only macOS is supported for now.

1. [Installation](#installation)
2. [Implementation](#implementation)
3. [References](#references)
4. [Debugging](#debugging)

## Installation

This library requires `clang` 22 or later.

```cmake
include(FetchContent)
FetchContent_Declare(hcaptcha-civue
  GIT_REPOSITORY https://code.vexcited.com/hcaptcha/civue.git
  GIT_TAG main)
FetchContent_MakeAvailable(hcaptcha-civue)
target_link_libraries(app PRIVATE hcaptcha::civue)
```

Civue is fetched along with it, unless a `civue::civue` target already exists or a `ciko-civue` checkout sits next to this one.

### Building

```sh
cmake -S . -B build -G Ninja
cmake --build build

build/examples/demo/demo
build/examples/demo/demo --open                      # asks at once, without the button.
build/examples/demo/demo --open <sitekey> <origin>   # another site's key.
```

## Implementation

> For a complete example, take a look at [`examples/demo`](/examples/demo).

### Basic Usage

```c
#include <civue/dsl.h>
#include <hcaptcha/hcaptcha.h>

static hcaptcha *captcha;

static void frame(void *ctx, civue_ui *ui) {
  div(w(px(400)), h(px(600)), rounded(lg), bg(rgb(0x1e1f22))) {
    hcaptcha_state s = hcaptcha_node(ui, captcha);

    if (s.status == HCAPTCHA_SOLVED) {
      send(s.token);
    }
    else if (s.status == HCAPTCHA_FAILED) {
      text(s.error);
    }
  }
}

int main(void) {
  captcha = hcaptcha_new(&(hcaptcha_options){
    .sitekey = "10000000-ffff-ffff-ffff-000000000001",
    .origin = "https://example.com",
    .theme = "dark",
  });

  // ...

  hcaptcha_free(captcha);
}
```

The page starts loading as soon as `hcaptcha_new` returns. Each frame the challenge is wanted, `hcaptcha_node` draws it over the open node and answers with its state, the app paints the rest.

### Asking again

```c
static void verify_button(civue_ui *ui) {
  div(key(id("verify")), cursor_pointer) {
    if (button().activated) {
      hcaptcha_reset(captcha);
    }

    text("Again");
  }
}
```

`hcaptcha_reset` loads a new challenge, after a token was used or expired, or after a failure.

## References

### Options

| Name | Values/Type | Required | Default | Description |
| ---- | ----------- | -------- | ------- | ----------- |
| `sitekey` | `const char *` | **Yes** | `-` | This is your sitekey, this allows you to load captcha. If you need a sitekey, please visit hCaptcha, and sign up to get your sitekey. |
| `origin` | `const char *` | **Yes** | `-` | The site the sitekey belongs to, like `"https://example.com"`. |
| `rqdata` | `const char *` | No | `NULL` | See enterprise docs. |
| `theme` | `"light" \| "dark"` | No | `NULL` | hCaptcha supports both a light and dark theme. If no theme is set, hCaptcha picks one. |
| `language` | `const char *` (ISO 639-2 code) | No | `NULL` | hCaptcha auto-detects language via the user's system. This overrides that to set a default UI language. See [language codes](https://hcaptcha.com/docs/languages). |
| `user_agent` | `const char *` | No | `NULL` | Overrides the user agent of the webview, the system's one is used by default. |

Every string is copied by `hcaptcha_new`, so they don't have to outlive the call.

### Statuses

| Status | Fields | Description |
| ------ | ------ | ----------- |
| `HCAPTCHA_LOADING` | `-` | When the page loads, or hCaptcha decides whether to ask anything. |
| `HCAPTCHA_OPEN` | `-` | When the user display of a challenge starts. |
| `HCAPTCHA_SOLVED` | `token` | When challenge is completed, with or without a challenge shown. The response `token` belongs to the `hcaptcha` until its status changes, copy it to keep it. |
| `HCAPTCHA_CANCELLED` | `-` | When the user dismisses a challenge. |
| `HCAPTCHA_EXPIRED` | `-` | When the user display of a challenge times out with no answer, or when the current token expires after about two minutes. |
| `HCAPTCHA_FAILED` | `error` | When an error occurs. The `error` is hCaptcha's error code or the webview's words. |

The status comes in the `status` field of the `hcaptcha_state` that `hcaptcha_show` and `hcaptcha_node` return every frame.

### Functions

| Function | Description |
| -------- | ----------- |
| `hcaptcha_new(const hcaptcha_options *o)` | Starts loading the challenge. Never returns `NULL`. |
| `hcaptcha_free(hcaptcha *h)` | Frees the challenge. |
| `hcaptcha_show(civue_ui *ui, hcaptcha *h, civue_rect r)` | Shows the challenge over `r` for this frame, while it loads or is open, and returns its state. |
| `hcaptcha_node(civue_ui *ui, hcaptcha *h)` | The same, over the open node of `<civue/layout.h>`. |
| `hcaptcha_reset(hcaptcha *h)` | Reset the current challenge, for a token used or expired, or after a failure. |

> **Note** \
> Make sure to reset the hCaptcha state when you submit your form by calling `hcaptcha_reset`! Passcodes are one-time use, so if your user submits the same passcode twice then it will be rejected by the server the second time.

Please note that hCaptcha always runs invisible here, which means that no hCaptcha button will be rendered. Whether a challenge shows up will depend on the sitekey difficulty level, a user it trusts goes straight from loading to solved.

## Debugging

### Give the challenge enough room

The challenge is centered in the rect, the rest of it left as the app painted it. It is made for a rect around 400 by 600 points, smaller ones may cut it.

### Keep calling it while it loads

The challenge only shows on the frames `hcaptcha_show` or `hcaptcha_node` is called. Loading fails after 15 seconds shown, with `HCAPTCHA_FAILED`.

### Stay on the main thread

Every function must be called from the main thread.
