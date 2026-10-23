#pragma once

#include <QMainWindow>
#include <QString>

#include <memory>

class AuctionWindow;
class LoginWindow;
class RegisterWindow;
class SettingsWindow;

class QCloseEvent;
class QPushButton;
class QWidget;

/**
 * Главное окно приложения (на весь экран, без рамки).
 *
 * Содержит шапку (настройки, профиль, выход), кнопку «АУКЦИОН» и ссылку на Discord.
 * Окна входа, регистрации и настроек — «плавающие» диалоги справа от центра.
 * Окно аукциона — отдельное полноэкранное окно, создаётся при первом открытии.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // Нажатия кнопок шапки
    void onProfileButtonClicked();
    void onSettingsButtonClicked();
    void onDiscordButtonClicked();
    void onAuctionButtonClicked();

    // Реакции на события дочерних окон
    void onLoginSuccess(const QString& login);
    void onShowRegisterWindow();
    void onShowLoginWindow();
    void onAuctionBackRequested();
    void onLogout();

private:
    void setupUi();
    void setupConnections();
    void tryAutoLogin();

    // Внешний вид
    void updateIcons();          ///< перекрашивает иконки кнопок (при смене темы)
    void updateProfileButton();  ///< иконка и текст кнопки профиля по m_currentUser
    void applyBlurEffect();
    void removeBlurEffect();

    // Работа с плавающими окнами
    void positionFloatingWindow(QWidget* window);
    void showFloatingWindow(QWidget* window);
    void hideFloatingWindows();

    // Кнопки шапки и низа
    QPushButton* m_profileBtn = nullptr;
    QPushButton* m_settingsBtn = nullptr;
    QPushButton* m_exitBtn = nullptr;
    QPushButton* m_discordBtn = nullptr;
    QPushButton* m_auctionBtn = nullptr;

    QWidget* m_container = nullptr;  ///< корневой виджет; именно его размываем под диалогами
    QString m_currentUser;           ///< пусто = никто не вошёл

    // Дочерние диалоги (принадлежат this, удаляются Qt автоматически)
    LoginWindow* m_loginWindow = nullptr;
    RegisterWindow* m_registerWindow = nullptr;
    SettingsWindow* m_settingsWindow = nullptr;

    // Окно аукциона без родителя (отдельное окно), поэтому им владеет unique_ptr.
    std::unique_ptr<AuctionWindow> m_auctionWindow;
};
