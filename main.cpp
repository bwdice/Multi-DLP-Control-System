#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <iostream>
#include <fstream>



//“Fusion”：融合风格，适用于跨平台的应用程序。
//“Windows”：Windows 风格，与当前操作系统的风格一致。
//“WindowsXP”：Windows XP 风格，类似于 Windows XP 的外观。
//“WindowsVista”：Windows Vista 风格，类似于 Windows Vista 的外观。
//“Windows7”：Windows 7 风格，类似于 Windows 7 的外观。
//“Windows8”：Windows 8 风格，类似于 Windows 8 的外观。
//“Windows10”：Windows 10 风格，类似于 Windows 10 的外观。
//“Macintosh”：Macintosh 风格，类似于 Mac 的外观。
//“Motif”：Motif 风格，类似于 Motif 工具包的外观。
//“CDE”：CDE 风格，类似于 Common Desktop Environment 的外观。
//“Plastique”：Plastique 风格，具有三维效果。
//“Cleanlooks”：Cleanlooks 风格，简单、清晰。
//“GTK+”：GTK+ 风格，类似于 GTK+ 应用程序的外观

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    //QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication a(argc, argv);
    a.setStyle(QStyleFactory::create("Fusion"));

    // 设置stdout流为新的文件流
    std::streambuf* originalStdoutBuffer = std::cout.rdbuf();
    std::ofstream newOutput("run_log.txt");
    std::cout.rdbuf(newOutput.rdbuf());

    std::cout << "mian start" << std::endl;

    MainWindow w;
    w.show();

    int ret = a.exec();

    // 还原stdout流
    std::cout.rdbuf(originalStdoutBuffer);
    // 关闭文件
    newOutput.close();

    return ret;
}
