#include "LoginWindow.h"

#include "DBManager.h"
#include "ThemeManager.h"

#include <QAction>
#include <QCheckBox>
#include <QColor>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QShowEvent>
#include <QVBoxLayout>

namespace {

    constexpr int kWindowWidth = 400;
    constexpr int kWindowHeight = 430;
    constexpr int kOuterMargin = 30;
    constexpr int kHeaderIconSize = 50;

    // Цвета «глазика»: серый — пароль скрыт, лаймовый — пароль виден.
    constexpr QLatin1String kColorMuted("#9CA3AF");
    constexpr QLatin1String kColorAccent("#A3E635");

    /// Серая подпись над полем ввода (стиль #inputLabel задан в dark_theme.qss).
    QLabel* makeCaption(const QString& text, QWidget* parent)
    {
        auto* label = new QLabel(text, parent);
        label->setObjectName(QStringLiteral("inputLabel"));
        return label;
    }

    /// Горизонтальный разделитель (стиль #dividerLine).
    QFrame* makeDivider(QWidget* parent)
    {
        auto* line = new QFrame(parent);
        line->setFrameShape(QFrame::HLine);
        line->setObjectName(QStringLiteral("dividerLine"));
        return line;
    }

} // namespace

LoginWindow::LoginWindow(QWidget* parent) : QDialog(parent)
{
    // Безрамочное окно с прозрачным фоном: скруглённые углы рисует QSS у #dialogContainer.
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(kWindowWidth, kWindowHeight);

    setupUi();
    setupConnections();
}

void LoginWindow::setupUi()
{
    // Окно -> container (рисует рамку и фон) -> mainLayout (всё содержимое).
    auto* container = new QWidget(this);
    container->setObjectName(QStringLiteral("dialogContainer"));

    auto* windowLayout = new QVBoxLayout(this);
    windowLayout->setContentsMargins(0, 0, 0, 0);
    windowLayout->addWidget(container);

    auto* mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(kOuterMargin, kOuterMargin, kOuterMargin, kOuterMargin);
    mainLayout->setSpacing(0);

    // ---------- Шапка: иконка, заголовок, крестик ----------
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto* iconLabel = new QLabel(container);
    iconLabel->setPixmap(ThemeManager::instance()
        .getIcon(QStringLiteral("profileBtn"),
            ThemeManager::instance().getDefaultIconColor(),
            QStringLiteral("LoginWindow"))
        .pixmap(kHeaderIconSize, kHeaderIconSize));
    iconLabel->setFixedSize(kHeaderIconSize, kHeaderIconSize);

    auto* textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);
    textLayout->setContentsMargins(0, 0, 0, 0);

    auto* titleLabel = new QLabel(tr("Войти"), container);
    titleLabel->setObjectName(QStringLiteral("loginTitle"));
    auto* subTitleLabel = new QLabel(tr("Откройте новые возможности"), container);
    subTitleLabel->setObjectName(QStringLiteral("loginSubtitle"));
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(subTitleLabel);

    m_closeBtn = new QPushButton(QStringLiteral("✕"), container);
    m_closeBtn->setObjectName(QStringLiteral("closeDialogBtn"));
    m_closeBtn->setFixedSize(24, 24);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setAutoDefault(false);  // Enter не должен нажимать крестик

    headerLayout->addWidget(iconLabel);
    headerLayout->addSpacing(10);
    headerLayout->addLayout(textLayout);
    headerLayout->addStretch();
    headerLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(headerLayout);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(makeDivider(container));
    mainLayout->addSpacing(15);

    // ---------- Поле «Логин» ----------
    mainLayout->addWidget(makeCaption(tr("ЛОГИН"), container));
    mainLayout->addSpacing(8);

    m_loginEdit = new QLineEdit(container);
    m_loginEdit->setPlaceholderText(tr("Введите ваш логин"));
    mainLayout->addWidget(m_loginEdit);
    mainLayout->addSpacing(12);

    // ---------- Поле «Пароль» + ссылка «Забыли пароль?» ----------
    auto* passHeaderLayout = new QHBoxLayout();
    passHeaderLayout->setContentsMargins(0, 0, 0, 0);

    m_forgotBtn = new QPushButton(tr("Забыли пароль?"), container);
    m_forgotBtn->setObjectName(QStringLiteral("linkButton"));
    m_forgotBtn->setCursor(Qt::PointingHandCursor);
    m_forgotBtn->setAutoDefault(false);
    // TODO: восстановление пароля ещё не реализовано — кнопка пока ни к чему не подключена.

    passHeaderLayout->addWidget(makeCaption(tr("ПАРОЛЬ"), container));
    passHeaderLayout->addStretch();
    passHeaderLayout->addWidget(m_forgotBtn);
    mainLayout->addLayout(passHeaderLayout);
    mainLayout->addSpacing(8);

    m_passwordEdit = new QLineEdit(container);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("Введите ваш пароль"));

    // «Глазик» справа внутри поля: показать/скрыть пароль.
    m_togglePasswordAction = new QAction(this);
    m_passwordEdit->addAction(m_togglePasswordAction, QLineEdit::TrailingPosition);
    updatePasswordToggleIcon();

    mainLayout->addWidget(m_passwordEdit);
    mainLayout->addSpacing(10);

    // ---------- «Запомнить меня» ----------
    m_rememberMeCheckbox = new QCheckBox(tr("Запомнить меня"), container);
    m_rememberMeCheckbox->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(m_rememberMeCheckbox);
    mainLayout->addSpacing(15);

    // ---------- Кнопка «ВОЙТИ» ----------
    m_loginBtn = new QPushButton(tr("ВОЙТИ"), container);
    m_loginBtn->setObjectName(QStringLiteral("loginSubmitBtn"));
    m_loginBtn->setFixedHeight(45);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setDefault(true);  // Enter в полях ввода нажимает именно её
    mainLayout->addWidget(m_loginBtn);

    mainLayout->addSpacing(15);
    mainLayout->addWidget(makeDivider(container));
    mainLayout->addSpacing(15);

    // ---------- Низ: «Нет аккаунта? [Создать аккаунт]» ----------
    auto* bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(0, 0, 0, 0);

    m_registerBtn = new QPushButton(tr("Создать аккаунт"), container);
    m_registerBtn->setObjectName(QStringLiteral("outlineButton"));
    m_registerBtn->setCursor(Qt::PointingHandCursor);
    m_registerBtn->setAutoDefault(false);

    bottomLayout->addWidget(makeCaption(tr("Нет аккаунта?"), container));
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_registerBtn);

    mainLayout->addLayout(bottomLayout);
    mainLayout->addStretch();
}

void LoginWindow::setupConnections()
{
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_registerBtn, &QPushButton::clicked, this, &LoginWindow::registerRequested);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_togglePasswordAction, &QAction::triggered, this, &LoginWindow::togglePasswordVisibility);
}

void LoginWindow::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

    // Каждый раз показываем «чистую» форму: пароль не должен висеть в поле
    // между открытиями, а чекбокс должен отражать сохранённую настройку
    // (например, после выхода из аккаунта).
    m_passwordEdit->clear();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    updatePasswordToggleIcon();

    const QSettings settings;
    m_rememberMeCheckbox->setChecked(settings.value(AuthSettings::kRememberMe, false).toBool());

    m_loginEdit->setFocus();
}

void LoginWindow::onLoginClicked()
{
    const QString login = m_loginEdit->text().trimmed();
    const QString password = m_passwordEdit->text();

    if (login.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, tr("Внимание"), tr("Заполните все поля."));
        return;
    }

    QString userRole;
    if (!DBManager::instance().authenticate(login, password, userRole)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Неверный логин или пароль."));
        return;
    }

    // Имя пользователя сохраняем только если стоит галочка «Запомнить меня».
    const bool remember = m_rememberMeCheckbox->isChecked();
    QSettings settings;
    settings.setValue(AuthSettings::kRememberMe, remember);
    if (remember)
        settings.setValue(AuthSettings::kUsername, login);
    else
        settings.remove(AuthSettings::kUsername);
    settings.sync();

    m_passwordEdit->clear();

    emit loginSuccessful(login);
    hide();
    emit windowClosed();
}

void LoginWindow::togglePasswordVisibility()
{
    const bool hidden = (m_passwordEdit->echoMode() == QLineEdit::Password);
    m_passwordEdit->setEchoMode(hidden ? QLineEdit::Normal : QLineEdit::Password);
    updatePasswordToggleIcon();
}

void LoginWindow::updatePasswordToggleIcon()
{
    const bool hidden = (m_passwordEdit->echoMode() == QLineEdit::Password);
    const QColor color(hidden ? kColorMuted : kColorAccent);

    m_togglePasswordAction->setIcon(ThemeManager::instance().getIcon(
        hidden ? QStringLiteral("passwordEditClose") : QStringLiteral("passwordEditOpen"),
        color,
        QStringLiteral("LoginWindow")));
    m_togglePasswordAction->setToolTip(hidden ? tr("Показать пароль") : tr("Скрыть пароль"));
}

void LoginWindow::reject()
{
    // QDialog::close() и Esc вызывают именно reject(), а не closeEvent(),
    // поэтому сигнал нужно отправлять здесь — иначе фон останется размытым.
    emit windowClosed();
    QDialog::reject();
}
