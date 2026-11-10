// Qt translations: Qt's own message catalogs, installed where QLibraryInfo
// says (TranslationsPath), load with QTranslator -- qt_de.qm is a
// meta-catalog that pulls in qtbase_de.qm and the others -- and translate
// Qt's user-visible strings (known answers from qtbase_de.ts / qtbase_fr.ts).
#include "common.h"

#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

int main(int argc, char** argv) {
  QT_BEGIN("qt6-translations");
  QCoreApplication app(argc, argv);
  const QString dir = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
  QStringList qm = QDir(dir).entryList({"*.qm"});
  printf("  info %s: %lld catalogs\n", qPrintable(dir), (long long)qm.size());
  QT_CHECK(qm.size() > 100 && qm.contains("qtbase_de.qm") && qm.contains("qt_ja.qm"), "catalogs installed");

  QTranslator de;
  QT_CHECK(de.load("qt_de", dir), "load qt_de.qm (meta-catalog)");
  QT_CHECK(de.translate("QPlatformTheme", "Cancel") == "Abbrechen", "de: QPlatformTheme Cancel = Abbrechen");

  QTranslator fr;
  QT_CHECK(fr.load(QLocale(QLocale::French, QLocale::France), "qtbase", "_", dir), "load qtbase_fr by locale");
  QT_CHECK(fr.language() == "fr" || fr.language().startsWith("fr"), "catalog language fr");
  QCoreApplication::installTranslator(&fr);
  QT_CHECK(QCoreApplication::translate("QPlatformTheme", "Cancel") == "Annuler",
           "installed translator: QCoreApplication::translate -> Annuler");
  QCoreApplication::removeTranslator(&fr);
  QT_CHECK(QCoreApplication::translate("QPlatformTheme", "Cancel") == "Cancel", "removed: untranslated again");
  QT_END();
}
