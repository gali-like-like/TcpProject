#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QTextBlock>
#include <QRegularExpression>
namespace Ui {
class FindReplaceDialog;
}

class FindReplaceDialog : public QDialog
{
    Q_OBJECT
    friend class MainWindow;
public:
    FindReplaceDialog(QTextEdit* textEdit,QWidget *parent = nullptr);
    ~FindReplaceDialog();
private slots:
    void on_btnFindNext_clicked();
signals:
    void findStrChanged(const QString& str,const QTextDocument::FindFlags& flag);
private:
    void setInitCursor(QTextCursor* textCursor);
    const QTextCursor getCursor();
    Ui::FindReplaceDialog *ui;
    QTextEdit* m_textEdit;
    QTextCursor* m_textCursor;
    QList<bool> checks;
};

#endif // FINDREPLACEDIALOG_H
