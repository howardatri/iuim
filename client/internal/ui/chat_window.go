package ui

import (
	"fmt"
	"log"
	"time"

	"fyne.io/fyne/v2"
	//"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/widget"

	"fyne-im/network"
)

// ChatWindow 聊天窗口结构体
type ChatWindow struct {
	window      fyne.Window
	netManager  *network.NetworkManager
	userID      int
	targetID    int
	serviceID   int
	targetName  string
	messages    []map[string]interface{} // 存储原始消息数据，便于调试
	messageList *widget.List
	inputEntry  *widget.Entry
	ticker      *time.Ticker
	lastMsgID   int // 用于轮询时判断新消息
}

// NewChatWindow 创建新的聊天窗口
func NewChatWindow( netManager *network.NetworkManager, userID, targetID, serviceID int, targetName string) *ChatWindow {
	// // 创建新的应用窗口
	// chatApp := app.New()
	// window := chatApp.NewWindow(fmt.Sprintf("与%s聊天", targetName))
	// 正确的做法：使用当前正在运行的App实例来创建新窗口
	window := fyne.CurrentApp().NewWindow(fmt.Sprintf("与 %s 的对话", targetName))
	window.Resize(fyne.NewSize(600, 500))

	cw := &ChatWindow{
		window:     window,
		netManager: netManager,
		userID:     userID,
		targetID:   targetID,
		serviceID:  serviceID,
		targetName: targetName,
		messages:   []map[string]interface{}{},
		lastMsgID:  0,
	}

	// 创建UI组件
	cw.setupUI()

	// 加载历史消息
	go cw.loadHistoryMessages()

	// 启动消息轮询
	cw.startAutoRefresh()

	// // 显示窗口 错误
	// window.Show()

	// 窗口关闭时停止轮询
	window.SetOnClosed(func() {
		if cw.ticker != nil {
			cw.ticker.Stop()
		}
	})

	return cw
}

// Show 显示聊天窗口 (新增一个辅助方法)
func (cw *ChatWindow) Show() {
	cw.window.Show()
}

// setupUI 设置用户界面
func (cw *ChatWindow) setupUI() {
	// 创建消息列表
	cw.messageList = widget.NewList(
		func() int {
			return len(cw.messages)
		},
		func() fyne.CanvasObject {
			// 创建消息项模板
			senderLabel := widget.NewLabel("发送者")
			senderLabel.TextStyle = fyne.TextStyle{Bold: true}
			
			contentLabel := widget.NewLabel("消息内容")
			contentLabel.Wrapping = fyne.TextWrapWord
			
			timeLabel := widget.NewLabel("时间")
			timeLabel.TextStyle = fyne.TextStyle{Italic: true}
			
			return container.NewVBox(
				senderLabel,
				contentLabel,
				timeLabel,
				widget.NewSeparator(),
			)
		},
		func(id widget.ListItemID, obj fyne.CanvasObject) {
			if id >= len(cw.messages) {
				return
			}
			
			message := cw.messages[id]
			container := obj.(*fyne.Container)
			
			senderLabel := container.Objects[0].(*widget.Label)
			contentLabel := container.Objects[1].(*widget.Label)
			timeLabel := container.Objects[2].(*widget.Label)
			
			// 获取发送者ID
			senderID := 0
			if sid, ok := message["sender_id"].(float64); ok {
				senderID = int(sid)
			}
			
			// 设置发送者显示
			if senderID == cw.userID {
				senderLabel.SetText("我")
				senderLabel.Alignment = fyne.TextAlignTrailing
				contentLabel.Alignment = fyne.TextAlignTrailing
				timeLabel.Alignment = fyne.TextAlignTrailing
			} else {
				senderLabel.SetText(cw.targetName)
				senderLabel.Alignment = fyne.TextAlignLeading
				contentLabel.Alignment = fyne.TextAlignLeading
				timeLabel.Alignment = fyne.TextAlignLeading
			}
			
			// 设置消息内容
			if content, ok := message["content"].(string); ok {
				contentLabel.SetText(content)
			}
			
			// // 设置时间
			// if timestamp, ok := message["timestamp"].(string); ok {
			// 	timeLabel.SetText(timestamp)
			// }
			// **【关键修改】**：将 "timestamp" 改为 "send_time"
			if sendTime, ok := message["send_time"].(string); ok {
				timeLabel.SetText(sendTime)
			} else {
                timeLabel.SetText("") // 如果没有时间戳，显示为空
            }
		},
	)

	// 创建输入框
	cw.inputEntry = widget.NewEntry()
	cw.inputEntry.SetPlaceHolder("输入消息...")
	cw.inputEntry.MultiLine = true
	cw.inputEntry.Resize(fyne.NewSize(400, 60))

	// 创建发送按钮
	sendButton := widget.NewButton("发送", func() {
		cw.sendMessage()
	})

	// 回车发送消息
	cw.inputEntry.OnSubmitted = func(text string) {
		cw.sendMessage()
	}

	// 创建输入区域
	inputContainer := container.NewBorder(
		nil, nil, nil, sendButton,
		cw.inputEntry,
	)

	// 创建主容器
	content := container.NewBorder(
		nil, inputContainer, nil, nil,
		container.NewScroll(cw.messageList),
	)

	cw.window.SetContent(content)

	//new
	// 设置窗口初始焦点到输入框
    cw.window.Canvas().Focus(cw.inputEntry)
}

// sendMessage 发送消息
func (cw *ChatWindow) sendMessage() {
	content := cw.inputEntry.Text
	if content == "" {
		return
	}

	// 清空输入框
	cw.inputEntry.SetText("")

	// 在goroutine中发送消息，避免阻塞UI
	go func() {
		log.Printf("ChatWindow: 准备发送消息 - userID: %d, targetID: %d, serviceID: %d, content: %s", 
			cw.userID, cw.targetID, cw.serviceID, content)

		result, err := cw.netManager.SendMessage(cw.userID, cw.targetID, cw.serviceID, content)
		if err != nil {
			log.Printf("ChatWindow: 发送消息失败 - error: %v", err)
			
			// 在UI中显示发送失败的消息
			fyne.Do(func() {
				failedMessage := map[string]interface{}{
					"sender_id": cw.userID,
					"content":   fmt.Sprintf("[发送失败] %s", content),
					"timestamp": time.Now().Format("2006-01-02 15:04:05"),
				}
				cw.messages = append(cw.messages, failedMessage)
				cw.messageList.Refresh()
				cw.scrollToBottom()
			})
			return
		}

		log.Printf("ChatWindow: 发送消息成功 - result: %v", result)

		// // 发送成功后立即刷新消息列表 错误
		// cw.refreshMessages()

		// 发送成功后, 直接调用 loadHistoryMessages 重新加载所有消息
		// 这比原来的 refreshMessages 更直接可靠
		cw.loadHistoryMessages()
	}()
}

// refreshMessages 刷新消息列表 (由定时器调用)
func (cw *ChatWindow) refreshMessages() {
	// 直接调用 loadHistoryMessages 即可，不再需要复杂的 lastMsgID 判断
	// 这可以确保任何时候都能获取到最新消息列表
	cw.loadHistoryMessages()
}

// loadHistoryMessages 加载历史消息
func (cw *ChatWindow) loadHistoryMessages() {
	log.Printf("ChatWindow: 加载历史消息 - userID: %d, targetID: %d, serviceID: %d",
		cw.userID, cw.targetID, cw.serviceID)

	// 为了调试，打印出从服务器收到的原始响应体
	result, err := cw.netManager.GetMessageHistory(cw.userID, cw.targetID, cw.serviceID, 1, 50) // 假设查询第一页，每页50条
	if err != nil {
		log.Printf("ChatWindow: 加载历史消息失败 - error: %v", err)
		return
	}

	log.Printf("ChatWindow: 历史消息原始响应 - result: %+v", result) // 加上这行日志，以后调试会很方便

	// 【关键修复】: 先解析外层的 "data" 对象
	if data, ok := result["data"].(map[string]interface{}); ok {
		// 然后从 "data" 对象中解析 "messages" 数组
		if messagesData, ok := data["messages"].([]interface{}); ok {
			fyne.Do(func() {
				// 反转消息，让最新的消息显示在最下面
				var reversedMessages []map[string]interface{}
				for i := len(messagesData) - 1; i >= 0; i-- {
					if msg, ok := messagesData[i].(map[string]interface{}); ok {
						reversedMessages = append(reversedMessages, msg)
					}
				}
				cw.messages = reversedMessages
				
				// 更新 lastMsgID
				if len(cw.messages) > 0 {
					lastMsg := cw.messages[len(cw.messages)-1]
					if msgID, ok := lastMsg["id"].(float64); ok {
						cw.lastMsgID = int(msgID)
					}
				}

				cw.messageList.Refresh()
				cw.scrollToBottom()
				log.Printf("成功加载并渲染 %d 条消息", len(cw.messages))
			})
		} else {
			log.Printf("ChatWindow: 解析失败, 'data' 对象中未找到 'messages' 数组")
		}
	} else {
		log.Printf("ChatWindow: 解析失败, 响应中未找到 'data' 对象")
	}
}

// startAutoRefresh 启动自动刷新
func (cw *ChatWindow) startAutoRefresh() {
	cw.ticker = time.NewTicker(5 * time.Second)
	go func() {
		for range cw.ticker.C {
			cw.refreshMessages()
		}
	}()
}

// refreshMessages 刷新消息列表
// func (cw *ChatWindow) refreshMessages() {
// 	log.Printf("ChatWindow: 刷新消息 - userID: %d, targetID: %d, serviceID: %d", 
// 		cw.userID, cw.targetID, cw.serviceID)

// 	result, err := cw.netManager.GetMessageHistory(cw.userID, cw.targetID, cw.serviceID)
// 	if err != nil {
// 		log.Printf("ChatWindow: 刷新消息失败 - error: %v", err)
// 		return
// 	}

// 	// 解析消息列表
// 	if messagesData, ok := result["messages"].([]interface{}); ok {
// 		newMessages := []map[string]interface{}{}
// 		hasNewMessage := false

// 		for _, msgData := range messagesData {
// 			if msg, ok := msgData.(map[string]interface{}); ok {
// 				// 检查是否是新消息
// 				if msgID, ok := msg["message_id"].(float64); ok {
// 					if int(msgID) > cw.lastMsgID {
// 						hasNewMessage = true
// 						cw.lastMsgID = int(msgID)
// 					}
// 				}
// 				newMessages = append(newMessages, msg)
// 			}
// 		}

// 		// 如果有新消息，更新UI
// 		if hasNewMessage {
// 			fyne.Do(func() {
// 				cw.messages = newMessages
// 				cw.messageList.Refresh()
// 				cw.scrollToBottom()
// 			})
// 			log.Printf("ChatWindow: 发现新消息，已更新UI")
// 		}
// 	}
// }

// scrollToBottom 滚动到底部
func (cw *ChatWindow) scrollToBottom() {
	if len(cw.messages) > 0 {
		cw.messageList.ScrollToBottom()
	}
}