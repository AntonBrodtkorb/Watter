/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "stdio.h"
//#include "i2c.h"
//#include "sensor.h"

#define addrADXL (0x53)

static uint8 rbuff[2]; // Read buffer
static uint8 wbuff[2]; // write buffer

static cy_stc_scb_i2c_master_xfer_config_t register_setting;

void waitForOperation()
{
    while(0 != (I2C_MasterGetStatus() &CY_SCB_I2C_MASTER_BUSY))
    {
//        CyDelayUs(1); // venter på at status ikk er busy mere.
    }
    return;
}

void handleError(uint8 x)
{
    Cy_GPIO_Write(RED_PORT,RED_NUM,0);
    Cy_GPIO_Write(GREEN_PORT,GREEN_NUM,1);
    printf(" error %d \r\n ", x);
}
//for at skrive til et register vil vi gerne skrive to værdier. 
// En addresse og en indstilling i form af data.

void writeRegister(uint8 reg_addr, uint8 data)
{
    wbuff[0] = reg_addr;    //tildel det første element til register addressen.
    wbuff[1] = data;        //tildel det andet element til den data værdi som vi gerne vil bruge til at indstille registeret.
    
    register_setting.slaveAddress = addrADXL;
    register_setting.buffer = wbuff;
    register_setting.bufferSize = 2;
    register_setting.xferPending = false;
    I2C_MasterWrite(&register_setting);
    waitForOperation();
}

// for at læse fra et register skal vi oplyse addressen på det register som vi gerne vil læse på. 

uint8 readRegister(uint8 reg_addr)
{
    wbuff[0] = reg_addr;
    
    register_setting.slaveAddress = addrADXL;
    register_setting.buffer = wbuff;
    register_setting.bufferSize = 1;
    register_setting.xferPending = true;
    
    I2C_MasterWrite(&register_setting);
    waitForOperation();
    
    register_setting.buffer = rbuff;
    register_setting.xferPending = false;
    
    I2C_MasterRead(&register_setting);
    waitForOperation();
    
    return rbuff[0];
}


int main(void)
{
    UART_Start();
    I2C_Start();
    
    uint8 xAxis[2], yAxis[2], zAxis[2];
    float xAxis2, yAxis2, zAxis2;
    
    cy_en_scb_i2c_status_t initStatus;
    cy_en_sysint_status_t sysStatus;
    
    initStatus = Cy_SCB_I2C_Init(I2C_HW, &I2C_config, &I2C_context);
    if(initStatus != CY_SCB_I2C_SUCCESS)
    {
        handleError(1);
        
    }
    sysStatus = Cy_SysInt_Init(&I2C_SCB_IRQ_cfg, &I2C_Interrupt);
    if(sysStatus != CY_SYSINT_SUCCESS)
    {
        handleError(2);
    }
    
    I2C_MasterSendStart(I2C_HW, CY_SCB_I2C_WRITE_XFER, 1);
    waitForOperation();
    
    /* Enable global interrupts. */
    __enable_irq(); 
    
    Cy_GPIO_Write(RED_PORT,RED_NUM,1);      // nulstiller RØD LED til debuging og advarsler.
    Cy_GPIO_Write(GREEN_PORT,GREEN_NUM,1);  // nulstiller GRØN LED til debuging og godkendelser.
    setvbuf ( stdin, NULL, _IONBF, 0 );
    setvbuf ( stdout, NULL, _IONBF, 0);
    
    writeRegister(0x2D, 8);
    writeRegister(0x27,112);
    I2C_MasterSendStop(1);
    
    for (;;)
    {
        Cy_GPIO_Write(GREEN_PORT,GREEN_NUM,0);
        Cy_GPIO_Write(RED_PORT,RED_NUM,1);
        
        I2C_MasterSendReStart(I2C_HW, CY_SCB_I2C_READ_XFER, 1);
        // Her udregnes Xaxis
        xAxis[0] = readRegister(0x32);
        xAxis[1] = readRegister(0x33);
        xAxis[1] = xAxis[1] << 8;
        xAxis2 = xAxis[0] + xAxis[1];
        //xAxis2 = xAxis2/256;
        
        // Her udregnes Yaxis
        yAxis[0] = readRegister(0x34);
        yAxis[1] = readRegister(0x35);
        yAxis[1] = yAxis[1] << 8;
        yAxis2 = yAxis[0] + yAxis[1];
        //yAxis2 = yAxis2/256;
        
        // Her udregnes Zaxis
        zAxis[0] = readRegister(0x36);
        zAxis[1] = readRegister(0x37);
        zAxis[1] = zAxis[1] << 8;
        zAxis2 = zAxis[0] + zAxis[1];
        //zAxis2 = zAxis2/256;
        
        I2C_MasterSendStop(1);
        CyDelay(25);
        // Her udskrives de tre Akser
        printf(" XA: %.1f YA: %.1f ZA: %.1f   \r\n", xAxis2, yAxis2, zAxis2 );
        
    }
    
    slut:
        return 0;
}


/* [] END OF FILE */
