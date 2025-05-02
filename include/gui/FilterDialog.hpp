#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class FilterDialog : public QDialog {
    Q_OBJECT

public:
    explicit FilterDialog(QWidget* parent = nullptr);
    QString getFilter() const { return filter_edit_->text(); }

private:
    QLineEdit* filter_edit_;
    QPushButton* ok_button_;
    QPushButton* cancel_button_;
}; 