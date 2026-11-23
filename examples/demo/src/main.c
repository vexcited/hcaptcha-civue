#include <stddefer.h>
#include <stdio.h>
#include <string.h>

#include <civue/dsl.h>
#include <hcaptcha/hcaptcha.h>

// hcaptcha's public test key, which passes without a challenge, on any site.
static const char TEST_SITEKEY[] = "10000000-ffff-ffff-ffff-000000000001";
static const char TEST_ORIGIN[] = "https://example.com";

typedef struct {
  hcaptcha_options options;
  hcaptcha *captcha;
  // the status last printed, -1 before any.
  int printed;
} Demo;

static const char *const names[] = {
  [HCAPTCHA_LOADING] = "loading", [HCAPTCHA_OPEN] = "open",
  [HCAPTCHA_SOLVED] = "solved",   [HCAPTCHA_CANCELLED] = "cancelled",
  [HCAPTCHA_EXPIRED] = "expired", [HCAPTCHA_FAILED] = "failed",
};

static void print(Demo *demo, hcaptcha_state s) {
  if (demo->printed == s.status) {
    return;
  }

  demo->printed = s.status;

  const char *more = s.token ? s.token : s.error ? s.error : "";
  fprintf(stderr, "hcaptcha: %s %s\n", names[s.status], more);
}

static void verify(Demo *demo) {
  if (demo->captcha) {
    hcaptcha_reset(demo->captcha);
  }
  else {
    demo->captcha = hcaptcha_new(&demo->options);
  }
}

static void verify_button(Demo *demo) {
  civue_color accent = rgb(0x5865f2);

  div(key(id("verify")), flex, items_center, justify_center, w(24), h(8),
      rounded(md), bg(accent), active(bg(civue_alpha(accent, .8f))),
      text_color(CIVUE_WHITE), cursor_pointer) {
    if (button().activated) {
      verify(demo);
    }

    text(demo->captcha ? "Again" : "Verify");
  }
}

// the card the app paints, the challenge over its well while there is one.
static void card(Demo *demo, civue_ui *ui) {
  hcaptcha_state s = {};

  div(flex, flex_col, gap(3), w(px(432)), p(4), rounded(xl),
      bg(rgb(0x2b2d31))) {
    text("hCaptcha over civue", text_size(px(17)), line_height(px(24)),
         font_weight(semibold));
    verify_button(demo);

    div(flex, items_center, justify_center, w(px(400)), h(px(600)), rounded(lg),
        bg(rgb(0x1e1f22)), text_color(rgb(0x949ba4))) {
      if (demo->captcha) {
        s = hcaptcha_node(ui, demo->captcha);
        print(demo, s);
      }

      text(demo->captcha ? names[s.status] : "Nothing asked yet");
    }

    text(s.token   ? s.token
         : s.error ? s.error
                   : " ",
         truncate, text_color(rgb(0x949ba4)));
  }
}

static void frame(void *ctx, civue_ui *ui) {
  Demo *demo = ctx;

  ui->appearance = CIVUE_APPEARANCE_DARK;

  div(size(full), flex, justify_center, p(6), text_size(px(13)),
      line_height(px(20)), text_color(rgb(0xdbdee1))) {
    card(demo, ui);
  }
}

static civue_color background([[maybe_unused]] void *ctx) {
  return rgb(0x111214);
}

static civue_app make([[maybe_unused]] civue_ui *ui, void *arg) {
  return (civue_app){arg, frame, background};
}

// demo [--open] [sitekey origin]: `--open` asks at once, without the button.
int main(int argc, char **argv) {
  Demo demo = {
    .options = {.sitekey = TEST_SITEKEY,
                .origin = TEST_ORIGIN,
                .theme = "dark"},
    .printed = -1,
  };
  defer hcaptcha_free(demo.captcha);

  int at = 1;
  bool open = argc > at && !strcmp(argv[at], "--open");

  at += open;

  if (argc > at + 1) {
    demo.options.sitekey = argv[at];
    demo.options.origin = argv[at + 1];
  }

  if (open) {
    verify(&demo);
  }

  civue_window_options options = {
    .title = "hCaptcha",
    .width = 480,
    .height = 780,
    .min_width = 480,
    .min_height = 780,
  };

  civue_window_run(&options, make, &demo);
  return 0;
}
