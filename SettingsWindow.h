#pragma once

#include <QDialog>

class QPushButton;

/// Окно настроек (пока заглушка с кнопкой «Закрыть»).
/// Закрывается само: кнопка вызывает reject(), который скрывает диалог.
class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget* parent = nullptr);

private:
    QPushButton* m_closeBtn = nullptr;
};
