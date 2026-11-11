#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class ReplaceDialog; }
QT_END_NAMESPACE

class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent = nullptr);
    ~ReplaceDialog();

    void setSearchText(const QString &text);
    void focusSearchField();

signals:
    void findNextRequested(const QString &text, bool caseSensitive, bool backward);
    void replaceRequested(const QString &text, const QString &replacement,
                          bool caseSensitive, bool backward);
    void replaceAllRequested(const QString &text, const QString &replacement, bool caseSensitive);

private slots:
    void onFindNextClicked();
    void onReplaceClicked();
    void onReplaceAllClicked();

private:
    bool checkSearchText();
    Ui::ReplaceDialog *ui;
};

#endif // REPLACEDIALOG_H
