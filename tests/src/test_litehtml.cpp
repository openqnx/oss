// litehtml: lay out a known HTML/CSS document with a minimal container
// (monospace metrics: every character 8 px wide, 16 px font -> 20 px lines)
// and check the geometry the CSS prescribes: block sizes and positions, a
// right float, a fixed-width table's cells, line wrapping in a 100 px box,
// the text and background colours the engine asks to draw, the <title>, and
// CSS selectors (select_one/select_all) on the parsed (gumbo) tree.
#include "common.h"
#include <litehtml.h>
#include <string>
#include <vector>

using namespace litehtml;

struct Drawn { std::string text; web_color color; position pos; };

class container : public document_container {
public:
  std::vector<Drawn> texts;
  std::vector<web_color> bgs;
  std::string caption;
  uint_ptr create_font(const char*, int size, int, font_style, unsigned int, font_metrics* fm) override {
    fm->height = size + 4; fm->ascent = size; fm->descent = 4; fm->x_height = size / 2; fm->draw_spaces = false;
    return (uint_ptr)1;
  }
  void delete_font(uint_ptr) override {}
  int text_width(const char* t, uint_ptr) override { return 8 * (int)strlen(t); }
  void draw_text(uint_ptr, const char* t, uint_ptr, web_color c, const position& p) override { texts.push_back({t, c, p}); }
  int pt_to_px(int pt) const override { return pt * 96 / 72; }
  int get_default_font_size() const override { return 16; }
  const char* get_default_font_name() const override { return "monospace"; }
  void draw_list_marker(uint_ptr, const list_marker&) override {}
  void load_image(const char*, const char*, bool) override {}
  void get_image_size(const char*, const char*, size& sz) override { sz.width = sz.height = 0; }
  void draw_background(uint_ptr, const std::vector<background_paint>& bg) override { bgs.push_back(bg.back().color); }
  void draw_borders(uint_ptr, const borders&, const position&, bool) override {}
  void set_caption(const char* c) override { caption = c; }
  void set_base_url(const char*) override {}
  void link(const std::shared_ptr<document>&, const element::ptr&) override {}
  void on_anchor_click(const char*, const element::ptr&) override {}
  void set_cursor(const char*) override {}
  void transform_text(string& t, text_transform tt) override {
    if (tt == text_transform_uppercase) for (auto& ch : t) ch = toupper((unsigned char)ch);
  }
  void import_css(string&, const string&, string&) override {}
  void set_clip(const position&, const border_radiuses&) override {}
  void del_clip() override {}
  void get_client_rect(position& c) const override { c.x = 0; c.y = 0; c.width = 400; c.height = 300; }
  element::ptr create_element(const char*, const string_map&, const std::shared_ptr<document>&) override { return nullptr; }
  void get_media_features(media_features& m) const override {
    m.type = media_type_screen; m.width = m.device_width = 400; m.height = m.device_height = 300;
    m.color = 8; m.resolution = 96;
  }
  void get_language(string& l, string& c) const override { l = "en"; c = ""; }
};

static const char* html =
  "<html><head><title>QNX litehtml</title><style>"
  "body { margin: 0; font-size: 16px; }"
  "#a { width: 200px; height: 50px; background: #ff0000; }"
  "#f { float: right; width: 100px; height: 10px; }"
  ".narrow { width: 100px; }"
  "td { padding: 0; width: 60px; } table { border-spacing: 0; }"
  "em { color: #0000ff; text-transform: uppercase; }"
  "</style></head><body>"
  "<div id=a></div>"
  "<div id=f></div>"
  "<p id=p style='margin:0'>hello <em>world</em></p>"
  "<div class=narrow id=w>aaaa bbbb cccc dddd</div>"
  "<table id=t><tr><td id=c1>x</td><td id=c2>y</td></tr></table>"
  "<ul><li>one</li><li>two</li><li>three</li></ul>"
  "</body></html>";

int main() {
  QT_BEGIN("litehtml");
  container c;
  document::ptr doc = document::createFromString(html, &c);
  QT_CHECK(doc != nullptr, "document parsed (gumbo) and styled");
  if (!doc) QT_END();
  int w = doc->render(400);
  printf("  info rendered width %d, height %d\n", w, doc->height());
  element::ptr root = doc->root();
  auto pl = [&](const char* sel) { element::ptr e = root->select_one(sel); return e ? e->get_placement() : position(); };
  position a = pl("#a"), f = pl("#f"), p = pl("#p"), wd = pl("#w"), c1 = pl("#c1"), c2 = pl("#c2");
  QT_CHECK(a.x == 0 && a.y == 0 && a.width == 200 && a.height == 50, "#a: 200x50 at (0,0)");
  QT_CHECK(f.x == 300 && f.y == 50 && f.width == 100, "#f floats right: x 300, below #a");
  QT_CHECK(p.y == 50 && p.height == 20, "the paragraph starts at y 50, one 20 px line");
  QT_CHECK(wd.width == 100 && wd.height == 40, "'aaaa bbbb cccc dddd' in 100 px wraps to two lines (40 px)");
  QT_CHECK(c1.width == 60 && c2.x == c1.x + 60, "table cells 60 px wide, side by side");
  QT_CHECK(root->select_all("li").size() == 3, "select_all('li'): 3 items");

  doc->draw((uint_ptr)0, 0, 0, nullptr);
  QT_CHECK(c.caption == "QNX litehtml", "the <title> becomes the caption");
  bool hello = false, world = false, red = false;
  for (auto& t : c.texts) {
    if (t.text == "hello" && t.pos.y >= 50 && t.pos.y < 70) hello = true;
    if (t.text == "WORLD" && t.color.blue == 255 && t.color.red == 0 && t.pos.x == 48) world = true;
  }
  for (auto& b : c.bgs) if (b.red == 255 && b.green == 0 && b.blue == 0) red = true;
  QT_CHECK(hello, "'hello' drawn on the paragraph's line");
  QT_CHECK(world, "'world' drawn upper-cased, blue, after 'hello ' (x 48)");
  QT_CHECK(red, "#a's red background drawn");
  QT_END();
}
