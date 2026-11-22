#ifndef HCAPTCHA_PAGE_H
#define HCAPTCHA_PAGE_H

#include <hcaptcha/hcaptcha.h>

// the html of the challenge's page, from malloc. it posts "ready" once
// hcaptcha's widget is rendered, then "open", "closed", "expired",
// "token:" and the response, or "error:" and why. its again() asks anew.
char *hcaptcha__page(const hcaptcha_options *o);

#endif
