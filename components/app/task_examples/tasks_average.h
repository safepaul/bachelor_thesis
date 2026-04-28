#ifndef TASKS_H
#define TASKS_H

void Telemetry_utask(void *pvParameters);
void Sensor_Fusion_utask(void *pvParameters);
void Control_Loop_utask(void *pvParameters);
void Vision_Processing_utask(void *pvParameters);
void Data_Logging_utask(void *pvParameters);
void Diagnostics_utask(void *pvParameters);
void Failsafe_Monitor_utask(void *pvParameters);


#endif // !TASKS_H
