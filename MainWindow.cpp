#include "MainWindow.h"

#include "AuctionWindow.h"
#include "DBManager.h"
#include "LoginWindow.h"
#include "RegisterWindow.h"
#include "SettingsWindow.h"
#include "ThemeManager.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFontMetrics>
#include <QGraphicsBlurEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSettings>
#include <QUrl>
#include <QVBoxLayout>

namespace {

    // ---------- Размеры ----------
    constexpr int kMargin = 50;                 // отступ содержимого от краёв экрана
    constexpr int kFloatingWindowOffset = 50;   // отступ диалогов от правого края
    constexpr int kBlurRadius = 10;
    constexpr int kSquareButtonSize = 40;       // настройки / выход / discord
    constexpr int kProfileButtonWidth = 270;
    constexpr int kProfileTextReserve = 70;     // место под иконку и отступы в кнопке профиля

    // ---------- Цвета иконок ----------
    // Иконки на лаймовых/красных кнопках — чёрные, на тёмных — белые.
    constexpr QLatin1String kIconOnAccent("#000000");
    constexpr QLatin1String kIconOnDark("#FFFFFF");

    constexpr QLatin1String kDiscordUrl("https://discord.gg/zYsh9xPefd");

} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle(tr("STALZONE Utility"));
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

    // Дочерние диалоги создаём сразу, но показываем только по запросу.
    m_loginWindow = new LoginWindow(this);
    m_registerWindow = new RegisterWindow(this);
    m_settingsWindow = new SettingsWindow(this);

    setupUi();
    setupConnections();
    tryAutoLogin();
    // На весь экран окно разворачивается в main() через showFullScreen().
}

MainWindow::~MainWindow() = default;  // m_auctionWindow удалит unique_ptr

// ============================================================================
// Построение интерфейса
// ============================================================================

void MainWindow::setupUi()
{
    m_container = new QWidget(this);
    m_container->setObjectName(QStringLiteral("mainContainer"));
    setCentralWidget(m_container);

    auto* mainLayout = new QVBoxLayout(m_container);
    mainLayout->setContentsMargins(kMargin, kMargin, kMargin, kMargin);

    // ---------- Шапка: слева заголовок, справа кнопки ----------
    auto* topLayout = new QHBoxLayout();

    auto* titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(5);

    auto* titleLabel = new QLabel(tr("STALZONE Utility"), m_container);
    titleLabel->setObjectName(QStringLiteral("mainTitle"));
    auto* subtitleLabel = new QLabel(tr("Твой цифровой артефакт и помощник в зоне"), m_container);
    subtitleLabel->setObjectName(QStringLiteral("mainSubtitle"));
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);

    m_settingsBtn = new QPushButton(m_container);
    m_settingsBtn->setObjectName(QStringLiteral("settingsButton"));
    m_settingsBtn->setFixedSize(kSquareButtonSize, kSquareButtonSize);
    m_settingsBtn->setIconSize(QSize(24, 24));

    m_profileBtn = new QPushButton(m_container);
    m_profileBtn->setObjectName(QStringLiteral("profileButton"));
    m_profileBtn->setFixedSize(kProfileButtonWidth, kSquareButtonSize);
    m_profileBtn->setIconSize(QSize(20, 20));

    m_exitBtn = new QPushButton(m_container);
    m_exitBtn->setObjectName(QStringLiteral("exitButton"));
    m_exitBtn->setFixedSize(kSquareButtonSize, kSquareButtonSize);
    m_exitBtn->setIconSize(QSize(24, 24));

    auto* buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(15);
    buttonsLayout->addWidget(m_settingsBtn);
    buttonsLayout->addWidget(m_profileBtn);
    buttonsLayout->addWidget(m_exitBtn);

    topLayout->addLayout(titleLayout);
    topLayout->addStretch();
    topLayout->addLayout(buttonsLayout);

    mainLayout->addLayout(topLayout);
    mainLayout->addSpacing(100);

    // ---------- Кнопка «АУКЦИОН» ----------
    m_auctionBtn = new QPushButton(tr("АУКЦИОН"), m_container);
    m_auctionBtn->setObjectName(QStringLiteral("auctionButton"));
    m_auctionBtn->setFixedSize(300, 60);
    m_auctionBtn->setCursor(Qt::PointingHandCursor);

    auto* auctionRow = new QHBoxLayout();
    auctionRow->addWidget(m_auctionBtn);
    auctionRow->addStretch();
    mainLayout->addLayout(auctionRow);

    mainLayout->addStretch();

    // ---------- Низ: Discord справа ----------
    m_discordBtn = new QPushButton(m_container);
    m_discordBtn->setObjectName(QStringLiteral("discordButton"));
    m_discordBtn->setFixedSize(kSquareButtonSize, kSquareButtonSize);
    m_discordBtn->setIconSize(QSize(24, 24));

    auto* bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_discordBtn);
    mainLayout->addLayout(bottomLayout);

    // Иконки и текст кнопки профиля задаются в одном месте — см. методы ниже.
    updateIcons();
    updateProfileButton();
}

void MainWindow::setupConnections()
{
    // Кнопки главного окна
    connect(m_exitBtn, &QPushButton::clicked, qApp, &QApplication::quit);
    connect(m_profileBtn, &QPushButton::clicked, this, &MainWindow::onProfileButtonClicked);
    connect(m_settingsBtn, &QPushButton::clicked, this, &MainWindow::onSettingsButtonClicked);
    connect(m_discordBtn, &QPushButton::clicked, this, &MainWindow::onDiscordButtonClicked);
    connect(m_auctionBtn, &QPushButton::clicked, this, &MainWindow::onAuctionButtonClicked);

    // Окно входа
    connect(m_loginWindow, &LoginWindow::loginSuccessful, this, &MainWindow::onLoginSuccess);
    connect(m_loginWindow, &LoginWindow::registerRequested, this, &MainWindow::onShowRegisterWindow);
    connect(m_loginWindow, &LoginWindow::windowClosed, this, &MainWindow::removeBlurEffect);

    // Окно регистрации
    connect(m_registerWindow, &RegisterWindow::registrationSuccessful, this, &MainWindow::onShowLoginWindow);
    connect(m_registerWindow, &RegisterWindow::loginRequested, this, &MainWindow::onShowLoginWindow);
    connect(m_registerWindow, &RegisterWindow::windowClosed, this, &MainWindow::removeBlurEffect);

    // Смена темы — перекрашиваем иконки
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
        updateIcons();
        updateProfileButton();
        });
}

void MainWindow::tryAutoLogin()
{
    const QSettings settings;
    if (!settings.value(AuthSettings::kRememberMe, false).toBool())
        return;

    // Входим автоматически, только если такой пользователь всё ещё есть в БД.
    const QString savedUser = settings.value(AuthSettings::kUsername).toString();
    if (!savedUser.isEmpty() && DBManager::instance().userExists(savedUser))
        onLoginSuccess(savedUser);
}

// ============================================================================
// Внешний вид
// ============================================================================

void MainWindow::updateIcons()
{
    auto& theme = ThemeManager::instance();
    const QColor onAccent{ kIconOnAccent };
    const QColor onDark{ kIconOnDark };

    m_settingsBtn->setIcon(theme.getIcon(QStringLiteral("settingsBtn"), onAccent));
    m_exitBtn->setIcon(theme.getIcon(QStringLiteral("exitBtn"), onAccent));
    m_discordBtn->setIcon(theme.getIcon(QStringLiteral("discordBtn"), onDark));
}

void MainWindow::updateProfileButton()
{
    const bool loggedIn = !m_currentUser.isEmpty();

    m_profileBtn->setIcon(ThemeManager::instance().getIcon(
        loggedIn ? QStringLiteral("profileBtnOn") : QStringLiteral("profileBtnOff"),
        QColor(kIconOnAccent)));

    if (!loggedIn) {
        m_profileBtn->setText(tr("Войти | Зарегистрироваться"));
        return;
    }

    // Длинный логин обрезаем с многоточием, чтобы он не вылез за кнопку.
    // Неразрывный пробел (U+00A0) в начале — небольшой зазор между иконкой и текстом.
    const QFontMetrics metrics(m_profileBtn->font());
    const QString name = metrics.elidedText(
        m_currentUser, Qt::ElideRight, kProfileButtonWidth - kProfileTextReserve);
    m_profileBtn->setText(QString(QChar(0x00A0)) + name);
}

void MainWindow::applyBlurEffect()
{
    if (!m_container)
        return;

    // setGraphicsEffect сам удаляет предыдущий эффект, так что утечки не будет.
    auto* effect = new QGraphicsBlurEffect(m_container);
    effect->setBlurRadius(kBlurRadius);
    m_container->setGraphicsEffect(effect);
}

void MainWindow::removeBlurEffect()
{
    if (m_container)
        m_container->setGraphicsEffect(nullptr);
}

// ============================================================================
// Плавающие окна (вход / регистрация / настройки)
// ============================================================================

void MainWindow::positionFloatingWindow(QWidget* window)
{
    // Прижимаем к правому краю экрана и центрируем по вертикали.
    const QPoint topRightGlobal = mapToGlobal(QPoint(width(), 0));
    const int x = topRightGlobal.x() - window->width() - kFloatingWindowOffset;
    const int y = topRightGlobal.y() + (height() - window->height()) / 2;
    window->move(x, y);
}

void MainWindow::showFloatingWindow(QWidget* window)
{
    positionFloatingWindow(window);
    window->show();
    window->raise();
    window->activateWindow();
}

void MainWindow::hideFloatingWindows()
{
    m_loginWindow->hide();
    m_registerWindow->hide();
    m_settingsWindow->hide();
}

// ============================================================================
// Слоты: кнопки
// ============================================================================

void MainWindow::onProfileButtonClicked()
{
    if (m_currentUser.isEmpty()) {
        // Не вошли — показываем окно входа поверх размытого фона.
        hideFloatingWindows();
        applyBlurEffect();
        showFloatingWindow(m_loginWindow);
        return;
    }

    // Уже вошли — по клику появляется меню с пунктом «Выйти».
    QMenu menu(this);
    QAction* logoutAction = menu.addAction(tr("Выйти"));
    const QPoint menuPos = m_profileBtn->mapToGlobal(QPoint(0, m_profileBtn->height()));

    if (menu.exec(menuPos) == logoutAction)
        onLogout();
}

void MainWindow::onSettingsButtonClicked()
{
    hideFloatingWindows();
    removeBlurEffect();  // если до этого было открыто окно входа, фон остался бы размытым
    showFloatingWindow(m_settingsWindow);
}

void MainWindow::onDiscordButtonClicked()
{
    QDesktopServices::openUrl(QUrl(QString(kDiscordUrl)));
}

void MainWindow::onAuctionButtonClicked()
{
    // Окно аукциона создаём лениво — при первом открытии.
    if (!m_auctionWindow) {
        m_auctionWindow = std::make_unique<AuctionWindow>();
        connect(m_auctionWindow.get(), &AuctionWindow::backRequested,
            this, &MainWindow::onAuctionBackRequested);
    }

    hideFloatingWindows();
    removeBlurEffect();

    m_auctionWindow->showFullScreen();
    m_auctionWindow->raise();
    m_auctionWindow->activateWindow();

    hide();  // главное окно прячем, пока открыт аукцион
}

// ============================================================================
// Слоты: события дочерних окон
// ============================================================================

void MainWindow::onAuctionBackRequested()
{
    showFullScreen();
    raise();
    activateWindow();

    if (m_auctionWindow)
        m_auctionWindow->hide();
}

void MainWindow::onLoginSuccess(const QString& login)
{
    m_currentUser = login;
    updateProfileButton();

    m_loginWindow->hide();
    removeBlurEffect();
}

void MainWindow::onShowRegisterWindow()
{
    m_loginWindow->hide();  // размытие остаётся: за регистрацией всё ещё «затемнённый» фон
    m_registerWindow->clearFields();
    showFloatingWindow(m_registerWindow);
}

void MainWindow::onShowLoginWindow()
{
    m_registerWindow->hide();
    showFloatingWindow(m_loginWindow);
}

void MainWindow::onLogout()
{
    m_currentUser.clear();

    // Забываем «запомнить меня», иначе при следующем запуске войдём снова.
    QSettings settings;
    settings.setValue(AuthSettings::kRememberMe, false);
    settings.remove(AuthSettings::kUsername);
    settings.sync();

    updateProfileButton();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QApplication::quit();
    QMainWindow::closeEvent(event);
}
