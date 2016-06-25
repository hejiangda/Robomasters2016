
#define __CLOUDMOTOR_GLOBALS

#include "Config.h"
#include "Task_CANSend.h"
#include "Driver_Control.h"
#include "Driver_CloudMotor.h"
#include "Driver_SuperGyroscope.h"

#include "Handler.h"


/*******************
电机参数见头文件宏定义
*******************/



/**
  * @brief  电机初始化
  * @param  void
  * @retval void
  */
void CloudMotor_InitConfig(void)
{
    CloudParam.Pitch.EncoderTargetAngle = PitchCenter;
    CloudParam.Pitch.ABSTargetAngle     = 0;
    CloudParam.Pitch.AngleMode          = AngleMode_Encoder;
    CloudParam.Pitch.FrameCounter       = 0;
    CloudParam.Pitch.FrameRate          = 0;
    CloudParam.Pitch.RealEncoderAngle   = PitchCenter;
    
    CloudParam.Yaw.EncoderTargetAngle   = YawCenter;
    CloudParam.Yaw.ABSTargetAngle       = 0;
    CloudParam.Yaw.AngleMode            = AngleMode_Encoder;
    CloudParam.Yaw.FrameCounter         = 0;
    CloudParam.Yaw.FrameRate            = 0;
    CloudParam.Yaw.RealEncoderAngle     = YawCenter;
    
    CloudParam.Lock = 1;
}
    
    
    
/**
  * @brief  云台电机电流控制
  * @param  pitch轴电流，yaw轴电流
  * @retval void
  */
void CloudMotorCurrent(int16_t Pitch, int16_t Yaw)
{
    static CanSend_Type   SendData;
    
    #if CANPORT == 1
    SendData.CANx = 1;
    #else
    SendData.CANx = 2;
    #endif
    
    
    SendData.SendCanTxMsg.DLC   =   8;
    SendData.SendCanTxMsg.IDE   =   CAN_ID_STD;
    SendData.SendCanTxMsg.RTR   =   CAN_RTR_Data;
    SendData.SendCanTxMsg.StdId =   COULDCONTROLID;
    
//    #if MOTORTYPE == 0
    SendData.SendCanTxMsg.Data[0] = Yaw >> 8;
    SendData.SendCanTxMsg.Data[1] = Yaw;
    SendData.SendCanTxMsg.Data[2] = Pitch >> 8;
    SendData.SendCanTxMsg.Data[3] = Pitch;
    SendData.SendCanTxMsg.Data[4] = 0;
    SendData.SendCanTxMsg.Data[5] = 0;
    SendData.SendCanTxMsg.Data[6] = 0;
    SendData.SendCanTxMsg.Data[7] = 0;
//    #else
//    SendData.SendCanTxMsg.Data[0] = (-Yaw) >> 8;
//    SendData.SendCanTxMsg.Data[1] = (-Yaw);
//    SendData.SendCanTxMsg.Data[2] = (-Pitch) >> 8;
//    SendData.SendCanTxMsg.Data[3] = (-Pitch);
//    SendData.SendCanTxMsg.Data[4] = 0;
//    SendData.SendCanTxMsg.Data[5] = 0;
//    SendData.SendCanTxMsg.Data[6] = 0;
//    SendData.SendCanTxMsg.Data[7] = 0;
//    #endif
    
    xQueueSend(Queue_CANSend, &SendData, 10);
}


/**
  * @brief  电机调节函数
  * @Parma  0 电机停机      1 PID调节
  * @retval void
  * @note   此函数应该周期性执行
  */
void Cloud_Adjust(uint8_t mode)
{
    int16_t PitchMotorCurrent, YawMotorCurrent;
    
    //未锁定
    if(mode)
    {
        //Yaw绝对角度目标模式
        if(CloudParam.Yaw.AngleMode == AngleMode_ABS)
        {
            CloudParam.Yaw.EncoderTargetAngle = (CloudParam.Yaw.ABSTargetAngle - SuperGyoAngle) * 22.7556F + YawCenter;
            CloudParam.Yaw.EncoderTargetAngle = CloudParam.Yaw.EncoderTargetAngle > YawLEFTLimit ? YawLEFTLimit : CloudParam.Yaw.EncoderTargetAngle;
            CloudParam.Yaw.EncoderTargetAngle = CloudParam.Yaw.EncoderTargetAngle < YawRIGHTLimit ? YawRIGHTLimit : CloudParam.Yaw.EncoderTargetAngle;
        }
        
        PitchMotorCurrent = Control_PitchPID();
        YawMotorCurrent = Control_YawPID();
    }
    //锁定
    else
    {
        PitchMotorCurrent = 0;
        YawMotorCurrent = 0;
    }
    
    //云台电机电流发送
    CloudMotorCurrent(PitchMotorCurrent, YawMotorCurrent);
}


/**
  * @brief  云台Yaw度及模式设置
  * @param  目标角度
  * @param  模式  0 绝对角度          1 相对角度（编码器角度,左正右负）
  * @retval void
  */
void Cloud_YawAngleSet(float Target, uint8_t mode)
{
    int16_t Buffer;
    
    if(mode)
    {
        CloudParam.Yaw.AngleMode = AngleMode_Encoder;
        
        Buffer = (int16_t)Target + YawCenter;
        Buffer = Buffer > YawLEFTLimit ? YawLEFTLimit : Buffer;
        Buffer = Buffer < YawRIGHTLimit ? YawRIGHTLimit : Buffer;
        
        CloudParam.Yaw.EncoderTargetAngle = Buffer;
    }
    else
    {
        CloudParam.Yaw.AngleMode = AngleMode_ABS;
        CloudParam.Yaw.ABSTargetAngle = Target;
    }
}


/**
  * @brief  云台Pitch角度及模式设置
  * @param  目标角度(编码器角度）
  * @retval void
  */
void Cloud_PitchAngleSet(float Target)
{
    int16_t Buffer;
    
    CloudParam.Pitch.AngleMode = AngleMode_Encoder;
    
    Buffer = (int16_t)Target + PitchCenter;
    Buffer = Buffer > PitchUPLimit ? PitchUPLimit : Buffer;
    Buffer = Buffer < PitchDOWNLimit ? PitchDOWNLimit : Buffer;
    
    CloudParam.Pitch.EncoderTargetAngle = Buffer;
}






