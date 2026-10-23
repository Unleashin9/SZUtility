#include "DBManager.h"
#include "MainWindow.h"
#include "AuctionWindow.h"
#include "ThemeManager.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QStandardPaths>

namespace {

    /// Путь к файлу БД в каталоге данных приложения.
    /// Раньше использовался относительный "app.db", и база создавалась в
    /// текущей папке запуска — при запуске из другого места пользователи «пропадали».
    QString databasePath()
    {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);

        const QString path = dir + QStringLiteral("/app.db");

        // Однократная миграция: если старая БД лежит рядом с местом запуска — переносим её.
        const QString legacyPath = QStringLiteral("app.db");
        if (!QFile::exists(path) && QFile::exists(legacyPath))
            QFile::copy(legacyPath, path);

        return path;
    }

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Имена нужны ДО первого использования QSettings и QStandardPaths:
    // по ним Qt выбирает, где хранить настройки и данные.
    QCoreApplication::setOrganizationName(QStringLiteral("StalzoneOrg"));
    QCoreApplication::setApplicationName(QStringLiteral("StalzoneUtility"));

    ThemeManager::instance().setTheme(ThemeManager::ThemeType::Dark);

    if (!DBManager::instance().initDatabase(databasePath())) {
        QMessageBox::critical(nullptr, QObject::tr("Ошибка"),
            QObject::tr("Не удалось открыть базу данных."));
        return EXIT_FAILURE;
    }

    AuctionWindow window;
    window.showFullScreen();

    const int exitCode = app.exec();

    // Закрываем БД, пока QApplication ещё жив: singleton иначе разрушился бы
    // уже после него, и Qt ругался бы на незакрытое соединение.
    DBManager::instance().closeDatabase();
    return exitCode;
}
