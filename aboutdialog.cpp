#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "windowchrome.h"

#include <QIcon>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    applyLightTitleBar(this);
    adjustSize();

    ui->labelIcon->setPixmap(QIcon(QStringLiteral(":/icons/app.svg")).pixmap(72, 72));
    connect(ui->btnOk, &QPushButton::clicked, this, &AboutDialog::accept);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
