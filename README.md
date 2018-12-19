# 导论创新作业

# 库文件
    edp.c 用了EDP方法实现端到端的通讯
# Ardiuno代码
### 摘取一部分有趣的代码
		__.ino
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
### 由于代码太长所以就不全部贴上了，代码都传到文件夹里了
    

