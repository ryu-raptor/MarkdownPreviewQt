#include <QtCore>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QtWebKitWidgets>
#include <QDebug>
extern "C" {
#include <mkdio.h>
#include <stdio.h>
#include <errno.h>
#include <libgen.h>
}
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->hide();

    //コマンドライン引数を取得
    QStringList argList = QCoreApplication::arguments();
    if (argList.size() == 2) {
        //Cのライブラリを用いるのでパスを変換
        QString path = argList.at(1);
        QByteArray ba = path.toUtf8();
        FILE* fp = fopen(ba.data(), "r");
        if (fp == NULL) {
            perror(ba.data());
        }
        MMIOT* doc = mkd_in(fp, 0);
        mkd_compile(doc, 0);
        char* text;
        mkd_document(doc, &text);
        QString html(text);

        //デフォルトのCSSを取得
        QString defaultCSS;
        defaultCSS = QStandardPaths::locate(QStandardPaths::AppDataLocation, "styles.css");

        //HTMLを整形
        html = "<html><head>"
               "<link href=\"" + defaultCSS + "\" rel=\"stylesheet\"/>"
               "<meta charset=\"utf-8\">"
               "</head><body>" + html + "</body></html>";

        QFileInfo fileinfo(path);
        QDir basedir = fileinfo.absoluteDir();
        QString basepath = basedir.absolutePath();

        QString previewPath = basepath + "/.preview.html";
        QFile previewFile(previewPath);
        if (!previewFile.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, "Error",
                                     previewFile.errorString());
            return;
        }
        QTextStream previewStream(&previewFile);
        previewStream << html;
        previewFile.close();

        ui->webView->load(QUrl("file://" + previewPath));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
