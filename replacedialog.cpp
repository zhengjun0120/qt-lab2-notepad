#include "replacedialog.h"
#include "ui_replacedialog.h"

#include "windowchrome.h"

#include <QMessageBox>

ReplaceDialog::ReplaceDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReplaceDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    applyLightTitleBar(this);
    adjustSize();

    setTabOrder(ui->lineEditFind, ui->lineEditReplace);
    setTabOrder(ui->lineEditReplace, ui->checkBoxCase);
    setTabOrder(ui->checkBoxCase, ui->radioUp);
    setTabOrder(ui->radioUp, ui->radioDown);
    setTabOrder(ui->radioDown, ui->btnFindNext);
    setTabOrder(ui->btnFindNext, ui->btnReplace);
    setTabOrder(ui->btnReplace, ui->btnReplaceAll);
    setTabOrder(ui->btnReplaceAll, ui->btnCancel);

    connect(ui->btnFindNext, &QPushButton::clicked, this, &ReplaceDialog::onFindNextClicked);
    connect(ui->btnReplace, &QPushButton::clicked, this, &ReplaceDialog::onReplaceClicked);
    connect(ui->btnReplaceAll, &QPushButton::clicked, this, &ReplaceDialog::onReplaceAllClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &ReplaceDialog::hide);
    connect(ui->lineEditFind, &QLineEdit::returnPressed, this, &ReplaceDialog::onFindNextClicked);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

void ReplaceDialog::setSearchText(const QString &text)
{
    ui->lineEditFind->setText(text);
}

void ReplaceDialog::focusSearchField()
{
    ui->lineEditFind->setFocus();
    ui->lineEditFind->selectAll();
}

bool ReplaceDialog::checkSearchText()
{
    if (!ui->lineEditFind->text().isEmpty())
        return true;

    QMessageBox::information(this, QStringLiteral("替换"), QStringLiteral("请先输入要查找的内容。"));
    return false;
}

void ReplaceDialog::onFindNextClicked()
{
    if (!checkSearchText())
        return;

    emit findNextRequested(ui->lineEditFind->text(),
                           ui->checkBoxCase->isChecked(),
                           ui->radioUp->isChecked());
}

void ReplaceDialog::onReplaceClicked()
{
    if (!checkSearchText())
        return;

    emit replaceRequested(ui->lineEditFind->text(),
                          ui->lineEditReplace->text(),
                          ui->checkBoxCase->isChecked(),
                          ui->radioUp->isChecked());
}

void ReplaceDialog::onReplaceAllClicked()
{
    if (!checkSearchText())
        return;

    emit replaceAllRequested(ui->lineEditFind->text(),
                             ui->lineEditReplace->text(),
                             ui->checkBoxCase->isChecked());
}
