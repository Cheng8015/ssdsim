#include "dynamicThreshold.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>


std::vector<int> fixedThresholds = { 8, 16, 32, 64, 128, 256 };

struct ssd_info * initialize_adjuster(struct ssd_info* ssd) {
    
    struct ThresholdAdjuster* adjuster = (struct ThresholdAdjuster*)malloc(sizeof(struct ThresholdAdjuster));
    memset(ssd->threshold_adjuster, 0, sizeof(struct ThresholdAdjuster));

    adjuster->currentIndex = 0;
    adjuster->S_target = 0.0;
    adjuster->epsilon = 0.5;
    adjuster->baseStep = 1;
    adjuster->consecutiveAdjust = 0;

    ssd->threshold_adjuster = adjuster;

    return ssd;
}

int adjustThreshold(struct ThresholdAdjuster * adjuster, double L_norm, double C_norm, double W_norm, double D_norm, double w1, double w2, double w3, double w4) {
    // �����ۺ�����
    double S = w1 * L_norm + w2 * C_norm + w3 * W_norm - w4 * D_norm;

    // ϵͳ���ؽϸߣ������ֵ����������ѡ����ߵ���ֵ��
    if (S > adjuster->S_target + adjuster->epsilon) {
        if (adjuster->consecutiveAdjust >= 0) {
            adjuster->consecutiveAdjust++;
        }
        else {
            adjuster->consecutiveAdjust = 1;
        }
        int step = adjuster->baseStep * adjuster->consecutiveAdjust;
        adjuster->currentIndex = std::min(adjuster->currentIndex + step, (int)fixedThresholds.size() - 1);
    }
    // ϵͳ���ؽϵͣ�������ֵ��������С��ѡ��ϵ͵���ֵ��
    else if (S < adjuster->S_target - adjuster->epsilon) {
        if (adjuster->consecutiveAdjust <= 0) {
            adjuster->consecutiveAdjust--;
        }
        else {
            adjuster->consecutiveAdjust = -1;
        }
        int step = adjuster->baseStep * std::abs(adjuster->consecutiveAdjust);
        adjuster->currentIndex = std::max(adjuster->currentIndex - step, 0);
    }
    // �ۺ����������̷�Χ�ڣ�������
    else {
        adjuster->consecutiveAdjust = 0;
    }

    // ���ص�ǰѡ�����ֵ������8��Ӧ4KB��
    return fixedThresholds[adjuster->currentIndex];
}