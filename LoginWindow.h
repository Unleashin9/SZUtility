#pragma once

#include <QDialog>
#include <QLatin1String>

class QAction;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QShowEvent;

/// Ключи QSettings, общие для LoginWindow и MainWindow (чтобы не дублировать строки).
namespace AuthSettings {
    inline constexpr QLatin1String kRememberMe("auth/remember_me");
    inline constexpr QLatin1String kUsername("auth/username");
}

/**
 * Окно входа (безрамочный диалог, показывается поверх главного окна).
 *
 * Сигналы:
 *   loginSuccessful(login) — логин/пароль верны;
 *   registerRequested()    — нажата «Создать аккаунт»;
 *   windowClosed()         — окно скрыто (крестик, Esc или успешный вход);
 *                            главное окно по нему снимает размытие фона.
 */
class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget* parent = nullptr);

    /// Вызывается и при нажатии на крестик, и при Esc, и при QDialog::close().
    void reject() override;

signals:
    void loginSuccessful(const QString& login);
    void registerRequested();
    void windowClosed();

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onLoginClicked();
    void togglePasswordVisibility();

private:
    void setupUi();
    void setupConnections();
    void updatePasswordToggleIcon();

    // Поля ввода
    QLineEdit* m_loginEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QAction* m_togglePasswordAction = nullptr;  ///< «глазик» внутри поля пароля
    QCheckBox* m_rememberMeCheckbox = nullptr;

    // Кнопки
    QPushButton* m_loginBtn = nullptr;
    QPushButton* m_registerBtn = nullptr;
    QPushButton* m_forgotBtn = nullptr;
    QPushButton* m_closeBtn = nullptr;
};
