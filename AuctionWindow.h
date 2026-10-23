#pragma once

#include <QWidget>

class QLabel;
class QPushButton;

class AuctionWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AuctionWindow(QWidget* parent = nullptr);
    ~AuctionWindow() override;

signals:
    void backRequested();

private slots:
    void onBackClicked();
    void onRecentClicked();
    void onFavoriteClicked();

private:
    void setupUi();
    void setupConnections();
    void updateIcons();

    QLabel* m_headerTitle = nullptr;
    QPushButton* m_backBtn = nullptr;
    QWidget* m_quickAccessPanel = nullptr;
    QPushButton* m_recentBtn = nullptr;
    QPushButton* m_favoritesBtn = nullptr;
};