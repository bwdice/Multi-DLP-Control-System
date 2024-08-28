#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QCloseEvent>
#include <QScreen>
#include <QToolButton>
#pragma execution_character_set("utf-8")


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{



    ui->setupUi(this);
    ui->textEdit_message->clear();

    ui->lineEdit_IP_addr->setText("192.168.0.5");

    ui->devicecontrl->setTabText(2, "调试光机");
    ui->devicecontrl->setTabText(1, "参数配置");
    ui->devicecontrl->setTabText(0, "模型打印");
    ui->devicecontrl->setCurrentIndex(0);

    //ui
    ui->lineEdit_thinkness->setText("0");
    ui->lineEdit_model_x->setText("0");
    ui->lineEdit_model_y->setText("0");
    ui->lineEdit_model_z->setText("0");
    ui->lineEdit_pixel_size->setText("300");
    ui->lineEdit_model_rate->setText("100");

    ui->comboBox_knife_mode->setStyleSheet("QComboBox{color:rgb(255,0,0);}");
    ui->comboBox_dlp_light_type->setStyleSheet("QComboBox{color:rgb(255,0,0);}");

    //setting
    ui->lineEdit_led_current->setText("80");
    ui->lineEdit_led2_current->setText("80");
    ui->lineEdit_motor_step->setText("1600");
    ui->lineEdit_motor_step_2->setText("1600");
    ui->lineEdit_motor_step_3->setText("1600");

    ui->lineEdit_motor_step_z->setText("100");
    ui->lineEdit_motor_step_knife->setText("1600");
    ui->lineEdit_motor_z_return->setText("100");
    ui->lineEdit_motor_z_start->setText("1000");

    //motor para 485
    ui->lineEdit_motor_current_1->setText("6");
    ui->lineEdit_motor_subdivision_1->setText("8");
    ui->lineEdit_motor_start_speed_1->setText("5");
    ui->lineEdit_motor_speed_up_time_1->setText("100");
    ui->lineEdit_motor_speed_down_time_1->setText("100");
    ui->lineEdit_motor_max_speed_1->setText("60");
    ui->lineEdit_motor_return_speed_1->setText("120");
    ui->lineEdit_motor_speed_after_zero_1->setText("60");
    ui->lineEdit_motor_up_down_time_after_zero_1->setText("100");

    ui->lineEdit_motor_current_2->setText("6");
    ui->lineEdit_motor_subdivision_2->setText("8");
    ui->lineEdit_motor_start_speed_2->setText("5");
    ui->lineEdit_motor_speed_up_time_2->setText("100");
    ui->lineEdit_motor_speed_down_time_2->setText("100");
    ui->lineEdit_motor_max_speed_2->setText("60");
    ui->lineEdit_motor_return_speed_2->setText("120");
    ui->lineEdit_motor_speed_after_zero_2->setText("60");
    ui->lineEdit_motor_up_down_time_after_zero_2->setText("100");

    ui->lineEdit_motor_current_3->setText("6");
    ui->lineEdit_motor_subdivision_3->setText("8");
    ui->lineEdit_motor_start_speed_3->setText("5");
    ui->lineEdit_motor_speed_up_time_3->setText("100");
    ui->lineEdit_motor_speed_down_time_3->setText("100");
    ui->lineEdit_motor_max_speed_3->setText("60");
    ui->lineEdit_motor_return_speed_3->setText("120");
    ui->lineEdit_motor_speed_after_zero_3->setText("60");
    ui->lineEdit_motor_up_down_time_after_zero_3->setText("100");

    //liquit auto ctrl
    ui->checkBox_liquit_ctrl_en->setChecked(false);
    ui->lineEdit_liquit_ctrl_posit->setText("30");
    ui->lineEdit_liquit_ctrl_range->setText("1");
    ui->lineEdit_liquit_ctrl_step->setText("500");
    ui->lineEdit_liquit_ctrl_time->setText("5");

    //show gray pic
    ui->lineEdit_show_gray_pic->setText("255");


    //show image
    //m_show_image = new QImage(1920,1080,QImage::Format_RGB888);
    m_show_image = new QImage(400,300,QImage::Format_RGB888);
    m_show_image->fill(QColor(Qt::black));
    ui->label_show_pic->setPixmap(QPixmap::fromImage(*m_show_image));

    //dlp
    m_dlp_power_on_off = false;
    m_dlp_power_on_time_cnt = 0;
    ui->lineEdit_led_current->setText("50");
    ui->lineEdit_led2_current->setText("50");
    ui->comboBox_dlp_light_type->setCurrentIndex(0);

    //print time
    ui->lineEdit_print_time_bottom->setText("3.0");
    ui->lineEdit_print_time_other->setText("3.0");
    ui->lineEdit_stop_time->setText("2.0");
    ui->lineEdit_bottom_layer_num->setText("10");

    //connect pixel
    ui->lineEdit_connect_pixel->setText("10");


    allmotor_normal = false;
    ui->ui_device_status->setStyleSheet("color: black;");
        ui->ui_device_status->setText("空闲中！！！");

    dlp1_powerstate = 0;
    dlp2_powerstate = 0;
    
    //tcp
    m_tcp_connect = false;

    //file
     m_bOpenRK3588File = false;

    //printing
    m_device_printing = false;
    m_device_pause = false;
    m_print_running_time = 0;
    m_print_leave_time = 0;
    m_printing_cnt = 0;


    //para
    m_print_pic_num = 0;
    m_model_size_x = 0;
    m_model_size_y = 0;
    m_model_size_z = 0;
    m_printing_size = 405;
    m_rate_persent = 100;
    m_stop_time = 5.0;
    m_printing_time = 5.0;
    m_bottom_time = 5.0;
    m_bottom_layer_num = 10;





    //thread
    thread = new QThread;
    tcpClient = new TcpClient;
    tcpClient->moveToThread(thread);


    qRegisterMetaType<SLC_FILE_LIST>("SLC_FILE_LIST");
    qRegisterMetaType<SYS_PARA>("SYS_PARA");
    qRegisterMetaType<PRINTING_IMAGE>("PRINTING_IMAGE");


    //connect
    connect( tcpClient, SIGNAL(thread_send_message(QString)), this, SLOT(main_show_message(QString)) );
    connect( this, SIGNAL(main_start_find_device(QString)), tcpClient, SLOT(thread_start_find_device(QString)) );
    connect( this, SIGNAL(main_stop_find_device()), tcpClient, SLOT(thread_stop_find_device()) );
    connect( tcpClient, SIGNAL(thread_tcp_connect(bool, QString)), this, SLOT(main_tcp_connect(bool, QString)) );
    connect( tcpClient, SIGNAL(thread_print_time_result(bool)), this, SLOT(main_print_time_result(bool)));
    connect( this, SIGNAL(main_file_path(QString)), tcpClient, SLOT(thread_file_path(QString)) );
    connect( this, SIGNAL(main_set_print_time(int, int, int, int)), tcpClient, SLOT(thread_set_print_time(int, int, int, int)) );
    connect( this, SIGNAL(main_set_dlp_size(int, int)), tcpClient, SLOT(thread_set_dlp_size(int, int)) );
    connect( this, SIGNAL(main_start_print()), tcpClient, SLOT(thread_start_print()) );
    connect( this, SIGNAL(main_pause_print()), tcpClient, SLOT(thread_pause_print()) );
    connect( this, SIGNAL(main_continue_print()), tcpClient, SLOT(thread_continue_print()) );
    connect( tcpClient, SIGNAL(thread_device_printing()), this, SLOT(main_device_printing()));
    connect( tcpClient, SIGNAL(thread_device_pause()), this, SLOT(main_device_pause()));
    connect( tcpClient, SIGNAL(thread_device_continue()), this, SLOT(main_device_continue()));
    connect( this, SIGNAL(main_stop_print()), tcpClient, SLOT(thread_stop_print()) );
    connect( tcpClient, SIGNAL(thread_device_stop_print()), this, SLOT(main_device_stop_print()));
    connect( tcpClient, SIGNAL(thread_print_pic_num(int)), this, SLOT(main_print_pic_num(int)));
    connect( tcpClient, SIGNAL(thread_print_finish()), this, SLOT(main_print_finish()));
    connect( this, SIGNAL(main_set_connect_pixel(int)), tcpClient, SLOT(thread_set_connect_pixel(int)) );
    connect( tcpClient, SIGNAL(thread_connect_pixel(int)), this, SLOT(main_connect_pixel(int)));
    connect( this, SIGNAL(main_bright_test()), tcpClient, SLOT(thread_bright_test()) );
    connect( this, SIGNAL(main_sharpness_test()), tcpClient, SLOT(thread_sharpness_test()) );
    connect( this, SIGNAL(main_homogeneity_test(int)), tcpClient, SLOT(thread_homogeneity_test(int)) );
    connect( this, SIGNAL(main_stop_test()), tcpClient, SLOT(thread_stop_test()) );
    connect( this, SIGNAL(main_motor_ctrl(int, int, int)), tcpClient, SLOT(thread_motor_ctrl(int, int, int)) );
    connect( this, SIGNAL(main_motor_reset(int)), tcpClient, SLOT(thread_motor_reset(int)) );
    connect( this, SIGNAL(main_motor_clearAlarmCode(int)), tcpClient, SLOT(thread_motor_clearAlarmCode(int)) );
    connect( this, SIGNAL(main_get_liquid_sensor()), tcpClient, SLOT(thread_get_liquid_sensor()) );
    connect( this, SIGNAL(main_set_liquit_auto_ctrl(int, int, int, int, int,int)), tcpClient, SLOT(thread_set_liquit_auto_ctrl(int, int, int, int, int,int)) );
    connect( this, SIGNAL(main_dlp_current_set(int, int, int)), tcpClient, SLOT(thread_dlp_current_set(int, int, int)) );
    connect( tcpClient, SIGNAL(thread_get_system_para(SYS_PARA)), this, SLOT(main_get_system_para(SYS_PARA)));
    connect( tcpClient, SIGNAL(thread_liquid_sensor_ret(int, int)), this, SLOT(main_liquid_sensor_ret(int, int)));
    connect( tcpClient, SIGNAL(thread_motor_reset_ret(int)), this, SLOT(main_motor_reset_ret(int)));
    connect( tcpClient, SIGNAL(thread_motor_clearAlarmCode_ret(int)), this, SLOT(main_motor_clearAlarmCode_ret(int)));
    connect( tcpClient, SIGNAL(thread_liquid_sensor_ret(int)), this, SLOT(main_liquid_sensor_ret(int)));
    connect( this, SIGNAL(main_show_gray_pic(int)), tcpClient, SLOT(thread_show_gray_pic(int)) );
    connect( this, SIGNAL(main_get_version()), tcpClient, SLOT(thread_get_version()) );
    connect( this, SIGNAL(main_set_print_motor_para(int, int, int, int, int)), tcpClient, SLOT(thread_set_print_motor_para(int, int, int, int, int)));
    connect( this, SIGNAL(main_dlp_power_on_off(int)), tcpClient, SLOT(thread_dlp_power_on_off(int)) );
    connect( this, SIGNAL(main_set_motor_para(int, int, int, int, int, int, int, int, int, int)), tcpClient, SLOT(thread_set_motor_para(int, int, int, int, int, int, int, int, int, int)));
    connect( tcpClient, SIGNAL(thread_send_file_persent(int)), this, SLOT(main_send_file_persent(int)));
    connect( tcpClient, SIGNAL(thread_slc_file_list_reflash(SLC_FILE_LIST)), this, SLOT(main_slc_file_list_reflash(SLC_FILE_LIST)));
    connect( tcpClient, SIGNAL(thread_slc_model_size(int, int, int, int, int, int)), this, SLOT(main_slc_model_size(int, int, int, int, int, int)));
    connect( this, SIGNAL(main_open_slc_file(QString)), tcpClient, SLOT(thread_open_slc_file(QString)) );
    connect( this, SIGNAL(main_delete_slc_file(QString)), tcpClient, SLOT(thread_delete_slc_file(QString)) );
    connect( tcpClient, SIGNAL(thread_software_version(QString)), this, SLOT(main_software_version(QString)));
    connect( tcpClient, SIGNAL(thread_printing_image(PRINTING_IMAGE)), this, SLOT(main_printing_image(PRINTING_IMAGE)));
    connect( tcpClient, SIGNAL(thread_get_motorinfor(unsigned char*, unsigned int)), this, SLOT(main_get_motorinfor(unsigned char*, unsigned int)));
  

    //start
    thread->start();


    //camera
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    //std::cout << "camera num: " << cameras.count() << std::endl;
    if(cameras.count() > 1)
    {
        foreach (const QCameraInfo &cameraInfo, cameras) {
            //std::cout << cameraInfo.description().toStdString() << std::endl;
        }
        camera = new QCamera(cameras.at(1));
    }
    else
    {
        return;
    }


    viewfinder = new QCameraViewfinder(ui->widget_camera);
    camera->setViewfinder(viewfinder);
    viewfinder->resize(580, 330);

    imageCapture = new QCameraImageCapture(camera);

    camera->start();

    AutoZoomButtonSize();
}

MainWindow::~MainWindow()
{
    thread->terminate();
    thread->quit();
    thread->wait();

    delete ui;
    delete tcpClient;
    delete m_show_image;
}


void MainWindow::closeEvent( QCloseEvent * event )
{
    if(m_device_printing)
    {
        QMessageBox::question(this,"提示","请先停止打印，再关闭软件");//提示窗口返回类型为int类型
        event->ignore();
    }
    else
    {
        event->accept();
    }
}





void MainWindow::timerEvent(QTimerEvent* ev)
{
    //锟斤拷时锟铰硷拷
    if(ev->timerId() == timer_id1)
    {
        int hour,min,sec;
        QString str;

        if(!m_device_pause)
        {
            m_print_running_time ++;
            hour = m_print_running_time/3600;
            min = (m_print_running_time%3600)/60;
            sec = m_print_running_time%60;
            str = QString("打印已用时： %1 时 %2 分 %3 秒").arg(hour).arg(min).arg(sec);
            ui->label_print_running_time->setText(str);
        }

        if(!m_device_pause)
        {
            m_print_leave_time --;
            if(m_print_leave_time < 0)
            {
                m_print_leave_time = 0;
            }
            hour = m_print_leave_time/3600;
            min = (m_print_leave_time%3600)/60;
            sec = m_print_leave_time%60;
            str = QString("预计打印剩余时间： %1 时 %2 分 %3 秒").arg(hour).arg(min).arg(sec);
            ui->label_print_leave_time->setText(str);
        }
    }
    else if(ev->timerId() == timer_id2)
    {
        m_dlp_power_on_time_cnt++;
        if(!m_dlp_power_on_off)
        {
            ui->progressBar_dlp_power_on->setValue(m_dlp_power_on_time_cnt);
        }
        else
        {
            ui->progressBar_dlp_power_on->setValue(100-m_dlp_power_on_time_cnt);
        }
        if(m_dlp_power_on_time_cnt >= 100)
        {
            m_dlp_power_on_time_cnt = 0;
            if(!m_dlp_power_on_off)
            {
                m_dlp_power_on_off = true;
            }
            else
            {
                m_dlp_power_on_off = false;
            }
            killTimer(timer_id2);
        }
    }
    else if(ev->timerId() == timer_id4)
    {
       // qDebug()<<"timer4";
       on_btn_get_liquid_sensor_clicked();
    }
}


void MainWindow::Show_Message(QString str)
{
    ui->textEdit_message->append(str);
}



void MainWindow::on_btn_find_device_clicked()
{

        QString ip = ui->lineEdit_IP_addr->text();
        m_tcp_connect = false;
        emit main_start_find_device(ip);

}




void MainWindow::on_btn_open_file_clicked()
{
    ui->progressBar_download->setValue(0);
    m_fileName = QFileDialog::getOpenFileName(this, "Open File", "QCoreApplication::applicationFilePath()", "*.slc");
    if (!m_fileName.isEmpty())
    {
        ui->lineEdit_filename->setText(m_fileName);
        m_bOpenFile = true;
    }
}



void MainWindow::on_btn_download_file_clicked()
{
    if(m_bOpenFile)
    {
        emit main_file_path(m_fileName);
    }
    else
    {
        QMessageBox::warning(this, QString("Warning"),QString("请先打开文件"));
    }
}

void MainWindow::on_btn_download_stop_clicked()
{

}

void MainWindow::on_btn_open_device_file_clicked()
{
    QString filename;
    filename = ui->comboBox_slc_file_list->currentText();
    if(filename.isEmpty())
    {
        QMessageBox::warning(this, QString("Warning"),QString("未选择文件，请先选择文件"));
    }
    else
    {
        m_bOpenRK3588File = true;
        emit main_open_slc_file(filename);
    }
}



void MainWindow::on_btn_delete_device_file_clicked()
{
    QString filename;
    filename = ui->comboBox_slc_file_list->currentText();
    if(filename.isEmpty())
    {
        QMessageBox::warning(this, QString("Warning"),QString("未选择文件，请先选择文件"));
    }
    else
    {
        emit main_delete_slc_file(filename);
    }
}



void MainWindow::on_btn_set_slice_clicked()
{
    QString pixel_size, model_rate;

    pixel_size = ui->lineEdit_pixel_size->text();
    if(pixel_size.isEmpty())
    {
        QMessageBox::warning(this, QString("Warning"),QString("水平最大尺寸非有效数据，请重新输入！"));
        return;
    }

    bool ok;
    int max_size = pixel_size.toInt(&ok);
    if(!ok)
    {
        QMessageBox::warning(this, QString("Warning"),QString("水平最大尺寸非有效数据，请重新输入！"));
        return;
    }

    model_rate = ui->lineEdit_model_rate->text();
    int rate = model_rate.toInt(&ok);
    if(!ok)
    {
        QMessageBox::warning(this, QString("Warning"),QString("模型缩放比例非有效数据，请重新输入！"));
        return;
    }

    m_printing_size = max_size;


    emit main_set_dlp_size(max_size, rate);

    Show_Message("层厚和像素大小设置完成");
}





void MainWindow::on_btn_set_print_time_clicked()
{
    double lfPrintTime = 0;
    int print_time_ms = 0;
    double lfStopTime = 0;
    int stop_time_ms = 0;
    double lfPrintTimeBottom = 0;
    int print_time_bottom_ms = 0;
    int bottom_layer_num = 0;


    if(!m_tcp_connect)
    {
        QMessageBox::warning(this, QString("Warning"),QString("未连接到设备，请先连接到设备！"));
    }
    else
    {
        lfPrintTimeBottom = ui->lineEdit_print_time_bottom->text().toDouble();
        bottom_layer_num = ui->lineEdit_bottom_layer_num->text().toInt();
        lfPrintTime = ui->lineEdit_print_time_other->text().toDouble();
        lfStopTime = ui->lineEdit_stop_time->text().toDouble();
        if((lfPrintTime < 1) || (lfStopTime < 1) || (lfPrintTimeBottom < 1))
        {
            QMessageBox::warning(this, QString("Warning"),QString("打印时间不对，请重新输入！"));
            return;
        }
        if(bottom_layer_num < 1)
        {
            QMessageBox::warning(this, QString("Warning"),QString("底层层数不对，请重新输入！"));
            return;
        }

        Show_Message("正在设置打印时间参数 ......");
        print_time_bottom_ms = (int)(lfPrintTimeBottom*1000);
        print_time_ms = (int)(lfPrintTime*1000);
        stop_time_ms = (int)(lfStopTime*1000);

        emit main_set_print_time(print_time_ms, stop_time_ms, print_time_bottom_ms, bottom_layer_num);
    }
}



void MainWindow::on_btn_start_print_clicked()
{
    if(m_device_printing)
    {
        return;
    }

    if(!m_tcp_connect)
    {
        QMessageBox::warning(this, QString("Warning"),QString("未连接到设备，请先连接到设备！"));
        return;
    }

    if(!m_dlp_power_on_off)
    {
        QMessageBox::warning(this, QString("Warning"),QString("光机未完成开机！"));
        return;
    }

    if(!m_bOpenRK3588File)
    {
        QMessageBox::warning(this, QString("Warning"),QString("请先打开设备打印文件！"));
        return;
    }

    double model_size_x_printing = m_model_size_x*m_rate_persent/100;
    double model_size_y_printing = m_model_size_y*m_rate_persent/100;
    double dlp_size_x = (double)m_printing_size;
    double dlp_size_y = dlp_size_x*4320/3840;
    if((model_size_x_printing > (dlp_size_x-1)) || (model_size_y_printing > (dlp_size_y-1)))
    {
        QMessageBox::warning(this, QString("Warning"),QString("打印模型大小超出范围，请调整缩放比例"));
        return;
    }

    ui->btn_stop_print->setEnabled(true);
    ui->btn_start_print->setEnabled(false);
    ui->ui_device_status->setStyleSheet("color: green;");
    ui->ui_device_status->setText("正在打印中...");
    emit main_start_print();
    Show_Message("开始控制设备打印 ......");
}




void MainWindow::on_btn_stop_print_clicked()
{
    //if(m_device_printing)
    {
        emit main_stop_print();
        Show_Message("控制设备停止打印 ......");
        ui->ui_device_status->setStyleSheet("color: black;");
        ui->ui_device_status->setText("打印已停止！！！");
    }
    ui->btn_stop_print->setEnabled(true);
    ui->btn_start_print->setEnabled(true);
    
}



void MainWindow::on_btn_pause_print_clicked()
{
    if(m_device_printing)
    {
        emit main_pause_print();
        Show_Message("控制设备暂停打印 ......");
        ui->ui_device_status->setStyleSheet("color: red;");
        ui->ui_device_status->setText("打印已暂停...");
    }
}


void MainWindow::on_btn_continue_print_clicked()
{
    if(m_device_printing)
    {
        emit main_continue_print();
        Show_Message("控制设备继续打印 ......");
        ui->ui_device_status->setStyleSheet("color: green;");
        ui->ui_device_status->setText("正在打印中...");
    }
}








//*****************************************************************************************************************
//
//                      main slot
//
//***************************************************************************************************************

void MainWindow::main_show_message(QString str)
{
    Show_Message(str);
}


void MainWindow::main_tcp_connect(bool flag, QString ip_addr)
{
    if(flag)
    {
        Show_Message(QString("设备已连接"));
        ui->btn_find_device->setText("设备已连接");
        m_tcp_connect = true;
    }
    else
    {
        Show_Message(QString("设备已断开连接"));
        ui->btn_find_device->setText("设备已断开连接");
        m_tcp_connect = false;
    }
}


void MainWindow::main_print_time_result(bool flag)
{
    if(flag)
    {
        Show_Message("打印时间间隔设置成功！");
    }
    else
    {
        Show_Message("数据发送超时！");
    }
}

void MainWindow::main_device_printing()
{
    Show_Message("设备开始打印");

    m_device_printing = true;
    m_device_pause = false;
    m_print_running_time = 0;
    m_print_leave_time = (int)((m_print_pic_num - m_bottom_layer_num)*(m_printing_time + m_stop_time) + m_bottom_layer_num*(m_bottom_time+m_stop_time));
    m_printing_cnt = 0;


    int leave_hour = m_print_leave_time/3600;
    int leave_min = (m_print_leave_time%3600)/60;
    int leave_second = m_print_leave_time%60;
    ui->label_print_running_time->setText("打印已用时: 0 时 0 分 0 秒");
    ui->label_print_leave_time->setText(QString("预计打印剩余时间：%1 时 %2 分 %3 秒").arg(leave_hour).arg(leave_min).arg(leave_second));
    ui->label_print_num->setText("");
    ui->label_current_state->setText("正在打印中......");
    ui->progressBar_printing->setValue(0);

    timer_id1 = startTimer(1000);
}


void MainWindow::main_device_pause()
{
    Show_Message("设备暂停打印");
    ui->label_current_state->setText(QString("当前状态：设备暂停打印"));
    m_device_pause = true;
}

void MainWindow::main_device_continue()
{
    Show_Message("设备继续打印");
    ui->label_current_state->setText(QString("当前状态：正在打印中......"));
    m_device_pause = false;
}


void MainWindow::main_device_stop_print()
{
    Show_Message("设备已停止打印");
    killTimer(timer_id1);
    m_device_printing = false;
    ui->label_current_state->setText(QString("当前状态：设备已停止打印"));
    ui->label_print_leave_time->setText("预计打印剩余时间： 0 时 0 分 0 秒");

    ui->label_print_num->setText("打印已停止");
    ui->btn_start_print->setEnabled(true);
}


void MainWindow::main_print_pic_num(int pic_num)
{
    QString str;
    str = QString("正在打印图片第 %1 张").arg(pic_num);
    ui->label_print_num->setText(str);
    //Show_Message(str);

    if(pic_num >= (m_bottom_layer_num+1))
    {
        m_print_leave_time = (int)((m_print_pic_num - pic_num+1)*m_printing_time) -2;
    }
    else
    {
        m_print_leave_time = (int)((m_print_pic_num - m_bottom_layer_num)*(m_printing_time+m_stop_time) + (m_bottom_layer_num -pic_num+1)*(m_bottom_time+m_stop_time)) -2;
    }

    int persent = (int)(pic_num*100/m_print_pic_num);
    ui->progressBar_printing->setValue(persent);
}




void MainWindow::main_print_finish()
{
    m_device_printing = false;
    ui->label_current_state->setText(QString("当前状态：打印已完成"));
    killTimer(timer_id1);
    ui->label_print_running_time->setText("打印已用时：");
    ui->label_print_leave_time->setText("预计打印剩余时间：");

    ui->label_print_num->setText("打印已完成");
    
    ui->ui_device_status->setStyleSheet("color: black;");
    ui->ui_device_status->setText("打印已停止！！！");

    ui->btn_stop_print->setEnabled(true);
    ui->btn_start_print->setEnabled(true);
}

void MainWindow::main_connect_pixel(int connect_pixel)
{
    ui->lineEdit_connect_pixel->setText(QString("%1").arg(connect_pixel));
}


void MainWindow::main_get_system_para(SYS_PARA sys_para)
{
    ui->lineEdit_print_time_other->setText(QString("%1").arg((double)sys_para.printint_time.printing_time/1000.0));
    ui->lineEdit_stop_time->setText(QString("%1").arg((double)sys_para.printint_time.stop_time/1000.0));
    ui->lineEdit_print_time_bottom->setText(QString("%1").arg((double)sys_para.printint_time.bottom_time/1000.0));
    ui->lineEdit_bottom_layer_num->setText(QString("%1").arg(sys_para.printint_time.bottom_layer_num));

    ui->lineEdit_pixel_size->setText(QString("%1").arg(sys_para.slice.printing_size));
    ui->lineEdit_model_rate->setText(QString("%1").arg(sys_para.slice.rate_persent));
    ui->lineEdit_connect_pixel->setText(QString("%1").arg(sys_para.slice.connect_pixel));

    ui->lineEdit_motor_step_z->setText(QString("%1").arg(sys_para.motor_print.z_step));
    ui->lineEdit_motor_step_knife->setText(QString("%1").arg(sys_para.motor_print.knife_step));
    ui->comboBox_knife_mode->setCurrentIndex(sys_para.motor_print.knife_mode);
    ui->lineEdit_motor_z_return->setText(QString("%1").arg(sys_para.motor_print.z_return));
    ui->lineEdit_motor_z_start->setText(QString("%1").arg(sys_para.motor_print.z_start));

    ui->lineEdit_motor_current_1->setText(QString("%1").arg(sys_para.motor_run[0].current));
    ui->lineEdit_motor_subdivision_1->setText(QString("%1").arg(sys_para.motor_run[0].subdivision));
    ui->lineEdit_motor_start_speed_1->setText(QString("%1").arg(sys_para.motor_run[0].start_speed));
    ui->lineEdit_motor_speed_up_time_1->setText(QString("%1").arg(sys_para.motor_run[0].speed_up_time));
    ui->lineEdit_motor_speed_down_time_1->setText(QString("%1").arg(sys_para.motor_run[0].speed_down_time));
    ui->lineEdit_motor_max_speed_1->setText(QString("%1").arg(sys_para.motor_run[0].running_speed));
    ui->lineEdit_motor_return_speed_1->setText(QString("%1").arg(sys_para.motor_run[0].return_speed));
    ui->lineEdit_motor_speed_after_zero_1->setText(QString("%1").arg(sys_para.motor_run[0].return_speed_after_zero));
    ui->lineEdit_motor_up_down_time_after_zero_1->setText(QString("%1").arg(sys_para.motor_run[0].return_speed_up_down_time));

    ui->lineEdit_motor_current_2->setText(QString("%1").arg(sys_para.motor_run[1].current));
    ui->lineEdit_motor_subdivision_2->setText(QString("%1").arg(sys_para.motor_run[1].subdivision));
    ui->lineEdit_motor_start_speed_2->setText(QString("%1").arg(sys_para.motor_run[1].start_speed));
    ui->lineEdit_motor_speed_up_time_2->setText(QString("%1").arg(sys_para.motor_run[1].speed_up_time));
    ui->lineEdit_motor_speed_down_time_2->setText(QString("%1").arg(sys_para.motor_run[1].speed_down_time));
    ui->lineEdit_motor_max_speed_2->setText(QString("%1").arg(sys_para.motor_run[1].running_speed));
    ui->lineEdit_motor_return_speed_2->setText(QString("%1").arg(sys_para.motor_run[1].return_speed));
    ui->lineEdit_motor_speed_after_zero_2->setText(QString("%1").arg(sys_para.motor_run[1].return_speed_after_zero));
    ui->lineEdit_motor_up_down_time_after_zero_2->setText(QString("%1").arg(sys_para.motor_run[1].return_speed_up_down_time));

    ui->lineEdit_motor_current_3->setText(QString("%1").arg(sys_para.motor_run[2].current));
    ui->lineEdit_motor_subdivision_3->setText(QString("%1").arg(sys_para.motor_run[2].subdivision));
    ui->lineEdit_motor_start_speed_3->setText(QString("%1").arg(sys_para.motor_run[2].start_speed));
    ui->lineEdit_motor_speed_up_time_3->setText(QString("%1").arg(sys_para.motor_run[2].speed_up_time));
    ui->lineEdit_motor_speed_down_time_3->setText(QString("%1").arg(sys_para.motor_run[2].speed_down_time));
    ui->lineEdit_motor_max_speed_3->setText(QString("%1").arg(sys_para.motor_run[2].running_speed));
    ui->lineEdit_motor_return_speed_3->setText(QString("%1").arg(sys_para.motor_run[2].return_speed));
    ui->lineEdit_motor_speed_after_zero_3->setText(QString("%1").arg(sys_para.motor_run[2].return_speed_after_zero));
    ui->lineEdit_motor_up_down_time_after_zero_3->setText(QString("%1").arg(sys_para.motor_run[2].return_speed_up_down_time));

    if(sys_para.liquit_ctrl.is_enable == 1)
    {
        ui->checkBox_liquit_ctrl_en->setChecked(true);
    }
    else
    {
        ui->checkBox_liquit_ctrl_en->setChecked(false);
    }
    ui->lineEdit_liquit_ctrl_posit->setText(QString("%1").arg((double)sys_para.liquit_ctrl.posit/1000));
    ui->lineEdit_liquit_ctrl_range->setText(QString("%1").arg((double)sys_para.liquit_ctrl.range/1000));
    ui->lineEdit_liquit_ctrl_step->setText(QString("%1").arg(sys_para.liquit_ctrl.step));
    ui->lineEdit_liquit_ctrl_time->setText(QString("%1").arg((double)sys_para.liquit_ctrl.time/1000)); 
    ui->lineEdit_liquit_max_disance->setText(QString("%1").arg((double)sys_para.liquit_ctrl.maxdis/1000));
    
    
    if(sys_para.dlp.light_source_type == 0)
    {
        ui->comboBox_dlp_light_type->setCurrentIndex(0);
        ui->lineEdit_led_current->setText(QString("%1").arg(sys_para.dlp.dlp1_current));
        ui->lineEdit_led2_current->setText(QString("%1").arg(sys_para.dlp.dlp2_current));
    }
    else
    {
        ui->comboBox_dlp_light_type->setCurrentIndex(1);
        ui->lineEdit_led_current->setText(QString("%1").arg((double)sys_para.dlp.dlp1_current/10.0));
        ui->lineEdit_led2_current->setText(QString("%1").arg((double)sys_para.dlp.dlp2_current/10.0));
    }

    #if 0
    if(para.dlp.dlp1_powerstate == 0x01)
    {
       Show_Message(QString("DLP1 已开机"));
    }
    else if(para.dlp.dlp1_powerstate == 0x02)
    {
       Show_Message(QString("DLP1 已待机"));
    }
    else if(para.dlp.dlp1_powerstate == 0x04)
    {
       Show_Message(QString("DLP1 正在开机"));
    }
    else if(para.dlp.dlp1_powerstate == 0x08)
    {
       Show_Message(QString("DLP1 正在待机"));
    }
    
    if(para.dlp.dlp2_powerstate == 0x01)
    {
       Show_Message(QString("DLP2 已开机"));
    }
    else if(para.dlp.dlp2_powerstate == 0x02)
    {
       Show_Message(QString("DLP2 已待机"));
    }
    else if(para.dlp.dlp2_powerstate == 0x04)
    {
       Show_Message(QString("DLP2 正在开机"));
    }
    else if(para.dlp.dlp2_powerstate == 0x08)
    {
       Show_Message(QString("DLP2 正在待机"));
    }

    #endif 



    //para
    m_printing_size = sys_para.slice.printing_size;
    m_rate_persent = sys_para.slice.rate_persent;
    m_stop_time = (double)sys_para.printint_time.stop_time/1000.0;
    m_printing_time = (double)sys_para.printint_time.printing_time/1000.0;
    m_bottom_time = (double)sys_para.printint_time.bottom_time/1000.0;
    m_bottom_layer_num = sys_para.printint_time.bottom_layer_num;
}



void MainWindow::main_liquid_sensor_ret(int success, int liquid_sensor)
{
    QString str;
    if(success != 1)
    {
        QMessageBox::warning(this, QString("Warning"),QString("读取液位传感器失败！"));
    }
    else
    {
        str = QString("液位位置： %1 mm").arg((double)liquid_sensor/1000);
        ui->label_liquid_sensor->setText(str);
    }
}



void MainWindow::main_motor_ctrl_ret()
{
    QMessageBox::warning(this, QString("Warning"),QString("电机运动完成！"));
}

void MainWindow::main_motor_reset_ret(int channel)
{
    QString str;
    str = QString("电机 %1 复位完成！").arg(channel+1);
    Show_Message(str);
}

void MainWindow::main_motor_clearAlarmCode_ret(int channel)
{
    QString str;
    str = QString("电机 %1 复位完成！").arg(channel+1);
    Show_Message(str);
}

void MainWindow::main_send_file_persent(int persent)
{
    ui->progressBar_download->setValue(persent);
}


void MainWindow::main_slc_file_list_reflash(SLC_FILE_LIST file_list)
{
    quint32 file_num;
    file_num = file_list.file_num;

    QString filename;
    if(file_num > 0)
    {
        ui->comboBox_slc_file_list->clear();
        for(quint32 i=0;i<file_num;i++)
        {
            filename = QString::fromUtf8((char *)(file_list.filename_buff+256*i));
            ui->comboBox_slc_file_list->addItem(filename);
        }
    }
}



void MainWindow::main_slc_model_size(int status, int x, int y, int z, int thinkness, int slice_num)
{
    if(status == 1)
    {
        ui->lineEdit_model_x->setText(QString("%1").arg((double)x/1000.0));
        ui->lineEdit_model_y->setText(QString("%1").arg((double)y/1000.0));
        ui->lineEdit_model_z->setText(QString("%1").arg((double)z/1000.0));
        ui->lineEdit_thinkness->setText(QString("%1").arg((double)thinkness/1000.0));
        ui->lineEdit_slice_num->setText(QString("%1").arg(slice_num));

        m_print_pic_num = slice_num;
        m_model_size_x = (double)x/1000.0;
        m_model_size_y = (double)y/1000.0;
        m_model_size_z = (double)z/1000.0;

        QString filename;
        filename = QString("当前打开文件：") + ui->comboBox_slc_file_list->currentText();
        ui->label_current_slc_file->setText(filename);
    }
    else
    {
        ui->lineEdit_model_x->clear();
        ui->lineEdit_model_y->clear();
        ui->lineEdit_model_z->clear();
        ui->lineEdit_thinkness->clear();
        ui->lineEdit_slice_num->clear();
        QMessageBox::warning(this, QString("Warning"),QString("设备打开模型文件失败！"));
    }
}



void MainWindow::main_software_version(QString version)
{
    ui->label_version->setText(version);
}


void MainWindow::main_printing_image(PRINTING_IMAGE image_data)
{
    Show_Message("Main显示图像");

    int width = image_data.image_w;
    int height = image_data.image_h;

    Show_Message(QString("width:%1, height:%2").arg(width).arg(height));

    QImage image(width, height, QImage::Format_Mono);
    uchar* pLine = image.scanLine(0); //
    memcpy(pLine, image_data.buff, width * height/8);

    QRect rect = ui->label_show_pic->rect();
    image = image.scaled(rect.width(), rect.height());
    ui->label_show_pic->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::main_poweroff_buttonenb(bool enb1,bool enb2)
{
    ui->btn_dlp_power_on_off->setEnabled(enb1);
    ui->btn_dlp_power_off->setEnabled(enb2);
}


// 得到电机数据
void MainWindow::main_get_motorinfor(unsigned char *pt,unsigned int len)
{
   
    memcpy(motor_paradata[0].motorinfo_buf,pt,44);
    memcpy(motor_paradata[1].motorinfo_buf,&pt[44],44);
    memcpy(motor_paradata[2].motorinfo_buf,&pt[88],44);
    dlp1_powerstate = pt[132];
    dlp2_powerstate = pt[133];


    if(dlp1_powerstate == 0x01)
    {
       ui->lineEdit_DLP1_POWERSTATE->setText("已开机");
       main_poweroff_buttonenb(false,true);
       if(m_dlp_power_on_off != true)
       {
          ui->progressBar_dlp_power_on->setValue(100);
          m_dlp_power_on_off = true;
          m_dlp_power_on_time_cnt = 0;
          killTimer(timer_id2);
       }
    }
    else if(dlp1_powerstate == 0x02)
    {
       ui->lineEdit_DLP1_POWERSTATE->setText("已待机");
       main_poweroff_buttonenb(true,false);
       if(m_dlp_power_on_off != false)
       {
          ui->progressBar_dlp_power_on->setValue(0);
          m_dlp_power_on_off = false;
          m_dlp_power_on_time_cnt = 0;
          killTimer(timer_id2);
       }
    }
    else if(dlp1_powerstate == 0x04)
    {
       ui->lineEdit_DLP1_POWERSTATE->setText("正在开机中,请稍候");
       main_poweroff_buttonenb(false,false);
    }
    else if(dlp1_powerstate == 0x08)
    {
       ui->lineEdit_DLP1_POWERSTATE->setText("正在待机中,请稍候");
       main_poweroff_buttonenb(false,false);
    }
    
    if(dlp2_powerstate == 0x01)
    {
       ui->lineEdit_DLP2_POWERSTATE->setText("已开机");
      
    }
    else if(dlp2_powerstate == 0x02)
    {
       ui->lineEdit_DLP2_POWERSTATE->setText("已待机");
    }
    else if(dlp2_powerstate == 0x04)
    {
       ui->lineEdit_DLP2_POWERSTATE->setText("正在开机中,请稍候");
    }
    else if(dlp2_powerstate == 0x08)
    {
       ui->lineEdit_DLP2_POWERSTATE->setText("正在待机中,请稍候");
    }
    


    
    //qDebug()<<"status:"<<motor_paradata[0].motorinfo.status;
    QString tmp;
    //tmp = uncharToQstring(pt, len);
    //qDebug()<<"motoinfo:"<<tmp;

    // 状态
    tmp = "";
    if(motor_paradata[0].motorinfo.status & CW_STATUS)
    {
       tmp += " CW";
    }
    if(motor_paradata[0].motorinfo.status & CCW_STATUS)
    {
       tmp += " CCW";
    }
    if(motor_paradata[0].motorinfo.status & RUNNING_STATUS)
    {
       tmp += " RUN";
    }
    else
    {
       tmp += " IDLE";
    }
    
   // tmp = QString::number(motor_paradata[0].motorinfo.status, 16).toUpper(); // 
    ui->lineEdit_state_Z_status->setText(tmp);

    tmp = "";
    if(motor_paradata[1].motorinfo.status & CW_STATUS)
    {
       tmp += " CW";
    }
    if(motor_paradata[1].motorinfo.status & CCW_STATUS)
    {
       tmp += " CCW";
    }
    if(motor_paradata[1].motorinfo.status & RUNNING_STATUS)
    {
       tmp += " RUN";
    }
    else
    {
       tmp += " IDLE";
    }
    //tmp = QString::number(motor_paradata[1].motorinfo.status, 16).toUpper(); // 
    ui->lineEdit_state_knife_status->setText(tmp);

    tmp = "";
    if(motor_paradata[2].motorinfo.status & CW_STATUS)
    {
       tmp += " CW";
    }
    if(motor_paradata[2].motorinfo.status & CCW_STATUS)
    {
       tmp += " CCW";
    }
    if(motor_paradata[2].motorinfo.status & RUNNING_STATUS)
    {
       tmp += " RUN";
    }
    else
    {
       tmp += " IDLE";
    }
    //tmp = QString::number(motor_paradata[2].motorinfo.status, 16).toUpper(); // 
    ui->lineEdit_state_liqut__status->setText(tmp);

    // 当前位置 
    ui->lineEdit_state_Z_pos->setText(QString::number(motor_paradata[0].motorinfo.pos));
    ui->lineEdit_state_knife_pos->setText(QString::number(motor_paradata[1].motorinfo.pos));
    ui->lineEdit_state_liqut_pos->setText(QString::number(motor_paradata[2].motorinfo.pos));

    // 当前速度
    ui->lineEdit_state_Z_speed->setText(QString::number(motor_paradata[0].motorinfo.speed));
    ui->lineEdit_state_knife_speed->setText(QString::number(motor_paradata[1].motorinfo.speed));
    ui->lineEdit_state_liqut__speed->setText(QString::number(motor_paradata[2].motorinfo.speed));
    //报警代码
    tmp = QString::number(motor_paradata[0].motorinfo.alalrmcode, 16).toUpper(); // 
    ui->lineEdit_state_Z_alarmcode->setText(tmp);

    tmp = QString::number(motor_paradata[1].motorinfo.alalrmcode, 16).toUpper(); // 
    ui->lineEdit_state_knife_alarmcode->setText(tmp);

    tmp = QString::number(motor_paradata[2].motorinfo.alalrmcode, 16).toUpper(); // 
    ui->lineEdit_state_liqut__alarmcode->setText(tmp);

    // 连接状态
    if(motor_paradata[0].motorinfo.connectfailcnt == 0)
    {
        ui->lineEdit_state_Z_connectsate->setStyleSheet("color: blue;");
        ui->lineEdit_state_Z_connectsate->setText("电机已连接");
    }
    else
    {
        ui->lineEdit_state_Z_connectsate->setStyleSheet("color: red;");
        ui->lineEdit_state_Z_connectsate->setText("电机未连接");
    }

    if(motor_paradata[1].motorinfo.connectfailcnt == 0)
    {
        ui->lineEdit_state_knife_connectsate->setStyleSheet("color: blue;");
        ui->lineEdit_state_knife_connectsate->setText("电机已连接");
    }
    else
    {
        ui->lineEdit_state_knife_connectsate->setStyleSheet("color: red;");
        ui->lineEdit_state_knife_connectsate->setText("电机未连接");
    }

    if(motor_paradata[2].motorinfo.connectfailcnt == 0)
    {
        ui->lineEdit_state_liqut_connectsate->setStyleSheet("color: blue;");
        ui->lineEdit_state_liqut_connectsate->setText("电机已连接");
    }
    else
    {
        ui->lineEdit_state_liqut_connectsate->setStyleSheet("color: red;");
        ui->lineEdit_state_liqut_connectsate->setText("电机未连接");
        
    }

    if(motor_paradata[0].motorinfo.connectfailcnt < 5 && motor_paradata[1].motorinfo.connectfailcnt < 5)
    {
        if(!allmotor_normal)
        {
           allmotor_normal = true;
           
        }
        
    }
    else
    {
        if(allmotor_normal)
        {
           allmotor_normal = false;
           on_btn_pause_print_clicked();
        }
    }


}



//*****************************************************************************************************************
//
//                      public
//
//***************************************************************************************************************







//*****************************************************************************************************************
//
//                      setting
//
//***************************************************************************************************************
void MainWindow::on_btn_show_grating_clicked()
{
    int connect_pixel = 0;


    if(!m_tcp_connect)
    {
        QMessageBox::warning(this, QString("Warning"),QString("未连接到设备，请先连接到设备！"));
    }
    else
    {
        connect_pixel = ui->lineEdit_connect_pixel->text().toInt();
        if(connect_pixel < 0)
        {
            QMessageBox::warning(this, QString("Warning"),QString("拼接像素输入不对，请重新输入！"));
        }
        else
        {
            Show_Message("正在设置拼接像素个数和显示网格 ......");

            emit main_set_connect_pixel(connect_pixel);
        }
    }
}



void MainWindow::on_btn_homogeneity_test_clicked()
{
    int image_num= 0;
    image_num = ui->comboBox_homogeneity_test->currentIndex() + 1;
    emit main_homogeneity_test(image_num);
}

void MainWindow::on_btn_brightness_test_clicked()
{
    emit main_bright_test();
}

void MainWindow::on_btn_sharpness_test_clicked()
{
    emit main_sharpness_test();
}

void MainWindow::on_btn_stop_test_clicked()
{
    emit main_stop_test();
}


void MainWindow::on_comboBox_dlp_light_type_activated(int index)
{
    if(index == 0)
    {
        ui->label_dlp1_current_unit->setText("");
        ui->label_dlp2_current_unit->setText("");
    }
    else
    {
        ui->label_dlp1_current_unit->setText("%");
        ui->label_dlp2_current_unit->setText("%");
    }
}

void MainWindow::on_btn_set_led_cnrrent_clicked()
{
    double lfCurrent1, lfCurrent2;
    int current1, current2;
    int light_type;
    QString str;


    light_type = ui->comboBox_dlp_light_type->currentIndex();

    bool ok;
    str = ui->lineEdit_led_current->text();
    lfCurrent1 = str.toDouble(&ok);
    if(!ok)
    {
        QMessageBox::warning(this, QString("Warning"),QString("LED1亮度输入不对，请重新输入！"));
        return;
    }
    str = ui->lineEdit_led2_current->text();
    lfCurrent2 = str.toDouble(&ok);
    if(!ok)
    {
        QMessageBox::warning(this, QString("Warning"),QString("LED2亮度输入不对，请重新输入！"));
        return;
    }

    if(light_type == 0) //led
    {
        current1 = (int)(lfCurrent1);
        current2 = (int)(lfCurrent2);
    }
    else //laser
    {
        current1 = (int)(lfCurrent1*10);
        current2 = (int)(lfCurrent2*10);
    }



    emit main_dlp_current_set(light_type, current1, current2);
}


void MainWindow::on_btn_motor_ctrl_clicked()
{
    int channel;
    int diret;
    int step;
    int speed;
    QString str;
    bool ok;

    channel = 0;
    diret = ui->combo_motor_direct->currentIndex();
    str = ui->lineEdit_motor_step->text();
    step = str.toInt(&ok);
    if(!ok)
    {
        QMessageBox::warning(this, QString("Warning"),QString("电机步进值输入不对，请重新输入！"));
        return;
    }

    emit main_motor_ctrl(channel, diret, step);
}


void MainWindow::on_btn_motor_ctrl_2_clicked()
{
    int channel;
    int diret;
    int step;
    QString str;
    bool ok;

    channel = 1;
    diret = ui->combo_motor_direct_2->currentIndex();
    str = ui->lineEdit_motor_step_2->text();
    step = str.toInt(&ok);
    if(!ok)
    {
        QMessageBox::warning(this, QString("Warning"),QString("电机步进值输入不对，请重新输入！"));
        return;
    }


    emit main_motor_ctrl(channel, diret, step);
}

void MainWindow::on_btn_motor_ctrl_3_clicked()
{
    int channel;
    int diret;
    int step;
    QString str;
    bool ok;

    channel = 2;
    diret = ui->combo_motor_direct_3->currentIndex();
    str = ui->lineEdit_motor_step_3->text();
    step = str.toInt(&ok);
    if(!ok)
    {
        QMessageBox::warning(this, QString("Warning"),QString("电机步进值输入不对，请重新输入！"));
        return;
    }

    emit main_motor_ctrl(channel, diret, step);
}



void MainWindow::on_btn_motor_reset_ctrl_clicked()
{
    int channel = 0;

    emit main_motor_reset(channel);
}


void MainWindow::on_btn_motor_reset_ctrl_2_clicked()
{
    int channel = 1;

    emit main_motor_reset(channel);
}

void MainWindow::on_btn_motor_reset_ctrl_3_clicked()
{
    int channel = 2;

    emit main_motor_reset(channel);
}


void MainWindow::on_btn_get_liquid_sensor_clicked()
{
    emit main_get_liquid_sensor();
}


void MainWindow::on_btn_set_liquit_ctrl_clicked()
{
    int is_check = 0;
    if(ui->checkBox_liquit_ctrl_en->isChecked())
    {
        is_check=1;
    }
    int posit = (int)(ui->lineEdit_liquit_ctrl_posit->text().toDouble()*1000);
    int range = (int)(ui->lineEdit_liquit_ctrl_range->text().toDouble()*1000);
    int step = ui->lineEdit_liquit_ctrl_step->text().toInt();
    int time = (int)(ui->lineEdit_liquit_ctrl_time->text().toDouble()*1000);
    int maxdisance = (int)(ui->lineEdit_liquit_max_disance->text().toDouble()*1000);
    
    emit main_set_liquit_auto_ctrl(is_check, posit, range, step, time,maxdisance);
}


void MainWindow::on_btn_gray_sub_clicked()
{
    int gray = ui->lineEdit_show_gray_pic->text().toInt();
    if(gray > 0) gray--;
    ui->lineEdit_show_gray_pic->setText(QString::number(gray));
}

void MainWindow::on_btn_gray_add_clicked()
{
    int gray = ui->lineEdit_show_gray_pic->text().toInt();
    if(gray < 255) gray++;
    ui->lineEdit_show_gray_pic->setText(QString::number(gray));
}

void MainWindow::on_btn_show_gray_pic_clicked()
{
    int gray = ui->lineEdit_show_gray_pic->text().toInt();
    if((gray < 0) || (gray > 255))
    {
        QMessageBox::warning(this, QString("Warning"),QString("灰度值输入不对，请重新输入！"));
    }
    else
    {
        emit main_show_gray_pic(gray);
    }
}

void MainWindow::on_btn_get_version_clicked()
{
    emit main_get_version();
}



void MainWindow::on_btn_print_motor_para_clicked()
{    
    int motor_step_z = ui->lineEdit_motor_step_z->text().toInt();
    int motor_step_knife = ui->lineEdit_motor_step_knife->text().toInt();
    int motor_knife_mode = ui->comboBox_knife_mode->currentIndex();
    int motor_z_return = ui->lineEdit_motor_z_return->text().toInt();
    int motor_z_start = ui->lineEdit_motor_z_start->text().toInt();

    emit main_set_print_motor_para(motor_step_z, motor_step_knife, motor_knife_mode, motor_z_return, motor_z_start);
}



void MainWindow::on_btn_dlp_power_on_off_clicked()
{
   // 开机
   if(m_tcp_connect == false ) return;
   
    if(m_dlp_power_on_time_cnt > 0)
    {
        return;
    }

    emit main_dlp_power_on_off(1);
    timer_id2 = startTimer(300);
    ui->progressBar_dlp_power_on->setValue(0);
    m_dlp_power_on_time_cnt = 0;
}


void MainWindow::on_btn_dlp_power_off_clicked()
{
    // 关机
    if(m_tcp_connect == false ) return;
    
    if(m_dlp_power_on_time_cnt != 0)
    {
        return;
    }

    emit main_dlp_power_on_off(0);
    timer_id2 = startTimer(300);
    ui->progressBar_dlp_power_on->setValue(100);
    m_dlp_power_on_time_cnt = 0;
}


void MainWindow::on_btn_motor_para_set_1_clicked()
{
    int current = ui->lineEdit_motor_current_1->text().toInt();
    int subdivision = ui->lineEdit_motor_subdivision_1->text().toInt();
    int start_speed = ui->lineEdit_motor_start_speed_1->text().toInt();
    int speed_up_time = ui->lineEdit_motor_speed_up_time_1->text().toInt();
    int speed_down_time = ui->lineEdit_motor_speed_down_time_1->text().toInt();
    int max_speed = ui->lineEdit_motor_max_speed_1->text().toInt();
    int return_speed = ui->lineEdit_motor_return_speed_1->text().toInt();
    int speed_after_zero = ui->lineEdit_motor_speed_after_zero_1->text().toInt();
    int return_up_down_time = ui->lineEdit_motor_up_down_time_after_zero_1->text().toInt();

    emit main_set_motor_para(0, current, subdivision, start_speed, speed_up_time, speed_down_time, max_speed, return_speed, speed_after_zero, return_up_down_time);
}

void MainWindow::on_btn_motor_para_set_2_clicked()
{
    int current = ui->lineEdit_motor_current_2->text().toInt();
    int subdivision = ui->lineEdit_motor_subdivision_2->text().toInt();
    int start_speed = ui->lineEdit_motor_start_speed_2->text().toInt();
    int speed_up_time = ui->lineEdit_motor_speed_up_time_2->text().toInt();
    int speed_down_time = ui->lineEdit_motor_speed_down_time_2->text().toInt();
    int max_speed = ui->lineEdit_motor_max_speed_2->text().toInt();
    int return_speed = ui->lineEdit_motor_return_speed_2->text().toInt();
    int speed_after_zero = ui->lineEdit_motor_speed_after_zero_2->text().toInt();
    int return_up_down_time = ui->lineEdit_motor_up_down_time_after_zero_2->text().toInt();

    emit main_set_motor_para(1, current, subdivision, start_speed, speed_up_time, speed_down_time, max_speed, return_speed, speed_after_zero, return_up_down_time);
}

void MainWindow::on_btn_motor_para_set_3_clicked()
{
    int current = ui->lineEdit_motor_current_3->text().toInt();
    int subdivision = ui->lineEdit_motor_subdivision_3->text().toInt();
    int start_speed = ui->lineEdit_motor_start_speed_3->text().toInt();
    int speed_up_time = ui->lineEdit_motor_speed_up_time_3->text().toInt();
    int speed_down_time = ui->lineEdit_motor_speed_down_time_3->text().toInt();
    int max_speed = ui->lineEdit_motor_max_speed_3->text().toInt();
    int return_speed = ui->lineEdit_motor_return_speed_3->text().toInt();
    int speed_after_zero = ui->lineEdit_motor_speed_after_zero_3->text().toInt();
    int return_up_down_time = ui->lineEdit_motor_up_down_time_after_zero_3->text().toInt();

    emit main_set_motor_para(2, current, subdivision, start_speed, speed_up_time, speed_down_time, max_speed, return_speed, speed_after_zero, return_up_down_time);
}

void MainWindow::on_btn_z_calc_clicked()
{
    //
    int kinessdata = ui->lineEdit_thinkness->text().toInt();  //
    int m_z_distance  = ui->lineEdit_motor_z_distance->text().toInt(); //
    int m_z_step = ui->lineEdit_motor_step->text().toInt(); //
    float tmp;
     //
    tmp = (float)m_z_distance / m_z_step;
    qDebug()<<"1step="<<tmp<<" mm";

    tmp = kinessdata / tmp;
    qDebug()<<"kiness="<<tmp<<" ";
    ui->lineEdit_motor_step_z->setText(QString("%1").arg((int)tmp));

}



void MainWindow::on_checkBox_priodreadliquitpos_clicked(bool checked)
{
    // 定时读取液位
    int tmp = ui->lineEdit_priod_readLuid->text().toInt();
    if(checked)
    {
          if(tmp < 1) tmp = 1;
          qDebug()<<"定时时间："<<tmp;
          timer_id4 = startTimer(tmp * 1000);
    }
    else
    {

        killTimer(timer_id4);
    }
}


/**************************************************
作者: DS小龙哥
环境: win10 QT5.12.6 VS2017 32位 Release
功能: 自适应工具栏按钮大小
**************************************************/
void MainWindow::AutoZoomButtonSize()
{
    //获取屏幕属性
    QScreen *screen = QGuiApplication::primaryScreen();

    //获取屏幕的分辨率
    int lcd_width = screen->size().width();
    int lcd_height = screen->size().height();
    qDebug() << "当前系统分辨率:" << screen->size();

    //计算横向纵向坐标的伸缩系统
    double factorx = lcd_width / 1920.0;  //1920 是开发电脑界面的分辨率
    double factory = lcd_height / 1080.0;

    //QList<QWidget*> widgets = this->findChildren<QWidget*>();

    //获取界面上所有的按钮
    QList<QToolButton*> buttonList = this->findChildren<QToolButton*>();

    //设置按钮大小
    for (int i = 0; i < buttonList.count(); i++)
    {
        qDebug() << "buttonList:" << buttonList.at(i)->objectName();
        ResetButtonGeometry(buttonList.at(i), factorx, factory);
    }

#if 0
    //设置工具栏的最大高度
    int ToolFrame_MaxH= lcd_height / 1080.0 * 40;
    qDebug() << "原工具栏的高度:" << ui.ToolFrame->height();
    qDebug() << "现工具栏的高度:" << ToolFrame_MaxH;
    ui.ToolFrame->setMaximumHeight(ToolFrame_MaxH);
    ui.ToolFrame->setMinimumHeight(ToolFrame_MaxH);
#endif
}


//重设按钮分辨率
void MainWindow::ResetButtonGeometry(QWidget *widget, double factorx, double factory)
{
    int oldX = widget->x();
    int oldY = widget->y();
    int oldW = widget->width();
    int oldH = widget->height();
    qDebug() << "widget->geometry()1:" << widget->geometry();
    widget->move(oldX*factorx, oldY*factory);
    widget->resize(oldW*factorx, oldH*factory);
    widget->setMaximumSize(oldW*factorx, oldH*factory);
    widget->setMinimumSize(oldW*factorx, oldH*factory);
    qDebug() << "widget->geometry()2:" << widget->geometry();
}

void MainWindow::on_Btn_ClearLog_clicked()
{
    ui->textEdit_message->setText("");
}

void MainWindow::on_btn_Z_clearalarmcode_clicked()
{
    // 
    int channel = 0;
    Show_Message("Z电机释放使能");
     emit main_motor_clearAlarmCode(channel);
}

void MainWindow::on_btn_knife_clearalarmcode_clicked()
{
    // 
    int channel = 1;
    Show_Message("刮刀电机释放使能");
     emit main_motor_clearAlarmCode(channel);
}

void MainWindow::on_btn_liqut_clearalarmcode_clicked()
{
    // 
    int channel = 2;
    Show_Message("液位释放使能");
     emit main_motor_clearAlarmCode(channel);
}




void MainWindow::on_horizontalSlider_led_current_valueChanged(int value)
{
    ui->lineEdit_led_current->setText(QString("%1").arg(value));

}

void MainWindow::on_lineEdit_led_current_textChanged(const QString &arg1)
{
    ui->horizontalSlider_led_current->setValue(arg1.toInt());

}

void MainWindow::on_horizontalSlider_led2_current_valueChanged(int value)
{
    ui->lineEdit_led2_current->setText(QString("%1").arg(value));
}

void MainWindow::on_lineEdit_led2_current_textChanged(const QString &arg1)
{
    ui->horizontalSlider_led2_current->setValue(arg1.toInt());
}


QString uncharToQstring(unsigned char * id,int len)
{
    QString temp,msg;
    int j = 0;

    while (j<len)
    {
        temp = QString(" %1").arg((int)id[j], 2, 16, QLatin1Char('0'));
        msg.append(temp);
        j++;
    }

    return msg;
}

