#include "RegisterWindow.h"

#include "DBManager.h"
#include "ThemeManager.h"

#include <QAction>
#include <QColor>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QShowEvent>
#include <QVBoxLayout>

namespace {

    constexpr int kWindowWidth = 400;
    constexpr int kWindowHeight = 690;
    constexpr int kOuterMargin = 30;
    constexpr int kHeaderIconSize = 50;

    // Ограничения на данные регистрации.
    constexpr int kMinLoginLength = 3;
    constexpr int kMaxLoginLength = 32;
    constexpr int kMinPasswordLength = 6;

    constexpr QLatin1String kColorMuted("#9CA3AF");   // пароль скрыт
    constexpr QLatin1String kColorAccent("#A3E635");  // пароль виден

    QLabel* makeCaption(const QString& text, QWidget* parent)
    {
        auto* label = new QLabel(text, parent);
        label->setObjectName(QStringLiteral("inputLabel"));
        return label;
    }

    QFrame* makeDivider(QWidget* parent)
    {
        auto* line = new QFrame(parent);
        line->setFrameShape(QFrame::HLine);
        line->setObjectName(QStringLiteral("dividerLine"));
        return line;
    }

    /// Мелкий серый юридический текст со ссылками (стиль #legalText).
    QLabel* makeLegalLabel(const QString& html, QWidget* parent)
    {
        auto* label = new QLabel(parent);
        label->setObjectName(QStringLiteral("legalText"));
        label->setWordWrap(true);
        label->setTextFormat(Qt::RichText);
        label->setText(html);
        // TODO: ссылки #privacy и #terms пока никуда не ведут —
        // подключите сюда linkActivated, когда появятся документы.
        return label;
    }

} // namespace

RegisterWindow::RegisterWindow(QWidget* parent) : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(kWindowWidth, kWindowHeight);

    setupUi();
    setupConnections();
}

void RegisterWindow::setupUi()
{
    // Окно -> container (рамка и фон) -> mainLayout (содержимое).
    auto* container = new QWidget(this);
    container->setObjectName(QStringLiteral("dialogContainer"));

    auto* windowLayout = new QVBoxLayout(this);
    windowLayout->setContentsMargins(0, 0, 0, 0);
    windowLayout->addWidget(container);

    auto* mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(kOuterMargin, kOuterMargin, kOuterMargin, kOuterMargin);
    mainLayout->setSpacing(0);

    // ---------- Шапка ----------
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(0, 0, 0, 0);

    auto* iconLabel = new QLabel(container);
    iconLabel->setPixmap(ThemeManager::instance()
        .getIcon(QStringLiteral("registerBtn"),
            ThemeManager::instance().getDefaultIconColor(),
            QStringLiteral("RegisterWindow"))
        .pixmap(kHeaderIconSize, kHeaderIconSize));
    iconLabel->setFixedSize(kHeaderIconSize, kHeaderIconSize);

    auto* textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);
    textLayout->setContentsMargins(0, 0, 0, 0);

    auto* titleLabel = new QLabel(tr("Создать аккаунт"), container);
    titleLabel->setObjectName(QStringLiteral("loginTitle"));
    auto* subTitleLabel = new QLabel(tr("Больше функций - больше возможностей!"), container);
    subTitleLabel->setObjectName(QStringLiteral("loginSubtitle"));
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(subTitleLabel);

    m_closeBtn = new QPushButton(QStringLiteral("✕"), container);
    m_closeBtn->setObjectName(QStringLiteral("closeDialogBtn"));
    m_closeBtn->setFixedSize(24, 24);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setAutoDefault(false);

    headerLayout->addWidget(iconLabel);
    headerLayout->addSpacing(10);
    headerLayout->addLayout(textLayout);
    headerLayout->addStretch();
    headerLayout->addWidget(m_closeBtn);

    mainLayout->addLayout(headerLayout);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(makeDivider(container));
    mainLayout->addSpacing(15);

    // ---------- Логин ----------
    mainLayout->addWidget(makeCaption(tr("ЛОГИН"), container));
    mainLayout->addSpacing(8);

    m_loginEdit = new QLineEdit(container);
    m_loginEdit->setPlaceholderText(tr("Введите ваш логин"));
    m_loginEdit->setMaxLength(kMaxLoginLength);
    mainLayout->addWidget(m_loginEdit);
    mainLayout->addSpacing(12);

    // ---------- Пароль и его повтор ----------
    mainLayout->addWidget(makeCaption(tr("ПАРОЛЬ"), container));
    mainLayout->addSpacing(8);
    m_passwordEdit1 = createPasswordEdit(tr("Введите ваш пароль"), container, m_togglePass1Action);
    mainLayout->addWidget(m_passwordEdit1);
    mainLayout->addSpacing(12);

    mainLayout->addWidget(makeCaption(tr("ПОВТОРИТЕ ПАРОЛЬ"), container));
    mainLayout->addSpacing(8);
    m_passwordEdit2 = createPasswordEdit(tr("Введите пароль еще раз"), container, m_togglePass2Action);
    mainLayout->addWidget(m_passwordEdit2);
    mainLayout->addSpacing(15);

    mainLayout->addWidget(makeDivider(container));
    mainLayout->addSpacing(15);

    // ---------- Реферальный код ----------
    // TODO: значение пока нигде не используется и не сохраняется в БД.
    mainLayout->addWidget(makeCaption(tr("РЕФЕРАЛЬНЫЙ КОД"), container));
    mainLayout->addSpacing(8);

    m_referralEdit = new QLineEdit(container);
    m_referralEdit->setPlaceholderText(tr("Введите реферальный код, если есть"));
    mainLayout->addWidget(m_referralEdit);
    mainLayout->addSpacing(15);

    // ---------- Юридические тексты ----------
    mainLayout->addWidget(makeLegalLabel(
        tr("Даю согласие на обработку персональных данных на условиях "
            "<a href='#privacy' style='color:#A3E635; text-decoration:none;'>"
            "Политики конфиденциальности</a>"),
        container));
    mainLayout->addSpacing(10);
    mainLayout->addWidget(makeLegalLabel(
        tr("Нажимая «Создать аккаунт» я принимаю "
            "<a href='#terms' style='color:#A3E635; text-decoration:none;'>"
            "Пользовательское соглашение</a> и подтверждаю, что мне есть 14 лет."),
        container));
    mainLayout->addSpacing(15);

    // ---------- Кнопка «СОЗДАТЬ АККАУНТ» ----------
    m_registerBtn = new QPushButton(tr("СОЗДАТЬ АККАУНТ"), container);
    m_registerBtn->setObjectName(QStringLiteral("loginSubmitBtn"));
    m_registerBtn->setFixedHeight(45);
    m_registerBtn->setCursor(Qt::PointingHandCursor);
    m_registerBtn->setDefault(true);  // Enter в полях ввода нажимает именно её
    mainLayout->addWidget(m_registerBtn);

    mainLayout->addSpacing(15);
    mainLayout->addWidget(makeDivider(container));
    mainLayout->addSpacing(15);

    // ---------- Низ: «Уже есть аккаунт? [Войти]» ----------
    auto* bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(0, 0, 0, 0);

    m_loginBtn = new QPushButton(tr("Войти"), container);
    m_loginBtn->setObjectName(QStringLiteral("outlineButton"));
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setAutoDefault(false);

    bottomLayout->addWidget(makeCaption(tr("Уже есть аккаунт?"), container));
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_loginBtn);

    mainLayout->addLayout(bottomLayout);
    mainLayout->addStretch();
}

void RegisterWindow::setupConnections()
{
    connect(m_registerBtn, &QPushButton::clicked, this, &RegisterWindow::onRegisterClicked);
    connect(m_loginBtn, &QPushButton::clicked, this, &RegisterWindow::loginRequested);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::reject);
}

QLineEdit* RegisterWindow::createPasswordEdit(const QString& placeholder, QWidget* parent,
    QAction*& toggleAction)
{
    auto* edit = new QLineEdit(parent);
    edit->setEchoMode(QLineEdit::Password);
    edit->setPlaceholderText(placeholder);

    toggleAction = new QAction(this);
    edit->addAction(toggleAction, QLineEdit::TrailingPosition);
    updatePasswordIcon(edit, toggleAction);

    // Одна и та же логика для обоих полей — отличаются только edit/action в лямбде.
    QAction* action = toggleAction;
    connect(action, &QAction::triggered, this,
        [this, edit, action]() { togglePasswordVisibility(edit, action); });
    return edit;
}

void RegisterWindow::clearFields()
{
    m_loginEdit->clear();
    m_passwordEdit1->clear();
    m_passwordEdit2->clear();
    m_referralEdit->clear();

    m_passwordEdit1->setEchoMode(QLineEdit::Password);
    m_passwordEdit2->setEchoMode(QLineEdit::Password);
    updatePasswordIcon(m_passwordEdit1, m_togglePass1Action);
    updatePasswordIcon(m_passwordEdit2, m_togglePass2Action);
}

void RegisterWindow::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    m_loginEdit->setFocus();
}

void RegisterWindow::togglePasswordVisibility(QLineEdit* edit, QAction* action)
{
    const bool hidden = (edit->echoMode() == QLineEdit::Password);
    edit->setEchoMode(hidden ? QLineEdit::Normal : QLineEdit::Password);
    updatePasswordIcon(edit, action);
}

void RegisterWindow::updatePasswordIcon(QLineEdit* edit, QAction* action)
{
    const bool hidden = (edit->echoMode() == QLineEdit::Password);
    const QColor color(hidden ? kColorMuted : kColorAccent);

    action->setIcon(ThemeManager::instance().getIcon(
        hidden ? QStringLiteral("passwordEditClose") : QStringLiteral("passwordEditOpen"),
        color,
        QStringLiteral("RegisterWindow")));
    action->setToolTip(hidden ? tr("Показать пароль") : tr("Скрыть пароль"));
}

void RegisterWindow::onRegisterClicked()
{
    const QString login = m_loginEdit->text().trimmed();
    const QString pass1 = m_passwordEdit1->text();
    const QString pass2 = m_passwordEdit2->text();

    // --- Проверка введённых данных (от простого к сложному) ---
    if (login.isEmpty() || pass1.isEmpty() || pass2.isEmpty()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Заполните обязательные поля."));
        return;
    }
    if (login.size() < kMinLoginLength) {
        QMessageBox::warning(this, tr("Ошибка"),
            tr("Логин должен содержать не менее %1 символов.").arg(kMinLoginLength));
        return;
    }
    if (pass1 != pass2) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Пароли не совпадают."));
        return;
    }
    if (pass1.size() < kMinPasswordLength) {
        QMessageBox::warning(this, tr("Ошибка"),
            tr("Пароль должен содержать не менее %1 символов.").arg(kMinPasswordLength));
        return;
    }

    // --- Создание пользователя ---
    switch (DBManager::instance().registerUser(login, pass1)) {
    case DBManager::RegisterResult::Ok:
        break;
    case DBManager::RegisterResult::LoginTaken:
        QMessageBox::warning(this, tr("Ошибка"), tr("Пользователь с таким именем уже существует."));
        return;
    case DBManager::RegisterResult::Error:
        QMessageBox::critical(this, tr("Ошибка"),
            tr("Не удалось создать аккаунт. Попробуйте позже."));
        return;
    }

    QMessageBox::information(this, tr("Успех"), tr("Вы успешно зарегистрированы."));
    clearFields();  // не держим пароль в полях ввода
    emit registrationSuccessful();
    hide();
}

void RegisterWindow::reject()
{
    // См. комментарий в LoginWindow::reject().
    emit windowClosed();
    QDialog::reject();
}
