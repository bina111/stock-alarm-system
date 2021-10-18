#include"login.h"
#include"ui_login.h"
#include"stockalarm.h"
#include"ui_stockalarm.h"
#include<QFile>
#include<QTextStream>
#include<QMessageBox>
#include<QDebug>

login::login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);

    //注册按钮
    connect(ui->pB_zhuce,&QPushButton::clicked,this,[=](){
        QString user = ui->lE_user->text(); //获取用户名
        QString pas = ui->lE_pas->text();   //获取密码
        if (user=="" || pas == "")
        {
            QMessageBox::information(this, "提示", "用户名或密码为空！");
            ui->lE_user->clear();
            ui->lE_pas->clear();
            return;
        }
        QString str = "user.txt";
        QFile file1(str);
        file1.open(QIODevice::ReadOnly);
        QTextStream in(&file1);
        in.readLine();   //将首行去掉
        while(!in.atEnd())
        {
            QString line = in.readLine();
            QStringList list = line.split("/");
            if (list[0] == user)
            {
                QMessageBox::information(this,"提示","该用户已经注册！");
                file1.close();
                ui->lE_user->clear();
                ui->lE_pas->clear();
                return;
            }
        }
        file1.close();

        QFile file2(str);
        file2.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream out(&file2);
        out << user << "/" << pas << "\n";
        file2.close();
        ui->lE_user->clear();
        ui->lE_pas->clear();
        QMessageBox::information(this,"提示","注册成功！");
    });

    //登录按钮
    connect(ui->pB_login,&QPushButton::clicked,this,[=](){
        QString user = ui->lE_user->text(); //获取用户名
        QString pas = ui->lE_pas->text();   //获取密码
        if (user=="" || pas == "")
        {
            QMessageBox::information(this, "提示", "用户名或密码为空！");
            ui->lE_user->clear();
            ui->lE_pas->clear();
            return;
        }
        QString str = "user.txt";
        QFile file1(str);
        file1.open(QIODevice::ReadOnly);
        QTextStream in(&file1);
        in.readLine();   //将首行去掉
        while(!in.atEnd())
        {
            QString line = in.readLine();
            QStringList list = line.split("/");
            if (list[0] == user)
            {
                if (list[1] == pas)
                {
                    file1.close();
                    QFile file2("temp.txt");
                    file2.open(QIODevice::WriteOnly | QIODevice::Text);
                    QTextStream out(&file2);
                    out << user;
                    file2.close();
                    stockAlarm *w1 = new stockAlarm();
                    w1 -> show();
                    this -> close();
                    return;
                }
                else
                {
                    file1.close();
                    QMessageBox::information(this,"提示","密码错误！");
                    ui->lE_user->clear();
                    ui->lE_pas->clear();
                    return;
                }
            }
        }
        QMessageBox::information(this,"提示","该用户不存在，请先注册！");
        file1.close();
    });

}

login::~login()
{
    delete ui;
}
