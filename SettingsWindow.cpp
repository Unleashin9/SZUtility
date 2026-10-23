#include "SettingsWindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

    constexpr int kWindowWidth = 320;
    constexpr int kWindowHeight = 220;

} // namespace

SettingsWindow::SettingsWindow(QWidget* parent) : QDialog(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(kWindowWidth, kWindowHeight);

    // Окно -> container (рамка и фон из QSS) -> заголовок и кнопка.
    auto* container = new QWidget(this);
    container->setObjectName(QStringLiteral("dialogContainer"));

    auto* windowLayout = new QVBoxLayout(this);
    windowLayout->setContentsMargins(0, 0, 0, 0);
    windowLayout->addWidget(container);

    auto* titleLabel = new QLabel(tr("Настройки"), container);
    titleLabel->setObjectName(QStringLiteral("settingsTitle"));  // стиль — в dark_theme.qss
    titleLabel->setAlignment(Qt::AlignCenter);

    m_closeBtn = new QPushButton(tr("Закрыть"), container);
    m_closeBtn->setCursor(Qt::PointingHandCursor);

    auto* containerLayout = new QVBoxLayout(container);
    containerLayout->addWidget(titleLabel);
    containerLayout->addWidget(m_closeBtn, 0, Qt::AlignCenter);

    // reject() скрывает диалог — то же самое произойдёт и по Esc.
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::reject);
}
