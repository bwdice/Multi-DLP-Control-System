#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <stdio.h>
#include <string.h>
#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QTimer>



#define PACK_BYTE_NUM       64000
#define FRAME_HEAD          0xAAAA5555
#define FRAME_TAIL          0x5555AAAA


#define IP_PORT             5020

#define PIC_FRAME_DATA_NUM  63900

//消息类型
enum MsgType{
    Msg_Get_System_Para,
    Msg_Get_Version,
    Msg_Set_Print_Time,
    Msg_Set_Print_Motor_Para,
    Msg_Set_Motor_Run_Para,
    Msg_Set_Slice_Para,
    Msg_Set_DLP_Para,
    Msg_Set_Connect_Pixel,
    Msg_Start_Send_SLC_File,
    Msg_Send_SLC_File,
    Msg_Stop_Send_SLC_File,
    Msg_Ret_SLC_Filename_List,
    Msg_Open_SLC_File,
    Msg_Start_Print,
    Msg_Stop_Print,
    Msg_Pause_Print,
    Msg_Continue_Print,
    Msg_Ret_Printing_num,
    Msg_Ret_Pringing_Image,
    Msg_Print_Finish,
    Msg_Bright_Test,
    Msg_Sharpness_Test,
    Msg_Homogeneity_Test_After_Connect,
    Msg_Show_Gray_Pic,
    Msg_Motro_Ctrl,
    Msg_Motor_Reset,
    Msg_Get_Liquid_Sensor_Data,
    Msg_DLP_Power_Ctrl,
    Msg_Ret_Error,
    Msg_Delete_SLC_File,
    Msg_Stop_Test,
    Msg_Lituit_Auto_Ctrl,
    Msg_Nop
};


enum ErrorCode{
    Error_OK,
    Error_Pic_No_Enough,
    Error_
};





typedef struct PRINTINT_TIME_PARA_t
{
    quint32 				stop_time;				// 停止时间
    quint32 				bottom_layer_num;		// 底层层数
    quint32					bottom_time;			// 底层打印时间
    quint32 				printing_time;			// 其它层打印时间
}PRINTINT_TIME_PARA;

typedef struct SLICE_PARA_t
{
    quint32 				printing_size;		// 打印区域水平大小
    quint32					rate_persent;		// 打印缩放比例
    quint32					connect_pixel;		// 拼接像素
}SLICE_PARA;


typedef struct MOTOR_RUN_PARA_t
{
    quint16 current; 					// 电机电流
    quint16 subdivision; 				// 电机细分
    quint16 start_speed; 				// 电机启动速度
    quint16 speed_up_time; 				// 电机加速时间
    quint16 speed_down_time; 			// 电机减速时间
    quint16 running_speed;				// 电机最大速度
    quint16 return_speed;				// 电机回原点运行速度
    quint16 return_speed_after_zero;	// 找到零点后运行速度
    quint16 return_speed_up_down_time;	// 电机回原点加减速时间
    quint16 return_foward_offset;		// 回原点后正向补偿
    quint16 return_reverse_offset;		// 回原点后反向补偿
}MOTOR_RUN_PARA;

typedef struct MOTOR_PRINT_PARA_t
{
    quint32 z_step; 							// z轴步长
    quint32 knife_step; 						// 刮刀步长
    quint32 knife_mode;                         // 刮刀模式，单次或来回
    quint32 z_return;                           // z轴返回
    quint32 z_start;                            // z轴起始打印点
}MOTOR_PRINT_PARA;


typedef struct DLP_PARA_t
{
    quint32 				dlp1_current;			// dlp1电流
    quint32 				dlp2_current;			// dlp2电流
    quint32 				light_source_type;		// 光源类型
}DLP_PARA;


typedef struct LIQUIT_AUTO_CTRL_t
{
    quint32 				is_enable;              // 是否开启自动控制
    quint32 				posit;                  // 自动控制液位位置
    quint32                 range;                  // 自动控制范围
    quint32                 step;                   // 电机步长
    quint32                 time;                   // 自动控制时间间隔
}LIQUIT_AUTO_CTRL;

Q_DECLARE_METATYPE(LIQUIT_AUTO_CTRL)


typedef struct SYS_PARA_t
{
    PRINTINT_TIME_PARA      printint_time;
    SLICE_PARA              slice;
    MOTOR_RUN_PARA          motor_run[3];
    MOTOR_PRINT_PARA        motor_print;
    LIQUIT_AUTO_CTRL        liquit_ctrl;
    DLP_PARA                dlp;
}SYS_PARA;

Q_DECLARE_METATYPE(SYS_PARA)


typedef struct SLC_FILE_LIST_t
{
    quint32                 file_num;
    quint8                  filename_buff[100*256];
}SLC_FILE_LIST;

Q_DECLARE_METATYPE(SLC_FILE_LIST)


typedef struct PRINTING_IMAGE_t
{
    quint32                 image_w;
    quint32                 image_h;
    quint8                  buff[PACK_BYTE_NUM];
}PRINTING_IMAGE;

Q_DECLARE_METATYPE(PRINTING_IMAGE)




typedef struct TCP_DATA_FRAME_t
{
    quint32     head;
    MsgType     msg;
    quint8      pack_data[PACK_BYTE_NUM];
    ErrorCode   err;
    quint32     tail;
} TCP_DATA_FRAME;

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient();

    void timerEvent(QTimerEvent* ev);

    void Tcp_System_Para_Process();
    int Tcp_Send_File_Start();
    int Tcp_Send_File();
    int Tcp_Send_File_Stop();
    void Tcp_Printing_Image_Process();


private slots:
    void onDisConnected();
    void onConnected();
    void tcpReady();
    void tcpError(QAbstractSocket::SocketError);

signals:
    void thread_send_message(QString str);
    void thread_tcp_connect(bool flag, QString ip_addr);
    void thread_print_time_result(bool);
    void thread_device_printing();
    void thread_device_pause();
    void thread_device_continue();
    void thread_device_stop_print();
    void thread_print_pic_num(int);
    void thread_print_finish();
    void thread_connect_pixel(int);
    void thread_get_system_para(SYS_PARA);
    void thread_liquid_sensor_ret(int, int);
    void thread_motor_ctrl_ret();
    void thread_motor_reset_ret(int);
    void thread_send_file_persent(int);
    void thread_slc_file_list_reflash(SLC_FILE_LIST);
    void thread_slc_model_size(int, int, int, int, int, int);
    void thread_software_version(QString);
    void thread_printing_image(PRINTING_IMAGE);



public slots:
    void thread_start_find_device(QString ip);
    void thread_file_path(QString image_fold);
    void thread_set_print_time(int print_time, int stop_time, int print_time_bottom, int bottom_layer_num);
    void thread_set_dlp_size(int dlp_size, int rate_persent);
    void thread_start_print();
    void thread_stop_print();
    void thread_pause_print();
    void thread_continue_print();
    void thread_set_connect_pixel(int connect_pixel);
    void thread_bright_test(void);
    void thread_sharpness_test(void);
    void thread_homogeneity_test(int image_index);
    void thread_stop_test();
    void thread_motor_ctrl(int channel, int direct, int step);
    void thread_motor_reset(int channel);
    void thread_get_liquid_sensor();
    void thread_dlp_current_set(int light_type, int current1, int current2);
    void thread_show_gray_pic(int gray);
    void thread_get_version();
    void thread_set_print_motor_para(int motor_step_z, int motor_step_knife, int motor_knife_mode, int motor_z_return, int motor_z_start);
    void thread_dlp_power_on_off(int power_on_off);
    void thread_set_motor_para(int index, int current, int subdivision, int start_speed, int speed_up_time, int speed_down_time, int max_speed, int return_speed, int speed_after_zero, int return_up_down_time);
    void thread_open_slc_file(QString filename);
    void thread_delete_slc_file(QString filename);
    void thread_set_liquit_auto_ctrl(int is_check, int posit, int range, int step, int time);

private:
    //public
    //show
    void Show_Message(QString str);
    int                 timer_id1;
    int                 timer_id2;
    int                 timer_id3;
    int                 timer_id4;

    //tcp
    int Tcp_Send_Frame(MsgType type, quint8 *buff, int len, ErrorCode err);
    int Tcp_Send_Cmd(MsgType type, quint8 *buff, int len);
    int Tcp_Recv_Process();
    void Send_Tcp_Nop_Pack(void);
    QTcpSocket          * m_tcpSocket;
    bool                m_tcp_connect;
    TCP_DATA_FRAME      *m_pTcpFrame;
    TCP_DATA_FRAME      *m_pTcpRecvFrame;
    qint32              m_tcpRecvCnt;
    unsigned char 		*m_pTcpBuff;
    unsigned char       *m_pTcpFrameChar;
    unsigned char       *m_pTcpRecvFrameChar;

    //File
    QFile               *m_localFile;     // 要发送的文件
    qint32              m_totalBytes;    // 发送数据的总大小
    qint32              m_bytesWritten;  // 已经发送数据大小
    qint32              m_bytesToWrite;  // 剩余数据大小
    qint32              m_payloadSize;   // 每次发送数据的大小
    QString             m_slc_filename;
    bool                m_bOpenFile;
    qint32              m_file_length;
    QByteArray          m_outBlock;  // 数据缓冲区，即存放每次要发送的数据块

};

#endif // TCPCLIENT_H
