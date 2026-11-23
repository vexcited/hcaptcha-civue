An hCaptcha challenge for [Civue](https://code.vexcited.com/ciko/civue) apps.

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

For a complete example, take a look at [`examples/demo`](/examples/demo).

## Including

```cmake
include(FetchContent)
FetchContent_Declare(hcaptcha-civue
  GIT_REPOSITORY https://code.vexcited.com/hcaptcha/civue.git
  GIT_TAG main)
FetchContent_MakeAvailable(hcaptcha-civue)
target_link_libraries(app PRIVATE hcaptcha::civue)
```

```sh
cmake -S . -B build -G Ninja
cmake --build build

build/examples/demo/demo
build/examples/demo/demo --open                      # asks at once, without the button
build/examples/demo/demo --open <sitekey> <origin>   # another site's key
```
