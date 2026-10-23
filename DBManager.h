#pragma once

#include <QSqlDatabase>
#include <QString>

/**
 * Доступ к локальной SQLite-базе с учётными записями (singleton).
 *
 * Порядок использования:
 *   1. main():            DBManager::instance().initDatabase(path);
 *   2. RegisterWindow:    DBManager::instance().registerUser(...);
 *   3. LoginWindow:       DBManager::instance().authenticate(...);
 *   4. конец main():      DBManager::instance().closeDatabase();
 *
 * Пароли никогда не хранятся в открытом виде. В колонке password_hash лежит
 * строка формата:
 *     pbkdf2_sha256$<итерации>$<соль в hex>$<хэш в hex>
 */
class DBManager
{
public:
    /// Результат регистрации — чтобы UI мог показать точную причину отказа.
    enum class RegisterResult {
        Ok,          ///< пользователь создан
        LoginTaken,  ///< такой логин уже существует
        Error        ///< БД не открыта или ошибка SQL
    };

    static DBManager& instance();

    /// Открывает (или создаёт) файл БД и таблицу users. Повторный вызов безопасен.
    bool initDatabase(const QString& dbPath = QStringLiteral("app.db"));

    /// Закрывает соединение и удаляет его из реестра Qt.
    void closeDatabase();

    RegisterResult registerUser(const QString& login, const QString& password,
        const QString& role = QStringLiteral("user"));

    /// true, если логин/пароль верны; роль пользователя возвращается в outRole.
    bool authenticate(const QString& login, const QString& password, QString& outRole);

    bool userExists(const QString& login) const;

private:
    DBManager() = default;
    ~DBManager();

    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;

    QSqlDatabase m_db;
};
