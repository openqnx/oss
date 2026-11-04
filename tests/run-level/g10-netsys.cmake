# Run-level tests, group g10-netsys: hiredis, libvalkey, valkey, and the other
# network/system components of this batch. Each test uses the component
# (known answers), not just its loading. Which test checks which component:
# g10-netsys.tsv; programs/data a test needs installed: g10-netsys.extras.

# ---- Redis/Valkey clients and server: each client test starts its own
# valkey-server (TCP, UNIX socket, TLS; libvalkey also a cluster node) ----
port_test(test_hiredis          test_hiredis.c           hiredis hiredis_ssl ssl crypto)
port_test(test_libvalkey        test_libvalkey.c         valkey valkey_tls ssl crypto)
port_test(test_valkey           test_valkey.c)

# ---- terminal libraries ----
port_test(test_libvterm         test_libvterm.c          vterm)
port_test(test_libtermkey       test_libtermkey.c        termkey)

# ---- small libraries ----
port_test(test_libasyncns       test_libasyncns.c        asyncns)
port_test(test_libpaper         test_libpaper.c          paper m)
port_test(test_geoip            test_geoip.c             GeoIP)
port_test(test_yubico_c_client  test_yubico_c_client.c   ykclient crypto)

port_test(test_tdb              test_tdb.c               tdb)

# ---- data: checked through the libraries that read them ----
port_test(test_publicsuffix_list test_publicsuffix_list.c psl)
port_test(test_dnssec_root      test_dnssec_root.c       crypto)
port_test(test_hwdata           test_hwdata.c)
port_test(test_bash_completion  test_bash_completion.c)

# ---- shell scripts ----

port_script(test_openresolv)
port_script(test_gnome_keyring)
port_script(test_samba)

# ---- D-Bus ----
port_test(test_dbus_glib        test_dbus_glib.c         dbus-glib-1 dbus-1 gobject-2.0 glib-2.0)
target_include_directories(test_dbus_glib PRIVATE "${INC}/dbus-1.0" "${LIB}/dbus-1.0/include" "${INC}/glib-2.0" "${LIB}/glib-2.0/include")

# ---- programs ----
port_test(test_lynx             test_lynx.c)
port_test(test_pinentry         test_pinentry.c)
