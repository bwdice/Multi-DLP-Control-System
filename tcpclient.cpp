#include "tcpclient.h"
#include <QDataStream>
#include <QDir>
#include <QDesktopServices>
#include <QTcpSocket>
#include <QFileInfo>
#include <QNetworkInterface>
#include <QTimerEvent>
#include <QImage>
#include <QThread>
#include <QApplication>


#pragma execution_character_set("utf-8")

TcpClient::TcpClient(QObject *parent) : QObject(parent)
{
    m_tcp_connect = false;
    m_tcpSocket = new QTcpSocket(this);

    connect( m_tcpSocket, SIGNAL(disconnected()), this, SLOT(onDisConnected()) );
    connect( m_tcpSocket, SIGNAL(connected()), this, SLOT(onConnected()) );
    connect( m_tcpSocket, SIGNAL(readyRead()), this, SLOT(tcpReady()) );
    connect( m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpError(QAbstractSocket::SocketError)) );

    //m_pTcpFrame = new TCP_DATA_FRAME();
    //m_pTcpRecvFrame = new TCP_DATA_FRAME();
    m_pTcpFrameChar = new unsigned char [PACK_BYTE_NUM+100];
    m_pTcpRecvFrameChar = new unsigned char [PACK_BYTE_NUM+100];
    m_pTcpFrame = (TCP_DATA_FRAME *)m_pTcpFrameChar;
    m_pTcpRecvFrame = (TCP_DATA_FRAME *)m_pTcpRecvFrameChar;
    m_pTcpBuff = new unsigned char [PACK_BYTE_NUM+100];
    m_tcpRecvCnt = 0;


    //file
    m_bOpenFile = false;

    //timer_id2 = startTimer(1000);
    //timer_id3 = startTimer(1000);
}


TcpClient::~TcpClient()
{
    if(m_tcp_connect)
    {
        m_tcpSocket->abort();
        m_tcpSocket->close();
    }

    //delete m_pTcpFrame;
    //delete m_pTcpRecvFrame;
    delete m_pTcpFrameChar;
    delete m_pTcpRecvFrameChar;
    delete m_pTcpBuff;
}

void TcpClient::Show_Message(QString str)
{
    emit thread_send_message(str);
}



void TcpClient::timerEvent(QTimerEvent* ev)
{

    //定时事件
    if(ev->timerId() == timer_id1)      //tcp连接设备
    {

    }
    else if(ev->timerId() == timer_id2)
    {

    }
    else if(ev->timerId() == timer_id3)
    {

    }
    else if(ev->timerId() == timer_id4)
    {

    }
}


//*****************************************************************************************************************
//
//                      TCP receive
//
//***************************************************************************************************************
void TcpClient::onConnected()
{
    m_tcp_connect = true;

    QString ip_addr;
    emit thread_tcp_connect(true, ip_addr);
}

void TcpClient::onDisConnected()
{
    m_tcp_connect = false;
    QString ip_addr;
    emit thread_tcp_connect(false, ip_addr);
}


void TcpClient::tcpError(QAbstractSocket::SocketError)
{

    m_tcp_connect = false;
    QString ip_addr;
    emit thread_tcp_connect(false, ip_addr);
}

void TcpClient::tcpReady()
{
    qint64 readBytes = m_tcpSocket->bytesAvailable();
    int frame_len = sizeof(TCP_DATA_FRAME);

    //Show_Message(QString("recv len: %1").arg(readBytes));

    qint64 max_recv_len;
    if((m_tcpRecvCnt + readBytes) <= frame_len)
    {
        max_recv_len = readBytes;
    }
    else
    {
        max_recv_len = frame_len - m_tcpRecvCnt;
    }

    //QByteArray data = m_tcpSocket->read(readBytes);
    qint64 recv_byte = m_tcpSocket->read((char *)m_pTcpBuff, max_recv_len);
    qint32 save_cnt = 0;
    save_cnt = recv_byte;

    if((m_tcpRecvCnt + save_cnt) <= frame_len)
    {
        memcpy((void *)(m_pTcpRecvFrameChar + m_tcpRecvCnt), (void *)(m_pTcpBuff), save_cnt);
        m_tcpRecvCnt += save_cnt;

        if(m_tcpRecvCnt >= frame_len)
        {
            m_tcpRecvCnt = 0;
            if((m_pTcpRecvFrame->head == FRAME_HEAD) && (m_pTcpRecvFrame->tail == FRAME_TAIL))
            {
                Tcp_Recv_Process();
            }
            else
            {
                Show_Message("TCP接收到错误数据帧");
            }
        }
    }
    else {
        Show_Message("数据帧长度超出");
    }
}

int TcpClient::Tcp_Recv_Process()
{
    QString str;
    QString str_version;


    switch(m_pTcpRecvFrame->msg)
    {
        case Msg_Get_System_Para:
            Tcp_System_Para_Process();
            break;
        case Msg_Get_Version:
            char version[100];
            memcpy(version, m_pTcpRecvFrame->pack_data, 100);
            str_version = QString::fromUtf8(version);
            Show_Message(QString("软件版本：") + str_version);
            emit thread_software_version(str_version);
            break;
        case Msg_Set_Print_Time:
            emit thread_print_time_result(true);
            break;
        case Msg_Set_Print_Motor_Para:
            break;
        case Msg_Set_Motor_Run_Para:
            break;
        case Msg_Set_Slice_Para:
            break;
        case Msg_Set_DLP_Para:
            break;
        case Msg_Set_Connect_Pixel:
            uint32_t connect_pixel;
            memcpy(&connect_pixel, m_pTcpRecvFrame->pack_data, 4);
            emit thread_connect_pixel((int)connect_pixel);
            break;
        case Msg_Start_Send_SLC_File:
            break;
        case Msg_Send_SLC_File:
            break;
        case Msg_Stop_Send_SLC_File:
            break;
        case Msg_Ret_SLC_Filename_List:
            SLC_FILE_LIST file_list;
            quint32 file_num;
            memcpy(&file_num,m_pTcpRecvFrame->pack_data, 4);
            file_list.file_num = file_num;
            memcpy(file_list.filename_buff,m_pTcpRecvFrame->pack_data+4, file_num*256);
            emit thread_slc_file_list_reflash(file_list);
            break;
        case Msg_Open_SLC_File:
            quint32 status, x, y, z, thinkness, slice_num;
            memcpy(&status,m_pTcpRecvFrame->pack_data, 4);
            memcpy(&x,m_pTcpRecvFrame->pack_data+4, 4);
            memcpy(&y,m_pTcpRecvFrame->pack_data+8, 4);
            memcpy(&z,m_pTcpRecvFrame->pack_data+12, 4);
            memcpy(&thinkness,m_pTcpRecvFrame->pack_data+16, 4);
            memcpy(&slice_num,m_pTcpRecvFrame->pack_data+20, 4);
            emit thread_slc_model_size(status, x, y, z, thinkness, slice_num);
            break;
        case Msg_Start_Print:
            emit thread_device_printing();
            break;
        case Msg_Stop_Print:
            emit thread_device_stop_print();
            break;
        case Msg_Pause_Print:
            emit thread_device_pause();
            break;
        case Msg_Continue_Print:
            emit thread_device_continue();
            break;
        case Msg_Ret_Printing_num:
            uint32_t printing_pic_num;
            memcpy(&printing_pic_num,m_pTcpRecvFrame->pack_data, 4);
            emit thread_print_pic_num((int)printing_pic_num);
            //str = QString("正在打印第 %1 张图片").arg(printing_pic_num);
            //Show_Message(str);
            break;
        case Msg_Ret_Pringing_Image:
            Tcp_Printing_Image_Process();
            break;
        case Msg_Print_Finish:
            emit thread_print_finish();
            break;
        case Msg_Bright_Test:
            break;
        case Msg_Sharpness_Test:
            break;
        case Msg_Homogeneity_Test_After_Connect:
            break;
        case Msg_Show_Gray_Pic:
            break;
        case Msg_Motro_Ctrl:
            emit thread_motor_ctrl_ret();
            break;
        case Msg_Motor_Reset:
            uint32_t motor_channel;
            memcpy(&motor_channel, m_pTcpRecvFrame->pack_data, 4);
            emit thread_motor_reset_ret((int)motor_channel);
            break;
        case Msg_Get_Liquid_Sensor_Data:
            uint32_t success;
            uint32_t liquid_sensor;
            memcpy(&success,m_pTcpRecvFrame->pack_data, 4);
            memcpy(&liquid_sensor,m_pTcpRecvFrame->pack_data+4, 4);
            emit thread_liquid_sensor_ret((int)success, (int)liquid_sensor);
            break;
        case Msg_DLP_Power_Ctrl:
            break;
        case Msg_Recv_MotoInfo:
            //Show_Message("recv motor data");
            emit thread_get_motorinfor(m_pTcpRecvFrame->pack_data,132);
        break;
        case Msg_Clear_AlarmCode:
        {
            uint32_t motor_channel;
            Show_Message("clear Motor Alarm code");
            memcpy(&motor_channel, m_pTcpRecvFrame->pack_data, 4);
            emit thread_motor_clearAlarmCode_ret((int)motor_channel);
        }
        break;
        case Msg_Ret_Error:
             break;
        case Msg_Nop:
            break;
        default:
            Show_Message("上位机接收未知命令");
            break;
    }

    return 0;
}



void TcpClient::Tcp_System_Para_Process()
{
    SYS_PARA para;
    int index = 0;

    memcpy(&para.printint_time.stop_time, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.printint_time.bottom_layer_num, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.printint_time.bottom_time, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.printint_time.printing_time, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;

    memcpy(&para.slice.printing_size, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.slice.rate_persent, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.slice.connect_pixel, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;

    memcpy(&para.motor_print.z_step, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.motor_print.knife_step, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.motor_print.knife_mode, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.motor_print.z_return, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.motor_print.z_start, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;

    for(int i=0;i<3;i++)
    {
        memcpy(&para.motor_run[i].current, m_pTcpRecvFrame->pack_data+index, 2);
        index += 2;
        memcpy(&para.motor_run[i].subdivision, m_pTcpRecvFrame->pack_data+index, 2);
        index += 2;
        memcpy(&para.motor_run[i].start_speed, m_pTcpRecvFrame->pack_data+index, 2);
        index += 2;
        memcpy(&para.motor_run[i].speed_up_time, m_pTcpRecvFrame->pack_data+index, 2);
        index += 2;
        memcpy(&para.motor_run[i].speed_down_time, m_pTcpRecvFrame->pack_data+index, 2);
        index += 2;
        memcpy(&para.motor_run[i].running_speed, m_pTcpRecvFrame->pack_data+index, 2);
        index += 2;
        memcpy(&para.motor_run[i].return_speed, m_pTcpRecvFrame->pack_data+index, 2);
        index += 2;
        memcpy(&para.motor_run[i].return_speed_after_zero, m_pTcpRecvFrame->pack_data+index, 2);
        index += 2;
        memcpy(&para.motor_run[i].return_speed_up_down_time, m_pTcpRecvFrame->pack_data+index, 2);
        index += 2;
        memcpy(&para.motor_run[i].return_foward_offset, m_pTcpRecvFrame->pack_data+index, 2);
        index += 2;
        memcpy(&para.motor_run[i].return_reverse_offset, m_pTcpRecvFrame->pack_data+index, 2);
        index += 2;
    }


    memcpy(&para.liquit_ctrl.is_enable, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.liquit_ctrl.posit, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.liquit_ctrl.range, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.liquit_ctrl.step, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.liquit_ctrl.time, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;



    memcpy(&para.dlp.dlp1_current, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.dlp.dlp2_current, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;
    memcpy(&para.dlp.light_source_type, m_pTcpRecvFrame->pack_data+index, 4);
    index += 4;

    emit thread_get_system_para(para);
    Show_Message(QString("接收到系统参数"));
}




int TcpClient::Tcp_Send_File_Start()
{
    emit thread_send_file_persent(0);
    QCoreApplication::processEvents(); // 处理事件

    if(!m_bOpenFile)
    {
        Show_Message(QString("文件未打开"));
        return -1;
    }

    QString fn = m_slc_filename.right(m_slc_filename.length()-m_slc_filename.lastIndexOf('/')-1);

    QFileInfo fileinfo(m_slc_filename);
    m_file_length = fileinfo.size();


    quint8 buff[300];
    memset(buff, 0, 300);
    memcpy((void *)(buff), (void *)(&m_file_length), 4);
    strcpy((char *)(buff+4),fn.toStdString().c_str());
    Tcp_Send_Cmd(Msg_Start_Send_SLC_File, buff, 300);

    return 0;
}


int TcpClient::Tcp_Send_File()
{
    int ret = 0;

    QFile file(m_slc_filename);
    bool ok = file.open(QIODevice::ReadOnly);
    if(ok==false)
    {
        Show_Message(QString("打开文件失败"));
        return -2;
    }

    quint8 buff[PACK_BYTE_NUM];
    qint32 send_cnt = 0;
    qint32 read_len = 0;
    qint32 send_persent =0;
    while(1)
    {
        read_len = file.read((char *)buff, PACK_BYTE_NUM);
        if(read_len == 0)
        {
            break;
        }
        else if(read_len < 0)
        {
            Show_Message(QString("打开读取失败"));
            file.close();
            return -3;
        }
        Tcp_Send_Cmd(Msg_Send_SLC_File, buff, read_len);
        send_cnt += read_len;
        double lf_send_cnt = (double)send_cnt;
        double lf_file_len = (double)m_file_length;
        if((int)(lf_send_cnt*100/lf_file_len) > send_persent)
        {
            send_persent = (int)(lf_send_cnt*100/lf_file_len);
            emit thread_send_file_persent(send_persent);
            QCoreApplication::processEvents(); // 处理事件
        }
        if(send_cnt >= m_file_length)
        {
            emit thread_send_file_persent(100);
            file.close();
            //Send_Tcp_Nop_Pack();
            Show_Message(QString("文件发送完成"));
            QThread::msleep(10);
            return 0;
        }
        QThread::msleep(1);
    }
    if(send_cnt >= m_file_length)
    {
        emit thread_send_file_persent(100);
        file.close();
        //Send_Tcp_Nop_Pack();
        Show_Message(QString("文件发送完成"));
        QThread::msleep(10);
        return 0;
    }
    else
    {
        emit thread_send_file_persent(0);
        file.close();
        Show_Message(QString("文件发送失败"));
        QThread::msleep(10);
        return -4;
    }

    return ret;
}


int TcpClient::Tcp_Send_File_Stop()
{
    Tcp_Send_Cmd(Msg_Stop_Send_SLC_File, nullptr, 0);

    return 0;
}


void TcpClient::Tcp_Printing_Image_Process()
{
    PRINTING_IMAGE image;

    int width, height;
    memcpy(&width, m_pTcpRecvFrame->pack_data, 4);
    memcpy(&height, m_pTcpRecvFrame->pack_data+4, 4);
    image.image_w = width;
    image.image_h = height;
    memcpy(image.buff, m_pTcpRecvFrame->pack_data+8, PACK_BYTE_NUM-8);

    emit thread_printing_image(image);
    Show_Message("TCP收到打印图像");
}



//*****************************************************************************************************************
//
//                      TCP send
//
//***************************************************************************************************************
int TcpClient::Tcp_Send_Frame(MsgType type, quint8 *buff, int len, ErrorCode err)
{
    int ret = 0;

    m_pTcpFrame->head = FRAME_HEAD;
    m_pTcpFrame->tail = FRAME_TAIL;
    m_pTcpFrame->msg = type;
    m_pTcpFrame->err = err;
    memset((void *)(m_pTcpFrame->pack_data), 0, PACK_BYTE_NUM);
    if(len > 0)
    {
        memcpy((void *)(m_pTcpFrame->pack_data), (void *)(buff), len);
    }

    int frame_len = sizeof(TCP_DATA_FRAME);
    QByteArray array((char *)m_pTcpFrame, frame_len);

    ret = m_tcpSocket->write(array);

    if(!m_tcpSocket->waitForBytesWritten(2*1000))
    {
        ret = -1;
    }

    array.resize(0);

    return ret;
}


int TcpClient::Tcp_Send_Cmd(MsgType type, quint8 *buff, int len)
{
    int ret = 0;

    ret = Tcp_Send_Frame(type, buff, len, Error_OK);

    return ret;
}



void TcpClient::Send_Tcp_Nop_Pack(void)
{
    Tcp_Send_Cmd(Msg_Nop, nullptr, 0);
}


//*****************************************************************************************************************
//
//                      slot process
//
//***************************************************************************************************************
void TcpClient::thread_start_find_device(QString ip)
{
    m_tcpSocket->abort();       //取消原有连接
    m_tcp_connect = false;

    m_tcpSocket->connectToHost(ip, IP_PORT);
}



void TcpClient::thread_file_path(QString image_fold)
{
    m_slc_filename = image_fold;
    m_bOpenFile = true;

    int ret = 0;
    ret = Tcp_Send_File_Start();
    if(ret == 0)
    {
        Tcp_Send_File();
        Tcp_Send_File_Stop();
    }
}

void TcpClient::thread_set_print_time(int print_time, int stop_time, int print_time_bottom, int bottom_layer_num)
{
    quint8 buff[100];
    int ret = 0;

    Show_Message(QString("设置时间: %1, %1").arg(print_time).arg(stop_time));

    memcpy((void *)(buff), (void *)(&print_time), 4);
    memcpy((void *)(buff+4), (void *)(&stop_time), 4);
    memcpy((void *)(buff+8), (void *)(&print_time_bottom), 4);
    memcpy((void *)(buff+12), (void *)(&bottom_layer_num), 4);
    ret = Tcp_Send_Cmd(Msg_Set_Print_Time, buff, 16);


    if(ret < 0)
    {
        emit thread_print_time_result(false);
    }
}

void TcpClient::thread_set_dlp_size(int dlp_size, int rate_persent)
{
    quint8 buff[100];

    memcpy((void *)(buff), (void *)(&dlp_size), 4);
    memcpy((void *)(buff+4), (void *)(&rate_persent), 4);
    Tcp_Send_Cmd(Msg_Set_Slice_Para, buff, 8);
}


void TcpClient::thread_start_print()
{
    Tcp_Send_Cmd(Msg_Start_Print, nullptr, 0);
}

void TcpClient::thread_stop_print()
{
    Tcp_Send_Cmd(Msg_Stop_Print, nullptr, 0);
}

void TcpClient::thread_pause_print()
{
    Tcp_Send_Cmd(Msg_Pause_Print, nullptr, 0);
}

void TcpClient::thread_continue_print()
{
    Tcp_Send_Cmd(Msg_Continue_Print, nullptr, 0);
}


void TcpClient::thread_set_connect_pixel(int connect_pixel)
{
    quint8 buff[100];

    memcpy((void *)(buff), (void *)(&connect_pixel), 4);
    Tcp_Send_Cmd(Msg_Set_Connect_Pixel, buff, 4);
}


void TcpClient::thread_bright_test(void)
{
    Tcp_Send_Cmd(Msg_Bright_Test, nullptr, 0);
}

void TcpClient::thread_sharpness_test(void)
{
    Tcp_Send_Cmd(Msg_Sharpness_Test, nullptr, 0);
}

void TcpClient::thread_homogeneity_test(int image_index)
{
    quint8 buff[100];

    memcpy((void *)(buff), (void *)(&image_index), 4);
    Tcp_Send_Cmd(Msg_Homogeneity_Test_After_Connect, buff, 4);
}

void TcpClient::thread_stop_test()
{
    Tcp_Send_Cmd(Msg_Stop_Test, nullptr, 0);
}


void TcpClient::thread_motor_ctrl(int channel, int direct, int step)
{
    quint8 buff[100];

    memcpy((void *)(buff), (void *)(&channel), 4);
    memcpy((void *)(buff+4), (void *)(&direct), 4);
    memcpy((void *)(buff+8), (void *)(&step), 4);

    Tcp_Send_Cmd(Msg_Motro_Ctrl, buff, 12);
}


void TcpClient::thread_motor_reset(int channel)
{
    quint8 buff[100];

    memcpy((void *)(buff), (void *)(&channel), 4);

    Show_Message(QString("Tcp控制电机复位"));
    Tcp_Send_Cmd(Msg_Motor_Reset, buff, 4);
}

void TcpClient::thread_motor_clearAlarmCode(int channel)
{
    quint8 buff[100];

    memcpy((void *)(buff), (void *)(&channel), 4);

    Show_Message(QString("Tcp清除电机故障代码"));
    Tcp_Send_Cmd(Msg_Clear_AlarmCode, buff, 4);
}


void TcpClient::thread_get_liquid_sensor()
{
    Tcp_Send_Cmd(Msg_Get_Liquid_Sensor_Data, nullptr, 0);
}

void TcpClient::thread_set_liquit_auto_ctrl(int is_check, int posit, int range, int step, int time)
{
    quint8 buff[100];

    memcpy((void *)(buff), (void *)(&is_check), 4);
    memcpy((void *)(buff+4), (void *)(&posit), 4);
    memcpy((void *)(buff+8), (void *)(&range), 4);
    memcpy((void *)(buff+12), (void *)(&step), 4);
    memcpy((void *)(buff+16), (void *)(&time), 4);

    Tcp_Send_Cmd(Msg_Lituit_Auto_Ctrl, buff, 20);
}

void TcpClient::thread_dlp_current_set(int light_type, int current1, int current2)
{
    quint8 buff[100];


    memcpy((void *)(buff), (void *)(&current1), 4);
    memcpy((void *)(buff+4), (void *)(&current2), 4);
    memcpy((void *)(buff+8), (void *)(&light_type), 4);

    Tcp_Send_Cmd(Msg_Set_DLP_Para, buff, 12);
}


void TcpClient::thread_show_gray_pic(int gray)
{
    quint8 buff[100];

    memcpy((void *)(buff), (void *)(&gray), 4);

    Tcp_Send_Cmd(Msg_Show_Gray_Pic, buff, 4);
    Show_Message(QString("显示灰度图片:%1").arg(gray));
}


void TcpClient::thread_get_version()
{
    quint8 buff[100];

    Tcp_Send_Cmd(Msg_Get_Version, buff, 0);
}

void TcpClient::thread_set_print_motor_para(int motor_step_z, int motor_step_knife, int motor_knife_mode, int motor_z_return, int motor_z_start)
{
    quint8 buff[100];

    memcpy((void *)(buff), (void *)(&motor_step_z), 4);
    memcpy((void *)(buff+4), (void *)(&motor_step_knife), 4);
    memcpy((void *)(buff+8), (void *)(&motor_knife_mode), 4);
    memcpy((void *)(buff+12), (void *)(&motor_z_return), 4);
    memcpy((void *)(buff+16), (void *)(&motor_z_start), 4);

    Tcp_Send_Cmd(Msg_Set_Print_Motor_Para, buff, 20);
    Show_Message(QString("设置打印电机参数：%1, %2, %3, %4").arg(motor_step_z).arg(motor_step_knife).arg(motor_knife_mode).arg(motor_z_return).arg(motor_z_start));
}

void TcpClient::thread_dlp_power_on_off(int power_on_off)
{
    quint8 buff[100];

    memcpy((void *)(buff), (void *)(&power_on_off), 4);

    Tcp_Send_Cmd(Msg_DLP_Power_Ctrl, buff, 4);
    Show_Message(QString("DLP开关机控制: %1").arg(power_on_off));
}


void TcpClient::thread_set_motor_para(int index, int current, int subdivision, int start_speed, int speed_up_time, int speed_down_time, int max_speed, int return_speed, int speed_after_zero, int return_up_down_time)
{
    quint8 buff[100];

    quint16 index_16bit, current_16bit, subdivision_16bit, start_speed_16bit, speed_up_time_16bit, speed_down_time_16bit, max_speed_16bit, return_speed_16bit, speed_after_zero_16bit,return_up_down_time_16bit;
    quint16 foward_offset, reverse_offset;

    index_16bit                 = index;
    current_16bit               = current;
    subdivision_16bit           = subdivision;
    start_speed_16bit           = start_speed;
    speed_up_time_16bit         = speed_up_time;
    speed_down_time_16bit       = speed_down_time;
    max_speed_16bit             = max_speed;
    return_speed_16bit          = return_speed;
    speed_after_zero_16bit      = speed_after_zero;
    return_up_down_time_16bit   = return_up_down_time;
    foward_offset               = 0;
    reverse_offset              = 0;


    memcpy((void *)(buff), (void *)(&index_16bit), 2);
    memcpy((void *)(buff+2), (void *)(&current_16bit), 2);
    memcpy((void *)(buff+4), (void *)(&subdivision_16bit), 2);
    memcpy((void *)(buff+6), (void *)(&start_speed_16bit), 2);
    memcpy((void *)(buff+8), (void *)(&speed_up_time_16bit), 2);
    memcpy((void *)(buff+10), (void *)(&speed_down_time_16bit), 2);
    memcpy((void *)(buff+12), (void *)(&max_speed_16bit), 2);
    memcpy((void *)(buff+14), (void *)(&return_speed_16bit), 2);
    memcpy((void *)(buff+16), (void *)(&speed_after_zero_16bit), 2);
    memcpy((void *)(buff+18), (void *)(&return_up_down_time_16bit), 2);
    memcpy((void *)(buff+20), (void *)(&foward_offset), 2);
    memcpy((void *)(buff+22), (void *)(&reverse_offset), 2);

    Tcp_Send_Cmd(Msg_Set_Motor_Run_Para, buff, 24);
    Show_Message(QString("设置485电机参数：%1, %2, %3, %4, %5, %6, %7, %8, %9, %10").arg(index).arg(current).arg(subdivision).arg(start_speed).arg(speed_up_time).arg(speed_down_time).arg(max_speed).arg(return_speed).arg(speed_after_zero).arg(return_up_down_time));
}


void TcpClient::thread_open_slc_file(QString filename)
{
    quint8 buff[256];

    memset(buff, 0, 256);
    strcpy((char *)buff, filename.toStdString().c_str());

    Tcp_Send_Cmd(Msg_Open_SLC_File, buff, 256);
    Show_Message(QString("打开SLC文件: ")+filename);
}


void TcpClient::thread_delete_slc_file(QString filename)
{
    quint8 buff[256];

    memset(buff, 0, 256);
    strcpy((char *)buff, filename.toStdString().c_str());

    Tcp_Send_Cmd(Msg_Delete_SLC_File, buff, 256);
    Show_Message(QString("删除SLC文件: ")+filename);
}


