#include "AuctionWindow.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "ThemeManager.h"

namespace {
    constexpr int kMargin = 50;
    constexpr int kBackButtonSize = 40;
    constexpr int kBackIconSize = 24;
    constexpr int kHeaderButtonHeight = 40;
    constexpr int kQuickAccessIconSize = 16;

    constexpr QLatin1String kIconOnRed("#000000");
    constexpr QLatin1String kIconWhite("#FFFFFF");
    constexpr QLatin1String kIconAccent("#A3E635");
}

AuctionWindow::AuctionWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle(tr("SZUtility - Аукцион"));
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setObjectName(QStringLiteral("auctionWindow"));

    setupUi();
    setupConnections();
}

AuctionWindow::~AuctionWindow() = default;

void AuctionWindow::setupUi()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(kMargin, kMargin, kMargin, kMargin);
    mainLayout->setSpacing(20);

    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(15);
    
    // НАДПИСЬ STALZONE Utility / / / АУКЦИОН
    m_headerTitle = new QLabel(this);
    m_headerTitle->setObjectName(QStringLiteral("auctionHeaderTitle"));
    m_headerTitle->setTextFormat(Qt::RichText);
    m_headerTitle->setText(tr("<span style='color:#A3E635; font-weight:bold;'> STALZONE Utility / / / АУКЦИОН</span>"));

    // КНОПКА ВЫХОДА
    m_backBtn = new QPushButton(this);
    m_backBtn->setObjectName(QStringLiteral("backButton"));
    m_backBtn->setFixedSize(kBackButtonSize, kBackButtonSize);
    m_backBtn->setIconSize(QSize(kBackIconSize, kBackIconSize));
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setToolTip(tr("На главную"));

    // РАСПОЛОЖЕНИЕ В ОКНЕ
    headerLayout->addWidget(m_headerTitle, 0, Qt::AlignLeft | Qt::AlignVCenter);
    headerLayout->addStretch();
    headerLayout->addWidget(m_backBtn, 0, Qt::AlignRight | Qt::AlignVCenter);

    mainLayout->addLayout(headerLayout);

    m_quickAccessPanel = new QWidget(this);
    m_quickAccessPanel->setObjectName(QStringLiteral("auctionPanel"));
    m_quickAccessPanel->setAttribute(Qt::WA_StyledBackground, true);

    // МОДАЛКА
    auto* auctionPanel = new QVBoxLayout(m_quickAccessPanel);
    auctionPanel->setContentsMargins(20, 15, 20, 15);
    auctionPanel->setSpacing(10);

    // ЗАГОЛОВОК МОДАЛКИ
    auto* panelTitle = new QLabel(tr("БЫСТРЫЙ ДОСТУП"), m_quickAccessPanel);
    panelTitle->setObjectName(QStringLiteral("panelTitle"));
    auctionPanel->addWidget(panelTitle);

    auto quickColumn = new QVBoxLayout();
    quickColumn->setSpacing(8);

    // КНОПКА НЕДАВНО ОТКРЫТЫЕ
    m_recentBtn = new QPushButton(tr("Недавно открытые"), m_quickAccessPanel);
    m_recentBtn->setObjectName(QStringLiteral("outlineButton"));
    m_recentBtn->setFixedHeight(36);
    m_recentBtn->setIconSize(QSize(kQuickAccessIconSize, kQuickAccessIconSize));
    m_recentBtn->setCursor(Qt::PointingHandCursor);
    m_recentBtn->setCheckable(true);
    m_recentBtn->setAutoExclusive(true);

    // КНОПКА ИЗБРАННОЕ
    m_favoritesBtn = new QPushButton(tr("Избранное"), m_quickAccessPanel);
    m_favoritesBtn->setObjectName(QStringLiteral("outlineButton"));
    m_favoritesBtn->setFixedHeight(36);
    m_favoritesBtn->setIconSize(QSize(kQuickAccessIconSize, kQuickAccessIconSize));
    m_favoritesBtn->setCursor(Qt::PointingHandCursor);
    m_favoritesBtn->setCheckable(true);
    m_favoritesBtn->setAutoExclusive(true);

    quickColumn->addWidget(m_recentBtn, 1);
    quickColumn->addWidget(m_favoritesBtn, 1);
    auctionPanel->addLayout(quickColumn);

    mainLayout->addWidget(m_quickAccessPanel);
    mainLayout->addStretch();

    updateIcons();
}

void AuctionWindow::setupConnections()
{
    connect(m_backBtn, &QPushButton::clicked, this, &AuctionWindow::onBackClicked);
    connect(m_recentBtn, &QPushButton::clicked, this, &AuctionWindow::onRecentClicked);
    connect(m_recentBtn, &QPushButton::toggled, this, &AuctionWindow::updateIcons);
    connect(m_favoritesBtn, &QPushButton::clicked, this, &AuctionWindow::onFavoriteClicked);
    connect(m_favoritesBtn, &QPushButton::toggled, this, &AuctionWindow::updateIcons);


    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
        this, &AuctionWindow::updateIcons);
}

void AuctionWindow::updateIcons()
{
    auto& theme = ThemeManager::instance();

    m_backBtn->setIcon(theme.getIcon(
        QStringLiteral("back"),
        QColor(kIconOnRed),
        QStringLiteral("AuctionWindow")));

    m_recentBtn->setIcon(theme.getIcon(
        QStringLiteral("recent"),
        m_recentBtn->isChecked() ? QColor(kIconAccent) : QColor(kIconWhite),
        QStringLiteral("AuctionWindow")));

    m_favoritesBtn->setIcon(theme.getIcon(
        QStringLiteral("favoriteoff"),
        m_favoritesBtn->isChecked() ? QColor(kIconAccent) : QColor(kIconWhite),
        QStringLiteral("AuctionWindow")));
}

void AuctionWindow::onBackClicked()
{
    emit backRequested();
}

void AuctionWindow::onRecentClicked()
{
}

void AuctionWindow::onFavoriteClicked()
{
}
