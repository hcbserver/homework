#include <SoftwareSerial.h>
#include "edp.c"

#define KEY  "YTBLt2PRVqho0SnXCZeqr7pAKwg="    //APIkey
#define ID   "505365683"                          //设备ID

// 串口
#define _baudrate   115200         //波特率设置
#define _rxpin      12                  //  输入口设置
#define _txpin      10                 //输出口设置
#define DBG_UART    Serial   //调试（DEBUG）打印串口

SoftwareSerial WIFI_UART( _rxpin, _txpin ); // WIFI模块串口
edp_pkt *pkt;

/*
* doCmdOk          该函数主要用于自动建立tcp连接 实际上在Setup中代替了手动的AT指令操作
* 发送命令至模块，从回复中获取期待的关键字     
* keyword: 所期待的关键字
* 成功找到关键字返回true，否则返回false
*/
bool doCmdOk(String data, char *keyword)             //个人认为该函数写的比较巧妙，也不难理解，推荐大家仔细阅读
{
  bool result = false;
  if (data != "")   //对于tcp连接命令，直接等待第二次回复
  {
    WIFI_UART.println(data);  //发送AT指令
    DBG_UART.print("SEND: ");
    DBG_UART.println(data);
  }
  if (data == "AT")   //检查模块存在
    delay(2000);
  else
    while (!WIFI_UART.available());  // 等待模块回复

  delay(200);
  if (WIFI_UART.find(keyword))   //返回值判断
  {
    DBG_UART.println("do cmd OK");
    result = true;
  }
  else
  {
    DBG_UART.println("do cmd ERROR");
    result = false;
  }
  while (Serial.available()) Serial.read();   //清空串口接收缓存
  delay(500); //指令时间间隔
  return result;
}


void setup()
{
  char buf[100] = {0};
  int tmp;
  
/*                    LED初始化                     */


  pinMode(13, OUTPUT);    //区域一
  pinMode(11,OUTPUT);      //区域二
pinMode(9,OUTPUT);         //区域三
pinMode(2,OUTPUT);        //区域四
pinMode(3,OUTPUT);        //区域五
pinMode(6,OUTPUT);        //区域六
  pinMode(8,OUTPUT);      //区域七
  pinMode(5, OUTPUT);    //区域八

/*     Wifi模块初始化 串口初始化           */
  WIFI_UART.begin(_baudrate);
  DBG_UART.begin( 9600);
  WIFI_UART.setTimeout(3000);    //设置find超时时间
  delay(3000);
  DBG_UART.println("hello world!");    //检验串口工作

  delay(2000);

  /*    连接wifi   建立tcp连接 开启透传模式   */
  while (!doCmdOk("AT", "OK"));
  while (!doCmdOk("AT+CWMODE=3", "OK"));            //工作模式
  while (!doCmdOk("AT+CWJAP=\"HUAWEI\",\"88888888\"", "OK"));
  while (!doCmdOk("AT+CIPSTART=\"TCP\",\"183.230.40.39\",876", "OK"));
  while (!doCmdOk("AT+CIPMODE=1", "OK"));           //透传模式
 doCmdOk("AT+CIPSEND", ">")           ;  //开始发送    这里本来有循环，但是似乎因为模块已经建立连接的话将不会有回复，将导致程序陷入死循环，所以我直接去掉了while循环，可酌情加上再进行调试。
}

void loop()
{
  static int edp_connect = 0;
  bool trigger = false;
  edp_pkt rcv_pkt;
  unsigned char pkt_type;
  int i, tmp;
  char num[10];

  /* 建立EDP 连接 */
  if (!edp_connect)
  {
    while (WIFI_UART.available()) WIFI_UART.read(); //清空串口接收缓存
    packetSend(packetConnect(ID, KEY));             //发送EPD连接包
    while (!WIFI_UART.available());                 //等待EDP连接应答
    if ((tmp = WIFI_UART.readBytes(rcv_pkt.data, sizeof(rcv_pkt.data))) > 0 )
    {
      rcvDebug(rcv_pkt.data, tmp);
      if (rcv_pkt.data[0] == 0x20 && rcv_pkt.data[2] == 0x00 && rcv_pkt.data[3] == 0x00)
      {
        edp_connect = 1;
        DBG_UART.println("EDP connected.");
      }
      else
        DBG_UART.println("EDP connect error.");
    }
    packetClear(&rcv_pkt);
  }

  while (WIFI_UART.available())
  {
    readEdpPkt(&rcv_pkt);
    if (isEdpPkt(&rcv_pkt))
    {
      pkt_type = rcv_pkt.data[0];
      switch (pkt_type)
      {
        case CMDREQ:
          char edp_command[50];
          char edp_cmd_id[40];
          long id_len, cmd_len, rm_len;
          char datastr[20];
          char val[10];
          memset(edp_command, 0, sizeof(edp_command));   //清空edp_command的值
          memset(edp_cmd_id, 0, sizeof(edp_cmd_id));     //清空edp_command的值
          edpCommandReqParse(&rcv_pkt, edp_cmd_id, edp_command, &rm_len, &id_len, &cmd_len);
          DBG_UART.print("rm_len: ");
          DBG_UART.println(rm_len, DEC);
          delay(10);
          DBG_UART.print("id_len: ");
          DBG_UART.println(id_len, DEC);
          delay(10);
          DBG_UART.print("cmd_len: ");
          DBG_UART.println(cmd_len, DEC);
          delay(10);
          DBG_UART.print("id: ");
          DBG_UART.println(edp_cmd_id);
          delay(10);
          DBG_UART.print("cmd: ");
          DBG_UART.println(edp_command);                  //将onenet数据包解析打印出来，便于调试观察

       
          sscanf(edp_command, "%[^:]:%s", datastr, val);     //这里的datastr是edp_command中 “：”之前的部分 val则是之后的部分   如edp_command="123:456" 则 datastr="123"   val="456"
                                                               //这么写的原因取决于onenet命令格式         

/*                 LED灯的控制代码                   */
                                                               
          if (atoi(val)==1)           //提取val中的数字并判断
           Openall();
          if(atoi(val)==0)
          Closeall();
          switch(atoi(val))
          {
            case 4:OPEN1();
            break;
            case 8:OPEN2();
            break;
            case 12:OPEN3();
            break;
            case 16:OPEN4();
            break;
            case 20:OPEN5();
            break;
            case 24:OPEN6();
            break;
            case 28:OPEN7();
            break;
            case 32:OPEN8();
            break;
            }
           if(atoi(edp_command)==1)
           {
            do
            {
              Mode1();
              Mode2();
            }while(!WIFI_UART.available());
            }
     if(atoi(edp_command)==2)
           {
            do
            {
              Mode3();
              Mode4();
            }while(!WIFI_UART.available());
            }
         if(atoi(edp_command)==3)
           {
            do
            {
              Mode5();
            }while(!WIFI_UART.available());
            } 
         if(atoi(edp_command)==4)
           {
            do
            {
              Mode6();
            }while(!WIFI_UART.available());
           }
          
/*                               将新数据值上传至数据流                      */
          packetSend(packetDataSaveTrans(NULL, datastr, val)); 
          break;
        default:
          DBG_UART.print("unknown type: ");
          DBG_UART.println(pkt_type, HEX);
          break;
      }
    }
    //delay(4);
  }
  if (rcv_pkt.len > 0)                          //清空接收的数据 在下一次循环中再次使用
    packetClear(&rcv_pkt);
  delay(150);
}

/*
* readEdpPkt
* 从串口缓存中读数据到接收缓存
*/
bool readEdpPkt(edp_pkt *p)
{
  int tmp;
  if ((tmp = WIFI_UART.readBytes(p->data + p->len, sizeof(p->data))) > 0 )
  {
    rcvDebug(p->data + p->len, tmp);
    p->len += tmp;
  }
  return true;
}

/*
* packetSend
* 将待发数据发送至串口，并释放到动态分配的内存
*/
void packetSend(edp_pkt* pkt)
{
  if (pkt != NULL)
  {
    WIFI_UART.write(pkt->data, pkt->len);    //串口发送
    WIFI_UART.flush();
    free(pkt);              //回收内存
  }
}

void rcvDebug(unsigned char *rcv, int len)
{
  int i;

  DBG_UART.print("rcv len: ");
  DBG_UART.println(len, DEC);
  for (i = 0; i < len; i++)
  {
    DBG_UART.print(rcv[i], HEX);
    DBG_UART.print(" ");
  }
  DBG_UART.println("");
}

/*         以下函数为LED闪烁模式函数，便于在loop中的直接调用                */
/*      开启所有灯             */
void Openall()
{
  digitalWrite(13, HIGH);    //区域一
           digitalWrite(11,HIGH);      //区域二
           digitalWrite(9,HIGH);         //区域三
           digitalWrite(2,HIGH);        //区域四
           digitalWrite(3,HIGH);        //区域五
           digitalWrite(6,HIGH);        //区域六
           digitalWrite(8,HIGH);      //区域七
           digitalWrite(5, HIGH);    //区域八
  }
  /*           关闭所有灯     */
void Closeall()
{
   digitalWrite(13, LOW);    //区域一
           digitalWrite(11,LOW);      //区域二
           digitalWrite(9,LOW);         //区域三
           digitalWrite(2,LOW);        //区域四
           digitalWrite(3,LOW);        //区域五
           digitalWrite(6,LOW);        //区域六
           digitalWrite(8,LOW);      //区域七
           digitalWrite(5, LOW);    //区域八

  }
  
  void Mode1()
  {
           digitalWrite(13, HIGH);
            digitalWrite(8,LOW);
     delay(200);   
           digitalWrite(11,HIGH);
           digitalWrite(5, LOW);
     delay(200)      ;
           digitalWrite(9,HIGH);
           digitalWrite(13,LOW);       
    delay(200);  
           digitalWrite(2,HIGH);
           digitalWrite(11,LOW);
    delay(200);
           digitalWrite(3,HIGH);
           digitalWrite(9,LOW);
    delay(200)    ;   
           digitalWrite(6,HIGH);
           digitalWrite(2,LOW);    
    delay(200)    ;    
           digitalWrite(8,HIGH);
           digitalWrite(3,LOW);
    delay(200)    ;       
           digitalWrite(5, HIGH);
           digitalWrite(6,LOW);
    delay(200)    ;   
    }
   void Mode2()
  {
     digitalWrite(5, HIGH);
            digitalWrite(11,LOW);
     delay(200);   
           digitalWrite(8,HIGH);
           digitalWrite(13, LOW);
     delay(200)      ;
           digitalWrite(6,HIGH);
           digitalWrite(5,LOW);       
    delay(200);  
           digitalWrite(3,HIGH);
           digitalWrite(8,LOW);
    delay(200);
           digitalWrite(2,HIGH);
           digitalWrite(6,LOW);
    delay(200)    ;   
           digitalWrite(9,HIGH);
           digitalWrite(3,LOW);    
    delay(200)    ;    
           digitalWrite(11,HIGH);
           digitalWrite(2,LOW);
    delay(200)    ;       
           digitalWrite(13, HIGH);
           digitalWrite(9,LOW);
    delay(200)    ;    
    }
  void Mode3()
    {    
           digitalWrite(3,HIGH);
           digitalWrite(13,HIGH);
           digitalWrite(5,LOW);
           digitalWrite(2,LOW);
        delay(200);
         digitalWrite(6,HIGH);
           digitalWrite(11,HIGH);
           digitalWrite(3,LOW);
           digitalWrite(13, LOW);       
    delay(200);  
         digitalWrite(8,HIGH);
           digitalWrite(9, HIGH);
           digitalWrite(6, LOW);
            digitalWrite(11,LOW);
     delay(200)      ;
         digitalWrite(5, HIGH);
            digitalWrite(2,HIGH);
            digitalWrite(8,LOW);
           digitalWrite(9,LOW);
     delay(200);   
      }
    void Mode4()
   {
    digitalWrite(5, HIGH);
            digitalWrite(2,HIGH);
            digitalWrite(3,LOW);
           digitalWrite(13,LOW);
     delay(200);   
           digitalWrite(8,HIGH);
           digitalWrite(9, HIGH);
           digitalWrite(5, LOW);
            digitalWrite(2,LOW);
     delay(200)      ;
           digitalWrite(6,HIGH);
           digitalWrite(11,HIGH);
           digitalWrite(8,LOW);
           digitalWrite(9, LOW);       
    delay(200);  
           digitalWrite(3,HIGH);
           digitalWrite(13,HIGH);
           digitalWrite(6,LOW);
           digitalWrite(11,LOW);
        delay(200);
      }
      void OPEN1()
{
     digitalWrite(13, HIGH);
 digitalWrite(11,LOW);
  digitalWrite(9,LOW);
  digitalWrite(2,LOW); 
  digitalWrite(3,LOW); 
  digitalWrite(6,LOW);
   digitalWrite(8,LOW);
    digitalWrite(5, LOW);
}
void OPEN2()
{
   digitalWrite(13, HIGH);
 digitalWrite(11,HIGH);
  digitalWrite(9,LOW);
  digitalWrite(2,LOW); 
  digitalWrite(3,LOW); 
  digitalWrite(6,LOW);
   digitalWrite(8,LOW);
    digitalWrite(5, LOW);
 } 
 void OPEN3()
 {
  digitalWrite(13, HIGH);
 digitalWrite(11,HIGH);
  digitalWrite(9,HIGH);
  digitalWrite(2,LOW); 
  digitalWrite(3,LOW); 
  digitalWrite(6,LOW);
   digitalWrite(8,LOW);
    digitalWrite(5, LOW);
  }
  void OPEN4()
  {
   digitalWrite(13, HIGH);
 digitalWrite(11,HIGH);
  digitalWrite(9,HIGH);
  digitalWrite(2,HIGH); 
  digitalWrite(3,LOW); 
  digitalWrite(6,LOW);
   digitalWrite(8,LOW);
    digitalWrite(5, LOW);
    }
    void OPEN5()
    {
          digitalWrite(13, HIGH);
 digitalWrite(11,HIGH);
  digitalWrite(9,HIGH);
  digitalWrite(2,HIGH); 
  digitalWrite(3,HIGH); 
  digitalWrite(6,LOW);
   digitalWrite(8,LOW);
    digitalWrite(5, LOW);
      }
      void OPEN6()
      {
             digitalWrite(13, HIGH);
 digitalWrite(11,HIGH);
  digitalWrite(9,HIGH);
  digitalWrite(2,HIGH); 
  digitalWrite(3,HIGH); 
  digitalWrite(6,HIGH);
   digitalWrite(8,LOW);
    digitalWrite(5, LOW);
        }
        void OPEN7()
        {
  digitalWrite(13, HIGH);
 digitalWrite(11,HIGH);
  digitalWrite(9,HIGH);
  digitalWrite(2,HIGH); 
  digitalWrite(3,HIGH); 
  digitalWrite(6,HIGH);
   digitalWrite(8,HIGH);
    digitalWrite(5, LOW);
          }
          void OPEN8()
          {
   digitalWrite(13, HIGH);
 digitalWrite(11,HIGH);
  digitalWrite(9,HIGH);
  digitalWrite(2,HIGH); 
  digitalWrite(3,HIGH); 
  digitalWrite(6,HIGH);
   digitalWrite(8,HIGH);
    digitalWrite(5, HIGH);  
            }
void Mode5()
      {
         for(int i=0;i<255;i=i+10)
         {analogWrite(13, i);    //区域一
           analogWrite(11,i);      //区域二
           analogWrite(9,i);         //区域三
           analogWrite(2,i);        //区域四
           analogWrite(3,i);        //区域五
           analogWrite(6,i);        //区域六
           analogWrite(8,i);      //区域七
           analogWrite(5, i);    //区域八
           delay(200);
         }
          for(int i=254;i>0;i=i-10)
         { analogWrite(13, i);    //区域一
           analogWrite(11,i);      //区域二
           analogWrite(9,i);         //区域三
           analogWrite(2,i);        //区域四
           analogWrite(3,i);        //区域五
           analogWrite(6,i);        //区域六
           analogWrite(8,i);      //区域七
           analogWrite(5, i);    //区域八
           delay(200);
         }
       }
void Mode6()
       {
        static int a;
        a=rand()%9;
        switch(a)
         {
          case 1:
          digitalWrite(13, HIGH);  
           digitalWrite(11,LOW);     
           digitalWrite(9,LOW);      
           digitalWrite(2,LOW);      
           digitalWrite(3,LOW);     
           digitalWrite(6,LOW);        
           digitalWrite(8,LOW);      
           digitalWrite(5, LOW);
           break;
           case 2:
          digitalWrite(13, LOW);  
           digitalWrite(11,HIGH);     
           digitalWrite(9,LOW);      
           digitalWrite(2,LOW);      
           digitalWrite(3,LOW);     
           digitalWrite(6,LOW);        
           digitalWrite(8,LOW);      
           digitalWrite(5, LOW);
           break;
           case 3:
          digitalWrite(13, LOW);  
           digitalWrite(11,LOW);     
           digitalWrite(9,HIGH);      
           digitalWrite(2,LOW);      
           digitalWrite(3,LOW);     
           digitalWrite(6,LOW);        
           digitalWrite(8,LOW);      
           digitalWrite(5, LOW);
           break;
           case 4:
          digitalWrite(13, LOW);  
           digitalWrite(11,LOW);     
           digitalWrite(9,LOW);      
           digitalWrite(2,HIGH);      
           digitalWrite(3,LOW);     
           digitalWrite(6,LOW);        
           digitalWrite(8,LOW);      
           digitalWrite(5, LOW);
           break;
           case 5:
          digitalWrite(13, LOW);  
           digitalWrite(11,LOW);     
           digitalWrite(9,LOW);      
           digitalWrite(2,LOW);      
           digitalWrite(3,HIGH);     
           digitalWrite(6,LOW);        
           digitalWrite(8,LOW);      
           digitalWrite(5, LOW);
           break;
           case 6:
          digitalWrite(13, LOW);  
           digitalWrite(11,LOW);     
           digitalWrite(9,LOW);      
           digitalWrite(2,LOW);      
           digitalWrite(3,LOW);     
           digitalWrite(6,HIGH);        
           digitalWrite(8,LOW);      
           digitalWrite(5, LOW);
           break;
           case 7:
          digitalWrite(13, LOW);  
           digitalWrite(11,LOW);     
           digitalWrite(9,LOW);      
           digitalWrite(2,LOW);      
           digitalWrite(3,LOW);     
           digitalWrite(6,LOW);        
           digitalWrite(8,HIGH);      
           digitalWrite(5, LOW);
           break;
           case 8:
          digitalWrite(13, LOW);  
           digitalWrite(11,LOW);     
           digitalWrite(9,LOW);      
           digitalWrite(2,LOW);      
           digitalWrite(3,LOW);     
           digitalWrite(6,LOW);        
           digitalWrite(8,LOW);      
           digitalWrite(5, HIGH);
           break;
           default:
           break;
         }
         delay(200);
        }
