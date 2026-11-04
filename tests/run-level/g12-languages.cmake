# Run-level tests, group g12-languages: language runtimes and their tools
# (tcl, ruby, lua5.2 + lua-compat53 + luarocks, vala), Sass (libsass, sassc),
# spell checking (hunspell, nuspell, mythes) and data packages (tzdata,
# docbook-xml, font-inconsolata, tree-sitter-cpp). Each test uses its
# component on the target with known answers. Which test checks which
# component: g12-languages.tsv; what a test needs installed beyond what it
# links: g12-languages.extras.
find_package(PkgConfig REQUIRED)
pkg_check_modules(G12_FONTS REQUIRED IMPORTED_TARGET fontconfig freetype2)

# ---- data and grammars ----
port_test(test_tree_sitter_cpp     test_tree_sitter_cpp.c      tree-sitter)
port_test(test_font_inconsolata    test_fonts.c                PkgConfig::G12_FONTS)
target_compile_definitions(test_font_inconsolata PRIVATE FONT_PKG="font-inconsolata")
port_test(test_tzdata              test_tzdata.c)
port_script(test_tzdata_tools)
port_script(test_docbook_xml)

# ---- Lua ----
port_test(test_lua5_2              test_lua5_2.c               lua5.2)
port_script(test_lua_compat53)
port_script(test_luarocks)

# ---- spell checking ----
port_test(test_hunspell            test_hunspell.c             hunspell-1.7)
port_test(test_nuspell             test_nuspell.cpp            nuspell icuuc)
port_test(test_mythes              test_mythes.cpp             mythes-1.2)

# ---- Sass ----
port_test(test_libsass             test_libsass.c              sass)
port_script(test_sassc)

# ---- Tcl ----
port_test(test_tcl                 test_tcl.c                  tcl8.6)
port_script(test_tclsh)
port_script(test_vala)

# ---- Ruby ----
port_script(test_ruby)
