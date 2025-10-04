package ui

import (
	"fmt"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	//"fyne.io/fyne/v2/layout"
	"fyne.io/fyne/v2/widget"

	"fyne-im/network"
)

// ServiceBaseView 服务基础视图
type ServiceBaseView struct {
	container   *fyne.Container
	window      fyne.Window
	netManager  *network.NetworkManager
	userID      int
	serviceID   int
	serviceName string
	contentArea *fyne.Container // 内容区域，用于放置服务特定的组件
}

// NewServiceBaseView 创建新的服务基础视图
func NewServiceBaseView(window fyne.Window, netManager *network.NetworkManager, userID, serviceID int, serviceName string) *ServiceBaseView {
	view := &ServiceBaseView{
		window:      window,
		netManager:  netManager,
		userID:      userID,
		serviceID:   serviceID,
		serviceName: serviceName,
	}

	// 创建标题栏
	titleLabel := widget.NewLabel(fmt.Sprintf("%s 服务", serviceName))
	titleLabel.TextStyle = fyne.TextStyle{Bold: true}
	titleLabel.Alignment = fyne.TextAlignCenter

	// 创建返回按钮
	backButton := widget.NewButton("返回", func() {
		// 返回服务大厅
		serviceHallView := NewServiceHallView(window, netManager, userID)
		window.SetContent(serviceHallView.GetContainer())
	})

	// 创建标题栏布局
	titleBar := container.NewBorder(nil, nil, backButton, nil, titleLabel)

	// 创建内容区域
	view.contentArea = container.NewVBox()

	// 组装界面
	view.container = container.NewBorder(
		container.NewVBox(titleBar, widget.NewSeparator()),
		nil, nil, nil,
		view.contentArea,
	)

	return view
}

// GetContainer 返回视图容器
func (v *ServiceBaseView) GetContainer() fyne.CanvasObject {
	return v.container
}

// SetContent 设置内容区域
func (v *ServiceBaseView) SetContent(content fyne.CanvasObject) {
	v.contentArea.RemoveAll()
	v.contentArea.Add(content)
}

// CreateQQServiceView 创建QQ服务界面
func CreateQQServiceView(window fyne.Window, netManager *network.NetworkManager, userID, serviceID int) fyne.CanvasObject {
	baseView := NewServiceBaseView(window, netManager, userID, serviceID, "QQ")
	
	// 创建QQ特有的界面组件
	friendsLabel := widget.NewLabel("好友列表")
	friendsLabel.TextStyle = fyne.TextStyle{Bold: true}
	
	friendsList := widget.NewList(
		func() int { return 5 },
		func() fyne.CanvasObject { return widget.NewLabel("好友") },
		func(id widget.ListItemID, obj fyne.CanvasObject) {
			obj.(*widget.Label).SetText(fmt.Sprintf("好友 %d", id+1))
		},
	)
	
	groupsLabel := widget.NewLabel("群组列表")
	groupsLabel.TextStyle = fyne.TextStyle{Bold: true}
	
	groupsList := widget.NewList(
		func() int { return 3 },
		func() fyne.CanvasObject { return widget.NewLabel("群组") },
		func(id widget.ListItemID, obj fyne.CanvasObject) {
			obj.(*widget.Label).SetText(fmt.Sprintf("群组 %d", id+1))
		},
	)
	
	statusLabel := widget.NewLabel("在线状态: 在线")
	
	messageArea := container.NewBorder(
		widget.NewLabel("消息区域"),
		container.NewBorder(nil, nil, nil, widget.NewButton("发送", func() {}),
			widget.NewMultiLineEntry()),
		nil, nil,
		widget.NewLabel("这里将显示消息内容"),
	)
	
	// 创建左侧面板
	leftPanel := container.NewVBox(
		friendsLabel,
		container.NewVScroll(friendsList),
		groupsLabel,
		container.NewVScroll(groupsList),
		statusLabel,
	)
	
	// 创建主内容区域
	content := container.NewHSplit(
		leftPanel,
		messageArea,
	)
	content.Offset = 0.3
	
	baseView.SetContent(content)
	return baseView.GetContainer()
}

// CreateWeChatServiceView 创建微信服务界面
func CreateWeChatServiceView(window fyne.Window, netManager *network.NetworkManager, userID, serviceID int) fyne.CanvasObject {
	baseView := NewServiceBaseView(window, netManager, userID, serviceID, "微信")
	
	// 创建微信特有的界面组件
	tabs := container.NewAppTabs(
		container.NewTabItem("聊天", widget.NewLabel("聊天会话列表将显示在这里")),
		container.NewTabItem("通讯录", widget.NewLabel("通讯录将显示在这里")),
		container.NewTabItem("发现", container.NewVBox(
			widget.NewButton("朋友圈", func() {}),
			widget.NewButton("扫一扫", func() {}),
			widget.NewButton("小程序", func() {}),
		)),
		container.NewTabItem("我", widget.NewLabel("个人信息将显示在这里")),
	)
	
	baseView.SetContent(tabs)
	return baseView.GetContainer()
}

// CreateWeiboServiceView 创建微博服务界面
func CreateWeiboServiceView(window fyne.Window, netManager *network.NetworkManager, userID, serviceID int) fyne.CanvasObject {
	baseView := NewServiceBaseView(window, netManager, userID, serviceID, "微博")
	
	// 创建微博特有的界面组件
	feedLabel := widget.NewLabel("微博动态")
	feedLabel.TextStyle = fyne.TextStyle{Bold: true}
	
	feedList := widget.NewList(
		func() int { return 10 },
		func() fyne.CanvasObject { 
			return container.NewVBox(
				widget.NewLabel("用户名"),
				widget.NewLabel("微博内容..."),
				container.NewHBox(
					widget.NewButton("点赞", func() {}),
					widget.NewButton("评论", func() {}),
					widget.NewButton("转发", func() {}),
				),
			)
		},
		func(id widget.ListItemID, obj fyne.CanvasObject) {
			box := obj.(*fyne.Container)
			username := box.Objects[0].(*widget.Label)
			content := box.Objects[1].(*widget.Label)
			
			username.SetText(fmt.Sprintf("用户 %d", id+1))
			content.SetText(fmt.Sprintf("这是第 %d 条微博内容，展示了用户分享的信息...", id+1))
		},
	)
	
	// 创建右侧面板
	rightPanel := container.NewVBox(
		widget.NewCard("热搜榜", "", widget.NewLabel("热搜内容将显示在这里")),
		widget.NewCard("关注/粉丝", "", container.NewVBox(
			widget.NewButton("查看关注", func() {}),
			widget.NewButton("查看粉丝", func() {}),
		)),
		widget.NewButton("发布微博", func() {}),
	)
	
	// 创建主内容区域
	content := container.NewBorder(
		feedLabel,
		nil, nil, nil,
		container.NewHSplit(
			container.NewVScroll(feedList),
			rightPanel,
		),
	)
	
	baseView.SetContent(content)
	return baseView.GetContainer()
}

// CreateServiceView 根据服务ID创建对应的服务界面
func CreateServiceView(window fyne.Window, netManager *network.NetworkManager, userID, serviceID int, serviceName string) fyne.CanvasObject {
	switch serviceID {
	case 1: // QQ
		return CreateQQServiceView(window, netManager, userID, serviceID)
	case 2: // 微信
		return CreateWeChatServiceView(window, netManager, userID, serviceID)
	case 3: // 微博
		return CreateWeiboServiceView(window, netManager, userID, serviceID)
	default:
		// 默认返回一个基础视图
		baseView := NewServiceBaseView(window, netManager, userID, serviceID, serviceName)
		baseView.SetContent(widget.NewLabel(fmt.Sprintf("服务 %s (ID: %d) 的界面尚未实现", serviceName, serviceID)))
		return baseView.GetContainer()
	}
}