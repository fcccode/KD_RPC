// UserProc.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string.h>
#include <windows.h>
#include "../public/debug.h"

/*
解析头文件也很简单，什么时候解析？
总之初始化程序运行起来会向服务器请求数据获得头文件并写在本地(或者内存)。

我们运行时读取这个头文件，完成获取以下内容：
序号，函数名(初始化时用)， 异步情况(区别处理)，参数信息（用做解析）

1. 方案1添加结构进去
2. 方案2以无效的变量进去
3. 方案3以将信息附加到结构体名称上-hack语法...
用注释的方式表示序号

所有的函数在客户端都是相同的，除了它们每个函数有一个独特的序号。

序号-函数名-异步情况-参数信息

序号是不是自动生成的?可以，也可以手动

序号：方便传输，和字符串一一对应。否则就得用char*函数名和len来表示了.
我觉得这种字符串不在网络上传输，就是函数调用和函数名的一种分离，是好的。

能否动态生成自由参数的调用呢？eg：
int MessageBoxA(HANDLE,char*,char*,int);
答案是不可以：因为字符串多长我不知道。所以还是需要修改参数。
所以不如让其参数全部统一到我们的新格式来。

*/

//!!序号!!异步情况!!
//!!SN!!1234
//!!async!!0
struct st_argv_MessageBoxA
{
	_In_opt_ HWND hWnd;
	_In_opt_ LPCSTR lpText;
	int lpText_len;
	_In_opt_ LPCSTR lpCaption;
	int lpCaption_len;
	_In_ UINT uType;

};

struct st_argv_QueryWeather
{
	const char*	string;
	int			string_len;

};


struct st_argv_Add
{
	int firstNumber;
	int secondNumber;
};
//int Add(PVOID pStruct,FARPROC callBack)





typedef void (_cdecl* RPC_CallBack)(const char*,int len);//RPC回调原形

typedef  int (__cdecl *int_FUN_ADD)(int,int);
typedef  int (__cdecl *int_FUN_REAL)(st_argv_MessageBoxA* p,RPC_CallBack callBack);

typedef  int (__cdecl *int_FUN_Standard)(char* ,RPC_CallBack callBack);//标准

//typedef  int (__cdecl *int_FUN_Standard)(st_argv_QueryWeather* ,RPC_CallBack callBack);//标准




void WeatherCallBack(const char*,int len);

/*
添加框架函数的步骤：
1. 按照规范添加服务器导出函数
2. 添加本地调用函数

3. 添加Client函数注册代码
4. 添加Service函数注册代码


*/

void Test4Callback(const char* cp,int len);

int _tmain(int argc, _TCHAR* argv[])
{
	bool Yuan = false;

	if (Yuan)
	{


		HMODULE  test = LoadLibraryA("ServiceDLL.dll");

		FARPROC fpAdd = GetProcAddress(test,"Add");		//导入算术测试
		FARPROC fpReal = GetProcAddress(test,"Real");	//导入MessageBoxA测试
		FARPROC fpQueryWeather = GetProcAddress(test,"QueryWeather");//导入回调测试

		int_FUN_ADD funAdd =(int_FUN_ADD)fpAdd;
		int_FUN_REAL funReal =(int_FUN_REAL)fpReal;
		int_FUN_Standard funQueryWeather =(int_FUN_Standard)fpQueryWeather;



		int AddResult  = funAdd(5,6);


		st_argv_MessageBoxA tmp_Message;
		tmp_Message.hWnd = NULL;
		tmp_Message.lpText = "Hi";
		tmp_Message.lpCaption = "title";
		tmp_Message.uType = MB_OK;
		int RealResult = funReal(&tmp_Message,nullptr);


		st_argv_QueryWeather tmp_we;
		tmp_we.string = "It's raining today";
		tmp_we.string_len = strlen(tmp_we.string);

		funQueryWeather((char*)&tmp_we,WeatherCallBack);//


		Sleep(6000);
	}
	else
	{
		/*
		1. 无指针参数：检查返回值
		*/
		HMODULE  test = LoadLibraryA("Client_fack.dll");
		{
			OutputDebug(L"Test1:Start:无指针参数:检查返回值.\r\n");


			if (test==0)
			{
				MessageBoxA(0,"不存在dll","",MB_OK);
			}
			FARPROC fpAdd = GetProcAddress(test,"Add");		//导入算术测试
			if (fpAdd==nullptr)
			{
				MessageBoxA(0,"存在dll,但不存在指定函数","",MB_OK);

			}


			int_FUN_Standard funAdd =(int_FUN_Standard)fpAdd;

			st_argv_Add tmp_Message;
			tmp_Message.firstNumber = 5;
			tmp_Message.secondNumber= 6;
			int RealResult = funAdd((char*)&tmp_Message,nullptr);
			if (RealResult==11)
			{
				OutputDebug(L"Test1:Pass:\r\n");//MessageBoxA(0,"测试通过","",MB_OK);
			}
			else
			{
				OutputDebug(L"Test1:Fault:");
				MessageBoxA(0,"测试返回值错误","",MB_OK);
			}
			//FreeLibrary(test);
			//test = 0;

		}
		/*
		2. 有指针参数：检查返回值和指针修改情况
		- 标准模式
		- 快速模式
		*/
		{

		}
		/*
		- 异步：
		1. 无回调
		*/
		{
			OutputDebug(L"Test3:Start:异步.无回调.\r\n");

			if (test==0)
			{
				MessageBoxA(0,"不存在dll","",MB_OK);
			}
			FARPROC fpAdd = GetProcAddress(test,"Add_Async_NoCallback");		//导入算术测试
			if (fpAdd==nullptr)
			{
				MessageBoxA(0,"存在dll,但不存在指定函数","",MB_OK);

			}


			int_FUN_Standard funAdd =(int_FUN_Standard)fpAdd;

			st_argv_Add tmp_Message;
			tmp_Message.firstNumber = 5;
			tmp_Message.secondNumber= 6;
			int RealResult = funAdd((char*)&tmp_Message,nullptr);
			if (RealResult==true)
			{
				;
			}
			else
			{
				OutputDebug(L"Test3:Fault:");
				MessageBoxA(0,"测试3返回值错误","",MB_OK);
			}
			//FreeLibrary(test);
			
		}
		/*
		2. 有回调：检查回调信息情况
		*/		
		{
			OutputDebug(L"Test4:Start:异步.回调.\r\n");

			if (test==0)
			{
				MessageBoxA(0,"不存在dll","",MB_OK);
			}
			FARPROC fpAdd = GetProcAddress(test,"Add_Async_NoCallback");		//导入算术测试
			if (fpAdd==nullptr)
			{
				MessageBoxA(0,"存在dll,但不存在指定函数","",MB_OK);

			}


			int_FUN_Standard funAdd =(int_FUN_Standard)fpAdd;

			st_argv_Add tmp_Message;
			tmp_Message.firstNumber = 5;
			tmp_Message.secondNumber= 6;
			int RealResult = funAdd((char*)&tmp_Message,Test4Callback);
			if (RealResult==true)
			{
				;
			}
			else
			{
				OutputDebug(L"Test4:Fault:");
				MessageBoxA(0,"测试4返回值错误","",MB_OK);
			}
			//FreeLibrary(test);
			
		}

	}
	Sleep(-1);

	return 0;
}

void Test4Callback(const char* cp,int len)
{
	char p[] = {'r','e','s','u','l','t','i','s',':','1','1','\0'};
	if (len!=(strlen(p)+1))
	{
		OutputDebug(L"Test4:Fault:length");
		return;
	}
	int stat = memcmp(p,cp,len);
	if (stat==0)
	{
		OutputDebug(L"Test4:Pass:");
	}
	else
	{
		OutputDebug(L"Test4:Fault:memcpy");
	}
	return;
}


void WeatherCallBack(const char* cp,int len)
{
	if (len==0)
	{
		return;
	}
	int destlen = len+1;
	char* p = new char[destlen]();
	strcpy_s(p,destlen,cp);
	MessageBoxA(0,p,"天气情况",MB_OK);

	return ;


}