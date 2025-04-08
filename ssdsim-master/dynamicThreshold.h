#pragma once

#ifndef CPP_DYNAMICTHRESHOLD_H
#define CPP_DYNAMICTHRESHOLD_H

#include <stdio.h>
#include "initialize.h"

#ifdef __cplusplus
extern "C" {
#endif


    // ���嶯̬��ֵ������
    struct ThresholdAdjuster {
        int currentIndex;             // ��ǰ��ֵ�ڹ̶������е�����
        double S_target;              // Ŀ���ۺ�����
        double epsilon;               // ���̴�����������Ŀ���epsilon��ʱ������
        int baseStep;                 // ������������ʾÿ�ε���ʱ��������С�ƶ�����
        int consecutiveAdjust;        // ��������������������ʾ�������󣬸�����ʾ������С��
    };

    /***********************************************************************************
    * adjustThreshold������
    * ����˵����
    *   adjuster - ��̬��ֵ��������������ǰ��ֵ�͵���������
    *   L_norm   - ��һ����I/O�ӳ�ָ�꣨��ֵ��Χ����Ϊ0~1����ֵԽ���ʾ�ӳ�Խ�ߣ�
    *   C_norm   - ��һ����IOPSָ�꣨0~1��
    *   W_norm   - ��һ����д��������ӳٻ���г���ָ�꣨0~1��
    *   D_norm   - ��һ������ɾ�����ʣ�0~1����ֵԽ�߱�ʾ�ռ��ʡЧ��Խ�ã�
    *   w1~w4    - ����ָ���Ȩ������
    *
    * �����ۺ����� S = w1 * L_norm + w2 * C_norm + w3 * W_norm - w4 * D_norm
    *
    * �� S > S_target + epsilon ʱ����Ϊϵͳ���ؽϸߣ���Ҫ������ɾ������
    * ���Ӧ������ֵ����������ƶ�����ѡ����ߵ���ɢֵ����
    *
    * �� S < S_target - epsilon ʱ��˵��ϵͳ���ؽϵͣ�
    * ��˿��Խ�����ֵ��������ɾ��Χ��������ǰ�ƶ�����ѡ���С����ɢֵ����
    *
    * ����Ӧ�������ƣ�����ͬһ�������ʱ���������ۼӣ�������Ӧ�����ĸ��ر仯��
    * ������ת�����ִ������̷�Χ��ʱ�������������á�
    *
    * ����ֵΪ�������ʵ����ֵ������8��Ӧ4KB����
    ***********************************************************************************/

    struct ssd_info * initialize_adjuster(struct ssd_info * ssd);

    int adjustThreshold(struct ThresholdAdjuster * adjuster, double L_norm, double C_norm, double W_norm, double D_norm, double w1, double w2, double w3, double w4);


#ifdef __cplusplus
}
#endif


#endif              // end of CPP_DYNAMICTHRESHOLD_H
