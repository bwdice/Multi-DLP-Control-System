#ifndef SLC_PROCESS_H
#define SLC_PROCESS_H

#include <QObject>
#include <QThread>

class slc_process : public QThread
{
    Q_OBJECT
public:
    explicit slc_process(QObject *parent = nullptr);
    ~slc_process();


    int FindString(char pSrc[], int srcSize, char pDest[], int dstSize);
    bool get_slice_info(QString filename, double *x_min, double *x_max, double *y_min, double *y_max, double *z_min, double *z_max, double *thinkness);
    bool slc_to_img(QString filename, QString img_fold, unsigned int dlp_x_pixel, unsigned int dlp_y_pixel, double thinkness);


    bool OpenSTLFile(QString filename);
    bool GetModelSize(double *x, double *y, double *z);
    bool SetSlicePara(double pixel_size);
    bool GetSlicePara(double *pixel_size, double *thinkness, int *slice_num);
    bool StartSclie();
    void StopSlice();
    void SetSliceNeed(int slice_need);


protected:
   void run() override;

public slots:

signals:
   void thread_slice_cnt(int);
   void thread_slice_stop();

private:
    QString currentLayout;
    volatile bool cancelslicing;

    double ModelsizeX;
    double ModelsizeY;
    double ModelsizeZ;
    volatile double m_model_x_min;
    volatile double m_model_x_max;
    volatile double m_model_y_min;
    volatile double m_model_y_max;
    volatile double m_model_z_min;
    volatile double m_model_z_max;
    volatile bool m_open_file;
    QString m_filename;
    volatile bool m_start_slice;
    volatile double m_pixel_size;
    volatile double m_thinkness;
    volatile int m_slice_num;
    volatile int m_slice_cnt;
    volatile int m_slice_need;
};

#endif // SLC_PROCESS_H
