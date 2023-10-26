#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkAddressEntry>
#include <QHostAddress>
#include <QLabel>
#include <QImage>
#include <QHostInfo>
#include <QTextCharFormat>
#include <QDateTime>
#include <QTimer>
#include <QStack>
#include <QFontDialog>
#include <QInputDialog>
#include <QColorDialog>
#include <QTextDocumentFragment>
#include <QTextBlockFormat>
#include <QActionGroup>
#include <QFileDialog>
#include <QImage>
#include <QTextImageFormat>
#include <QTextStream>
#include <QFile>
#include <QMenuBar>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMimeDatabase>
#include <QTextTableFormat>
#include <QFileInfo>
#include <QBuffer>
#include <QInputDialog>
#include "findreplacedialog.h"
#include <QTextDocument>
#include <QTextBlock>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void do_triggered_align(QAction* action);
    void do_newConnection();
    void do_socketStateChanged(QAbstractSocket::SocketState socketStatus);
    void do_clientConnected();
    void do_clientDisconnected();
    void do_socketReadyRead();
    void on_actionListening_triggered();

    void on_actionStop_triggered();

    void on_actionClear_triggered();

    void on_btnSend_clicked();

    void on_actionQuit_triggered();

    void on_actionBold_triggered();

    void do_errorOccurred(QTcpSocket::SocketError error);

    void do_serviceError(QAbstractSocket::SocketError error);
    void on_actionItalic_triggered();

    void on_actionFont_triggered();

    void on_actionFontSize_triggered();

    void on_actionColor_triggered();

    void on_actionImage_triggered();

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void on_actionZoomOut_triggered();
    void do_finded_by_str(const QString& str,const QTextDocument::FindFlags& flag);
    void on_actionZoomIn_triggered();
    void do_reply_finished();
    void do_reply_readyread();
    void do_reply_errorOccurred(QNetworkReply::NetworkError error);
    void do_reply_downloadProgress(qint64 nowSize,qint64 total);
    void on_actionShow_triggered();
    void on_actionFind_triggered();
    void do_find_cursorChanged(QTextCursor& cursor,const int& row,const int& col,const int& pos);
protected:
    void dragEnterEvent(QDragEnterEvent* e);
    void dropEvent(QDropEvent* e);
private:
    void showFile(const QUrl& url);
    void getLocalIp();
    Ui::MainWindow *ui;
    QLabel* labelStatus;
    QLabel* labelListen;
    QLabel* currentStyle;
    QTcpServer* service;
    QActionGroup* actionH;
    QActionGroup* actionV;
    QMenu* menuAlign;
    QNetworkAccessManager* manger;
    QNetworkReply* reply;
    QByteArray bytes;
    QBuffer* buffer;
    QTcpSocket* connectSocket;
    QStack<QTcpSocket*> enterSockets;
    QStack<QTcpSocket*> leavelSockets;
    QLabel* labelRow;
    QLabel* labelCol;
    QLabel* labelCursor;
    FindReplaceDialog* dialog;
};
#endif // MAINWINDOW_H
