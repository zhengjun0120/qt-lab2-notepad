#include "finddialog.h"
#include "ui_finddialog.h"

#include "windowchrome.h"

#include <QMessageBox>

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    applyLightTitleBar(this);
    adjustSize();

    setTabOrder(ui->lineEditFind, ui->checkBoxCase);
    setTabOrder(ui->checkBoxCase, ui->radioUp);
    setTabOrder(ui->radioUp, ui->radioDown);
    setTabOrder(ui->radioDown, ui->btnFindNext);
    setTabOrder(ui->btnFindNext, ui->btnCancel);

    connect(ui->btnFindNext, &QPushButton::clicked, this, &FindDialog::onFindNextClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &FindDialog::hide);
    connect(ui->lineEditFind, &QLineEdit::returnPressed, this, &FindDialog::onFindNextClicked);
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::setSearchText(const QString &text)
{
    ui->lineEditFind->setText(text);
}

void FindDialog::focusSearchField()
{
    ui->lineEditFind->setFocus();
    ui->lineEditFind->selectAll();
}

void FindDialog::onFindNextClicked()
{
    const QString text = ui->lineEditFind->text();
    if (text.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("查找"), QStringLiteral("请先输入要查找的内容。"));
        return;
    }

    emit findNextRequested(text, ui->checkBoxCase->isChecked(), ui->radioUp->isChecked());
}
