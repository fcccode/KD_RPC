#include "stdafx.h"
#include "../public/debug.h"
#include "Data_Format_Client.h"
#include "../public/StateManage.h"




//客户端 异步状态管理
extern CAsyncStateManage* g_pasm;
//客户端 同步状态管理
extern CSyncStateManage* g_pssm;



CData_Format_Client::CData_Format_Client(void)
{
}


CData_Format_Client::~CData_Format_Client(void)
{
}

struct st_async_callback_argv_return
{
	char* data;
	LONG64 data_len;
};

typedef void (*callback_real)(char*,LONG64); 
unsigned int WINAPI  CData_Format_Client::Client_FlowToFormat_Execute(LPVOID lp) //由它来自行区分是否异步,并走不同的流程。
{
	int ret_value = 0;//也就是错误码哟

	st_thread_FlowToFormat_Excute_argvs* p=(st_thread_FlowToFormat_Excute_argvs*) lp;

	if (nullptr == p)
	{
		OutputDebug("Client_FlowToFormat_Execute:Input pointer==nullptr");
		return -1;
	}

	if (nullptr == p->flow)
	{
		OutputDebug("Client_FlowToFormat_Execute:Input struct ponter==nullptr");
		return -2;
	}

	st_data_flow* pTmp_Flow = (st_data_flow*)p->flow;

	if (pTmp_Flow->length_of_this_struct!=p->flow_len)
	{
		OutputDebug("Client_FlowToFormat_Execute:Input struct Format Error:flow_LEN!=length_of_this_struct");
		return -0x10;
	}

	//copy 指针
	const st_data_flow* pFlowBase =(st_data_flow*) new char[p->flow_len]();

	LONG64 stat = memcpy_s((char*)pFlowBase,p->flow_len,p->flow,p->flow_len);
	if (stat)
	{
		throw("memcpy_s return err.");
	}

	//通知 拷贝完成
	if(0==SetEvent(p->hdEvent_Copy_Finish))
	{
		throw("Event is invalid");
	}
	//让指针们无效
	pTmp_Flow = nullptr;
	p = nullptr;//p无效了。
	//////////////////////////////////////////////////////////////////////////
	/*Flow2Format*/

	char* pArgvCall  = nullptr;
	CSafeQueueAutoPointerManage* queue_memory_manage = nullptr;
	
	LONG64 real_len = 0;//Format长度
	try
	{
		real_len = CDataFormat::Flow2Format((char*)pFlowBase,pFlowBase->length_of_this_struct,nullptr,0,nullptr,nullptr,nullptr);
		if (real_len==0)
		{
			;//没有指针数据传回来,可能是同步函数中只有返回值的
		}
		else
		{
			pArgvCall = new char[real_len]();
			queue_memory_manage = new CSafeQueueAutoPointerManage();
		}

		CDataFormat::Flow2Format((char*)pFlowBase,pFlowBase->length_of_this_struct,pArgvCall,real_len,queue_memory_manage,nullptr,nullptr);
	}
	catch (char* string)
	{
		throw(string);//内部结构错误
	}
	catch (int errCode)//文件格式错误忽略之
	{
		OutputDebug("Flow Format Err,code:0x%x",errCode);
		return errCode;
	}
	//////////////////////////////////////////////////////////////////////////

	/*
	对方发回的数据可能性：
	1. 格式错误,及其他异常
	2. ID_proc在：同步栈搜索，异步栈搜索
	知道是哪种
	3. 同步栈
	- 标准模式：一堆:反正就是直接去覆盖修改咯。
	- 快速模式：空指针，代表不需要修改。
	3. 异步栈
	只有一个指针和长度
	*/

	const LONG m_ID_proc = pFlowBase->ID_proc;
	const bool bAsync = pFlowBase->async;

	try
	{
		if (bAsync)
		{
			const LONG64 AsyncArgvLimit = 1;
			if (AsyncArgvLimit!=pFlowBase->number_Of_Argv_Pointer)
			{
				OutputDebug("Warming:ID_proc:0x%x in async,\
					number_Of_Argv_Pointer number fault is 0x%d.",pFlowBase->number_Of_Argv_Pointer);
			}
			FARPROC m_callback;
			g_pasm->findAndPop(m_ID_proc,m_callback);
			/*
			1. 参数必须只有一个
			*/
			callback_real Real_Call = (callback_real)m_callback;

			try
			{
				Real_Call(((st_async_callback_argv_return*)pArgvCall)->data,((st_async_callback_argv_return*)pArgvCall)->data_len);
			}
			catch(...)
			{
				OutputDebug("Client Call_Back happen Exception.Ignoring...");
			}


		}
		else//同步
		{
			LONG64* p_ret_to_wait_sync_value	=	nullptr;
			char* pFormat	=	nullptr;
			LONG64 PointerNumber= 0;
			HANDLE hdEvent = INVALID_HANDLE_VALUE;

			p_ret_to_wait_sync_value = g_pssm->findAndPop(m_ID_proc,pFormat,PointerNumber,hdEvent);
			
			if (PointerNumber!=pFlowBase->number_Of_Argv_Pointer)
			{
				OutputDebug("Client_FlowToFormat_Execute:PointerNumber!=pFlowBase->number_Of_Argv_Pointer");
				goto Recive_Data_Client_End;
			}
			//Format 将结果上去
			for (LONG64 i=0,offset=0;i<PointerNumber;++i,offset+=sizeof(LONG64)*2)
			{
				
				char* pOldData = (char*)*(LONG64*)(pFormat+offset);
				char* pNewData = (char*)*(LONG64*)(pArgvCall+offset);
				if (pNewData!=nullptr&&pOldData==nullptr)
				{
					OutputDebug("Client_FlowToFormat_Execute:Data Format Error,Old Pointer=0,New Pointer!=0,return.");
					goto Recive_Data_Client_End;
				}

				if (pNewData==nullptr&&pOldData==nullptr)
				{
					if (0!=*(LONG64*)(pArgvCall+offset+sizeof(LONG64)))
					{
						OutputDebug("Client_FlowToFormat_Execute:Data Format Error,New Pointer=nullptr,Pointer_len!=0,return.");
						goto Recive_Data_Client_End;
					}
					else
					{
						continue;;
					}
				}
				else if (pNewData==nullptr&&pOldData!=nullptr)
				{
					if (QUICK_FLOW_MODE == pFlowBase->work_type)
					{
						continue;
					}
					else if (STANDARD_FLOW_MODE == pFlowBase->work_type)
					{
						OutputDebug("Client_FlowToFormat_Execute:Format err:STANDARD_FLOW_MODE like QUICK_FLOW_MODE?");
					}
					else
					{
						OutputDebug("Client_FlowToFormat_Execute:unexpected work_type:0x%x",pFlowBase->work_type);
					}
					goto Recive_Data_Client_End;
					
				}
				else//两者都有数据
				{
					//检查长度一致的情况
					if(*(LONG64*)(pFormat+offset+sizeof(LONG64))!=*(LONG64*)(pArgvCall+offset+sizeof(LONG64)))
					{
						OutputDebug("Client_FlowToFormat_Execute:New Data and Old Data len differ.");
						goto Recive_Data_Client_End;
					}
					else
					{
						LONG64 len = *(LONG64*)(pFormat+offset+sizeof(LONG64));//会出问题

						LONG64 stat = memcpy_s(pOldData,len,pNewData,len);
						
						if (stat)
						{
							throw("memcpy_s p_format_end_data return err.");
						}
						continue;
					}
					
				}
			}//-end-for
			

			*p_ret_to_wait_sync_value = pFlowBase->return_value;
			if(0==SetEvent(hdEvent))
			{
				OutputDebug("Client_FlowToFormat_Execute:SetEvent invalid");
				goto Recive_Data_Client_End;
			}


		}
	}
	catch(...)
	{
		//该ID_proc在对应序列找不到
		OutputDebug("Warming:ID_proc:0x%x in async:%s,do'nt find record.",m_ID_proc,bAsync?"True":"False");
		goto Recive_Data_Client_End;
	}



	ret_value = 1;

Recive_Data_Client_End:


	delete(pArgvCall);
	pArgvCall = nullptr;
	delete(queue_memory_manage);
	queue_memory_manage = nullptr;

	ret_value-=1;

	return ret_value;
}