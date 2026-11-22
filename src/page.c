#include "page.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *v;
  size_t len, cap;
} Text;

static void put_n(Text *t, const char *s, size_t n) {
  if (t->len + n + 1 > t->cap) {
    t->cap = (t->len + n + 1) * 2;
    t->v = realloc(t->v, t->cap);

    if (!t->v) {
      abort();
    }
  }

  memcpy(t->v + t->len, s, n);
  t->len += n;
  t->v[t->len] = 0;
}

static void put(Text *t, const char *s) {
  put_n(t, s, strlen(s));
}

// a json string, or null, that an inline script can hold: what would end the
// script or the string is written as escapes, as react-native-hcaptcha does.
static void put_json(Text *t, const char *s) {
  if (!s) {
    put(t, "null");
    return;
  }

  put(t, "\"");

  for (const unsigned char *c = (const unsigned char *)s; *c; c++) {
    // the line and paragraph separators, which end a line of javascript.
    bool separator = c[0] == 0xe2 && c[1] == 0x80 && (c[2] & 0xfe) == 0xa8;

    if (separator) {
      put(t, c[2] == 0xa8 ? "\\u2028" : "\\u2029");
      c += 2;
    }
    else if (*c < 0x20 || strchr("\"\\<>&'", *c)) {
      char escaped[8];
      snprintf(escaped, sizeof escaped, "\\u%04x", *c);
      put(t, escaped);
    }
    else {
      put_n(t, (const char *)c, 1);
    }
  }

  put(t, "\"");
}

static void put_field(Text *t, const char *name, const char *value) {
  put(t, name);
  put(t, ": ");
  put_json(t, value);
  put(t, ",\n");
}

// transparent, so the app's card shows around the challenge, which hcaptcha
// centers in the page itself. behind an open one it lays a white sheet over
// the page, half clear, the one fixed element it adds: the sheet stays, a
// click on it closes the challenge, but dims nothing.
static const char head[] =
  "<!DOCTYPE html>\n"
  "<html>\n"
  "<head>\n"
  "<meta charset=\"UTF-8\">\n"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
  "<style>\n"
  "html, body {\n"
  "  margin: 0;\n"
  "  width: 100%;\n"
  "  height: 100%;\n"
  "  overflow: hidden;\n"
  "  background: transparent;\n"
  "}\n"
  "body > div > div[style*=\"position: fixed\"] {\n"
  "  background-color: transparent !important;\n"
  "}\n"
  "</style>\n"
  "<script>\n"
  "var config = {\n";

// react-native-hcaptcha's page without its loader: api.js rendered by hand
// and calling back once it is there, an invisible widget run at once, the
// request data set before each run, and every callback posted to the app.
static const char rest[] =
  "};\n"
  "var widget = null;\n"
  "var post = function (kind, data) {\n"
  "  window.civue.postMessage(data === undefined ? kind : kind + ':' + data);\n"
  "};\n"
  "var why = function (e) {\n"
  "  return typeof e === 'string' ? e\n"
  "    : (e && (e.message || e.name)) || 'error';\n"
  "};\n"
  "var run = function () {\n"
  "  try {\n"
  "    if (config.rqdata) {\n"
  "      hcaptcha.setData(widget, { rqdata: config.rqdata });\n"
  "    }\n"
  "    hcaptcha.execute(widget);\n"
  "  }\n"
  "  catch (e) {\n"
  "    post('error', why(e));\n"
  "  }\n"
  "};\n"
  "var again = function () {\n"
  "  try {\n"
  "    hcaptcha.reset(widget);\n"
  "  }\n"
  "  catch (e) {}\n"
  "  run();\n"
  "};\n"
  "var onHcaptchaLoad = function () {\n"
  "  try {\n"
  "    var options = {\n"
  "      sitekey: config.sitekey,\n"
  "      size: 'invisible',\n"
  "      callback: function (token) { post('token', token); },\n"
  "      'open-callback': function () { post('open'); },\n"
  "      'close-callback': function () { post('closed'); },\n"
  "      'expired-callback': function () { post('expired'); },\n"
  "      'chalexpired-callback': function () { post('expired'); },\n"
  "      'error-callback': function (e) { post('error', why(e)); }\n"
  "    };\n"
  "    if (config.theme) {\n"
  "      options.theme = config.theme;\n"
  "    }\n"
  "    widget = hcaptcha.render('hcaptcha', options);\n"
  "    post('ready');\n"
  "    run();\n"
  "  }\n"
  "  catch (e) {\n"
  "    post('error', why(e));\n"
  "  }\n"
  "};\n"
  "var script = document.createElement('script');\n"
  "script.src = 'https://js.hcaptcha.com/1/api.js'\n"
  "  + '?render=explicit&onload=onHcaptchaLoad'\n"
  "  + '&host=' + encodeURIComponent(location.hostname)\n"
  "  + (config.language ? '&hl=' + encodeURIComponent(config.language) : '');\n"
  "script.async = true;\n"
  "script.onerror = function () { post('error', 'script-error'); };\n"
  "document.head.appendChild(script);\n"
  "</script>\n"
  "</head>\n"
  "<body>\n"
  "<div id=\"hcaptcha\"></div>\n"
  "</body>\n"
  "</html>\n";

char *hcaptcha__page(const hcaptcha_options *o) {
  Text t = {};

  put(&t, head);
  put_field(&t, "sitekey", o->sitekey);
  put_field(&t, "rqdata", o->rqdata);
  put_field(&t, "theme", o->theme);
  put_field(&t, "language", o->language);
  put(&t, rest);

  return t.v;
}
