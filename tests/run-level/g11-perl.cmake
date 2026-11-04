# Run-level tests, group g11-perl: CPAN modules for the target perl (the
# libwww-perl stack, TLS, XML, HTML, test helpers, module build tools, small
# XS modules). Each test is a shell script running perl on the target with
# known answers; one script checks several modules. Which test checks which
# component: g11-perl.tsv; files a test needs installed: g11-perl.extras.
port_script(test_perl_http)
port_script(test_perl_html)
port_script(test_perl_xml)
port_script(test_perl_ssl)
port_script(test_perl_xs_misc)
port_script(test_perl_testmods)
port_script(test_perl_buildtools)
