#include <string.h>
#include "esp_log.h"

#include "volc_i2c.h"
#include "power.h"


static const char* TAG = "power_500w";
static i2c_master_dev_handle_t power_dev_handle[TOTAL_SUPPORT_POWER_TYPE];

static POWER_TYPE g_power_type = UNKNOWN_POWER;

esp_err_t yg_power_init(uint8_t i2c_master_index)
{
	ESP_LOGI(TAG, "yg_power_init %hhd", i2c_master_index);
	return volc_i2c_add_device(i2c_master_index, YIGONG_500W_POWER_DEFAULT_I2C_ADDR>>1, power_dev_handle, TAG);
}

/*
yg_wread: 0 ab 0 83 0 4 1 0 e5 1 6e cd 
yg_wread: 0 0 0 0 0 0 0 0 0 0 0 0 
ERRORMSG setPowerVoltage 0 0 0 0

red line: SDA, yellow line: SCL, black line: GND
*/
esp_err_t yg_write(int cmd, uint8_t* data, int len)
{
	esp_err_t ret=ESP_FAIL;
	uint8_t send_data[12]={0,0,0,0,0,0,0,0,0,0};
	uint8_t sum=0;
	uint8_t realLen=0;

	send_data[0]=0xAB;
	send_data[1]=0;
	send_data[2]=cmd;
	send_data[3]=0;  //sta;
	send_data[4]=len;
	for(int i=0;i<len;i++){
		send_data[5+i]=data[i];
	}

	sum=send_data[1]+send_data[2]+send_data[3]+send_data[4];
	for(int i=0;i<len;i++){
		sum+=data[i];
	}
	
	send_data[4+len+1]=sum;
	send_data[4+len+2]=0xCD;

	realLen=5+len+1+1;
	ret = volc_i2c_write(power_dev_handle[YG_500W_POWER_TYPE_INDEX], send_data, realLen);
	ESP_LOGD(TAG, "volc i2c write: %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8"", 
		send_data[0], send_data[1], send_data[2], send_data[3], send_data[4], send_data[5],
		send_data[6], send_data[7], send_data[8], send_data[9], send_data[10], send_data[11] 
	);

	return ret;
}

esp_err_t yg_wread(int cmd, uint8_t* data, int len, uint8_t* rdata){
	uint8_t send_data[12]={0,0,0,0,0,0,0,0,0,0,0,0};
	uint8_t read_data_back[12]={0,0,0,0,0,0,0,0,0,0,0,0};
	esp_err_t ret=ESP_FAIL;
	uint8_t sum=0;
	uint8_t realLen=0;

	send_data[0]=0;
	send_data[1]=0xAB;
	send_data[2]=0;
	send_data[3]=cmd;
	send_data[4]=0;  //sta;
	send_data[5]=len;
	for(int i=0;i<len;i++){
		send_data[6+i]=data[i];
	}

	sum=send_data[2]+send_data[3]+send_data[4]+send_data[5];
	for(int i=0;i<len;i++){
		sum+=data[i];
	}
	
	send_data[5+len+1]=sum;
	send_data[5+len+2]=0xCD;

	realLen=6+len+1+1;

	ret = volc_i2c_write_read(power_dev_handle[YG_500W_POWER_TYPE_INDEX], send_data, realLen, read_data_back, realLen);
	ESP_LOGD(TAG, "volc i2c write: %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8"", 
		send_data[0], send_data[1], send_data[2], send_data[3], send_data[4], send_data[5],
		send_data[6], send_data[7], send_data[8], send_data[9], send_data[10], send_data[11] 
	);
	ESP_LOGD(TAG, "volc i2c read: %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8"", 
		read_data_back[0], read_data_back[1], read_data_back[2], read_data_back[3], read_data_back[4], read_data_back[5],
		read_data_back[6], read_data_back[7], read_data_back[8], read_data_back[9], read_data_back[10], read_data_back[11] 
	);

	memcpy(rdata, read_data_back+6, len);

	return ret;
}

esp_err_t powerOnYiGong(){
	uint8_t wdata[2] = {0,0x1};
	uint8_t rdata[2] = {0, 0};

	ESP_LOGI(TAG, "ERRORMSG powerOn now...");
	
	if(yg_wread(0x85, wdata, 2, rdata)<0){
		ESP_LOGI(TAG, "ERRORMSG power write error");
		return ESP_FAIL;
	}else{
		ESP_LOGI(TAG, "ERRORMSG power write data:%x %x",wdata[0],wdata[1]);
		return ESP_OK;
	}
}

esp_err_t powerOffYiGong(){
	uint8_t wdata[2]={1,0x2};
	uint8_t rdata[2]={0, 0};

	ESP_LOGI(TAG, "ERRORMSG powerOff now...");

	if(yg_wread(0x85, wdata, 2, rdata) != ESP_OK){
		ESP_LOGI(TAG, "ERRORMSG power write error");
		return ESP_FAIL;
	}else{
		ESP_LOGI(TAG, "ERRORMSG power write data:%x %x",wdata[0],wdata[1]);
		return ESP_OK;
	}
}

esp_err_t setPowerVoltageYiGong(uint16_t voltage){
	uint8_t wdata[4]={0x1,0,0,0};
	wdata[2]=(uint8_t)voltage;
	wdata[3]=(uint8_t)(voltage>>8);
	uint8_t rdata[4] = {0, 0, 0, 0};

	if(yg_wread(0x83, wdata, 4, rdata) != ESP_OK){
		ESP_LOGI(TAG, "ERRORMSG setPowerVoltage error");
		return ESP_FAIL;
	}else{
		ESP_LOGI(TAG, "ERRORMSG setPowerVoltage %x %x %x %x",wdata[0],wdata[1],wdata[2],wdata[3]);
		return ESP_OK;
	}
}

/*
ERRORMSG getPowerID now...
yq_read: 0 ab 0 8 0 4 0 0 0 0 c cd 
yq_read back: 0 ab 0 8 0 4 55 aa 1 a0 ac cd 
ERRORMSG power read data:55 aa 1 a0
*/
esp_err_t yg_checkPowerID(){
	esp_err_t ret = ESP_FAIL;
	uint8_t rdata[4]={0,0,0,0};

	ESP_LOGI(TAG, "ERRORMSG getPowerID now...");

	if(ESP_OK != yg_wread(0x08, rdata, 4, rdata)){
		ESP_LOGI(TAG, "ERRORMSG power read error");
	}else{
		ESP_LOGI(TAG, "ERRORMSG power read data:%x %x %x %x",rdata[0],rdata[1],rdata[2],rdata[3]);

		if((rdata[2]==3 && rdata[3]==0xA2)||(rdata[2]==1 && rdata[3]==0xA0)){
			ret = ESP_OK;
		}
	}

	return ret;
}

double yg_outputVoltage(){
	double retVol = 0.0;
	uint8_t rdata[4]={1,0,0,0};

	rdata[0]=1;
	rdata[1]=rdata[2]=rdata[3]=0;
	if(yg_wread(0x2, rdata, 4, rdata)<0){
		ESP_LOGW(TAG, "YG fail to read output voltage.");
		return retVol;
	}

	retVol = ((double)(rdata[3]<<8|rdata[2]))/100;
	return retVol;
}

double yg_outputCurrent(){
	double retCurrent=0.0;
	uint8_t rdata[4]={1,0,0,0};
	
	rdata[0]=1;
	rdata[1]=rdata[2]=rdata[3]=0;
	if(yg_wread(0x87, rdata, 4, rdata)<0){
		ESP_LOGW(TAG, "YG fail to read output current.");
		return retCurrent;
	}

	retCurrent = ((double)(rdata[3]<<8|rdata[2]))/100;
	return retCurrent;
}

double yg_inputPower(){
	double retPower = 0.0;
	uint8_t rdata[4]={1,0,0,0};
	
	rdata[0]=1;
	rdata[1]=rdata[2]=rdata[3]=0;
	if(yg_wread(0x6, rdata, 4, rdata)<0){
		ESP_LOGW(TAG,"YG fail to read input power.");
		return -1;
	}
	
	ESP_LOGI(TAG,"Input Power :%x %x %x %x--%f W ",rdata[0],rdata[1],rdata[2],rdata[3],((float)(rdata[3]<<8|rdata[2])));
	retPower = ((double)(rdata[3]<<8|rdata[2]));

	return retPower;
}

/*output power？*/
double yg_outputPower(){

	return yg_outputVoltage()*yg_outputCurrent();
}

double yg_inputCurrent(){
	double ret_inputCurrent = 0.0;
	uint8_t rdata[4]={1,0,0,0};	
	
	rdata[0]=1;
	rdata[1]=rdata[2]=rdata[3]=0;
	if(yg_wread(0x5, rdata, 4, rdata)<0){
		ESP_LOGW(TAG,"YG fail to read input current.");
		return -1;
	}

	//ESP_LOGI(TAG,"Input Power Current:%x %x %x %x--%.2f A ",rdata[0],rdata[1],rdata[2],rdata[3],((float)(rdata[3]<<8|rdata[2]))/100);
	ret_inputCurrent = (double)(((rdata[3]<<8|rdata[2]))/100);

	return ret_inputCurrent;
}

double yg_inputVoltage(){
	double ret_inputVoltage = 0.0;
	uint8_t rdata[4]={1,0,0,0};		

	rdata[0]=1;
	rdata[1]=rdata[2]=rdata[3]=0;
	if(yg_wread(0x4, rdata, 4, rdata)<0){
		ESP_LOGW(TAG,"YG fail to read input voltage.");
		return ret_inputVoltage;
	}
	ret_inputVoltage = ((double)(rdata[3]<<8|rdata[2]))/100;

	return ret_inputVoltage;
}

int powerParamYiGong(){
	int ret=0;
	uint8_t rdata[4]={1,0,0,0};

	//applog(LOG_ERR,"ERRORMSG getPowerStatusYiGong now...");
	rdata[0]=1;
	rdata[1]=rdata[2]=rdata[3]=0;
	if(yg_wread(0x2, rdata, 4, rdata)<0){
		ESP_LOGW(TAG,"ERRORMSG powerParam error");
		return -1;
	}
	ESP_LOGI(TAG,"Output Power Voltage:%x %x %x %x--%.2f V",rdata[0],rdata[1],rdata[2],rdata[3],((float)(rdata[3]<<8|rdata[2]))/100);

	rdata[0]=1;
	rdata[1]=rdata[2]=rdata[3]=0;
	if(yg_wread(0x4, rdata, 4, rdata)<0){
		ESP_LOGW(TAG,"ERRORMSG powerParam error");
		return -1;
	}
	ESP_LOGI(TAG,"Input Power Voltage:%x %x %x %x--%.2f V ",rdata[0],rdata[1],rdata[2],rdata[3],((float)(rdata[3]<<8|rdata[2]))/100);

	rdata[0]=1;
	rdata[1]=rdata[2]=rdata[3]=0;
	if(yg_wread(0x5, rdata, 4, rdata)<0){
		ESP_LOGW(TAG,"ERRORMSG powerParam error");
		return -1;
	}
	ESP_LOGI(TAG,"Input Power Current:%x %x %x %x--%.2f A ",rdata[0],rdata[1],rdata[2],rdata[3],((float)(rdata[3]<<8|rdata[2]))/100);

	rdata[0]=1;
	rdata[1]=rdata[2]=rdata[3]=0;
	if(yg_wread(0x6, rdata, 4, rdata)<0){
		ESP_LOGW(TAG,"ERRORMSG powerParam error");
		return -1;
	}
	ESP_LOGI(TAG,"Input Power :%x %x %x %x--%f W ",rdata[0],rdata[1],rdata[2],rdata[3],((float)(rdata[3]<<8|rdata[2])));

	rdata[0]=1;
	rdata[1]=rdata[2]=rdata[3]=0;
	if(yg_wread(0x87, rdata, 4, rdata)<0){
		ESP_LOGW(TAG,"ERRORMSG powerParam error");
		return -1;
	}
	ESP_LOGI(TAG,"Real Current :%x %x %x %x--%f A ",rdata[0],rdata[1],rdata[2],rdata[3],((float)(rdata[3]<<8|rdata[2]))/100);

	return 0;
}

/*
driver_btm-sia.c 3251
outActivePowerYg
inActivePowerYg
voltCurrentYg
fanSpeed
outputVolYg
outputCurrentYg
*/
esp_err_t gos_power_init(uint8_t i2c_master_index)
{
	ESP_LOGI(TAG, "gos_power_init %hhd", i2c_master_index);
	return volc_i2c_add_device( i2c_master_index, GOS_500W_POWER_DEFAULT_I2C_ADDR>>1, 
								power_dev_handle+GOS_500W_POWER_TYPE_INDEX, TAG);
}

uint8_t crc8(uint8_t* pushMsg, uint32_t usDataLen){
	uint8_t uCRC=0x0;
	uint8_t num=0;
	uint8_t x=0;
	for(num=0;num<usDataLen;num++){
		uCRC=(*pushMsg++)^uCRC;
		for(x=0;x<8;x++){
			if(uCRC & 0x80){
				uCRC=uCRC<<1;
				uCRC = uCRC^0x07;
			}else{
				uCRC=uCRC<<1;
			}
		}
	}
	return uCRC;
}

esp_err_t gos_write(int cmd, uint8_t* data, int len)
{
	int i;
	uint8_t crc_data[10]={0,0,0,0,0,0,0,0,0,0};//=malloc(1+1+len);
	uint8_t send_data[10]={0,0,0,0,0,0,0,0,0,0};
	uint8_t crc;
	esp_err_t ret=ESP_FAIL;
	
	crc_data[0]=GOS_500W_POWER_DEFAULT_I2C_ADDR;
	crc_data[1]=cmd;
	for(i=0;i<len;i++){
		crc_data[2+i]=data[i];
	}
	crc=crc8(crc_data,len+2);
	memcpy(send_data,data,len);
	send_data[len]=crc;

	uint8_t realSend[12];
	realSend[0]=cmd;
	memcpy(realSend+1,send_data,len+1);

	ret = volc_i2c_write(power_dev_handle[GOS_500W_POWER_TYPE_INDEX], realSend, len+2);
	ESP_LOGI(TAG, "volc i2c write: %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8"", 
		realSend[0], realSend[1], realSend[2], realSend[3], realSend[4], 
		realSend[5], realSend[6], realSend[7], realSend[8], realSend[9],
		realSend[10], realSend[11]
	);

	return ret;
}

esp_err_t gos_read(int cmd, uint8_t* read_data_back, int len)
{
	uint8_t send_data[10]={0,0,0,0,0,0,0,0,0,0};//=malloc(1+1+len);
	uint8_t crc_data[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	esp_err_t ret=ESP_FAIL;	

	send_data[0] = (uint8_t)cmd;

	ret = volc_i2c_write_read(power_dev_handle[GOS_500W_POWER_TYPE_INDEX], send_data, 1, read_data_back, len);
	ESP_LOGI(TAG, "volc i2c write: %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8"", 
		send_data[0], send_data[1], send_data[2], send_data[3], send_data[4], 
		send_data[5], send_data[6], send_data[7], send_data[8], send_data[9] 
	);
	ESP_LOGI(TAG, "volc i2c read: %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8"", 
		read_data_back[0], read_data_back[1], read_data_back[2], read_data_back[3], read_data_back[4], read_data_back[5],
		read_data_back[6], read_data_back[7], read_data_back[8], read_data_back[9], read_data_back[10], read_data_back[11] 
	);

	/*no crc check in G1853*/
	/*
	crc_data[0]=GOS_500W_POWER_DEFAULT_I2C_ADDR;
	crc_data[1]=cmd;
	crc_data[2]=GOS_500W_POWER_DEFAULT_I2C_ADDR|0x1;
	memcpy(crc_data+3, read_data_back, len);
	crc=crc8(crc_data, len+3);

	if(crc!=read_data_back[len]){
		ESP_LOGW(TAG, "ERRORMSG gos_read: crc error %"PRIx8" %"PRIx8" ", crc, read_data_back[len]);
		ret = ESP_FAIL;
	}
	*/

	return ret;
}

esp_err_t powerGos_checkPowerID()
{
	uint8_t mfr_id_cmd=0x99, mfr_model_cmd=0x9a;
	uint8_t send_data_back[10] = {0};
	char id[6] = {0}, manufacture[10] = {0};
	esp_err_t ret=ESP_FAIL;

	//MSG("ERRORMSG  .....check Gospower ID.");
	/* len("GOSPOWER") + 1 */
	ret = gos_read(mfr_id_cmd, send_data_back, 9); 
	if(ret != ESP_OK){
		ESP_LOGW(TAG, "ERRORMSG powerpowerID_read1: I2C_Puts get NACK!!!!");
		return ret;
	}
	memcpy(manufacture, send_data_back+1, 9);

	ret = gos_read(mfr_model_cmd, send_data_back, 6);
	if(ret != ESP_OK){
		ESP_LOGW(TAG, "ERRORMSG powerpowerID_read2: I2C_Puts get NACK!!!!");
		return ret;
	}

	memcpy(id, send_data_back+1, 5);
	if(0 == strcmp(manufacture, "GOSPOWER") && 0 == strcmp(id, "G1853")){
		g_power_type = GOS_500W;
		ret = ESP_OK;
	}

	return ret;
}

//on:0x80 off:0x0
esp_err_t poweronGos(uint8_t onOff)
{
	unsigned char cmd=0x01;
	uint8_t send_data[1];
	esp_err_t ret=ESP_FAIL;

	send_data[0]=onOff;

	ret = gos_write(cmd, send_data, 1);
	if(ret != ESP_OK){
		ESP_LOGW(TAG, "ERRORMSG power: I2C_Puts get NACK!!!!");
	}

	return ret;
}

esp_err_t powerGos_read(uint8_t* onOff)
{
	unsigned char cmd=0x01;
	uint8_t send_data_back[1];
	esp_err_t ret=ESP_FAIL;

	ret = gos_read(cmd, send_data_back, 1);
	if(ret<0){
		ESP_LOGW(TAG, "ERRORMSG power_read: I2C_Puts get NACK!!!!");
	}

	*onOff=send_data_back[0];

	return ret;
}

esp_err_t powerGos_op_watchDog(uint8_t onOff)
{
	unsigned char cmd=0x07;
	uint8_t send_data[1];
	esp_err_t ret=ESP_FAIL;

	send_data[0]=onOff;

	ret = gos_write(cmd, send_data, 1);
	if(ret != ESP_OK){
		ESP_LOGW(TAG, "ERRORMSG powerGos_op_watchDog: I2C_Puts get NACK!!!!");
	}

	return ret;
}

esp_err_t setPowerVoltageGos(int voltage)
{
	double v = voltage/100.0;
	unsigned char cmd=0x21;
	uint8_t send_data[2];
	esp_err_t ret=ESP_FAIL;

	uint16_t N=(uint16_t)(v*512);//36.6*128=0x124C
	
	send_data[0]=(uint8_t)N;
	send_data[1]=(uint8_t)(N>>8);

	ret = gos_write(cmd, send_data, 2);
	if(ret != ESP_OK){
		ESP_LOGW(TAG, "ERRORMSG setPowerVoltage: I2C_Puts get NACK!!!!");
	}

	return ret;
}

double powerGosVout_read(int8_t exp)
{
	unsigned char cmd=0x21;
	double ret = 0.0;

	gos_read_In16Linear(cmd, exp, &ret);

	return ret;
}

double powerGosIout_read()
{
	unsigned char cmd=0x8C;
	double ret = 0.0;

	gos_read_In11Linear(cmd, &ret);

	return ret;
}

double powerGosPout_read()
{
	unsigned char cmd=0x96;
	double ret = 0.0;

	gos_read_In11Linear(cmd, &ret);

	return ret;
}

esp_err_t powerGosRevision_read()
{
	uint8_t pri_fw_rev_cmd = 0xDB, sec_fw_rev_cmd = 0xDC, thrd_fw_rev_cmd = 0xDE; //isp_psu_hw_rev_cmd = 0xDD;
	uint8_t pri_fw_rev_buf[20] = {0}, sec_fw_rev_buf[20] = {0}, thrd_fw_rev_buf[20] = {0}; //hw_rev_buf[20] = {0};
	esp_err_t ret=ESP_FAIL;

	volc_delay(100);
	ret = gos_read(pri_fw_rev_cmd, pri_fw_rev_buf, 16);
	if(ret != ESP_OK){
		ESP_LOGW(TAG, "ERRORMSG powerGosRevision_read 1: I2C_Puts get NACK!!!!");
	}
	volc_delay(100);
	ret = gos_read(sec_fw_rev_cmd, sec_fw_rev_buf, 16);
	if(ret != ESP_OK){
		ESP_LOGW(TAG, "ERRORMSG powerGosRevision_read 2: I2C_Puts get NACK!!!!");
	}
	volc_delay(100);
	ret = gos_read(thrd_fw_rev_cmd, thrd_fw_rev_buf, 16);
	if(ret != ESP_OK){
		ESP_LOGW(TAG, "ERRORMSG powerGosRevision_read 3: I2C_Puts get NACK!!!!");
	}else{
		ESP_LOGW(TAG, "ERRORMSG Power Revision %s, %s, %s", pri_fw_rev_buf, sec_fw_rev_buf, thrd_fw_rev_buf);
	}

	return ret;
}

esp_err_t powerGosStatusRead()
{
	uint8_t read_status_cmd = 0x79;
	uint8_t read_status_buf[2+1] = {0}, status_array[8] = {0};
	esp_err_t ret = -1;
	int cmd = 0x7a;

	ret = gos_read(read_status_cmd, read_status_buf, 2);
	if(ret < 0){
		ESP_LOGW(TAG, "ERRORMSG powerStatusRead read_status_cmd: I2C_Puts get NACK!!!!");
		return -1;
	}

	for(cmd = 0x7a; cmd <= 0x80; cmd++){
		ret = gos_read(cmd, status_array+cmd-0x7a, 1);
		if(ret < 0){
			ESP_LOGW(TAG, "ERRORMSG powerStatusRead %"PRIx8": I2C_Puts get NACK!!!!", cmd);
			return ret;
		}
	}

	ESP_LOGW(TAG, "ERRORMSG powerStatusRead(0x79~0x80) %"PRIx8"%"PRIx8", %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8" %"PRIx8".",
		read_status_buf[0], read_status_buf[1],  
		status_array[0], status_array[1], status_array[2], status_array[3], 
		status_array[4], status_array[5], status_array[6], status_array[7]
	);

	return ret;
}

double linear_11_to_double(linear11_val_t t)
{
	if(t.linear.exponent<0){
		return t.linear.mantissa / (double)(1 << (-1*t.linear.exponent));
	}else{
		return t.linear.mantissa * (double)(1 << t.linear.exponent);
	}
}

esp_err_t gos_read_In11Linear(uint8_t cmd, double *d_value)
{
	uint8_t send_data_back[2];
	esp_err_t ret = ESP_FAIL;
	linear11_val_t t;

	ret = gos_read(cmd, send_data_back, 2);

	if(ret){
		ESP_LOGW(TAG, "ERRORMSG: gos_read_In11Linear:  I2C_Puts get NACK!!!!");
	}else{
		t.linear.mantissa=(int16_t)((send_data_back[1]<<8|send_data_back[0])&0x7FF);
		t.linear.exponent=((int16_t)((send_data_back[1]<<8|send_data_back[0])&0xF800))>>11;
		*d_value = linear_11_to_double(t);
		ESP_LOGI(TAG, "gos_read_In11Linear read: %x %x --%d %d -- %f",
				send_data_back[0],send_data_back[1], t.linear.mantissa, t.linear.exponent, *d_value
			);
	}

	return ret;
}

esp_err_t gos_read_output_mode(uint8_t *mode, int32_t *exp)
{
	esp_err_t ret = ESP_FAIL;
	uint8_t cmd = 0x20;
	uint8_t send_data_back[1];
	bool is_negative = false;
	uint8_t abs_value = 0;

	ret = gos_read(cmd, send_data_back, 1);

	if(ESP_OK == ret){
		*mode = send_data_back[0] >> 5;
		if(0 == *mode){
			is_negative = (send_data_back[0] >> 4) & 0x1;
			abs_value = (send_data_back[0] & 0xf);
			abs_value = (~abs_value) & 0xf;
			abs_value = (abs_value + 1) & 0xf;

			if(is_negative)
				*exp = -abs_value;
			else
				*exp = abs_value;

			ESP_LOGI(TAG, "gos_read_output_mode: %x ---> %"PRIu8", %"PRIi32".", 
					send_data_back[0], *mode, *exp);
		}
	}
	return ret;
}


esp_err_t gos_read_In16Linear(uint8_t cmd, int8_t exp, double *d_value)
{
	uint8_t send_data_back[2];
	uint16_t u16_v;
	esp_err_t ret=ESP_FAIL; 

	ret = gos_read(cmd, send_data_back, 2);
	u16_v = ((uint16_t)(send_data_back[1]) << 8) + (uint16_t)(send_data_back[0]);

	if(ret){
		ESP_LOGW(TAG, "ERRORMSG: gos_read_In16Linear:  I2C_Puts get NACK!!!!");
	}else{
		if(exp < 0)
			*d_value = (double)u16_v / ((double) (1 << -exp)) ;
		else
			*d_value = (double)(u16_v * (1 << exp)) ;
	}

	return ret;
}

esp_err_t powerGosReadFanTemperature()
{
	esp_err_t ret = ESP_FAIL;
	uint8_t read_temperature_cmd = 0x8D, read_fanspeed1_cmd = 0x90, read_fanspeed2_cmd = 0x91;
	double d_temperature = 0.0, d_fanspeed1 = 0.0, d_fanspeed2 = 0.0;

	ret = gos_read_In11Linear(read_temperature_cmd, &d_temperature);
	if(ret != ESP_OK){
		ESP_LOGW(TAG, "ERRORMSG powerReadFanTemperature read_temperature_cmd: I2C_Puts get NACK!!!!");
		return ret;
	}

	ret = gos_read_In11Linear(read_fanspeed1_cmd, &d_fanspeed1);
	if(ret != ESP_OK){
		ESP_LOGW(TAG, "ERRORMSG powerReadFanTemperature read_fanspeed1_cmd: I2C_Puts get NACK!!!!");
		return ret;
	}

	ret = gos_read_In11Linear(read_fanspeed2_cmd, &d_fanspeed2);
	if(ret != ESP_OK){
		ESP_LOGW(TAG, "ERRORMSG powerReadFanTemperature read_fanspeed2_cmd: I2C_Puts get NACK!!!!");
		return ret;
	}else{
		ret = ESP_OK;
	}

	ESP_LOGW(TAG, "ERRORMSG powerReadFanTemperature: %.2f degree, fan1 %.2f, fan2 %.2f", d_temperature, d_fanspeed1, d_fanspeed2);
	return ret;
}

/*
int powerGosReadVoltage(double* output_voltage, double* input_voltage)
{
	int ret = -1;

	
	return 0;
}
*/

esp_err_t power_detect_type(uint8_t i2c_master_index)
{
	esp_err_t ret = ESP_FAIL;

	if(ESP_OK == yg_power_init(i2c_master_index) && ESP_OK == yg_checkPowerID()){
		ESP_LOGI(TAG, "YG 500W is detected.");
		g_power_type = YG_500W;
		ret = ESP_OK;
	}else if(ESP_OK == gos_power_init(i2c_master_index) && ESP_OK == powerGos_checkPowerID()){
		ESP_LOGI(TAG, "GOS 500W is detected.");
		g_power_type = GOS_500W;
		ret = ESP_OK;
	}

	return ret;
}

esp_err_t power_on()
{
	esp_err_t ret = ESP_FAIL;

	switch (g_power_type)
	{
		case GOS_500W:
			ret = powerGos_op_watchDog(0);
			ret += poweronGos(0x80);
			break;
		case YG_500W:
			ret = powerOnYiGong();
			break;

		default:
			break;
	}

	return ret;
}

esp_err_t power_off()
{
	esp_err_t ret = ESP_FAIL;

	switch (g_power_type)
	{
		case GOS_500W:
			ret = poweronGos(0x0);
			ret += powerGos_op_watchDog(0x1);
			break;
		case YG_500W:
			ret = powerOffYiGong();
			break;

		default:
			break;
	}

	return ret;
}

esp_err_t power_set_voltage(uint16_t voltage)
{
	esp_err_t ret = ESP_FAIL;

	switch (g_power_type)
	{
		case GOS_500W:
			if(voltage > GOS_500W_MAX_OUTPUT_VOLTAGE){
				ESP_LOGW(TAG, "voltage %"PRIu16" > MAX output voltage %d", voltage, GOS_500W_MAX_OUTPUT_VOLTAGE);
			}else if(voltage < GOS_500W_MIN_OUTPUT_VOLTAGE){
				ESP_LOGW(TAG, "voltage %"PRIu16" < MIN output voltage %d", voltage, GOS_500W_MIN_OUTPUT_VOLTAGE);
			}else{
				ret = setPowerVoltageGos(voltage);
			}
			break;
		case YG_500W:
			ret = setPowerVoltageYiGong(voltage);
			break;

		default:
			break;
	}

	return ret;	
}

esp_err_t power_get_nominal_voltage(int *input_voltage)
{
	esp_err_t ret = ESP_OK;

	switch (g_power_type)
	{
		case GOS_500W:
			*input_voltage = 220;
			break;
		default:
			*input_voltage = 220;
			break;
	}

	return ret;

}

esp_err_t power_get_max_power(float *max_power)
{
	esp_err_t ret = ESP_OK;

	switch (g_power_type)
	{
		case GOS_500W:
			*max_power = 500;
			break;
		default:
			*max_power = 500;
			break;
	}

	return ret;
}

double power_get_output_voltage()
{
	double output_voltaget = 0.0;
	
	switch (g_power_type)
	{
		case GOS_500W:
		    uint8_t mode;
    		int32_t exp;
			gos_read_output_mode(&mode, &exp);
			output_voltaget = powerGosVout_read(exp);
			break;
		case YG_500W:
			output_voltaget = yg_outputVoltage();
			break;
		default:
			break;
	}

	return output_voltaget;
}

double power_get_output_current()
{
	double output_current = 0.0;
	
	switch (g_power_type)
	{
		case GOS_500W:
			output_current = powerGosIout_read();
			break;
		case YG_500W:
			output_current = yg_outputCurrent();
			break;
		default:
			break;
	}

	return output_current;
}

double power_get_output_power()
{
	double output_power = 0.0;
	
	switch (g_power_type)
	{
		case GOS_500W:
			output_power = powerGosPout_read();
			break;
		case YG_500W:
			output_power = yg_outputPower();
			break;
		default:
			break;
	}

	return output_power;
} 

double power_get_input_voltage()
{
	double input_voltage = 0.0;

	switch(g_power_type)
	{
		case YG_500W:
			input_voltage = yg_inputVoltage();
			break;
		default:
			break;
	}

	return input_voltage;
}

double power_get_input_current()
{
	double input_current = 0.0;

	switch (g_power_type)
	{
		case YG_500W:
			input_current = yg_inputCurrent();
			break;
		
		default:
			break;
	}

	return input_current;
}