#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "aboutdialog.h"
#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"
#include "windowchrome.h"

#include <QAction>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFontDialog>
#include <QGuiApplication>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStringDecoder>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTime>
#include <QTimer>
#include <QToolBar>

namespace {

// Qt 自带的中文翻译漏了文件对话框里的两个标签，这里补上
void localizeFileDialog(QFileDialog &dialog)
{
    if (QLabel *label = dialog.findChild<QLabel *>(QStringLiteral("lookInLabel")))
        label->setText(QStringLiteral("查找范围："));
    if (QLabel *label = dialog.findChild<QLabel *>(QStringLiteral("fileTypeLabel")))
        label->setText(QStringLiteral("文件类型："));
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));
    m_lastDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QFont editorFont(QStringLiteral("Consolas"), 11);
    editorFont.setStyleHint(QFont::Monospace);
    editorFont.setFixedPitch(true);
    ui->editor->setFont(editorFont);
    ui->editor->setTabStopDistance(4 * ui->editor->fontMetrics().horizontalAdvance(QLatin1Char(' ')));
    ui->editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    ui->editor->document()->setModified(false);
    ui->editor->setFocus();

    m_lengthLabel = new QLabel(this);
    m_linesLabel = new QLabel(this);
    m_positionLabel = new QLabel(this);
    for (QLabel *label : {m_lengthLabel, m_linesLabel, m_positionLabel}) {
        label->setStyleSheet(QStringLiteral("padding: 0 10px; color: #40464d;"));
    }
    statusBar()->addPermanentWidget(m_lengthLabel);
    statusBar()->addPermanentWidget(m_linesLabel);
    statusBar()->addPermanentWidget(m_positionLabel);
    statusBar()->setSizeGripEnabled(false);

    connect(ui->actNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actSaveAs, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actExit, &QAction::triggered, this, &MainWindow::close);

    connect(ui->actUndo, &QAction::triggered, ui->editor, &QPlainTextEdit::undo);
    connect(ui->actRedo, &QAction::triggered, ui->editor, &QPlainTextEdit::redo);
    connect(ui->actCut, &QAction::triggered, ui->editor, &QPlainTextEdit::cut);
    connect(ui->actCopy, &QAction::triggered, ui->editor, &QPlainTextEdit::copy);
    connect(ui->actPaste, &QAction::triggered, ui->editor, &QPlainTextEdit::paste);
    connect(ui->actSelectAll, &QAction::triggered, ui->editor, &QPlainTextEdit::selectAll);
    connect(ui->actFind, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui->actReplace, &QAction::triggered, this, &MainWindow::showReplaceDialog);

    connect(ui->actFont, &QAction::triggered, this, &MainWindow::chooseFont);
    connect(ui->actFontColor, &QAction::triggered, this, &MainWindow::chooseFontColor);
    connect(ui->actTextBackground, &QAction::triggered, this, &MainWindow::chooseTextBackground);
    connect(ui->actEditorBackground, &QAction::triggered, this, &MainWindow::chooseEditorBackground);

    connect(ui->actWordWrap, &QAction::toggled, this, [this](bool checked) {
        ui->editor->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
        statusBar()->showMessage(checked ? QStringLiteral("已打开自动换行") : QStringLiteral("已关闭自动换行"), 2000);
    });
    connect(ui->actLineNumber, &QAction::toggled, this, [this](bool checked) {
        ui->editor->setLineNumberVisible(checked);
        statusBar()->showMessage(checked ? QStringLiteral("已显示行号") : QStringLiteral("已隐藏行号"), 2000);
    });
    connect(ui->actToolBar, &QAction::toggled, ui->toolBar, &QToolBar::setVisible);
    connect(ui->actStatusBar, &QAction::toggled, statusBar(), &QStatusBar::setVisible);
    connect(ui->actAbout, &QAction::triggered, this, &MainWindow::showAbout);

    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setInterval(60 * 1000);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);
    connect(ui->actAutoSave, &QAction::toggled, this, [this](bool checked) {
        if (checked) {
            m_autoSaveTimer->start();
            statusBar()->showMessage(QStringLiteral("已开启定时自动保存，间隔 60 秒"), 3000);
        } else {
            m_autoSaveTimer->stop();
            statusBar()->showMessage(QStringLiteral("已关闭定时自动保存"), 2000);
        }
    });
    if (ui->actAutoSave->isChecked())
        m_autoSaveTimer->start();

    connect(ui->editor, &QPlainTextEdit::textChanged, this, &MainWindow::updateStatusBar);
    connect(ui->editor, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatusBar);
    connect(ui->editor, &QPlainTextEdit::copyAvailable, this, &MainWindow::updateSelectionActions);
    connect(ui->editor->document(), &QTextDocument::undoAvailable, ui->actUndo, &QAction::setEnabled);
    connect(ui->editor->document(), &QTextDocument::redoAvailable, ui->actRedo, &QAction::setEnabled);
    connect(ui->editor->document(), &QTextDocument::modificationChanged, this, &MainWindow::updateWindowTitle);
    connect(QGuiApplication::clipboard(), &QClipboard::dataChanged, this, &MainWindow::updatePasteAction);

    ui->actUndo->setEnabled(false);
    ui->actRedo->setEnabled(false);
    updateSelectionActions(false);
    updatePasteAction();
    updateStatusBar();
    updateWindowTitle();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::displayName() const
{
    if (m_filePath.isEmpty())
        return QStringLiteral("新建文本文件");
    return QFileInfo(m_filePath).fileName();
}

void MainWindow::newFile()
{
    if (!maybeSave())
        return;

    ui->editor->clear();
    m_filePath.clear();
    ui->editor->document()->setModified(false);
    ui->editor->document()->clearUndoRedoStacks();
    ui->editor->setFocus();
    updateWindowTitle();
    updateStatusBar();
    statusBar()->showMessage(QStringLiteral("已新建空白文档"), 2000);
}

bool MainWindow::openFile()
{
    if (!maybeSave())
        return false;

    QFileDialog dialog(this, QStringLiteral("打开文件"), m_lastDir,
                       QStringLiteral("文本文件 (*.txt *.md *.log *.cpp *.h *.py);;所有文件 (*.*)"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    localizeFileDialog(dialog);
    applyLightTitleBar(&dialog);
    if (dialog.exec() != QDialog::Accepted)
        return false;

    return loadFile(dialog.selectedFiles().value(0));
}

bool MainWindow::loadFile(const QString &path)
{
    const QFileInfo info(path);
    if (!info.exists()) {
        QMessageBox::warning(this, QStringLiteral("打开失败"),
                             QStringLiteral("文件不存在：\n%1").arg(QDir::toNativeSeparators(path)));
        return false;
    }
    if (info.isDir()) {
        QMessageBox::warning(this, QStringLiteral("打开失败"),
                             QStringLiteral("%1 是一个文件夹，不是文本文件。")
                                 .arg(QDir::toNativeSeparators(path)));
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, QStringLiteral("打开失败"),
                              QStringLiteral("无法读取文件：\n%1\n\n%2")
                                  .arg(QDir::toNativeSeparators(path), file.errorString()));
        return false;
    }
    const QByteArray data = file.readAll();
    file.close();

    if (data.contains('\0')) {
        QMessageBox::warning(this, QStringLiteral("打开失败"),
                             QStringLiteral("这个文件里含有二进制数据，不是纯文本，编辑器打不开。"));
        return false;
    }

    QStringDecoder decoder(QStringDecoder::Utf8);
    QString text = decoder.decode(data);
    if (decoder.hasError()) {
        QMessageBox::warning(this, QStringLiteral("打开失败"),
                             QStringLiteral("文件不是有效的 UTF-8 文本，可能是 GBK 编码。\n"
                                            "先用记事本另存为 UTF-8，再打开。"));
        return false;
    }
    if (text.startsWith(QChar(0xFEFF)))
        text.remove(0, 1);

    ui->editor->setPlainText(text);
    ui->editor->document()->setModified(false);
    ui->editor->document()->clearUndoRedoStacks();

    m_filePath = info.absoluteFilePath();
    m_lastDir = info.absolutePath();
    ui->editor->setFocus();
    updateWindowTitle();
    updateStatusBar();
    statusBar()->showMessage(QStringLiteral("已打开 %1").arg(info.fileName()), 3000);
    return true;
}

void MainWindow::openPath(const QString &path)
{
    loadFile(path);
}

bool MainWindow::saveFile()
{
    if (m_filePath.isEmpty())
        return saveFileAs();
    return writeFile(m_filePath, false);
}

bool MainWindow::saveFileAs()
{
    QFileDialog dialog(this, QStringLiteral("另存为"), m_lastDir,
                       QStringLiteral("文本文件 (*.txt);;Markdown (*.md);;所有文件 (*.*)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix(QStringLiteral("txt"));
    dialog.selectFile(displayName());
    localizeFileDialog(dialog);
    applyLightTitleBar(&dialog);
    if (dialog.exec() != QDialog::Accepted)
        return false;

    return writeFile(dialog.selectedFiles().value(0), false);
}

bool MainWindow::writeFile(const QString &path, bool quiet)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (quiet)
            statusBar()->showMessage(QStringLiteral("自动保存失败：%1").arg(file.errorString()), 4000);
        else
            QMessageBox::critical(this, QStringLiteral("保存失败"),
                                  QStringLiteral("无法写入文件：\n%1\n\n%2")
                                      .arg(QDir::toNativeSeparators(path), file.errorString()));
        return false;
    }

    const QByteArray data = ui->editor->toPlainText().toUtf8();
    if (file.write(data) != data.size() || !file.commit()) {
        if (quiet)
            statusBar()->showMessage(QStringLiteral("自动保存失败：%1").arg(file.errorString()), 4000);
        else
            QMessageBox::critical(this, QStringLiteral("保存失败"),
                                  QStringLiteral("写入文件时出错：\n%1\n\n%2")
                                      .arg(QDir::toNativeSeparators(path), file.errorString()));
        return false;
    }

    m_filePath = QFileInfo(path).absoluteFilePath();
    m_lastDir = QFileInfo(path).absolutePath();
    ui->editor->document()->setModified(false);
    updateWindowTitle();
    updateStatusBar();

    if (quiet)
        statusBar()->showMessage(
            QStringLiteral("已自动保存 %1").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss"))), 4000);
    else
        statusBar()->showMessage(QStringLiteral("已保存 %1").arg(QFileInfo(path).fileName()), 3000);
    return true;
}

bool MainWindow::maybeSave()
{
    if (!ui->editor->document()->isModified())
        return true;

    QMessageBox box(this);
    box.setWindowTitle(QStringLiteral("简单文本编辑器"));
    box.setIcon(QMessageBox::Question);
    box.setText(QStringLiteral("文档已修改，是否保存？"));
    box.setInformativeText(QStringLiteral("如果不保存，对 %1 的修改会丢失。").arg(displayName()));
    QPushButton *saveButton = box.addButton(QStringLiteral("保存(&S)"), QMessageBox::AcceptRole);
    QPushButton *discardButton = box.addButton(QStringLiteral("不保存(&N)"), QMessageBox::DestructiveRole);
    box.addButton(QStringLiteral("取消"), QMessageBox::RejectRole);
    box.setDefaultButton(saveButton);
    box.exec();

    if (box.clickedButton() == saveButton)
        return saveFile();
    if (box.clickedButton() == discardButton)
        return true;
    return false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

void MainWindow::showFindDialog()
{
    if (!m_findDialog) {
        m_findDialog = new FindDialog(this);
        connect(m_findDialog, &FindDialog::findNextRequested, this, &MainWindow::findNext);
    }
    const QString selected = ui->editor->textCursor().selectedText();
    if (!selected.isEmpty() && !selected.contains(QChar(0x2029)))
        m_findDialog->setSearchText(selected);
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
    m_findDialog->focusSearchField();
}

void MainWindow::showReplaceDialog()
{
    if (!m_replaceDialog) {
        m_replaceDialog = new ReplaceDialog(this);
        connect(m_replaceDialog, &ReplaceDialog::findNextRequested, this, &MainWindow::findNext);
        connect(m_replaceDialog, &ReplaceDialog::replaceRequested, this, &MainWindow::replaceOne);
        connect(m_replaceDialog, &ReplaceDialog::replaceAllRequested, this, &MainWindow::replaceAll);
    }
    const QString selected = ui->editor->textCursor().selectedText();
    if (!selected.isEmpty() && !selected.contains(QChar(0x2029)))
        m_replaceDialog->setSearchText(selected);
    m_replaceDialog->show();
    m_replaceDialog->raise();
    m_replaceDialog->activateWindow();
    m_replaceDialog->focusSearchField();
}

QTextCursor MainWindow::findText(const QString &text, bool caseSensitive,
                                 bool backward, bool *wrapped) const
{
    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;
    if (backward)
        flags |= QTextDocument::FindBackward;

    QTextCursor cursor = ui->editor->textCursor();
    if (cursor.hasSelection())
        cursor.setPosition(backward ? cursor.selectionStart() : cursor.selectionEnd());

    QTextCursor found = ui->editor->document()->find(text, cursor, flags);
    *wrapped = false;
    if (found.isNull()) {
        QTextCursor wrapCursor(ui->editor->document());
        wrapCursor.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
        found = ui->editor->document()->find(text, wrapCursor, flags);
        *wrapped = !found.isNull();
    }
    return found;
}

void MainWindow::findNext(const QString &text, bool caseSensitive, bool backward)
{
    if (text.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("请输入查找内容"), 2000);
        return;
    }

    bool wrapped = false;
    const QTextCursor found = findText(text, caseSensitive, backward, &wrapped);
    if (found.isNull()) {
        QMessageBox::information(this, QStringLiteral("查找"),
                                 QStringLiteral("找不到“%1”。").arg(text));
        return;
    }

    ui->editor->setTextCursor(found);
    ui->editor->ensureCursorVisible();
    if (wrapped) {
        statusBar()->showMessage(backward ? QStringLiteral("已回到文档末尾继续查找")
                                          : QStringLiteral("已回到文档开头继续查找"), 3000);
    } else {
        statusBar()->showMessage(QStringLiteral("已找到“%1”").arg(text), 2000);
    }
}

void MainWindow::replaceOne(const QString &text, const QString &replacement,
                            bool caseSensitive, bool backward)
{
    if (text.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("请输入查找内容"), 2000);
        return;
    }

    QTextCursor cursor = ui->editor->textCursor();
    bool sameSelection = false;
    if (cursor.hasSelection()) {
        const QString selected = cursor.selectedText();
        sameSelection = caseSensitive
                ? selected == text
                : selected.compare(text, Qt::CaseInsensitive) == 0;
    }

    if (sameSelection) {
        cursor.insertText(replacement);
        ui->editor->setTextCursor(cursor);
        statusBar()->showMessage(QStringLiteral("已替换 1 处"), 2000);
        return;
    }

    findNext(text, caseSensitive, backward);
}

void MainWindow::replaceAll(const QString &text, const QString &replacement, bool caseSensitive)
{
    if (text.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("请输入查找内容"), 2000);
        return;
    }

    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;

    QTextCursor editCursor(ui->editor->document());
    editCursor.beginEditBlock();

    int count = 0;
    QTextCursor found = ui->editor->document()->find(text, 0, flags);
    while (!found.isNull()) {
        found.insertText(replacement);
        ++count;
        found = ui->editor->document()->find(text, found, flags);
    }
    editCursor.endEditBlock();

    if (count == 0) {
        QMessageBox::information(this, QStringLiteral("替换"),
                                 QStringLiteral("找不到“%1”，没有替换任何内容。").arg(text));
    } else {
        QMessageBox::information(this, QStringLiteral("替换"),
                                 QStringLiteral("共替换 %1 处。").arg(count));
    }
}

void MainWindow::chooseFont()
{
    QFontDialog dialog(ui->editor->font(), this);
    dialog.setOption(QFontDialog::DontUseNativeDialog, true);
    dialog.setWindowTitle(QStringLiteral("选择字体"));
    applyLightTitleBar(&dialog);
    if (dialog.exec() != QDialog::Accepted)
        return;

    const QFont font = dialog.selectedFont();
    ui->editor->setFont(font);
    ui->editor->setTabStopDistance(4 * ui->editor->fontMetrics().horizontalAdvance(QLatin1Char(' ')));
    ui->editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    ui->editor->refreshLineNumberArea();
    updateStatusBar();
    statusBar()->showMessage(QStringLiteral("字体已改为 %1 %2 磅").arg(font.family()).arg(font.pointSize()), 3000);
}

void MainWindow::chooseFontColor()
{
    QColorDialog dialog(ui->editor->currentCharFormat().foreground().color(), this);
    dialog.setWindowTitle(QStringLiteral("字体颜色"));
    applyLightTitleBar(&dialog);
    if (dialog.exec() != QDialog::Accepted)
        return;

    const QColor color = dialog.currentColor();

    QTextCharFormat format;
    format.setForeground(color);
    ui->editor->mergeCurrentCharFormat(format);
    statusBar()->showMessage(QStringLiteral("字体颜色已修改"), 2000);
}

void MainWindow::chooseTextBackground()
{
    QColorDialog dialog(Qt::yellow, this);
    dialog.setWindowTitle(QStringLiteral("字体背景色"));
    applyLightTitleBar(&dialog);
    if (dialog.exec() != QDialog::Accepted)
        return;

    const QColor color = dialog.currentColor();

    QTextCharFormat format;
    format.setBackground(color);
    ui->editor->mergeCurrentCharFormat(format);
    statusBar()->showMessage(QStringLiteral("字体背景色已修改"), 2000);
}

void MainWindow::chooseEditorBackground()
{
    QColorDialog dialog(ui->editor->palette().color(QPalette::Base), this);
    dialog.setWindowTitle(QStringLiteral("编辑器背景色"));
    applyLightTitleBar(&dialog);
    if (dialog.exec() != QDialog::Accepted)
        return;

    const QColor color = dialog.currentColor();

    QPalette palette = ui->editor->palette();
    palette.setColor(QPalette::Base, color);
    ui->editor->setPalette(palette);
    ui->editor->refreshLineNumberArea();
    statusBar()->showMessage(QStringLiteral("编辑器背景色已修改"), 2000);
}

void MainWindow::showAbout()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::updateStatusBar()
{
    const QString text = ui->editor->toPlainText();
    m_lengthLabel->setText(QStringLiteral("length: %1").arg(text.length()));
    m_linesLabel->setText(QStringLiteral("lines: %1").arg(ui->editor->blockCount()));

    const QTextCursor cursor = ui->editor->textCursor();
    m_positionLabel->setText(QStringLiteral("Ln: %1   Col: %2")
                                 .arg(cursor.blockNumber() + 1)
                                 .arg(cursor.positionInBlock() + 1));
}

void MainWindow::updateWindowTitle()
{
    const QString star = ui->editor->document()->isModified() ? QStringLiteral("* ") : QString();
    setWindowTitle(star + displayName() + QStringLiteral(" - 简单文本编辑器"));
}

void MainWindow::updatePasteAction()
{
    const QMimeData *mime = QGuiApplication::clipboard()->mimeData();
    ui->actPaste->setEnabled(mime && mime->hasText());
}

void MainWindow::updateSelectionActions(bool hasSelection)
{
    ui->actCut->setEnabled(hasSelection);
    ui->actCopy->setEnabled(hasSelection);
}

void MainWindow::autoSave()
{
    if (!ui->editor->document()->isModified())
        return;
    if (m_filePath.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("当前文档还没有保存过，自动保存已跳过"), 3000);
        return;
    }
    writeFile(m_filePath, true);
}
