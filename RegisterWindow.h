#pragma once

#include <QDialog>

class QAction;
class QLineEdit;
class QPushButton;
class QShowEvent;

/**
 * Окно регистрации (безрамочный диалог).
 *
 * Сигналы:
 *   registrationSuccessful() — аккаунт создан;
 *   loginRequested()         — нажата «Войти» (у пользователя уже есть аккаунт);
 *   windowClosed()           — окно закрыто крестиком/Esc (главное окно снимает размытие).
 */
class RegisterWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget* parent = nullptr);

    /// Очищает все поля и прячет пароли (вызывается перед показом окна).
    void clearFields();

    /// Вызывается при крестике, Esc и QDialog::close().
    void reject() override;

signals:
    void registrationSuccessful();
    void loginRequested();
    void windowClosed();

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onRegisterClicked();

private:
    void setupUi();
    void setupConnections();

    /// Создаёт поле пароля с «глазиком». Указатель на действие возвращается в toggleAction.
    QLineEdit* createPasswordEdit(const QString& placeholder, QWidget* parent,
        QAction*& toggleAction);
    void togglePasswordVisibility(QLineEdit* edit, QAction* action);
    void updatePasswordIcon(QLineEdit* edit, QAction* action);

    QLineEdit* m_loginEdit = nullptr;
    QLineEdit* m_passwordEdit1 = nullptr;
    QLineEdit* m_passwordEdit2 = nullptr;
    QLineEdit* m_referralEdit = nullptr;

    QAction* m_togglePass1Action = nullptr;
    QAction* m_togglePass2Action = nullptr;

    QPushButton* m_registerBtn = nullptr;
    QPushButton* m_loginBtn = nullptr;
    QPushButton* m_closeBtn = nullptr;
};
