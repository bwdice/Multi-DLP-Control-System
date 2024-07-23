#include "slc_process.h"

#include <QSettings>
#include <QVector2D>
#include <QVector3D>
#include <QFileDialog>
#include <QMessageBox>
#include <QtCore/qmath.h>
#include <qpainter.h>
#include <QInputDialog>
#include <QCoreApplication>



//opencv
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <codecvt>
#include <locale>



#pragma execution_character_set("utf-8")


using namespace cv;
using namespace std;



std::wstring string_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}


slc_process::slc_process(QObject *parent)
{
    //slicing
    cancelslicing = false;


    ModelsizeX = 0;
    ModelsizeY = 0;
    ModelsizeZ = 0;
    m_model_x_min = 0;
    m_model_x_max = 0;
    m_model_y_min = 0;
    m_model_y_max = 0;
    m_model_z_min = 0;
    m_model_z_max = 0;
    m_open_file = false;
    m_start_slice = false;
    m_pixel_size = 300.0/3840;
    m_thinkness = 0.1;
    m_slice_num = 0;
    m_slice_cnt = 0;
    m_slice_need = 10;
}

slc_process::~slc_process()
{

}





int slc_process::FindString(char pSrc[], int srcSize, char pDest[], int dstSize)
{
    int iFind = -1;
    for(size_t i=0;i<(size_t)srcSize;i++){
        int iCnt = 0;
        for (size_t j=0; j<(size_t)dstSize; j++) {
            if(pDest[j] == pSrc[i+j])
                iCnt++;
        }
        if (iCnt==dstSize) {
            iFind = i;
            break;
        }
    }
    return iFind;//返回比对的字符起始位置
}


bool slc_process::get_slice_info(QString filename, double *x_min, double *x_max, double *y_min, double *y_max, double *z_min, double *z_max, double *thinkness)
{
    //************处理文件头的字符变量，解析完毕后，关闭当前文件*********************
    //关于文件的文件头变量
    char str_temp[300];
    float minx, maxx, miny, maxy, minz, maxz;


    string str_filename = filename.toStdString();
    const std::wstring wstr = string_to_wstring(str_filename);
    const wchar_t* wchar_ptr = wstr.c_str();
    const std::wstring flag_wstr = string_to_wstring("r");
    const wchar_t* flag_wchar_ptr = flag_wstr.c_str();
    FILE * fid = _wfopen(wchar_ptr,flag_wchar_ptr); //用于处理文件头的信息
    std::cout << str_filename << std::endl;
    if(fid == NULL)
    {
        std::cout << "open failed 1" << std::endl;

        return false;
    }

    //此处添加文件头处理方式
    fread(str_temp,sizeof(char),300,fid);
    //-EXTENTS <minx,maxx miny,maxy minz,maxz> CAD模型 x,y,z轴的范围
    int ssss = FindString(str_temp,300,(char *)"-EXTENTS",8);
    strncpy(str_temp,str_temp+ssss,100);//提取出XYZ的范围数据

    //提取XYZ的范围（前6个浮点值）
    char str[100];//无关变量
    sscanf(str_temp, "%[(A-Z)|^-]%f%[(^ )|(^,)]%f%[(^ )|(^,)]%f%[(^ )|(^,)]%f%[(^ )|(^,)]%f%[(^ )|(^,)]%f%[(A-Z)|^*]", str, &minx, str, &maxx, str, &miny, str, &maxy, str, &minz, str, &maxz, str);

    *x_min = minx;
    *x_max = maxx;
    *y_min = miny;
    *y_max = maxy;
    *z_min = minz;
    *z_max = maxz;

    fclose(fid);
    //********************************************************************************




    FILE *fd; 		//文件描述符
    char m_data;	//读取到的数据
    int size=0;		//读取到的数据长度


    const std::wstring flag_wstr_1 = string_to_wstring("rb");
    const wchar_t* flag_wchar_ptr_1 = flag_wstr_1.c_str();
    fd = _wfopen(wchar_ptr,flag_wchar_ptr_1); //用于处理文件头的信息
    //fd = fopen(c_filename, "rb");


    unsigned int i=0;
    unsigned int j=0;
    float   n_float;
    //**************************处理头文件部分**************************
    while(1)
    {
        i++;
        if(i==1024)
        {
            fclose(fd);
            std::cout << "open failed 2" << std::endl;
            return false;
        }

        size = fread(&m_data, 1, 1, fd);

        switch(m_data)
        {
            case 0x0d:
                j=1;
                break;
            case 0x0a:
                if(j==1)
                    j=2;
                break;
            case 0x1a:
                if(j==2)
                    j=3;
                break;
            default:
                j=0;
                break;
        }
        if(j==3)
            break;
    }

    //******************************************************************
    //***************************处理预留部分***************************
    for(i=0;i<256;i++)
    {
        size = fread(&m_data, 1, 1, fd);
    }
    //******************************************************************
    //**************************处理样本表部分**************************
    size = fread(&m_data, 1, 1, fd);
    while(m_data)
    {
        size = fread(&n_float, 4, 1, fd);//Minimum Z Level
        size = fread(&n_float, 4, 1, fd);//Layer Thickness

        *thinkness = n_float;

        size = fread(&n_float, 4, 1, fd);    //Line Width Compensation
        size = fread(&n_float, 4, 1, fd);    //Reserved
        m_data--;

        break;
    }

    fclose(fd);

    std::cout << "open ok" << std::endl;

    return true;
}

bool slc_process::slc_to_img(QString filename, QString img_fold, unsigned int dlp_x_pixel, unsigned int dlp_y_pixel, double thinkness)
{
    FILE *fd; 		//文件描述符
    char m_data;	//读取到的数据
    int size=0;		//读取到的数据长度
    unsigned int i=0;
    unsigned int j=0;
    float   n_float;


    string str_filename = filename.toStdString();
    const std::wstring wstr = string_to_wstring(str_filename);
    const wchar_t* wchar_ptr = wstr.c_str();
    const std::wstring flag_wstr = string_to_wstring("rb");
    const wchar_t* flag_wchar_ptr = flag_wstr.c_str();
    fd = _wfopen(wchar_ptr,flag_wchar_ptr); //用于处理文件头的信息
    //fd = fopen(c_filename, "rb");
    //**************************处理头文件部分**************************
    while(1)
    {
        i++;
        if(i==1024)
        {
            fclose(fd);
            return false;
        }

        size = fread(&m_data, 1, 1, fd);

        switch(m_data)
        {
            case 0x0d:
                j=1;
                break;
            case 0x0a:
                if(j==1)
                    j=2;
                break;
            case 0x1a:
                if(j==2)
                    j=3;
                break;
            default:
                j=0;
                break;
        }
        if(j==3)
            break;
    }

    //******************************************************************
    //***************************处理预留部分***************************
    for(i=0;i<256;i++)
    {
        size = fread(&m_data, 1, 1, fd);
    }
    //******************************************************************
    //**************************处理样本表部分**************************
    size = fread(&m_data, 1, 1, fd);
    while(m_data)
    {
        size = fread(&n_float, 4, 1, fd);//Minimum Z Level
        size = fread(&n_float, 4, 1, fd);//Layer Thickness

        size = fread(&n_float, 4, 1, fd);    //Line Width Compensation
        size = fread(&n_float, 4, 1, fd);    //Reserved
        m_data--;
    }


    //******************************************************************
    //*************************处理轮廓数据部分*************************
    float dlp_x_pixel_size = m_pixel_size;
    float dlp_y_pixel_size = m_pixel_size;
    int img_y_pixel = dlp_y_pixel;              //4320
    int img_x_pixel = dlp_x_pixel;              //3840
    float dlp_x_size = dlp_x_pixel_size*img_x_pixel;
    float dlp_y_size = dlp_y_pixel_size*img_y_pixel;
    float model_cent_x = (m_model_x_max + m_model_x_min)/2;
    float model_cent_y = (m_model_y_max + m_model_y_min)/2;

    Mat dst = Mat::zeros(img_y_pixel, img_x_pixel, CV_8UC1);//生成的图片，其分辨率由实际的FrameBuffer来决定
    CvScalar color=cvScalar(0);

    vector<Point> contour;       	 //单个轮廓坐标值
    vector<vector<Point>> v_contour; //当前层所有轮廓集合
    vector<int> flag_swap_vector;	//轮廓排序用
    vector<vector<Point>> vctint;	//轮廓排序用
    float flag_swap=0;				//轮廓排序用
    unsigned int n_boundary,n_vertices,n_gaps;
    float   n_layer;
    float   n_polylineX,n_polylineY;
    float d;


    m_slice_cnt = 0;
    while(1)
    {
        size = fread(&n_layer, 4, 1, fd);                                                                       //Z轴高度
        size = fread(&n_boundary, 4, 1, fd);                                                                    //轮廓数

        if(n_boundary==0xFFFFFFFF)  //结束符
            break;

        for(i=0;i<n_boundary;i++)   //把同一层多个轮廓都放在同一容器中，
        {                           //显示跟数据处理时 要根据起始点和同轮廓的终点相等来判断是否为同一轮廓
            size = fread(&n_vertices, 4, 1, fd);                                                                //一个轮廓环中的点数
            size = fread(&n_gaps, 4, 1, fd);                                                                    //一个轮廓间隙数
            contour.clear();//删除容器中的所有元素
            for(j=0;j<n_vertices;j++)   //一个轮廓点处理
            {
                size = fread(&n_polylineX, 4, 1, fd);                                                           //轮廓点X坐标
                size = fread(&n_polylineY, 4, 1, fd);                                                           //轮廓点Y坐标
                n_polylineX -= model_cent_x;
                n_polylineY -= model_cent_y;

                contour.push_back(Point((long)((n_polylineX+dlp_x_size/2)/dlp_x_pixel_size),(long)((n_polylineY+dlp_y_size/2)/dlp_y_pixel_size))); //向轮廓坐标尾部添加点坐标
            }

            v_contour.push_back(contour);//追加当前轮廓数据到当前层容器变量中
            contour.clear();//删除容器中的所有元素
        }

        //通过冒泡法实现容器中轮廓的排序，使得较小轮廓始终位于较大轮廓后，能够判断是否出现交叉异常(注：两个分离的轮廓也会进行排序，不影响填充）
        int n; //需要排序的轮廓个数
        n=v_contour.size();//获取轮廓的个数

        for(size_t cmpnum = n-1; cmpnum != 0; --cmpnum)
        {
            for (size_t i = 0; i != cmpnum; ++i)
            {
                for(size_t k=0;k<v_contour[i+1].size();k++)
                {

                    flag_swap=pointPolygonTest(v_contour[i], v_contour[i+1][k], false); // 对于每个点都去检测
                    flag_swap_vector.push_back(flag_swap);
                }

                for(size_t z=0;z<flag_swap_vector.size()-1;z++)
                {
                    if(flag_swap_vector[z]!=flag_swap_vector[z+1])
                    {
                        //有存在交叉现象
                        //这里应该去做相应的异常处理
                    }
                }

                flag_swap_vector.clear();//删除容器中的所有元素

                if (flag_swap == -1)
                {
                    swap(v_contour[i],v_contour[i+1]);
                }
            }
        }

        //清除图像
        dst.setTo(Scalar(0));//把像素点值清零
        for(i=0;i<n_boundary;i++)   //把同一层多个轮廓都放在同一容器中，
        {                           //显示跟数据处理时 要根据起始点和同轮廓的终点相等来判断是否为同一轮廓
            d = 0;
            for (size_t j = 0; j < v_contour[i].size()-1; j++)
            {
                d += -0.5*(v_contour[i][j+1].y+v_contour[i][j].y)*(v_contour[i][j+1].x-v_contour[i][j].x);
            }

            // a) 存放单通道图像中像素：cvScalar(255);
            // b) 存放三通道图像中像素：cvScalar(255,255,255);
            if(d > 0)
            {
                //填充白色
                color=cvScalar(255);
            }
            else
            {
                //填充黑色
                color=cvScalar(0);
            }
            drawContours( dst,v_contour ,i, color, CV_FILLED );
        }

        QString qstr;
        qstr = img_fold + "/" + QString("img_%1.bmp").arg(m_slice_cnt+1);
        string str_img_name;
        str_img_name = qstr.toStdString();
        imwrite(str_img_name,dst);
        m_slice_cnt ++;
        emit thread_slice_cnt(m_slice_cnt);

        v_contour.clear();//删除容器中的所有元素，这里的元素是同一层中所有轮廓数据


        while(1)
        {
            //QApplication::processEvents();

            if(cancelslicing)
            {
                cancelslicing = false;
                fclose(fd);
                return false;
            }

            if(m_slice_cnt >= m_slice_num)
                break;

            if(m_slice_cnt < m_slice_need)
            {
                break;
            }
        }

        if(m_slice_cnt >= m_slice_num)
            break;
    }

    fclose(fd);
    return true;
}








bool slc_process::OpenSTLFile(QString filename)
{
    double thickness;
    unsigned int globalLayers;
    bool ok;

    double x_min, x_max, y_min, y_max, z_min, z_max;
    ok = get_slice_info(filename, &x_min, &x_max, &y_min, &y_max, &z_min, &z_max, &thickness);
    if(!ok)  return false;

    m_thinkness = thickness;
    ModelsizeX = (x_max - x_min);
    ModelsizeY = (y_max - y_min);
    ModelsizeZ = (z_max - z_min);
    m_model_x_min = x_min;
    m_model_x_max = x_max;
    m_model_y_min = y_min;
    m_model_y_max = y_max;
    m_model_z_min = z_min;
    m_model_z_max = z_max;

    //calculate how many layers we need
    globalLayers = qCeil(ModelsizeZ/thickness);
    m_slice_num = globalLayers;

    m_filename = filename;
    m_open_file = true;

    return true;
}

bool slc_process::GetModelSize(double *x, double *y, double *z)
{
    if(m_open_file)
    {
        *x = ModelsizeX;
        *y = ModelsizeY;
        *z = ModelsizeZ;

        return true;
    }
    else
    {
        return false;
    }
}

bool slc_process::SetSlicePara(double pixel_size)
{
    if((pixel_size <= 0))
    {
        return false;
    }

    m_pixel_size = pixel_size;

    return true;
}

bool slc_process::GetSlicePara(double *pixel_size, double *thinkness, int *slice_num)
{
    *pixel_size = m_pixel_size;
    *thinkness = m_thinkness;
    *slice_num = m_slice_num;

    return true;
}

bool slc_process::StartSclie()
{
    if(m_open_file)
    {
        m_start_slice = true;
        return true;
    }
    else
    {
        return false;
    }
}

void slc_process::StopSlice()
{
    std::cout << "stop slice" << std::endl;
    cancelslicing = true;
}

void slc_process::SetSliceNeed(int slice_need)
{
    //std::cout << "set slice need: " << slice_need <<  std::endl;
    m_slice_need = slice_need;
}


void slc_process::run()
{
    bool start = false;
    while(1)
    {
        if(!start)
        {
            std::cout << "start run" << std::endl;
            start = true;
        }
        if(m_open_file & m_start_slice)
        {
            std::cout << "start slice" << std::endl;
            m_start_slice = false;
            m_slice_cnt = 0;

            QString img_fold = "slice_image";
            unsigned int dlp_x_pixel    = 3840;
            unsigned int dlp_y_pixel    = 4320;
            slc_to_img(m_filename, img_fold, dlp_x_pixel, dlp_y_pixel, m_thinkness);

            std::cout << "slc img thread return" << std::endl;

            emit thread_slice_stop();
        }
    }
}


