1.源码编译方式
  1. cd app/
  2. ./gen_misc.sh 
  
2.Gagent库版本
  04020024
  
3.烧录固件
  esp_init_data_default.bin          0x3fc000
  blank.bin                          0x3fe000
  boot_v1.6.bin                      0x00000
  user1.4096.new.6.bin               0x01000
  
  选项：CrystalFreq=26M  SPI_SPEED=40MHz SPI_MODE=QIO FLASH_SIZE=32Mbit-C1, 其他默认，串口115200
  进入uart烧录模式后，点击start下载即可！
  
4.OTA测试
	OTA固件版本号位置：gizwits_product.h
		#define SDK_VERSION                             "25"    //OTA固件版本号,必须为两位数, 默认为当前Gagent库版本号
	MAC：
		查看云端产品管理->运行状态->在线设备详情->设备MAC
	注意：
		1.编译固件时的Makefile与烧录工具的设置：
			"FLASH SIZE" : 32Mbit-C1
			"SPI MODE" : QIO
		2.推送的“OTA固件版本号”必须大于正工作的软件版本。
		3.固件类型：WiFi 推送方式：v4.1