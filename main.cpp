#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <iostream>
#include <fstream>
#include <qwidget.h>
#include <QScreen>
#include <QTextCodec>

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
   // QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
   // QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    //QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#if 1
   // QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
   // QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    //这个是Windows平台用来获取屏幕宽度的代码，
    //因为在qApplication实例初始化之前，QGuiApplication::screens();无法使用。

    QApplication a(argc, argv);

    //设置中文字体
        a.setFont(QFont("Microsoft Yahei", 9));
#if 0
        //设置中文编码
    #if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
    #if _MSC_VER
        QTextCodec *codec = QTextCodec::codecForName("GBK");
    #else
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    #endif
        QTextCodec::setCodecForLocale(codec);
        QTextCodec::setCodecForCStrings(codec);
        QTextCodec::setCodecForTr(codec);
    #else
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        QTextCodec::setCodecForLocale(codec);
    #endif
#endif
    a.setStyle(QStyleFactory::create("Fusion"));


    // 获取主屏幕
       QScreen *screen = QGuiApplication::primaryScreen();

       // 获取屏幕的可用尺寸（不包括任务栏等系统元素占用的部分）
       QRect availableGeometry = screen->availableGeometry();

       int width = availableGeometry.width();
       int height = availableGeometry.height();

       qDebug() << "屏幕宽度: " << width;
       qDebug() << "屏幕高度: " << height;



   // qreal scale = width / height;				// 960 = 1920 / 2
   // qputenv("QT_SCALE_FACTOR", QString::number(scale).toLatin1());





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
#else



        QApplication a(argc, argv);
        a.setStyle(QStyleFactory::create("Fusion"));
        Widget w;
        w.show();

        QGraphicsView* view = new QGraphicsView;
        QGraphicsScene* scene = new QGraphicsScene();
        QGraphicsProxyWidget* wt = scene->addWidget(&w);
        view->setStyleSheet("background:transparent");
        view->setWindowFlag(Qt::FramelessWindowHint);
        view->setAttribute(Qt::WA_TranslucentBackground);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setScene(scene);
        view->setFocusPolicy(Qt::NoFocus); //防止焦点被View抢走，造成主界面事件失效。
        view->setFixedSize(800,600);//view->setFixedSize(1920,1080); 调整窗口显示大小，根据要显示的分辨率来。
        view->scale(800/1280.00,800/1280.00);     //将1280分辨率的窗口进行缩放到分辨率800的窗口上。
        view->setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform);
        view->move(0,0);
        view->show();

        return a.exec();

  #endif
}
