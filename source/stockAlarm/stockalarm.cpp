#include "stockalarm.h"
#include "ui_stockalarm.h"
#include<QNetworkAccessManager>
#include<QNetworkReply>
#include<QNetworkRequest>
#include<QByteArray>
#include<QUrl>
#include<QDebug>
#include<QJsonParseError>
#include<QJsonDocument>
#include<QJsonObject>
#include<QJsonArray>
#include<QJsonValue>
#include<QMessageBox>
#include<QFile>
#include<QTextStream>
#include<QTextCodec>

stockAlarm::stockAlarm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::stockAlarm)
{
    ui->setupUi(this);

    //stackedWidget默认在首页
    ui->stackedWidget->setCurrentIndex(0);

    //左侧选择栏的选择
    connect(ui->tB_start,&QToolButton::clicked,this,[=](){
        ui->stackedWidget->setCurrentIndex(0);
    });
    connect(ui->tB_concerned,&QToolButton::clicked,this,[=](){
        ui->stackedWidget->setCurrentIndex(1);
    });

    //首页的tableWidget的表头
    QTableWidgetItem *headerItem;
    QStringList headerText;
    headerText<<"代码"<<"名称"<<"最新价"<<"涨跌幅"<<"涨跌额"<<"振幅"<<"最高"<<"最低"<<"市盈率"<<"市净率"<<"加关注";
    ui->tableWidget_1->setColumnCount((headerText.count()));  //设置列数
    for (int i = 0; i < ui->tableWidget_1->columnCount(); i++)   //列编号从0开始
    {
        headerItem=new QTableWidgetItem(headerText.at(i));  //新建一个QTableWidgetItem， headerText.at(i)获取headerText的i行字符串
        QFont font=headerItem->font();                      //获取原有字体设置
        font.setBold(true);                                 //设置为粗体
        font.setPointSize(12);                              //字体大小
        headerItem->setTextColor(Qt::red);                  //字体颜色
        headerItem->setFont(font);                          //设置字体
        ui->tableWidget_1->setHorizontalHeaderItem(i,headerItem); //设置表头单元格的Item
        ui->tableWidget_1->setColumnWidth(i, 100);                //设置表项宽度
    }
    ui->tableWidget_1->setSelectionBehavior(QAbstractItemView::SelectRows);   //整行选中
    ui->tableWidget_1->setSelectionMode (QAbstractItemView::SingleSelection); //设置选择模式，选择单行

    //已关注页面的tableWidget_2的表头
    ui->tableWidget_2->setColumnCount((headerText.count()));  //设置列数
    for (int i = 0; i < ui->tableWidget_2->columnCount(); i++)   //列编号从0开始
    {
        headerItem=new QTableWidgetItem(headerText.at(i));  //新建一个QTableWidgetItem， headerText.at(i)获取headerText的i行字符串
        QFont font=headerItem->font();                      //获取原有字体设置
        font.setBold(true);                                 //设置为粗体
        font.setPointSize(12);                              //字体大小
        headerItem->setTextColor(Qt::red);                  //字体颜色
        headerItem->setFont(font);                          //设置字体
        ui->tableWidget_2->setHorizontalHeaderItem(i,headerItem); //设置表头单元格的Item
        ui->tableWidget_2->setColumnWidth(i, 100);                //设置表项宽度
    }
    ui->tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);   //整行选中
    ui->tableWidget_2->setSelectionMode (QAbstractItemView::SingleSelection); //设置选择模式，选择单行

    //获取temp.txt的用户，看是哪个用户登录了，并将该用户所关注的股票显示已关注的列表中
    QFile file1("temp.txt");
    file1.open(QIODevice::ReadOnly);
    user = QString(file1.readAll());
    file1.close();

    //获取用户关注的股票代码，并将其显示在关注列表中
    QFile file2("stock.txt");
    file2.open(QIODevice::ReadOnly);
    QTextStream in(&file2);
    in.readLine();          //去掉首行
    while(!in.atEnd())
    {
        QString line = in.readLine();
        QStringList list = line.split("/");
        if (list[0] == user)
        {
            for(int i = 1; i < list.length(); i++)
            {
                stockList.append(list[i]);
            }
            break;
        }
    }
    file2.close();

    //从东方财富网获取深沪个股科创版的股票信息
    manger = new QNetworkAccessManager();
    QString url = "http://32.push2.eastmoney.com/api/qt/clist/get?cb=jQuery112409803814194851075_1623642572055&pn=1&pz=5000&po=1&np=1&ut=bd1d9ddb04089700cf9c27f6f7426281&fltt=2&invt=2&fid=f3&fs=m:0+t:6,m:0+t:80,m:1+t:2,m:1+t:23&fields=f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f12,f13,f14,f15,f16,f17,f18,f20,f21,f23,f24,f25,f22,f11,f62,f128,f136,f115,f152&_=1623642572056";
    manger->get(QNetworkRequest(QUrl(url)));
    connect(manger,&QNetworkAccessManager::finished,this,&stockAlarm::dealData);

    // 查询股票代码并显示该股票信息，没有相关股票代码则弹窗提醒
    connect(ui->pB_search,&QPushButton::clicked,this,[=](){
        QString textNum = ui->lineEdit->text();
        if(textNum == "")
            QMessageBox::information(this,"提示","股票代码为空！！！请先输入股票代码再查看");

        if (ui->stackedWidget->currentIndex() == 0)
        {
            //获取符合条件的索引
            QList <QTableWidgetItem *> item = ui->tableWidget_1->findItems(textNum, Qt::MatchContains);

            for (int j = 0; j < ui->tableWidget_1->rowCount(); j++) {
                ui->tableWidget_1->setRowHidden(j, true);        //隐藏所有行
            }
            if (!item.isEmpty()) {  //不为空
                for (int j = 0; j < item.count(); j++) {
                    ui->tableWidget_1->setRowHidden(item.at(j)->row(),false);     //输出查看的代码股票的相关内容
                }
            }
            else{
                QMessageBox::information(this,"提示","该股票代码不存在！！！");
            }
        }
        else
        {
            //获取符合条件的索引
            QList <QTableWidgetItem *> item = ui->tableWidget_2->findItems(textNum, Qt::MatchContains);

            for (int j = 0; j < ui->tableWidget_2->rowCount(); j++) {
                ui->tableWidget_2->setRowHidden(j, true);        //隐藏所有行
            }
            if (!item.isEmpty()) {  //不为空
                for (int j = 0; j < item.count(); j++) {
                    ui->tableWidget_2->setRowHidden(item.at(j)->row(),false);     //输出查看的代码股票的相关内容
                }
            }
            else{
                QMessageBox::information(this,"提示","该股票代码不在已关注列表中！！！");
            }
        }

    });

    // 恢复表格数据内容
    connect(ui->pB_huifu,&QPushButton::clicked,this,[=](){
        ui->lineEdit->clear();
        if (ui->stackedWidget->currentIndex() == 0)
        {
            for (int j = 0; j < ui->tableWidget_1->rowCount(); j++) {
                ui->tableWidget_1->setRowHidden(j, false);        //恢复所有行
            }
        }
        else
        {
            for (int j = 0; j < ui->tableWidget_2->rowCount(); j++) {
                ui->tableWidget_2->setRowHidden(j, false);        //恢复所有行
            }
        }
    });

    // 勾选关注即加入已关注
    connect(ui->tableWidget_1,&QTableWidget::cellClicked,this,[=](int row){
        if(ui->tableWidget_1 ->item(row, 10)->checkState() == Qt::Checked)
        {
            int curRow = ui->tableWidget_2->rowCount();//当前行号
            ui->tableWidget_2->insertRow(curRow);//在表格尾部添加一行
            for(int j = 0; j < 10; j++){
                ui->tableWidget_2->setItem(curRow, j, new QTableWidgetItem(ui->tableWidget_1->item(row, j)->text()));
            }
            QTableWidgetItem *item = new QTableWidgetItem("关注");
            item->setCheckState(Qt::Checked);
            ui->tableWidget_2->setItem(curRow, 10, item);
            stockList.append(ui->tableWidget_1->item(row, 0)->text());
        }
        else
        {
            QString text1 = ui->tableWidget_1->item(row, 0)->text();
            for(int j = 0; j < ui->tableWidget_2->rowCount(); j++){
                if(text1 == ui->tableWidget_2->item(j, 0)->text()){
                    ui->tableWidget_2->removeRow(j);
                    stockList.removeAll(text1);
                    break;
                }
            }
        }
    });

    // 当将关注列表的股票的已关注复选框取消时则从关注列表中删除
    connect(ui->tableWidget_2,&QTableWidget::cellClicked,this,[=](int row){
        if(ui->tableWidget_2 ->item(row, 10)->checkState() == Qt::Unchecked)
        {
            QString text1 = ui->tableWidget_2->item(row, 0)->text();
            for(int j = 0; j < ui->tableWidget_1->rowCount(); j++){
                if(text1 == ui->tableWidget_1->item(j, 0)->text()){
                    ui->tableWidget_1->item(j, 10)->setCheckState(Qt::Unchecked);
                    stockList.removeAll(text1);
                    break;
                }
            }
            ui->tableWidget_2->removeRow(row);
        }
    });

    //报警阈值设置
    connect(ui->lineEdit_2,&QLineEdit::returnPressed,this,[=](){
        ui->textBrowser->clear();
        int count = ui->tableWidget_2->rowCount();
        double yuZhi = ui->lineEdit_2->text().remove(-1, 1).toDouble();
        for(int i = 0; i < count; i++)
        {
            double price = ui->tableWidget_2->item(i, 3)->text().remove(-1, 1).toDouble();
            QString name = ui->tableWidget_2->item(i, 1)->text();
            QString temp = ui->tableWidget_2->item(i, 3)->text();
            if ( price > 0)
            {
                if (price > yuZhi)
                {
                    QMessageBox::information(this, "报警", "股票名字为"+name+"今日涨了"+temp);
                    ui->textBrowser->append("股票名字为"+name+"今日涨了"+temp+"\n");
                }
            }
            else
            {
                price = -price;
                if ( price > yuZhi)
                {
                    QMessageBox::information(this, "报警", "股票名字为"+name+"今日降了"+temp);
                    ui->textBrowser->append("股票名字为"+name+"今日降了"+temp+"\n");
                }
            }
        }
    });

    //退出应用
    connect(ui->tB_exit,&QToolButton::clicked,this,&stockAlarm::close);

}

//将获取的股票信息进行处理
void stockAlarm::dealData(QNetworkReply *reply)//获取网页数据并将数据显示在表格
{
    QByteArray data1 = reply->readAll();
    QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    //qDebug() << "contentType - " << contentType;
    int charsetIndex = contentType.indexOf("charset=");
    if(charsetIndex > 0)
    {
        charsetIndex += 8;
        QString charset = contentType.mid(charsetIndex).trimmed().toLower();
        if(charset.startsWith("gbk") || charset.startsWith("gb2312"))
        {
            QTextCodec *codec = QTextCodec::codecForName("GBK");
            if(codec)
            {
                data1 = codec->toUnicode(data1).toUtf8();
            }
        }
    }
    int parenthesisLeft = data1.indexOf('(');
    int parenthesisRight = data1.lastIndexOf(')');
    if(parenthesisLeft >=0 && parenthesisRight >=0)
    {
        parenthesisLeft++;
        data1 = data1.mid(parenthesisLeft, parenthesisRight - parenthesisLeft);
    }
    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(data1, &jsonError);        // 转化为 JSON 文档
    if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {  // 解析未发生错误
        if (doucment.isObject()) {    // JSON 文档为对象
            QJsonObject object = doucment.object();  // 转化为对象
            if (object.contains("data")) {   // 包含指定的 key
                QJsonValue value1 = object.value("data");   // 获取指定 key 对应的 value
                if (value1.isObject()) {     // 判断 value1 是否为对象
                    QJsonObject object1 = value1.toObject();  // 转化为对象
                    if (object1.contains("diff")) {
                        QJsonValue value2 = object1.value("diff");
                        if (value2.isArray()) {
                            QJsonArray array1 = value2.toArray();
                            int nSize = array1.size();
                            double yuZhi = ui->lineEdit_2->text().remove(-1, 1).toDouble();
                            for (int i = 0; i < nSize; ++i) {
                                QJsonValue value3 = array1.at(i);
                                if (value3.isObject()) {
                                    QJsonObject object2 = value3.toObject();            // 转化为对象
                                    QString f[10];

                                    f[0] = object2.value("f12").toString();      //股票代码
                                    f[1] = object2.value("f14").toString();      //股票名称
                                    f[2] = QString::number(object2.value("f2").toDouble(), '.', 2);        //最新价
                                    f[3] = QString::number(object2.value("f3").toDouble(), '.', 2) + "%";  //跌涨幅
                                    double price = object2.value("f3").toDouble();
                                    f[4] = QString::number(object2.value("f4").toDouble(), '.', 2);        //涨跌额
                                    f[5] = QString::number(object2.value("f7").toDouble(), '.', 2) + "%";  //振幅
                                    f[6] = QString::number(object2.value("f15").toDouble(), '.', 2);       //最高价
                                    f[7] = QString::number(object2.value("f16").toDouble(), '.', 2);       //最低价
                                    f[8] = QString::number(object2.value("f9").toDouble(), '.', 2);        //市盈率
                                    //double shiYing = object2.value("f9").toDouble();
                                    f[9] = QString::number(object2.value("f23").toDouble(), '.', 2);       //市净率
                                    // 将数据放入tableWidget_1
                                    ui->tableWidget_1->insertRow(i);//在表格尾部添加一行
                                    for(int j = 0; j < 10; j++){
                                        ui->tableWidget_1->setItem(i, j, new QTableWidgetItem(f[j]));
                                    }
                                    if (!stockList.isEmpty())
                                    {
                                        if(stockList.contains(f[0]))
                                        {
                                            // 将已关注的股票数据放入tableWidget_2
                                            int rowCou = ui->tableWidget_2->rowCount();
                                            ui->tableWidget_2->insertRow(rowCou);//在表格尾部添加一行
                                            for(int j = 0; j < 10; j++){
                                                ui->tableWidget_2->setItem(rowCou, j, new QTableWidgetItem(f[j]));
                                            }
                                            QTableWidgetItem *item1 = new QTableWidgetItem("关注");
                                            item1->setCheckState(Qt::Checked);
                                            ui->tableWidget_2->setItem(rowCou, 10, item1);
                                            QTableWidgetItem *item2 = new QTableWidgetItem("关注");
                                            item2->setCheckState(Qt::Checked);
                                            ui->tableWidget_1->setItem(i, 10, item2);
                                            if ( price > 0)
                                            {
                                                if (price > yuZhi)
                                                {
                                                    QMessageBox::information(this, "报警", "股票名字为"+f[1]+"今日涨了"+f[3]);
                                                }
                                            }
                                            else
                                            {
                                                price = -price;
                                                if ( price > yuZhi)
                                                {
                                                    QMessageBox::information(this, "报警", "股票名字为"+f[1]+"今日降了"+f[3]);
                                                }
                                            }
                                        }

                                        else
                                        {
                                            QTableWidgetItem *item = new QTableWidgetItem("关注");
                                            item->setCheckState(Qt::Unchecked);
                                            ui->tableWidget_1->setItem(i, 10, item);
                                        }
                                    }
                                    else
                                    {
                                        QTableWidgetItem *item = new QTableWidgetItem("关注");
                                        item->setCheckState(Qt::Unchecked);
                                        ui->tableWidget_1->setItem(i, 10, item);
                                    }

                               }

                            }
                        }
                    }
                }
            }
        }
    }
    reply->deleteLater();
}

void stockAlarm::closeEvent(QCloseEvent *)  //重载退出函数
{
    bool flag = false;
    QFile file3("stock.txt");
    file3.open(QIODevice::ReadOnly);
    QTextStream in(&file3);
    QStringList tempList;
    in.readLine();          //去掉首行
    while(!in.atEnd())
    {
        QString line = in.readLine();
        QStringList list = line.split("/");
        if (list[0] == user)
        {
            QString templine = user;
            for (int i = 0; i<stockList.length(); i++)
            {
                templine = templine + "/" + stockList[i];
            }
            tempList.append(templine);
            flag = true;
        }
        else
        {
            tempList.append(line);
        }
    }
    file3.close();
    if (flag == false)
    {
        QString templine = user;
        for (int i = 0; i<stockList.length(); i++)
        {
            templine = templine + "/" + stockList[i];
        }
        tempList.append(templine);
    }
    QString str = "用户名/关注的股票代码";
    QFile file4("stock.txt");
    file4.open(QIODevice::WriteOnly);
    QTextStream out(&file4);
    out << str << "\n";
    for(int i=0; i<tempList.length(); i++)
    {
        out << tempList[i] << "\n";
    }
    file4.close();
    qApp->quit();
}

stockAlarm::~stockAlarm()
{
    delete ui;
}
