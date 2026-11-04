/* md4c: Markdown to HTML with known output -- CommonMark headings,
 * emphasis, code spans, lists, links, a fenced code block with its
 * language, HTML escaping of <, & and ", entity references; GitHub
 * extensions (tables with alignment, strikethrough, task lists) on and off;
 * and the SAX parser itself (md_parse): block and span callbacks counted
 * for a document, heading levels reported. */
#include "common.h"
#include <stdlib.h>
#include <md4c.h>
#include <md4c-html.h>

static char obuf[4096];
static void out(const MD_CHAR *t, MD_SIZE n, void *u) { (void)u; strncat(obuf, t, n); }
static const char *html(const char *md, unsigned flags) {
  obuf[0] = 0;
  if (md_html(md, (MD_SIZE)strlen(md), out, NULL, flags, 0) != 0) return "(error)";
  return obuf;
}
static int check(const char *md, unsigned flags, const char *want) {
  const char *got = html(md, flags);
  if (strcmp(got, want)) printf("  -- got:\n%s  -- want:\n%s", got, want);
  return !strcmp(got, want);
}

static int nblocks, nspans, hlevels;
static int enter_block(MD_BLOCKTYPE t, void *d, void *u) {
  (void)u; nblocks++;
  if (t == MD_BLOCK_H) hlevels = hlevels * 10 + (int)((MD_BLOCK_H_DETAIL *)d)->level;
  return 0;
}
static int leave_block(MD_BLOCKTYPE t, void *d, void *u) { (void)t; (void)d; (void)u; return 0; }
static int enter_span(MD_SPANTYPE t, void *d, void *u) { (void)t; (void)d; (void)u; nspans++; return 0; }
static int leave_span(MD_SPANTYPE t, void *d, void *u) { (void)t; (void)d; (void)u; return 0; }
static int text(MD_TEXTTYPE t, const MD_CHAR *s, MD_SIZE n, void *u) { (void)t; (void)s; (void)n; (void)u; return 0; }

int main(void) {
  QT_BEGIN("md4c");
  QT_CHECK(check("# Title\n\nSome *em*, **strong** and `code`.\n", 0,
                 "<h1>Title</h1>\n<p>Some <em>em</em>, <strong>strong</strong> and <code>code</code>.</p>\n"),
           "heading, emphasis, strong, code span");
  QT_CHECK(check("- a\n- b\n\n1. one\n2. two\n", 0,
                 "<ul>\n<li>a</li>\n<li>b</li>\n</ul>\n<ol>\n<li>one</li>\n<li>two</li>\n</ol>\n"),
           "bullet and ordered lists");
  QT_CHECK(check("[QNX](https://qnx.com \"home\")\n", 0,
                 "<p><a href=\"https://qnx.com\" title=\"home\">QNX</a></p>\n"), "link with title");
  QT_CHECK(check("```c\nint x = a < b && c;\n```\n", 0,
                 "<pre><code class=\"language-c\">int x = a &lt; b &amp;&amp; c;\n</code></pre>\n"),
           "fenced code block: language class, <, & escaped");
  QT_CHECK(check("5 &lt; 6 &amp; &copy; \"q\"\n", 0, "<p>5 &lt; 6 &amp; \xC2\xA9 &quot;q&quot;</p>\n"),
           "entities: &copy; to UTF-8, quotes escaped");
  QT_CHECK(check("Setext\n======\n\n> quoted\n", 0, "<h1>Setext</h1>\n<blockquote>\n<p>quoted</p>\n</blockquote>\n"),
           "setext heading and block quote");
  const char *tbl = "| a | b |\n|:--|--:|\n| 1 | 2 |\n";
  QT_CHECK(check(tbl, MD_FLAG_TABLES,
                 "<table>\n<thead>\n<tr>\n<th align=\"left\">a</th>\n<th align=\"right\">b</th>\n</tr>\n</thead>\n"
                 "<tbody>\n<tr>\n<td align=\"left\">1</td>\n<td align=\"right\">2</td>\n</tr>\n</tbody>\n</table>\n"),
           "table with left/right alignment");
  QT_CHECK(strstr(html(tbl, 0), "<table>") == NULL, "no table without MD_FLAG_TABLES");
  QT_CHECK(check("~~gone~~\n", MD_FLAG_STRIKETHROUGH, "<p><del>gone</del></p>\n"), "strikethrough");
  QT_CHECK(check("- [x] done\n- [ ] todo\n", MD_FLAG_TASKLISTS,
                 "<ul>\n<li class=\"task-list-item\"><input type=\"checkbox\" class=\"task-list-item-checkbox\" disabled checked>done</li>\n"
                 "<li class=\"task-list-item\"><input type=\"checkbox\" class=\"task-list-item-checkbox\" disabled>todo</li>\n</ul>\n"),
           "task list");

  MD_PARSER p = { 0, MD_FLAG_TABLES, enter_block, leave_block, enter_span, leave_span, text, NULL, NULL };
  const char *doc = "# A\n\n## B\n\ntext *x* [l](u)\n\n### C\n";
  QT_CHECK(md_parse(doc, (MD_SIZE)strlen(doc), &p, NULL) == 0, "md_parse ok");
  /* blocks: document, h1, h2, p, h3 = 5; spans: em, a = 2 */
  QT_CHECK(nblocks == 5 && nspans == 2, "SAX: 5 blocks, 2 spans");
  QT_CHECK(hlevels == 123, "SAX: heading levels 1, 2, 3 in order");
  QT_END();
}
