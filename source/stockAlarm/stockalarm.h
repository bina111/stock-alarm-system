#ifndef STOCKALARM_H
#define STOCKALARM_H

#include <QMainWindow>
#include<QNetworkReply>
#include<QNetworkAccessManager>
#include<QCloseEvent>

namespace Ui {
class stockAlarm;
}

class stockAlarm : public QMainWindow
{
    Q_OBJECT

public:
    explicit stockAlarm(QWidget *parent = 0);
    void closeEvent(QCloseEvent *);
    ~stockAlarm();

private slots:
    void dealData(QNetworkReply *reply);

private:
    Ui::stockAlarm *ui;
    QNetworkAccessManager *manger;
    QStringList stockList;      //存放用户已关注的股票代码
    QString user;               //记录登录的用户
};

#endif // STOCKALARM_H
