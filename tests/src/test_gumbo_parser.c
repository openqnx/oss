/* gumbo-parser: parse an HTML5 fragment with the usual errors (unclosed <p>,
 * missing <html>/<head>/<body>, an entity, an attribute without quotes) and
 * check the tree the WHATWG algorithm prescribes: html > head + body, the
 * implied end tags, the attributes, the decoded text, and the source
 * positions; then a <table> with stray text (foster parenting). */
#include "common.h"
#include <gumbo.h>

static const GumboNode *child_el(const GumboNode *n, GumboTag tag, int nth) {
  const GumboVector *c = &n->v.element.children;
  for (unsigned i = 0; i < c->length; i++) {
    const GumboNode *k = c->data[i];
    if (k->type == GUMBO_NODE_ELEMENT && k->v.element.tag == tag && nth-- == 0) return k;
  }
  return NULL;
}
static int count_tag(const GumboNode *n, GumboTag tag) {
  if (n->type != GUMBO_NODE_ELEMENT) return 0;
  int k = n->v.element.tag == tag;
  for (unsigned i = 0; i < n->v.element.children.length; i++) k += count_tag(n->v.element.children.data[i], tag);
  return k;
}
static const char *text_of(const GumboNode *n) {
  if (!n || n->type != GUMBO_NODE_ELEMENT || n->v.element.children.length == 0) return "";
  const GumboNode *t = n->v.element.children.data[0];
  return t->type == GUMBO_NODE_TEXT ? t->v.text.text : "";
}

int main(void) {
  QT_BEGIN("gumbo-parser");
  const char *html = "<title>T&amp;Q</title><p class=a id=first>one<p>two &lt;3&gt;<a href=\"x.html\">link</a>";
  GumboOutput *out = gumbo_parse(html);
  QT_CHECK(out && out->root->v.element.tag == GUMBO_TAG_HTML, "root is an implied <html>");
  const GumboNode *head = child_el(out->root, GUMBO_TAG_HEAD, 0), *body = child_el(out->root, GUMBO_TAG_BODY, 0);
  QT_CHECK(head && body, "implied <head> and <body>");
  const GumboNode *title = head ? child_el(head, GUMBO_TAG_TITLE, 0) : NULL;
  QT_CHECK(title && !strcmp(text_of(title), "T&Q"), "<title> in head, &amp; decoded to 'T&Q'");
  const GumboNode *p1 = body ? child_el(body, GUMBO_TAG_P, 0) : NULL, *p2 = body ? child_el(body, GUMBO_TAG_P, 1) : NULL;
  QT_CHECK(p1 && p2 && count_tag(out->root, GUMBO_TAG_P) == 2, "the unclosed <p> is closed: two sibling <p>");
  GumboAttribute *cls = p1 ? gumbo_get_attribute(&p1->v.element.attributes, "class") : NULL;
  GumboAttribute *id = p1 ? gumbo_get_attribute(&p1->v.element.attributes, "id") : NULL;
  QT_CHECK(cls && !strcmp(cls->value, "a") && id && !strcmp(id->value, "first"), "unquoted attributes class=a id=first");
  QT_CHECK(p1 && !strcmp(text_of(p1), "one") && p2 && !strcmp(text_of(p2), "two <3>"), "texts 'one' and 'two <3>'");
  const GumboNode *a = p2 ? child_el(p2, GUMBO_TAG_A, 0) : NULL;
  GumboAttribute *href = a ? gumbo_get_attribute(&a->v.element.attributes, "href") : NULL;
  QT_CHECK(a && href && !strcmp(href->value, "x.html") && !strcmp(text_of(a), "link"), "<a href=x.html>link</a> inside the second <p>");
  QT_CHECK(p1 && p1->v.element.start_pos.offset == strstr(html, "<p") - html && p1->v.element.start_pos.line == 1, "source position of the first <p>");
  gumbo_destroy_output(&kGumboDefaultOptions, out);

  out = gumbo_parse("<table><tr><td>cell</td></tr>stray</table>");
  body = child_el(out->root, GUMBO_TAG_BODY, 0);
  const GumboNode *first = body && body->v.element.children.length ? body->v.element.children.data[0] : NULL;
  QT_CHECK(first && first->type == GUMBO_NODE_TEXT && !strcmp(first->v.text.text, "stray"), "foster parenting: stray table text moved before the table");
  const GumboNode *table = body ? child_el(body, GUMBO_TAG_TABLE, 0) : NULL;
  QT_CHECK(table && child_el(table, GUMBO_TAG_TBODY, 0), "implied <tbody>");
  gumbo_destroy_output(&kGumboDefaultOptions, out);
  QT_CHECK(gumbo_tag_enum("blockquote") == GUMBO_TAG_BLOCKQUOTE && !strcmp(gumbo_normalized_tagname(GUMBO_TAG_DIV), "div"), "tag names");
  QT_END();
}
