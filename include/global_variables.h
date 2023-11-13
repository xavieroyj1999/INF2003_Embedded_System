#ifndef GLOBAL_H
#define GLOBAL_H

uint8_t g_left_encoder_interrupts = 0;
uint8_t g_right_encoder_interrupts = 0;
uint32_t g_distance_travelled = 0;

float g_initial_degree = 0;
float g_degree = 0;

double g_duty_cycle = 0;

float g_inital_degree = 0;
float g_degree_thresholds[4];

#endif