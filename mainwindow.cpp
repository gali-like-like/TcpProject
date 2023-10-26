#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->menuBar->addAction(ui->actionListening);
    ui->menuBar->addAction(ui->actionStop);
    ui->menuBar->addAction(ui->actionClear);
    ui->menuBar->addAction(ui->actionQuit);
    ui->menuBar->addAction(ui->actionBold);
    ui->actionBold->setCheckable(true);
    ui->actionItalic->setCheckable(true);
    ui->menuBar->addAction(ui->actionItalic);
    ui->menuBar->addAction(ui->actionFontSize);
    ui->menuBar->addAction(ui->actionFont);
    ui->menuBar->addAction(ui->actionColor);
    ui->menuBar->addAction(ui->actionImage);
    ui->menuBar->addAction(ui->actionOpen);
    ui->menuBar->addAction(ui->actionSave);
    ui->menuBar->addAction(ui->actionZoomIn);
    ui->menuBar->addAction(ui->actionZoomOut);
    ui->menuBar->addAction(ui->actionShow);
    ui->menuBar->addAction(ui->actionFind);
    menuAlign = new QMenu("文本对齐",this);
    connect(menuAlign,&QMenu::triggered,this,&MainWindow::do_triggered_align);
    actionH = new QActionGroup(this);

    actionH->setExclusive(true);
    actionH->addAction(ui->actionLeft);
    actionH->addAction(ui->actionHorCenter);
    actionH->addAction(ui->actionRight);
    actionH->addAction(ui->actionJustify);

    menuAlign->addAction(ui->actionLeft);
    menuAlign->addAction(ui->actionHorCenter);
    menuAlign->addAction(ui->actionRight);
    menuAlign->addAction(ui->actionJustify);

    ui->menuBar->addMenu(menuAlign);
    labelListen = new QLabel(this);
    labelStatus = new QLabel("socket状态",this);
    labelRow = new QLabel("row:",this);
    labelCol = new QLabel("col",this);
    labelCursor = new QLabel("pos",this);
    ui->statusbar->addWidget(labelListen,1);
    ui->statusbar->addWidget(labelStatus,1);
    ui->statusbar->addWidget(labelRow,1);
    ui->statusbar->addWidget(labelCol,1);
    ui->statusbar->addWidget(labelCursor,1);
    ui->comboBoxAddress->addItem("127.0.0.1");
    ui->comboBoxAddress->setCurrentIndex(0);
    this->getLocalIp();
    dialog = new FindReplaceDialog(ui->textEditSendMsg,this);
    connect(dialog,&FindReplaceDialog::findStrChanged,this,&MainWindow::do_finded_by_str);
    this->setAcceptDrops(true);
    currentStyle = new QLabel("字体信息",this);
    currentStyle->hide();
    service = new QTcpServer(this);
    service->setMaxPendingConnections(10);
    ui->textEditSendMsg->setAcceptDrops(false);
    ui->textEdit->setAcceptDrops(false);
    ui->textEditSendMsg->setAutoFormatting(QTextEdit::AutoBulletList);
    ui->textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    ui->textEdit->setTabChangesFocus(true);
    ui->textEdit->setTabStopDistance(40);
    connect(service,&QTcpServer::newConnection,this,&MainWindow::do_newConnection);
    connect(service,&QTcpServer::acceptError,this,&MainWindow::do_serviceError);
    buffer = new QBuffer(this);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* e)
{
    if(e->mimeData()->hasUrls())
    {
        qDebug()<<e->mimeData()->urls().at(0).url();
        e->acceptProposedAction();
    }
    else
        e->ignore();
}

void MainWindow::dropEvent(QDropEvent* e)
{
    qDebug()<<"进入放置函数"<<e->mimeData()->text();
    if(e->mimeData()->hasImage())
    {
        qDebug()<<"图形通道";
        QTextCursor textCursor = ui->textEdit->textCursor();
        bool ok = textCursor.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,1);
        qDebug()<<"成功："<<ok;
        QImage image = e->mimeData()->imageData().value<QImage>();
        textCursor.insertImage(image);
    }
    else if(e->mimeData()->hasUrls())
    {
        qDebug()<<"urls通道";
        QList<QUrl> urls = e->mimeData()->urls();
        foreach (QUrl url, urls) {
            if(url.isValid())
            {
                this->showFile(url);
            }
            else
                qDebug()<<"url无效";
        }
    }
    e->accept();
}

void MainWindow::showFile(const QUrl& url)
{
    bool ok = url.isLocalFile();
    QString path = url.toLocalFile();
    if(ok)
    {
        QFile file(path);
        QFileInfo fileInfo(path);
        if(file.open(QIODeviceBase::ReadOnly))
        {
            QString suffixName = fileInfo.completeSuffix();
            QMimeType mimeType = QMimeDatabase().mimeTypeForFile(path);
            QByteArray content = file.readAll();
            QString mimeName = mimeType.name();
            qDebug()<<" name:"<<mimeType.name()<<" parentName:"<<mimeType.parentMimeTypes()<<" 后缀名:"<<suffixName;
            if(suffixName == "png" || suffixName == "jpg" || suffixName == "ico" || suffixName == "jpeg")
            {
                QImage img;
                bool res = img.loadFromData(content);
                if(!res) {qDebug()<<"图形加载失败";return;}
                ui->textEdit->moveCursor(QTextCursor::Down,QTextCursor::MoveAnchor);
                QTextCursor textCursor = ui->textEdit->textCursor();
                textCursor.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,1);
                textCursor.insertImage(img);
                ui->textEdit->setTextCursor(textCursor);
            }
            else if(suffixName.contains("md"))
                   ui->textEditSendMsg->setMarkdown(content);
            else if(suffixName.contains("html"))
                ui->textEdit->insertHtml(content);
            else
                ui->textEdit->append(content);
        }
        else
            qDebug()<<"文件打开失败";
    }
    else
    {
        QString filePath = url.fileName();
        QFileInfo networkInfo(filePath);
        QString suffix = networkInfo.completeSuffix();
        qDebug()<<QString("协议:%1,主机：%2,端口:%3,用户名:%4,密码:%5,查询:%6,文件名:%7")
                        .arg(url.scheme()).arg(url.host()).arg(url.port())
                        .arg(url.userName()).arg(url.password()).arg(url.query())
                        .arg(url.fileName());
        qDebug()<<QString("路径:%1,数据段:%2,后缀名:%3").arg(url.path()).arg(url.fragment()).arg(suffix);
        if(filePath.isEmpty())
        {
            qDebug()<<"url没有文件";
            return;
        }
        if(suffix.isEmpty())
        {
            qDebug()<<"这不是文件";
            return;
        }
        QNetworkRequest quest;
        quest.setUrl(url);
        manger = new QNetworkAccessManager(this);
        reply = manger->get(quest);
        connect(reply,&QNetworkReply::readyRead,this,&MainWindow::do_reply_readyread);
        connect(reply,&QNetworkReply::finished,this,&MainWindow::do_reply_finished);
        connect(reply,&QNetworkReply::errorOccurred,this,&MainWindow::do_reply_errorOccurred);
        connect(reply,SIGNAL(downloadProgress(qint64,qint64)),SLOT(do_reply_downloadProgress(qint64,qint64)));
    }
}

void MainWindow::do_reply_errorOccurred(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    qDebug()<<reply->errorString();
}

void MainWindow::do_reply_downloadProgress(qint64 nowSize,qint64 total)
{
    qDebug()<<"当前接收到的字节数："<<QString::number(nowSize);
}

void MainWindow::do_reply_finished()
{
    qDebug()<<"响应url:"<<reply->url()<<" size:"<<reply->readAll().size();
    QByteArray bytes = reply->readAll();
    QMimeType mimeType = QMimeDatabase().mimeTypeForData(bytes);
    QString name = mimeType.name();
    qDebug()<<"name:"<<name;
    if(name.contains("image"))
    {
        QImage img;
        bool ok = img.loadFromData(bytes);
        ok?qDebug()<<"成功":qDebug()<<"失败";
        QTextCursor textCursor = ui->textEditSendMsg->textCursor();
        textCursor.insertImage(img);
        ui->textEditSendMsg->setTextCursor(textCursor);
    }
    else
    {
        ui->textEdit->append(QString::fromUtf8(bytes));
    }
}

void MainWindow::do_reply_readyread()
{
}

void MainWindow::do_triggered_align(QAction* action)
{
    QAction* actionHor = actionH->checkedAction();
    int indexH = actionH->actions().indexOf(actionHor);
    QTextCursor textCursor = ui->textEditSendMsg->textCursor();
    QTextBlockFormat block = textCursor.blockFormat();
    Qt::Alignment alignHor = block.alignment();
    qDebug()<<"被选中的index:"<<QString::number(indexH)<<" text:"<<action->text()<<" 当前对齐:"<<alignHor;
    switch (indexH) {
    case 0:
        alignHor.setFlag(Qt::AlignLeft,true);
        alignHor.setFlag(Qt::AlignRight,false);
        alignHor.setFlag(Qt::AlignCenter,false);
        alignHor.setFlag(Qt::AlignJustify,false);
        break;
    case 1:
        alignHor.setFlag(Qt::AlignLeft,false);
        alignHor.setFlag(Qt::AlignRight,false);
        alignHor.setFlag(Qt::AlignCenter,true);
        alignHor.setFlag(Qt::AlignJustify,false);
        break;
    case 2:
        alignHor.setFlag(Qt::AlignLeft,false);
        alignHor.setFlag(Qt::AlignRight,true);
        alignHor.setFlag(Qt::AlignCenter,false);
        alignHor.setFlag(Qt::AlignJustify,false);
        break;
    case 3:
        alignHor.setFlag(Qt::AlignLeft,false);
        alignHor.setFlag(Qt::AlignRight,false);
        alignHor.setFlag(Qt::AlignCenter,false);
        alignHor.setFlag(Qt::AlignJustify,true);
        break;
    }
    qDebug()<<"设置后的align:"<<alignHor<<"alignHor value:"<<alignHor.toInt();
    block.setAlignment(alignHor);
    textCursor.setBlockFormat(block);
    ui->textEditSendMsg->setTextCursor(textCursor);
}

void MainWindow::do_serviceError(QAbstractSocket::SocketError error)
{
//    Q_UNUSED(error);
//    this->do_errorOccurred(error);
}

void MainWindow::do_clientConnected()
{
    QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString ip = connectSocket->peerAddress().toString();
    QString port = QString::number(connectSocket->peerPort());
    QString msg = QString("%1 %2:%3 客户端连接").arg(now).arg(ip).arg(port);
    this->setWindowTitle(ip+":"+port);
    ui->textEdit->setFontPointSize(20);
    ui->textEdit->setTextColor(QColor("green"));
    ui->textEdit->append(msg);
    QTextCursor cursor = ui->textEditSendMsg->textCursor();

    QTextBlockFormat blockFormat;
    blockFormat.setLeftMargin(2);
    blockFormat.setRightMargin(2);
    blockFormat.setBottomMargin(2);
    blockFormat.setTopMargin(2);
    blockFormat.setLineHeight(120,QTextBlockFormat::ProportionalHeight);
    blockFormat.setTextIndent(2);
    blockFormat.setNonBreakableLines(true);
    blockFormat.setAlignment(Qt::AlignLeft);
    blockFormat.setHeadingLevel(3);
    cursor.setBlockFormat(blockFormat);
    cursor.setBlockCharFormat(ui->textEdit->currentCharFormat());
    ui->textEditSendMsg->setTextCursor(cursor);
    ui->textEditSendMsg->setText("123456\n456789");
    QString html = ui->textEditSendMsg->toHtml();
    qDebug()<<"服务器发送的html:"<<html;
    connectSocket->write(html.toUtf8());
}

void MainWindow::do_clientDisconnected()
{
    QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString ip = connectSocket->peerAddress().toString();
    QString port = QString::number(connectSocket->peerPort());
    QString msg = QString("%1 %2:%3 客户端断开连接").arg(now).arg(ip).arg(port);
    ui->textEdit->setTextColor(QColor("green"));
    ui->textEdit->setFontPointSize(20);
    ui->textEdit->append(msg);
    enterSockets.pop();
    leavelSockets.push(connectSocket);
    connectSocket = enterSockets.top();
    qDebug()<<enterSockets;
}

void MainWindow::do_socketStateChanged(QTcpSocket::SocketState state)
{
    switch (state) {
    case QTcpSocket::UnconnectedState:
        labelStatus->setText("socket状态:未连接");
        break;
    case QTcpSocket::HostLookupState:
        labelStatus->setText("socket状态:查找中");
        break;
    case QTcpSocket::ConnectingState:
        labelStatus->setText("socket状态:连接中");
        break;
    case QTcpSocket::ConnectedState:
        labelStatus->setText("socket状态:已连接");
        break;
    case QTcpSocket::BoundState:
        labelStatus->setText("socket状态:已绑定");
        break;
    case QTcpSocket::ClosingState:
        labelStatus->setText("socket状态:关闭中");
        break;
    case QTcpSocket::ListeningState:
        labelStatus->setText("socket状态:监听中");
        break;
    }
}

void MainWindow::do_socketReadyRead()
{
    foreach(QTcpSocket* socket,enterSockets)
    {
        QByteArray data = socket->readAll();
        ui->textEdit->append(data);
    }
}

void MainWindow::do_newConnection()
{
    connectSocket = service->nextPendingConnection();
    enterSockets.push(connectSocket);
    qDebug()<<enterSockets.length();
    connect(connectSocket,&QTcpSocket::connected,this,&MainWindow::do_clientConnected);
    do_clientConnected();
    connect(connectSocket,&QTcpSocket::disconnected,this,&MainWindow::do_clientDisconnected);

    connect(connectSocket,&QTcpSocket::stateChanged,this,&MainWindow::do_socketStateChanged);
    do_socketStateChanged(connectSocket->state());
    connect(connectSocket,&QTcpSocket::readyRead,this,&MainWindow::do_socketReadyRead);
    connect(connectSocket,&QTcpSocket::errorOccurred,this,&MainWindow::do_errorOccurred);
    qDebug()<<enterSockets;
}

void MainWindow::do_errorOccurred(QTcpSocket::SocketError error)
{
//    Q_UNUSED(error);
//    qDebug()<<connectSocket->errorString();
//    ui->textEdit->setTextColor(QColor("red"));
//    ui->textEdit->setFontPointSize(20);
//    QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
//    QString msg = QString("%1 %2").arg(now).arg(connectSocket->errorString());
//    ui->textEdit->append(msg);
}

void MainWindow::getLocalIp()
{
    QString hostName = QHostInfo::localHostName();
    QHostInfo hostInfo = QHostInfo::fromName(hostName);
    QList<QHostAddress> addresses = hostInfo.addresses();
    foreach (QHostAddress address,addresses)
    {
        if(address.protocol() == QHostAddress::IPv4Protocol)
        {
            QString ipv4 = address.toString();
            ui->comboBoxAddress->addItem(ipv4);
        }
    }
}

MainWindow::~MainWindow()
{
    foreach(QTcpSocket* socket,enterSockets)
    {
        socket->disconnectFromHost();
    }
    if(service->isListening())
        service->close();
    delete ui;
}


void MainWindow::on_actionListening_triggered()
{
    QString ip = ui->comboBoxAddress->currentText();
    quint16 port = ui->spinBoxPort->value();
    QHostAddress address(ip);
    service->listen(address,port);
    QTextCharFormat charFormat;
    charFormat.setFont(QFont("黑体",25,25,true));
    charFormat.setFontWordSpacing(3);
    ui->textEdit->setCurrentCharFormat(charFormat);
    ui->textEdit->append("开启监听");
    ui->textEdit->append("服务器地址:"+service->serverAddress().toString()+
                                         " 服务器端口:"+QString::number(service->serverPort()));
    ui->actionListening->setEnabled(false);
    ui->actionStop->setEnabled(true);

    labelListen->setText("监听状态:正在监听");
}


void MainWindow::on_actionStop_triggered()
{
    if(service->isListening())
    {
        foreach(QTcpSocket* nowSocket,enterSockets)
        {
            nowSocket->disconnectFromHost();
        }
        service->close();
        ui->actionListening->setEnabled(true);
        ui->actionStop->setEnabled(false);
        labelListen->setText("监听状态:已关闭");
    }
}

void MainWindow::on_actionClear_triggered()
{
    ui->textEdit->clear();
    ui->textEditSendMsg->clear();
}

void MainWindow::on_btnSend_clicked()
{
    QString contentHtml = ui->textEditSendMsg->toHtml();
    qDebug()<<contentHtml;
    ui->textEditSendMsg->clear();
    ui->textEdit->append(contentHtml);
    QByteArray data = contentHtml.toUtf8();
    data.append("\n");
    qDebug()<<"原始数据："<<data;
    foreach(QTcpSocket* socket,enterSockets)
    {
        socket->write(data);
    }
}

void MainWindow::on_actionQuit_triggered()
{
    this->close();
}

void MainWindow::on_actionBold_triggered()
{
    bool ok = ui->actionBold->isChecked();
    QTextCursor textCursor = ui->textEditSendMsg->textCursor();
    QTextCharFormat format = textCursor.charFormat();
    QFont font = ui->textEditSendMsg->currentFont();
    font.setBold(ok);
    if(ok)
    {
        QString fontFamily = ui->textEditSendMsg->fontFamily();
        QString fontSize = QString::number(ui->textEditSendMsg->fontPointSize());
        QString fontBold = ok?"加粗":"非加粗";
        QString fontliatic = ui->textEditSendMsg->fontItalic()?"倾斜":"非倾斜";
        QString color = ui->textEditSendMsg->textColor().name(QColor::HexArgb);
    }
    format.setFont(font);
    textCursor.setCharFormat(format);
    ui->textEditSendMsg->setTextCursor(textCursor);
    qDebug()<<ui->textEditSendMsg->currentFont().bold();
}

void MainWindow::on_actionItalic_triggered()
{
    ui->textEditSendMsg->setFontItalic(ui->actionItalic->isChecked());
}

void MainWindow::on_actionFont_triggered()
{
    bool ok = false;
    QFont font = QFontDialog().getFont(&ok,this);
    if(ok)
        ui->textEditSendMsg->setFont(font);
    else
        qDebug()<<"字体样式无效";
}

void MainWindow::on_actionFontSize_triggered()
{
    bool ok =false;

    int pointSize = QInputDialog::getInt(this,"字体大小","请选择字体大小",ui->textEditSendMsg->fontPointSize(),1,100,1,&ok);
    if(ok)
        ui->textEditSendMsg->setFontPointSize(pointSize);
    else
        qDebug()<<"设置字体大小无效";
}

void MainWindow::on_actionColor_triggered()
{
    QColor color = QColorDialog::getColor(Qt::white,this,"颜色对话框",QColorDialog::DontUseNativeDialog);
    if(color.isValid())
        ui->textEditSendMsg->setTextColor(color);
    else
        qDebug()<<"颜色无效";
}

void MainWindow::on_actionImage_triggered()
{
    QString file = QFileDialog::getOpenFileName(this,"选择图片","C:\\Users\\14729\\Desktop\\test","image file (*.png *jpg *ico)");
    if(!file.isEmpty())
    {
//        QImage image(file);
//        QImage newImage = image.convertedTo(QImage::Format_ARGB32);
//        QTextCursor textCursor = ui->textEditSendMsg->textCursor();
//        textCursor.insertImage(newImage,file);
          //2、
        QTextImageFormat imageFormat;
        imageFormat.setQuality(100);
        imageFormat.setName(file);
        QTextCursor textCursor = ui->textEditSendMsg->textCursor();
        textCursor.insertImage(imageFormat);
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this,"打开","E:\\SaveTest","rich text File (*.html *.md);;format text file(*.json *.csv *.xml);;plain text(*.txt)");
    if(filePath.isEmpty())
    {
        qDebug()<<"文件为空";
        return;
    }
    QFile file(filePath);
    if(file.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text))
    {
        QString content = file.readAll();
        qDebug()<<"文件内容"<<content;
        file.close();
        QColor nowColor = ui->textEditSendMsg->textColor();
        int fontSize = ui->textEditSendMsg->fontPointSize();
        ui->textEditSendMsg->setFontPointSize(fontSize+10);
        ui->textEditSendMsg->setTextColor(QColor("red"));
        ui->textEditSendMsg->append(filePath);
        ui->textEditSendMsg->setTextColor(nowColor);
        ui->textEditSendMsg->setFontPointSize(fontSize);
        if(filePath.contains("html",Qt::CaseInsensitive))
        {
            ui->textEditSendMsg->setHtml(content);
        }
        else if(filePath.contains("md",Qt::CaseInsensitive))
        {
            ui->textEditSendMsg->setMarkdown(content);
        }

//            QStringList lines = content.split("\n");
//            qDebug()<<"切割换行后的内容："<<lines;
//            int rows = lines.count();
//            int nowCols = lines[0].split(",").count();
//            QTextCursor textCursor = ui->textEdit->textCursor();
//            QTextTableFormat tableFormat;
//            tableFormat.setAlignment(Qt::AlignCenter);
//            tableFormat.setCellPadding(20);
//            tableFormat.setCellSpacing(0);
//            tableFormat.setBorderCollapse(false);
//            textCursor.insertTable(rows,nowCols,tableFormat);
//            for(int i = 0;i<rows;i++)
//            {
//                QStringList fields = lines.at(i).split(",");
//                qDebug()<<"切割后："<<fields;
//                for(int j = 0;j<nowCols;j++)
//                {
//                    textCursor.insertText(fields[j]);
//                    textCursor.movePosition(QTextCursor::NextCell,QTextCursor::MoveAnchor,1);
//                }
//                textCursor.movePosition(QTextCursor::NextRow,QTextCursor::MoveAnchor,1);
//            }
        else
            ui->textEditSendMsg->setPlainText(content);
    }
}

void MainWindow::on_actionSave_triggered()
{
    QString filePath = QFileDialog::getSaveFileName(this,"保存","E:\\SaveTest","rich text File (*.html *.md);;format text file(*.json *.csv *.xml);;plain text(*.txt)");
    if(filePath.isEmpty()) {return;}
    QFile saveFile(filePath);
    QFileInfo saveInfo(filePath);
    QString content;
    if(saveFile.open(QIODeviceBase::WriteOnly | QIODeviceBase::Text))
    {
        QString suffixName = saveInfo.completeSuffix();
        if(suffixName.contains("md"))
            content = ui->textEditSendMsg->toMarkdown(QTextDocument::MarkdownDialectCommonMark);
        else if(suffixName.contains("html"))
            content = ui->textEditSendMsg->toHtml();
        else if(suffixName.contains("json"))
        {
            content = ui->textEditSendMsg->toPlainText();
            QJsonParseError* error = new QJsonParseError;
            QJsonDocument jsonDocu = QJsonDocument::fromJson(content.toUtf8());
            if(error->error != QJsonParseError::NoError) {qDebug()<<"发生错误";return;}
            QJsonDocument newJsonDoc(jsonDocu);
            qDebug()<<"实际导出的json："<<newJsonDoc.toJson(QJsonDocument::Indented);
            content = newJsonDoc.toJson(QJsonDocument::Indented);
            delete error;
        }
        else
        {
            content = ui->textEditSendMsg->toPlainText();
        }
        qDebug()<<"保存的内容为："<<content;
        saveFile.write(content.toUtf8());
        saveFile.close();
    }
}

void MainWindow::on_actionZoomOut_triggered()
{
    ui->textEditSendMsg->zoomIn(2);
}

void MainWindow::on_actionZoomIn_triggered()
{
    ui->textEditSendMsg->zoomOut(2);
}

void MainWindow::on_actionShow_triggered()
{
    QTextCursor textCursor = ui->textEditSendMsg->textCursor();
    QTextBlockFormat blockFormat = textCursor.blockFormat();
    Qt::Alignment align = blockFormat.alignment();
    int lineHeight = blockFormat.lineHeight();
    int indent =blockFormat.indent();
    int topMargin = blockFormat.topMargin();
    int bottomMargin = blockFormat.bottomMargin();
    int leftMargin = blockFormat.leftMargin();
    int rightMargin = blockFormat.rightMargin();
    int textIndent = blockFormat.textIndent();
    QFont font = ui->textEditSendMsg->currentFont();
    QColor color = ui->textEditSendMsg->textColor();
    QColor background = ui->textEditSendMsg->textBackgroundColor();
    QTextEdit::LineWrapMode lineWrapMode = ui->textEditSendMsg->lineWrapMode();
    bool acceptRich = ui->textEditSendMsg->acceptRichText();
    bool isReadOnly = ui->textEditSendMsg->isReadOnly();
    bool canUndoRedo = ui->textEditSendMsg->isUndoRedoEnabled();
    QTextCharFormat charFormat = ui->textEditSendMsg->currentCharFormat();
    int wordInterval = charFormat.fontWordSpacing();
    int letterInterval = charFormat.fontLetterSpacing();
    bool isItalic = charFormat.fontItalic();
    bool isOverLine = charFormat.fontOverline();
    bool isUnderLine = charFormat.fontUnderline();
    bool isStrikeLine = charFormat.fontStrikeOut();
    int fontWeight = charFormat.fontWeight();
    int fontPointSize = charFormat.fontPointSize();
    QPen penTextOutLine = charFormat.textOutline();
    QColor underLineColor = charFormat.underlineColor();
    QTextCharFormat::UnderlineStyle underLineStyle = charFormat.underlineStyle();
}

void MainWindow::do_find_cursorChanged(QTextCursor& cursor,const int& row,const int& col,const int& pos)
{
    labelRow->setText(QString("row:%1").arg(row));
    labelCol->setText(QString("col:%1").arg(col));
    labelCursor->setText(QString("pos:%1").arg(pos));
    ui->textEditSendMsg->setTextCursor(cursor);
}

void MainWindow::do_finded_by_str(const QString& str,const QTextDocument::FindFlags& flag)
{
    QTextDocument* document = ui->textEditSendMsg->document();
    QTextCursor textCursor = ui->textEditSendMsg->textCursor();
    QTextCursor result = document->find(str,textCursor.position(),flag);
    bool res = result.block().isValid();
    qDebug()<<res;
    res?ui->textEditSendMsg->setTextCursor(result):ui->textEditSendMsg->setTextCursor(textCursor);
}

void MainWindow::on_actionFind_triggered()
{
//    bool ok = false;
//    QString text = QInputDialog::getText(this,"请输入要查找的字符","",QLineEdit::Normal,"",&ok);
//    qDebug()<<"text:"<<text<<" ok:"<<ok;
//    if(text.isEmpty() && ok) {qDebug()<<"失败";return;}
    dialog->show();
//    QTextDocument* document = ui->textEditSendMsg->document();
//    QTextCursor textCursor = ui->textEditSendMsg->textCursor();
//    QTextCursor result = document->find(text,textCursor.position(),QTextDocument::FindBackward);
//    bool res = result.block().isValid();
//    qDebug()<<res;
//    ok?ui->textEditSendMsg->setTextCursor(result):ui->textEditSendMsg->setTextCursor(textCursor);
}

