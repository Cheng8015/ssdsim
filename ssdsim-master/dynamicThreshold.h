#pragma once

#ifndef CPP_DYNAMICTHRESHOLD_H
#define CPP_DYNAMICTHRESHOLD_H

#include <stdio.h>
#include "initialize.h"

#ifdef __cplusplus
extern "C" {
#endif


    // 定义动态阈值调整器
    struct ThresholdAdjuster {
        int currentIndex;             // 当前阈值在固定数组中的索引
        double S_target;              // 目标综合评分
        double epsilon;               // 容忍带，当评分在目标±epsilon内时不调整
        int baseStep;                 // 基础步长，表示每次调整时索引的最小移动步数
        int consecutiveAdjust;        // 连续调整计数（正数表示连续增大，负数表示连续减小）
    };

    /***********************************************************************************
    * adjustThreshold函数：
    * 参数说明：
    *   adjuster - 动态阈值调整器（包含当前阈值和调整参数）
    *   L_norm   - 归一化的I/O延迟指标（数值范围假设为0~1，数值越大表示延迟越高）
    *   C_norm   - 归一化的IOPS指标（0~1）
    *   W_norm   - 归一化的写请求队列延迟或队列长度指标（0~1）
    *   D_norm   - 归一化的重删收益率（0~1，数值越高表示空间节省效果越好）
    *   w1~w4    - 各项指标的权重因子
    *
    * 计算综合评分 S = w1 * L_norm + w2 * C_norm + w3 * W_norm - w4 * D_norm
    *
    * 当 S > S_target + epsilon 时，认为系统负载较高，需要降低重删操作，
    * 因此应增大阈值（索引向后移动，即选择更高的离散值）。
    *
    * 当 S < S_target - epsilon 时，说明系统负载较低，
    * 因此可以降低阈值以扩大重删范围（索引向前移动，即选择较小的离散值）。
    *
    * 自适应步长机制：连续同一方向调整时，步长会累加，快速响应持续的负载变化；
    * 当方向反转或评分处于容忍范围内时，连续计数重置。
    *
    * 返回值为调整后的实际阈值（例如8对应4KB）。
    ***********************************************************************************/

    struct ssd_info * initialize_adjuster(struct ssd_info * ssd);

    int adjustThreshold(struct ThresholdAdjuster * adjuster, double L_norm, double C_norm, double W_norm, double D_norm, double w1, double w2, double w3, double w4);


#ifdef __cplusplus
}
#endif


#endif              // end of CPP_DYNAMICTHRESHOLD_H
