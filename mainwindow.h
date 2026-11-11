#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextCursor>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QLabel;
class QTimer;
class FindDialog;
class ReplaceDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void openPath(const QString &path);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    bool openFile();
    bool saveFile();
    bool saveFileAs();

    void showFindDialog();
    void showReplaceDialog();
    void findNext(const QString &text, bool caseSensitive, bool backward);
    void replaceOne(const QString &text, const QString &replacement,
                    bool caseSensitive, bool backward);
    void replaceAll(const QString &text, const QString &replacement, bool caseSensitive);

    void chooseFont();
    void chooseFontColor();
    void chooseTextBackground();
    void chooseEditorBackground();
    void showAbout();

    void updateStatusBar();
    void updateWindowTitle();
    void updatePasteAction();
    void updateSelectionActions(bool hasSelection);
    void autoSave();

private:
    bool maybeSave();
    bool loadFile(const QString &path);
    bool writeFile(const QString &path, bool quiet);
    QTextCursor findText(const QString &text, bool caseSensitive,
                         bool backward, bool *wrapped) const;
    QString displayName() const;

    Ui::MainWindow *ui;
    QString m_filePath;
    QString m_lastDir;
    FindDialog *m_findDialog = nullptr;
    ReplaceDialog *m_replaceDialog = nullptr;
    QTimer *m_autoSaveTimer = nullptr;
    QLabel *m_lengthLabel = nullptr;
    QLabel *m_linesLabel = nullptr;
    QLabel *m_positionLabel = nullptr;
};

#endif // MAINWINDOW_H
