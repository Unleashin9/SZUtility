#include "DBManager.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QPasswordDigestor>   // PBKDF2 (модуль Qt Network)
#include <QRandomGenerator>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

namespace {

    // ---------- Настройки БД ----------

    constexpr auto kConnectionName = "stalzone_main";

    // ---------- Настройки хэширования паролей ----------

    constexpr QLatin1String kHashScheme("pbkdf2_sha256");
    constexpr int kPbkdf2Iterations = 200'000;  // чем больше, тем медленнее подбор
    constexpr int kSaltBytes = 16;
    constexpr int kHashBytes = 32;              // длина выхода SHA-256

    /// Криптографически стойкие случайные байты (для соли).
    QByteArray randomBytes(int count)
    {
        QByteArray out;
        out.reserve(count);
        while (out.size() < count) {
            const quint32 chunk = QRandomGenerator::system()->generate();
            out.append(reinterpret_cast<const char*>(&chunk), sizeof(chunk));
        }
        out.truncate(count);
        return out;
    }

    /// Сравнение за постоянное время — не выдаёт, на каком байте произошло расхождение.
    bool constantTimeEquals(const QByteArray& a, const QByteArray& b)
    {
        if (a.size() != b.size())
            return false;
        quint8 diff = 0;
        for (int i = 0; i < a.size(); ++i)
            diff |= static_cast<quint8>(a.at(i) ^ b.at(i));
        return diff == 0;
    }

    QByteArray deriveKey(const QString& password, const QByteArray& salt, int iterations)
    {
        return QPasswordDigestor::deriveKeyPbkdf2(
            QCryptographicHash::Sha256, password.toUtf8(), salt, iterations, kHashBytes);
    }

    /// Создаёт строку для хранения в БД: схема, итерации, случайная соль и хэш.
    QString hashPassword(const QString& password)
    {
        const QByteArray salt = randomBytes(kSaltBytes);
        const QByteArray key = deriveKey(password, salt, kPbkdf2Iterations);
        return QStringLiteral("%1$%2$%3$%4")
            .arg(kHashScheme)
            .arg(kPbkdf2Iterations)
            .arg(QString::fromLatin1(salt.toHex()))
            .arg(QString::fromLatin1(key.toHex()));
    }

    /// Проверяет пароль по сохранённой строке.
    /// needsUpgrade = true, если хэш устарел и его стоит пересчитать
    /// (старый формат «голый SHA-256» или мало итераций).
    bool verifyPassword(const QString& password, const QString& stored, bool& needsUpgrade)
    {
        needsUpgrade = false;

        const QStringList parts = stored.split(QLatin1Char('$'));
        if (parts.size() == 4 && parts.at(0) == kHashScheme) {
            bool ok = false;
            const int iterations = parts.at(1).toInt(&ok);
            if (!ok || iterations <= 0)
                return false;

            const QByteArray salt = QByteArray::fromHex(parts.at(2).toLatin1());
            const QByteArray expected = QByteArray::fromHex(parts.at(3).toLatin1());

            needsUpgrade = iterations < kPbkdf2Iterations;
            return constantTimeEquals(deriveKey(password, salt, iterations), expected);
        }

        // Совместимость со старой версией приложения: sha256(password) в hex без соли.
        // При успешном входе такой хэш будет автоматически заменён на PBKDF2.
        const QByteArray legacy =
            QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
        if (constantTimeEquals(legacy, stored.toLatin1())) {
            needsUpgrade = true;
            return true;
        }
        return false;
    }

} // namespace

// ============================================================================
// Singleton / жизненный цикл
// ============================================================================

DBManager& DBManager::instance()
{
    static DBManager s_instance;
    return s_instance;
}

DBManager::~DBManager()
{
    closeDatabase();
}

bool DBManager::initDatabase(const QString& dbPath)
{
    if (m_db.isOpen())
        return true;

    // Если прошлая попытка открытия не удалась, соединение уже зарегистрировано —
    // переиспользуем его, иначе Qt выдаст предупреждение о дубликате.
    const QString connection = QLatin1String(kConnectionName);
    m_db = QSqlDatabase::contains(connection)
        ? QSqlDatabase::database(connection, /*open*/ false)
        : QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "DB Error:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query(m_db);
    const bool created = query.exec(QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS users (
            id            INTEGER PRIMARY KEY AUTOINCREMENT,
            login         TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            role          TEXT NOT NULL DEFAULT 'user',
            created_at    DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )"));

    if (!created) {
        qWarning() << "Table Error:" << query.lastError().text();
        closeDatabase();
        return false;
    }
    return true;
}

void DBManager::closeDatabase()
{
    if (!m_db.isValid())
        return;

    const QString connection = m_db.connectionName();
    if (m_db.isOpen())
        m_db.close();

    // Qt требует: сначала уничтожить все QSqlDatabase-объекты соединения,
    // и только потом вызывать removeDatabase(), иначе будет предупреждение.
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connection);
}

// ============================================================================
// Пользователи
// ============================================================================

bool DBManager::userExists(const QString& login) const
{
    if (!m_db.isOpen())
        return false;

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT 1 FROM users WHERE login = :login LIMIT 1"));
    query.bindValue(QStringLiteral(":login"), login);
    return query.exec() && query.next();
}

DBManager::RegisterResult DBManager::registerUser(const QString& login,
    const QString& password,
    const QString& role)
{
    if (!m_db.isOpen())
        return RegisterResult::Error;

    if (userExists(login))
        return RegisterResult::LoginTaken;

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "INSERT INTO users (login, password_hash, role) VALUES (:login, :hash, :role)"));
    query.bindValue(QStringLiteral(":login"), login);
    query.bindValue(QStringLiteral(":hash"), hashPassword(password));
    query.bindValue(QStringLiteral(":role"), role);

    if (!query.exec()) {
        qWarning() << "Add User Error:" << query.lastError().text();
        return RegisterResult::Error;
    }
    return RegisterResult::Ok;
}

bool DBManager::authenticate(const QString& login, const QString& password, QString& outRole)
{
    if (!m_db.isOpen())
        return false;

    // Хэш нельзя сравнивать в SQL (у каждого пользователя своя соль),
    // поэтому достаём запись по логину и проверяем пароль в коде.
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(
        "SELECT id, password_hash, role FROM users WHERE login = :login"));
    query.bindValue(QStringLiteral(":login"), login);

    if (!query.exec() || !query.next())
        return false;

    const qint64 id = query.value(0).toLongLong();
    const QString storedHash = query.value(1).toString();
    const QString role = query.value(2).toString();
    query.finish();

    bool needsUpgrade = false;
    if (!verifyPassword(password, storedHash, needsUpgrade))
        return false;

    // Тихо обновляем устаревший хэш на актуальный формат.
    if (needsUpgrade) {
        QSqlQuery update(m_db);
        update.prepare(QStringLiteral("UPDATE users SET password_hash = :hash WHERE id = :id"));
        update.bindValue(QStringLiteral(":hash"), hashPassword(password));
        update.bindValue(QStringLiteral(":id"), id);
        if (!update.exec())
            qWarning() << "Hash upgrade error:" << update.lastError().text();
    }

    outRole = role;
    return true;
}
