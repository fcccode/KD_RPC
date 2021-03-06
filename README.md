# KD_RPC

我已经将它在今年(2018)年中嵌入了所在公司的客户端,在上千客户端中承担信息交互工作.

## 优势
- 使用网络和本地调用**Native**一致的使用体验.
- 不需要任何额外显式初始化工作
- 支持64位和32位互通讯


## 项目介绍：
主页:https://github.com/dalerkd/KD_RPC

代码量4k+

本项目开始于2017年9月8日

于2017年10月7日解决了已知的所有问题。达到可用阶段。

标志性测试通过于2017年9月29日星期五 20:06


## 功能
支持同步，异步调用。
支持指针数据传输，较大限度的提供符合C语言本地调用的网络调用体验。

未来支持：动态修改接口。但说实话，我觉得这样做的优先级不高。



## 工程
一共5个子工程：
- Client
客户端代理程序
- Service
服务端代理程序
- ServiceDLL
服务器上工作的用户模块
- UserProc
用户程序
- testEverythin
无用，这是我用来测试各种小技术的。



UserProc< - >Client< - >......网络......< - >Service< - >ServiceDLL


我已经添加了9种情况的**测试用例**。涵盖了绝大多数提供服务的情况。直接编译所有工程并启动UserProc即可测试.

采取和C语言常用用法一致调用方式,形如:
```
        st_argv_Add tmp_Message;
	tmp_Message.firstNumber = 5;
	tmp_Message.secondNumber= 6;
	LONG64 RealResult = funAdd((char*)&tmp_Message,nullptr);//两个参数-加法
        
        st_argv_test2 tmp_Message;
	tmp_Message.firstStr = nullptr;
	tmp_Message.firstStr_len = 0;
	tmp_Message.secondStr= "SecondStr";
	tmp_Message.secondStr_len= strlen(tmp_Message.secondStr)+1;
	tmp_Message.other_argv_c = 1;
	tmp_Message.f_f = (float)1.1;
	LONG64 RealResult = funAdd((char*)&tmp_Message,nullptr);//指针(字符串)作为参数
        
	st_argv_Add tmp_Message;
	tmp_Message.firstNumber = 5;
	tmp_Message.secondNumber= 6;
	LONG64 RealResult = funAdd((char*)&tmp_Message,Test4Callback);//两个参数-加法-将结果返回给回调Test4Callback

```



## 添加框架函数的步骤：
1. 按照规范添加服务器导出函数
2. 添加本地调用函数

3. 添加Client函数注册代码
4. 添加Service函数注册代码

### 详细步骤:如果你希望增加一个RPC函数
以下是所有需要添加的位置:
- UserProc.cpp
1. 如果你有指针请使用int_FUN_Standard:在其第一个参数放所有的参数,
指针必须是:指针1,数据1长度,指针2,数据2长度.其他参数.
如果你的函数没有指针就简单很多:
直接用就好了eg:1号函数Add(..)
2. 没有回调也必须用第二个参数为空来表示无回调.
- Client.cpp
增加一个需要增加一个假函数.
- ServiceDLL.h
导出服务函数
- ServiceDLL.cpp
真正的工作方,必须形如:
LONG64 StartBrowser(st_argv_StartBrowser* p, RPC_CallBack cb)
有回调时会在cb中,回调也是有两个参数:指针和它的长度.来表示所有的数据.
- Service.cpp
需要在Start()中添加服务函数的信息注册.
- Client->dllmain.cpp
需要添加服务函数的信息的注册.



## 用前需知

1. 如果你需要FreeLibrary(test)卸载模块，请先调用以下DLL接口来通知RPC自动合理释放空间和结束工作线程：
```
ExitAllWor

2. 建议将网络模块替换成你自己的高速模块。因为我为了方便使用了MailSlot网络这种天底下最慢的网络模块。
```


## 用途
- 支持同步和回调

- 支持指针数据修改
将指针作为数据传输给远程，支持远程主机将数据修改结果回传在同步下。
支持形如下面的调用形式：

```
struct Argvs
{
  char* data;
  char data_len;
  float other;
}
int Function(Argvs* pArgvs,FARRPOC Callback);
```

## 优势
- 和本地调用没有差别的使用体验

流畅的接口，不需要额外显式初始化工作。

- 接口动态更新-待增加

可以对接口功能能按照需要进行动态更新。


## 未来
- 更好的接口
  - 无缝替换
最新的计划是设计出能够无缝替换本地服务的框架:wink:这非常吊哈哈。
  - 类支持
  
- 网络
  - 多机协商
这才是RPC更广阔的天地，比如和爬虫结合。
  - 长连接保持


## 为什么设计该项目？
为了自我提升。
我最早听说RPC是在两年前。
后随系统架构学习的深入，了解到Windows，Linux在本地，远程调用方面的设计。

近年来较深入了解网络开发和前端技术。认识到网络应用的巨大作用。

[知乎:如何通过自学找到一份开发的工作？](https://www.zhihu.com/question/26421707)

发现有推荐设计RPC项目。将机器和机器相互连接起来。同样也是为了方便自己未来使用。因原团队解散，这次正有时间。
