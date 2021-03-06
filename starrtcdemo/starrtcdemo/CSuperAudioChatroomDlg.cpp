// CSuperAudioChatroomDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "starrtcdemo.h"
#include "CSuperAudioChatroomDlg.h"
#include "afxdialogex.h"
#include "CInterfaceUrls.h"
#include "CCreateLiveDialog.h"
// CSuperAudioChatroomDlg 对话框

IMPLEMENT_DYNAMIC(CSuperAudioChatroomDlg, CDialogEx)

CSuperAudioChatroomDlg::CSuperAudioChatroomDlg(CUserManager* pUserManager, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SUPER_AUDIO_CHATROOM, pParent)
{
	m_pUserManager = pUserManager;
	XHSuperRoomManager::addChatroomGetListListener(this);
	m_pXHSuperRoomManager = new XHSuperRoomManager(this);
	m_pSoundManager = new CSoundManager(this);

	m_pXHSuperRoomManager->setRtcMediaType(LIVE_MEDIA_TYPE_AUDIO_ONLY);
	m_pCurrentLive = NULL;
	m_pConfig = NULL;
}

CSuperAudioChatroomDlg::~CSuperAudioChatroomDlg()
{
	KillTimer(0);
	if (m_pXHSuperRoomManager != NULL)
	{
		delete m_pXHSuperRoomManager;
		m_pXHSuperRoomManager = NULL;
	}
}

void CSuperAudioChatroomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTCONTROL_SUPER_AUDIO_CHATROOM_LIST, m_List);
	DDX_Control(pDX, IDC_STATIC_SUPER_AUDIO_CHATROOM_NAME, m_SuperAudioCharroomName);
	DDX_Control(pDX, IDC_LIST_SUPER_AUDIO_CHATROOM_HISTORY_MSG, m_MsgList);
	DDX_Control(pDX, IDC_EDIT_SUPER_AUDIO_CHATROOM_SEND_MSG, m_SendMsg);
	DDX_Control(pDX, IDC_BUTTON_SEND_SUPER_AUDIO, m_AudioButton);
	DDX_Control(pDX, IDC_STATIC_USER1, m_User1);
	DDX_Control(pDX, IDC_STATIC_USER2, m_User2);
	DDX_Control(pDX, IDC_STATIC_USER3, m_User3);
	DDX_Control(pDX, IDC_STATIC_USER4, m_User4);
	DDX_Control(pDX, IDC_STATIC_USER5, m_User5);
	DDX_Control(pDX, IDC_STATIC_USER6, m_User6);
	DDX_Control(pDX, IDC_STATIC_USER7, m_User7);
}


BEGIN_MESSAGE_MAP(CSuperAudioChatroomDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SUPER_AUDIO_CHATROOM_LISTBRUSH, &CSuperAudioChatroomDlg::OnBnClickedButtonSuperAudioChatroomListbrush)
	ON_NOTIFY(NM_CLICK, IDC_LISTCONTROL_SUPER_AUDIO_CHATROOM_LIST, &CSuperAudioChatroomDlg::OnNMClickListcontrolSuperAudioChatroomList)
	ON_BN_CLICKED(IDC_BUTTON_CREATE_SUPER_AUDIO_CHATROOM, &CSuperAudioChatroomDlg::OnBnClickedButtonCreateSuperAudioChatroom)
	ON_BN_CLICKED(IDC_BUTTON_SUPER_AUDIO_CHATROOM_SEND_MSG, &CSuperAudioChatroomDlg::OnBnClickedButtonSuperAudioChatroomSendMsg)
	ON_BN_CLICKED(IDC_BUTTON_SEND_SUPER_AUDIO, &CSuperAudioChatroomDlg::OnBnClickedButtonSendSuperAudio)
	ON_WM_CLOSE()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CSuperAudioChatroomDlg 消息处理程序


void CSuperAudioChatroomDlg::OnBnClickedButtonSuperAudioChatroomListbrush()
{
	getSuperRoomList();
}

void CSuperAudioChatroomDlg::OnBnClickedButtonCreateSuperAudioChatroom()
{
	m_MsgList.ResetContent();
	m_SendMsg.SetSel(0, -1); // 选中所有字符
	m_SendMsg.ReplaceSel(_T(""));
	m_SuperAudioCharroomName.SetWindowText("");
	KillTimer(0);

	if (m_pSoundManager != NULL)
	{
		m_pSoundManager->stopSoundData();
	}
	if (m_pCurrentLive != NULL)
	{
		delete m_pCurrentLive;
		m_pCurrentLive = NULL;
	}

	CString strName = "";
	bool bPublic = false;

	XH_CHATROOM_TYPE chatRoomType = XH_CHATROOM_TYPE::XH_CHATROOM_TYPE_GLOBAL_PUBLIC;
	XH_LIVE_TYPE channelType = XH_LIVE_TYPE::XH_LIVE_TYPE_GLOBAL_PUBLIC;

	CCreateLiveDialog dlg(m_pUserManager);
	if (dlg.DoModal() == IDOK)
	{
		strName = dlg.m_strLiveName;
		bPublic = dlg.m_bPublic;
	}
	else
	{
		return;
	}
	if (m_pXHSuperRoomManager != NULL)
	{
		string strLiveId = m_pXHSuperRoomManager->createSuperRoom(strName.GetBuffer(0), chatRoomType, channelType);
		if (strLiveId != "")
		{
			string strInfo = "{\"id\":\"";
			strInfo += strLiveId;
			strInfo += "\",\"creator\":\"";
			strInfo += m_pUserManager->m_ServiceParam.m_strUserId;
			strInfo += "\",\"name\":\"";
			strInfo += strName;
			strInfo += "\"}";
			if (m_pConfig != NULL && m_pConfig->m_bAEventCenterEnable)
			{
				CInterfaceUrls::demoSaveToList(m_pUserManager->m_ServiceParam.m_strUserId, CHATROOM_LIST_TYPE_SUPER_ROOM, strLiveId, strInfo);
			}
			else
			{
				m_pXHSuperRoomManager->saveToList(m_pUserManager->m_ServiceParam.m_strUserId, CHATROOM_LIST_TYPE_SUPER_ROOM, strLiveId, strInfo);
			}

			bool bRet = m_pXHSuperRoomManager->joinSuperRoom(strLiveId);
			if (bRet)
			{
				if (m_pCurrentLive == NULL)
				{
					m_pCurrentLive = new CLiveProgram();
				}
				m_pCurrentLive->m_strId = strLiveId.c_str();
				m_pCurrentLive->m_strName = strName;
				m_pCurrentLive->m_strCreator = m_pUserManager->m_ServiceParam.m_strUserId.c_str();
				if (m_pSoundManager != NULL)
				{
					m_pSoundManager->startSoundData(false);
				}
				m_SuperAudioCharroomName.SetWindowText(m_pCurrentLive->m_strName);
				//onJoined(m_pCurrentLive->m_strId.GetBuffer(0), m_pUserManager->m_ServiceParam.m_strUserId);
				SetTimer(0, 3000, 0);
			}
			getSuperRoomList();
		}
		else
		{
			AfxMessageBox("创建超级对讲聊天室失败!");
		}
	}
}
/**
* 查询聊天室列表回调
*/
int CSuperAudioChatroomDlg::chatroomQueryAllListOK(list<ChatroomInfo>& listData)
{
	mVLivePrograms.clear();
	m_List.DeleteAllItems();

	list<ChatroomInfo>::iterator iter = listData.begin();
	int i = 0;
	for (; iter != listData.end(); iter++)
	{
		CLiveProgram liveProgram;
		liveProgram.m_strName = (char*)(*iter).m_strName.c_str();
		liveProgram.m_strId = (char*)(*iter).m_strRoomId.c_str();
		liveProgram.m_strCreator = (char*)(*iter).m_strCreaterId.c_str();
		mVLivePrograms.push_back(liveProgram);
	}
	int nRowIndex = 0;
	CString strStatus = "";
	for (int i = 0; i < (int)mVLivePrograms.size(); i++)
	{
		m_List.InsertItem(i, mVLivePrograms[i].m_strName);
		//m_liveList.AddString(mVLivePrograms[i].m_strName);
		m_List.SetItemText(i, LIVE_VIDEO_ID, mVLivePrograms[i].m_strId);

		m_List.SetItemText(i, LIVE_VIDEO_CREATER, mVLivePrograms[i].m_strCreator);
		if (mVLivePrograms[i].m_liveState)
		{
			strStatus = "正在直播";
		}
		else
		{
			strStatus = "直播未开始";
		}
		m_List.SetItemText(i, LIVE_VIDEO_STATUS, strStatus);
	}
	return 0;
}

void CSuperAudioChatroomDlg::OnNMClickListcontrolSuperAudioChatroomList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	KillTimer(0);
	m_MsgList.ResetContent();
	m_SendMsg.SetSel(0, -1); // 选中所有字符
	m_SendMsg.ReplaceSel(_T(""));
	m_SuperAudioCharroomName.SetWindowText("");

	if (m_pSoundManager != NULL)
	{
		m_pSoundManager->stopSoundData();
	}
	if (m_pCurrentLive != NULL)
	{
		delete m_pCurrentLive;
		m_pCurrentLive = NULL;
	}

	int nItem = -1;
	if (pNMItemActivate != NULL)
	{
		nItem = pNMItemActivate->iItem;
		if (nItem >= 0)
		{
			CString strId = m_List.GetItemText(nItem, LIVE_VIDEO_ID);
			CString strName = m_List.GetItemText(nItem, LIVE_VIDEO_NAME);
			CString strCreater = m_List.GetItemText(nItem, LIVE_VIDEO_CREATER);

			CLiveProgram* pLiveProgram = getLiveProgram(strId, strName, strCreater);
			CString strUserId = m_pUserManager->m_ServiceParam.m_strUserId.c_str();
			if (pLiveProgram != NULL)
			{
				//if (/*pLiveProgram->m_liveState == false && */pLiveProgram->m_strCreator != strUserId)
				//{
				//	AfxMessageBox("直播尚未开始");
				//	return;
				//}
				string strId = pLiveProgram->m_strId.GetBuffer(0);
				if (strId.length() == 32)
				{
					bool bRet = m_pXHSuperRoomManager->joinSuperRoom(strId);
					if (bRet)
					{
						if (m_pCurrentLive == NULL)
						{
							m_pCurrentLive = new CLiveProgram();
						}
						m_pCurrentLive->m_strId = pLiveProgram->m_strId;
						m_pCurrentLive->m_strName = pLiveProgram->m_strName;
						m_pCurrentLive->m_strCreator = pLiveProgram->m_strCreator;
						m_SuperAudioCharroomName.SetWindowText(m_pCurrentLive->m_strName);
						if (m_pSoundManager != NULL)
						{
							m_pSoundManager->startSoundData(false);
						}
						SetTimer(0, 3000, 0);
					}
					else
					{
						AfxMessageBox("观看直播失败");
					}
				}

				delete pLiveProgram;
				pLiveProgram = NULL;
			}

		}
	}
}

void CSuperAudioChatroomDlg::OnBnClickedButtonSuperAudioChatroomSendMsg()
{
	CString strMsg = "";
	m_SendMsg.GetWindowText(strMsg);

	if (strMsg == "")
	{
		return;
	}
	if (m_pXHSuperRoomManager != NULL && m_pCurrentLive != NULL)
	{
		CIMMessage* pIMMessage = m_pXHSuperRoomManager->sendMessage(strMsg.GetBuffer(0));
		CString strMsg = "";
		strMsg.Format("%s:%s", pIMMessage->m_strFromId.c_str(), pIMMessage->m_strContentData.c_str());
		m_MsgList.InsertString(m_MsgList.GetCount(), strMsg);
		delete pIMMessage;
		pIMMessage = NULL;
		m_SendMsg.SetSel(0, -1); // 选中所有字符
		m_SendMsg.ReplaceSel(_T(""));
	}
}


void CSuperAudioChatroomDlg::OnBnClickedButtonSendSuperAudio()
{
	
}

void CSuperAudioChatroomDlg::setConfig(CConfigManager* pConfig)
{
	m_pConfig = pConfig;
}

/*
* 获取音频聊天室列表
*/
void CSuperAudioChatroomDlg::getSuperRoomList()
{
	char strListType[10] = { 0 };
	sprintf_s(strListType, "%d,%d", CHATROOM_LIST_TYPE_SUPER_ROOM, CHATROOM_LIST_TYPE_SUPER_ROOM_PUSH);
	if (m_pConfig != NULL && m_pConfig->m_bAEventCenterEnable)
	{
		list<ChatroomInfo> listData;
		CInterfaceUrls::demoQueryList(strListType, listData);
		chatroomQueryAllListOK(listData);
	}
	else
	{
		XHSuperRoomManager::getSuperRoomList("", strListType);
	}
}

void CSuperAudioChatroomDlg::updateUpUser()
{
	vector<string>::iterator iter = m_UpUsers.begin();
	int nIndex = 0;
	for (; iter != m_UpUsers.end(); iter++)
	{
		m_UserIdArr[nIndex]->SetWindowText((*iter).c_str());
		nIndex++;
	}

	for (int i = nIndex; i < 7; i++)
	{
		m_UserIdArr[i]->SetWindowText("");
	}
}

CLiveProgram* CSuperAudioChatroomDlg::getLiveProgram(CString strId, CString strName, CString strCreator)
{
	CLiveProgram* pRet = NULL;
	for (int i = 0; i < (int)mVLivePrograms.size(); i++)
	{
		if (//mVLivePrograms[i].m_strId == strId &&
			mVLivePrograms[i].m_strName == strName)// &&
			//mVLivePrograms[i].m_strCreator == strCreator)
		{
			pRet = new CLiveProgram();
			pRet->m_strId = mVLivePrograms[i].m_strId;
			pRet->m_strName = mVLivePrograms[i].m_strName;
			pRet->m_strCreator = mVLivePrograms[i].m_strCreator;
			pRet->m_liveState = mVLivePrograms[i].m_liveState;
			break;
		}
	}
	return pRet;
}

/**
* 有新用户加入会议
* @param meetingID 会议ID
* @param userID 新加入者ID
*/
void CSuperAudioChatroomDlg::onJoined(string meetID, string userID)
{
	vector<string>::iterator iter = m_UpUsers.begin();
	bool bFind = false;
	for (; iter != m_UpUsers.end(); iter++)
	{
		if (*iter == userID)
		{
			bFind = true;
			break;
		}
	}
	if (!bFind)
	{
		m_UpUsers.push_back(userID);
		updateUpUser();
	}
}

/**
 * 有人离开会议
 * @param meetingID 会议ID
 * @param userID 离开者ID
 */
void CSuperAudioChatroomDlg::onLeft(string meetingID, string userID)
{
	vector<string>::iterator iter = m_UpUsers.begin();
	bool bFind = false;
	for (; iter != m_UpUsers.end(); iter++)
	{
		if (*iter == userID)
		{
			m_UpUsers.erase(iter);
			updateUpUser();
			break;
		}
	}
}

/**
 * 一些异常情况引起的出错，请在收到该回调后主动断开会议
 * @param id
 * @param error 错误信息
 */
void CSuperAudioChatroomDlg::onError(string id, string error)
{
	KillTimer(0);
	m_MsgList.ResetContent();
	m_SendMsg.SetSel(0, -1); // 选中所有字符
	m_SendMsg.ReplaceSel(_T(""));
	m_SuperAudioCharroomName.SetWindowText("");

	if (m_pSoundManager != NULL)
	{
		m_pSoundManager->stopSoundData();
	}
	if (m_pCurrentLive != NULL)
	{
		delete m_pCurrentLive;
		m_pCurrentLive = NULL;
	}
}


/**
 * 聊天室成员数变化
 * @param number 变化后的会议人数
 */
void CSuperAudioChatroomDlg::onMembersUpdated(int number)
{
	CString strVal = "";
	if (m_pCurrentLive != NULL)
	{
		strVal.Format("%s(%d)", m_pCurrentLive->m_strName, number);
	}
	m_SuperAudioCharroomName.SetWindowText(strVal);
}

/**
 * 自己被踢出聊天室
 */
void CSuperAudioChatroomDlg::onSelfKicked()
{
}

/**
 * 自己被禁言
 */
void CSuperAudioChatroomDlg::onSelfMuted(int seconds)
{
	CString strMsg = "";
	strMsg.Format("被禁言 %d 秒", seconds);
	AfxMessageBox(strMsg);
}

/**
 * 连麦者的连麦被强制停止
 * @param liveID 直播ID
 */
void CSuperAudioChatroomDlg::onCommandToStopPlay(string liveID)
{
	if (m_pSoundManager != NULL)
	{
		m_pSoundManager->stopSoundData();
	}
	if (m_pXHSuperRoomManager != NULL)
	{
		m_pXHSuperRoomManager->leaveSuperRoom();
	}
	if (m_pCurrentLive != NULL)
	{
		delete m_pCurrentLive;
		m_pCurrentLive = NULL;
	}
}

/**
 * 收到消息
 * @param message
 */
void CSuperAudioChatroomDlg::onReceivedMessage(CIMMessage* pMessage)
{
	CString strMsg = "";
	strMsg.Format("%s:%s", pMessage->m_strFromId.c_str(), pMessage->m_strContentData.c_str());
	m_MsgList.InsertString(m_MsgList.GetCount(), strMsg);
}

/**
 * 收到私信消息
 * @param message
 */
void CSuperAudioChatroomDlg::onReceivePrivateMessage(CIMMessage* pMessage)
{
	CString strMsg = "";
	strMsg.Format("%s:%s", pMessage->m_strFromId.c_str(), pMessage->m_strContentData.c_str());
	m_MsgList.AddString(strMsg);
}

int CSuperAudioChatroomDlg::getRealtimeData(string strUserId, uint8_t* data, int len)
{
	return 1;
}

/**
 * 收到视频数据
 * @param upId
 * @param w 宽度
 * @param h 高度
 * @param videoData 数据
 * @param videoDataLen 数据长度
 */
int CSuperAudioChatroomDlg::getVideoRaw(string strUserId, int w, int h, uint8_t* videoData, int videoDataLen)
{
	return 1;
}

void CSuperAudioChatroomDlg::getLocalSoundData(char* pData, int nLength)
{
	if (m_pXHSuperRoomManager != NULL)
	{
		m_pXHSuperRoomManager->insertAudioRaw((uint8_t*)pData, nLength);
	}
}

void CSuperAudioChatroomDlg::querySoundData(char** pData, int* nLength)
{
	if (m_pXHSuperRoomManager != NULL)
	{
		m_pXHSuperRoomManager->querySoundData((uint8_t**)pData, nLength);
	}
}

void CSuperAudioChatroomDlg::OnLButtonDownCallback()
{
	if (m_pXHSuperRoomManager != NULL)
	{
		bool bRet = m_pXHSuperRoomManager->pickUpMic();
		if (bRet)
		{
			if (m_pSoundManager != NULL && m_pCurrentLive != NULL)
			{
				m_pSoundManager->startGetSoundData();
			}
		}
		else
		{
			AfxMessageBox("发送语音失败");
		}
	}
	
}

void CSuperAudioChatroomDlg::OnLButtonUpCallback()
{
	if (m_pXHSuperRoomManager != NULL)
	{
		m_pXHSuperRoomManager->layDownMic();
	}

	if (m_pSoundManager != NULL && m_pCurrentLive != NULL)
	{
		m_pSoundManager->stopGetSoundData();
	}
}


BOOL CSuperAudioChatroomDlg::OnInitDialog()
{
	__super::OnInitDialog();

	m_AudioButton.setCallback(this);
	LONG lStyle;
	lStyle = GetWindowLong(m_List.m_hWnd, GWL_STYLE);
	lStyle &= ~LVS_TYPEMASK;
	lStyle |= LVS_REPORT;
	SetWindowLong(m_List.m_hWnd, GWL_STYLE, lStyle);

	DWORD dwStyleLiveList = m_List.GetExtendedStyle();
	dwStyleLiveList |= LVS_EX_FULLROWSELECT;                                        //选中某行使整行高亮(LVS_REPORT)
	dwStyleLiveList |= LVS_EX_GRIDLINES;                                            //网格线(LVS_REPORT)
	//dwStyle |= LVS_EX_CHECKBOXES;                                            //CheckBox
	m_List.SetExtendedStyle(dwStyleLiveList);

	m_List.InsertColumn(LIVE_VIDEO_ID, _T("ID"), LVCFMT_LEFT, 110);
	m_List.InsertColumn(LIVE_VIDEO_NAME, _T("Name"), LVCFMT_LEFT, 120);
	m_List.InsertColumn(LIVE_VIDEO_CREATER, _T("Creator"), LVCFMT_LEFT, 80);
	m_List.InsertColumn(LIVE_VIDEO_STATUS, _T("liveState"), LVCFMT_LEFT, 100);

	getSuperRoomList();

	m_UserIdArr[0] = &m_User1;
	m_UserIdArr[1] = &m_User2;
	m_UserIdArr[2] = &m_User3;
	m_UserIdArr[3] = &m_User4;
	m_UserIdArr[4] = &m_User5;
	m_UserIdArr[5] = &m_User6;
	m_UserIdArr[6] = &m_User7;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CSuperAudioChatroomDlg::OnClose()
{
	if (m_pSoundManager != NULL)
	{
		m_pSoundManager->stopSoundData();
	}
	__super::OnClose();
}


void CSuperAudioChatroomDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_pXHSuperRoomManager != NULL && m_pCurrentLive != NULL)
	{
		m_pXHSuperRoomManager->getOnlineNumber(m_pCurrentLive->m_strId.GetBuffer(0));
	}
	__super::OnTimer(nIDEvent);
}
