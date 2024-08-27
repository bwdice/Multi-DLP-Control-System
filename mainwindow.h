#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QTcpSocket>
#include <QFile>
#include <QThread>
#include "tcpclient.h"
#include <QImage>



// ���״̬
#define CCW_STATUS  0x8000   // CCW ״̬
#define CW_STATUS   0x4000   // CW״̬
#define RUNNING_STATUS 0x200 // ���б�־



#pragma pack(push)
#pragma pack(1)

typedef struct _Motor_info{
    unsigned char addr;
    unsigned char connectfailcnt;
	int connectflag;
	int status;
	int alalrmcode;
	int speed;
	int pos;
	unsigned char  productname[22];

}tMotor_info;

typedef union _UMOTOR_INFO{
    unsigned char motorinfo_buf[44];
    tMotor_info  motorinfo;
}uMotor_Info;
#pragma pack(pop)



class QCamera;
class QCameraViewfinder;
class QCameraImageCapture;


#define SLICE_TEST          1


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void timerEvent(QTimerEvent* ev);
    void closeEvent(QCloseEvent *ev);//�����رմ����¼�

    QThread *thread;
    TcpClient *tcpClient;

    //show
    void Show_Message(QString str);
    int                 timer_id1;
    int                 timer_id2;
    int                 timer_id3;
    int                 timer_id4;

    //tcp
    bool                m_tcp_connect;

    //dlp
    bool                m_dlp_power_on_off;
    int                 m_dlp_power_on_time_cnt;
    unsigned char dlp1_powerstate;
    unsigned char dlp2_powerstate;
    

    //File
    QFile               *m_localFile;     // Ҫ���͵��ļ�
    QString             m_fileName;     // �����ļ�·��
    bool                m_bOpenFile;
    bool                m_bOpenRK3588File;

    //printing
    bool                m_device_printing;
    bool                m_device_pause;
    int                 m_print_running_time;
    int                 m_print_leave_time;
    int                 m_printing_cnt;

    //para
    int                 m_print_pic_num;
    double              m_model_size_x;
    double              m_model_size_y;
    double              m_model_size_z;
    int                 m_printing_size;
    int                 m_rate_persent;
    double              m_stop_time;
    double              m_printing_time;
    double              m_bottom_time;
    int                 m_bottom_layer_num;


    //show image
    QImage              *m_show_image;

    uMotor_Info motor_paradata[3];  //�������
    bool allmotor_normal;  // ���е���Ƿ�������־
public:
   void AutoZoomButtonSize();
   void ResetButtonGeometry(QWidget *widget, double factorx, double factory);
   void main_poweroff_buttonenb(bool enb1,bool enb2);
private slots:
    void on_btn_find_device_clicked();
    void on_btn_download_file_clicked();
    void on_btn_download_stop_clicked();
    void on_btn_open_device_file_clicked();
    void on_btn_open_file_clicked();
    void on_btn_set_print_time_clicked();
    void on_btn_start_print_clicked();
    void on_btn_stop_print_clicked();
    void on_btn_set_slice_clicked();
    void on_btn_show_grating_clicked();

    //thread
    void main_show_message(QString str);
    void main_tcp_connect(bool flag, QString ip_addr);
    void main_print_time_result(bool flag);
    void main_device_printing();
    void main_device_pause();
    void main_device_continue();
    void main_device_stop_print();
    void main_print_pic_num(int pic_num);
    void main_print_finish();
    void main_connect_pixel(int connect_pixel);
    void main_get_system_para(SYS_PARA sys_para);
    void main_liquid_sensor_ret(int success, int liquid_sensor);
    void main_motor_ctrl_ret();
    void main_motor_reset_ret(int channel);
    void main_motor_clearAlarmCode_ret(int channel);
    void main_send_file_persent(int persent);
    void main_slc_file_list_reflash(SLC_FILE_LIST file_list);
    void main_slc_model_size(int status, int x, int y, int z, int thinkness, int slice_num);
    void main_software_version(QString version);
    void main_printing_image(PRINTING_IMAGE image_data);
    void main_get_motorinfor(unsigned char *pt,unsigned int len);


    void on_btn_homogeneity_test_clicked();
    void on_btn_brightness_test_clicked();
    void on_btn_sharpness_test_clicked();
    void on_btn_set_led_cnrrent_clicked();
    void on_btn_motor_ctrl_clicked();
    void on_btn_motor_reset_ctrl_clicked();
    void on_btn_get_liquid_sensor_clicked();
    void on_btn_show_gray_pic_clicked();
    void on_btn_get_version_clicked();
    void on_btn_gray_sub_clicked();
    void on_btn_gray_add_clicked();
    void on_btn_print_motor_para_clicked();
    void on_btn_pause_print_clicked();
    void on_btn_continue_print_clicked();
    void on_btn_motor_ctrl_2_clicked();
    void on_btn_motor_ctrl_3_clicked();
    void on_btn_motor_reset_ctrl_2_clicked();
    void on_btn_motor_reset_ctrl_3_clicked();
    void on_btn_dlp_power_on_off_clicked();
    void on_btn_motor_para_set_1_clicked();
    void on_btn_motor_para_set_2_clicked();
    void on_btn_motor_para_set_3_clicked();


    void on_btn_delete_device_file_clicked();

    void on_btn_dlp_power_off_clicked();

    void on_btn_z_calc_clicked();


	
	void on_btn_stop_test_clicked();
	
    void on_btn_set_liquit_ctrl_clicked();

    void on_comboBox_dlp_light_type_activated(int index);

    void on_checkBox_priodreadliquitpos_clicked(bool checked);

    void on_Btn_ClearLog_clicked();

    void on_btn_liqut_clearalarmcode_clicked();
    void on_btn_Z_clearalarmcode_clicked();
    void on_btn_knife_clearalarmcode_clicked();

    void on_horizontalSlider_led_current_valueChanged(int value);

    void on_lineEdit_led_current_textChanged(const QString &arg1);

    void on_horizontalSlider_led2_current_valueChanged(int value);

    void on_lineEdit_led2_current_textChanged(const QString &arg1);

signals:
    void main_start_find_device(QString);
    void main_file_path(QString);
    void main_set_print_time(int, int, int, int);
    void main_set_dlp_size(int, int);
    void main_start_print();
    void main_stop_print();
    void main_pause_print();
    void main_continue_print();
    void main_set_connect_pixel(int);
    void main_bright_test(void);
    void main_sharpness_test(void);
    void main_homogeneity_test(int);
     void main_stop_test();
    void main_motor_ctrl(int, int, int);
    void main_motor_reset(int);
    void main_get_liquid_sensor(void);
    void main_dlp_current_set(int, int, int);
    void main_show_gray_pic(int);
    void main_get_version();
    void main_set_print_motor_para(int, int, int, int, int);
    void main_dlp_power_on_off(int);
    void main_set_motor_para(int, int, int, int, int, int, int, int, int, int);
    void main_open_slc_file(QString);
    void main_delete_slc_file(QString);
    void main_set_liquit_auto_ctrl(int, int, int, int, int,int);
    void main_motor_clearAlarmCode(int);

private:
    QCamera *camera;
    QCameraViewfinder *viewfinder;
    QCameraImageCapture *imageCapture;


private:
    Ui::MainWindow *ui;
};


QString uncharToQstring(unsigned char * id,int len);

#endif // MAINWINDOW_H
