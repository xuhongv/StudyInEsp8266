download:
AT firmware which can upgrade by "AT+CIUPDATE"
boot_v1.2+.bin      	0x00000
user1.1024.new.2.bin  	0x01000
blank.bin           	0x7e000 & 0xfe000

or
AT firmware which can not upgrade by "AT+CIUPDATE", but fit in 4Mbit flash
eagle.flash.bin		0x00000
eagle.irom0text.bin	0x40000
blank.bin		0x3e000 & 0x7e000

*NOTICE*:
1.If you use Esp Flash Download Tool, please MAKE SURE to choose FLASH SIZE to 8Mbit.
2.UPDATE is not supported in non-boot mode

Update steps
1.Make sure TE(terminal equipment) is in sta or sta+ap mode
ex. AT+CWMODE=3
    OK
    
    AT+RST

2.Make sure TE got ip address
ex. AT+CWJAP="ssid","12345678"
    OK

    AT+CIFSR
    192.168.1.134

3.Let's update
ex. AT+CIUPDATE
    +CIPUPDATE:1    found server
    +CIPUPDATE:2    connect server
    +CIPUPDATE:3    got edition
    +CIPUPDATE:4    start start

    OK

note. If there are mistakes in the updating, then break update and print ERROR.