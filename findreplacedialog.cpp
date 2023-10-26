#include "findreplacedialog.h"
#include "ui_findreplacedialog.h"

FindReplaceDialog::FindReplaceDialog(QTextEdit* textEdit,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindReplaceDialog)
{
    ui->setupUi(this);
    this->setParent(parent);
    this->m_textEdit = textEdit;
}

FindReplaceDialog::~FindReplaceDialog()
{
    delete ui;
}

void FindReplaceDialog::setInitCursor(QTextCursor* textCursor)
{
    this->m_textCursor = textCursor;
}

void FindReplaceDialog::on_btnFindNext_clicked()
{
    QString findString = ui->lineEditFind->text();
    qDebug()<<"查找的字符串："<<findString;
    QTextDocument::FindFlags flag;
    flag.setFlag(QTextDocument::FindWholeWords,ui->checkAllMatch->isChecked());
    flag.setFlag(QTextDocument::FindBackward,ui->checkReverse->isChecked());
    flag.setFlag(QTextDocument::FindCaseSensitively,ui->checkCaseSesitively->isChecked());
    bool ok = ui->checkRegrep->isChecked();
    emit this->findStrChanged(findString,flag);
    this->accept();
}

