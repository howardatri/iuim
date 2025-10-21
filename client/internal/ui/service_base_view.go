package ui

import (
	"fmt"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/dialog"

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

	// 创建好友列表组件
	friendList := NewFriendList(window, netManager, userID, serviceID)

	// 创建群组管理组件
	groupManager := NewGroupManager(window, netManager, userID, serviceID)
	groupList := NewGroupList(window, netManager, userID, serviceID)

	// 创建在线状态
	statusLabel := widget.NewLabel("在线状态: 在线")

	// 创建消息区域
	messageArea := container.NewBorder(
		widget.NewLabel("消息区域"),
		container.NewBorder(nil, nil, nil, widget.NewButton("发送", func() {}),
			widget.NewMultiLineEntry()),
		nil, nil,
		widget.NewLabel("这里将显示消息内容"),
	)

	// ==================== 第六阶段增强：QQ服务差异化集成 ====================
	// QQ服务：在群组项旁显示管理员标识，点击打开完整管理界面
	qqGroupHeader := container.NewHBox(
		widget.NewLabel("QQ群组"),
		widget.NewLabel("💡 点击群组可查看详情和管理功能"),
	)

	// 创建QQ群组选项卡，强调管理功能
	groupTabs := container.NewAppTabs(
		container.NewTabItem("我的QQ群 👑", groupList.GetContainer()),
		container.NewTabItem("群组管理", groupManager.GetContainer()),
	)

	// 创建左侧面板，集成好友管理和群组功能
	leftPanel := container.NewVBox(
		widget.NewLabel("QQ好友"),
		friendList.GetContainer(),
		widget.NewSeparator(),
		qqGroupHeader,
		groupTabs,
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

	// 创建好友列表组件
	friendList := NewFriendList(window, netManager, userID, serviceID)

	// 创建群组管理组件
	groupManager := NewGroupManager(window, netManager, userID, serviceID)
	groupList := NewGroupList(window, netManager, userID, serviceID)

	// ==================== 第六阶段增强：微信服务差异化集成 ====================
	// 微信服务：为群主显示特殊的管理入口，普通成员显示基本信息
	wechatGroupHeader := container.NewVBox(
		widget.NewLabel("微信群聊"),
		widget.NewLabel("💬 点击群聊查看详情，群主可进行管理"),
	)

	// 创建通讯录内容，包含好友和群组，强调角色差异
	contactsTabs := container.NewAppTabs(
		container.NewTabItem("微信好友", friendList.GetContainer()),
		container.NewTabItem("微信群聊 🏠", container.NewVBox(
			wechatGroupHeader,
			groupList.GetContainer(),
		)),
		container.NewTabItem("群组管理", groupManager.GetContainer()),
	)

	// 创建微信特有的界面组件
	tabs := container.NewAppTabs(
		container.NewTabItem("聊天", widget.NewLabel("聊天会话列表将显示在这里")),
		container.NewTabItem("通讯录", container.NewVBox(
			widget.NewLabel("微信通讯录"),
			contactsTabs,
		)),
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

	// 创建好友列表组件
	friendList := NewFriendList(window, netManager, userID, serviceID)

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

	// 创建群组管理组件
	groupManager := NewGroupManager(window, netManager, userID, serviceID)
	groupList := NewGroupList(window, netManager, userID, serviceID)

	// ==================== 第六阶段增强：微博服务差异化集成 ====================
	// 微博服务：根据社区角色显示不同的操作选项
	weiboGroupHeader := container.NewVBox(
		widget.NewLabel("微博社区"),
		widget.NewLabel("🌟 点击超话查看详情，管理员可进行社区管理"),
	)

	// 创建关注管理选项卡，强调社区角色
	followTabs := container.NewAppTabs(
		container.NewTabItem("我的关注", friendList.GetContainer()),
		container.NewTabItem("微博社区 🌟", container.NewVBox(
			weiboGroupHeader,
			groupList.GetContainer(),
		)),
		container.NewTabItem("社区管理", groupManager.GetContainer()),
	)

	// 创建右侧面板，集成好友管理和群组功能
	rightPanel := container.NewVBox(
		widget.NewCard("热搜榜", "", widget.NewLabel("热搜内容将显示在这里")),
		widget.NewCard("关注管理", "", followTabs),
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

// ==================== 第六阶段增强：统一导航入口和权限引导 ====================

// ServiceNavigationManager 服务导航管理器
type ServiceNavigationManager struct {
	openWindows map[string]*GroupDetailWindow // 管理打开的群组详情窗口
	netManager  *network.NetworkManager
	userID      int
}

// NewServiceNavigationManager 创建服务导航管理器
func NewServiceNavigationManager(netManager *network.NetworkManager, userID int) *ServiceNavigationManager {
	return &ServiceNavigationManager{
		openWindows: make(map[string]*GroupDetailWindow),
		netManager:  netManager,
		userID:      userID,
	}
}

// OpenGroupDetailWindow 统一的群组详情窗口打开入口
func (snm *ServiceNavigationManager) OpenGroupDetailWindow(groupID, serviceID int, parentWindow fyne.Window) {
	windowKey := fmt.Sprintf("group_%d_%d", groupID, serviceID)

	// 检查窗口是否已经打开
	if existingWindow, exists := snm.openWindows[windowKey]; exists {
		// 如果窗口已存在，将其置于前台
		existingWindow.RequestFocus()
		return
	}

	// 创建新的群组详情窗口
	detailWindow := NewGroupDetailWindow(snm.netManager, snm.userID, groupID, serviceID)

	// 设置窗口关闭回调，从管理器中移除
	detailWindow.SetCloseCallback(func() {
		delete(snm.openWindows, windowKey)
	})

	// 添加到管理器
	snm.openWindows[windowKey] = detailWindow

	// 显示窗口
	detailWindow.Show()
}

// CloseAllGroupWindows 关闭所有群组详情窗口
func (snm *ServiceNavigationManager) CloseAllGroupWindows() {
	for _, detailWindow := range snm.openWindows {
		detailWindow.Close()
	}
	snm.openWindows = make(map[string]*GroupDetailWindow)
}

// GetOpenWindowsCount 获取打开的窗口数量
func (snm *ServiceNavigationManager) GetOpenWindowsCount() int {
	return len(snm.openWindows)
}

// ShowPermissionGuidance 显示权限引导界面
func ShowPermissionGuidance(userRole int, groupName string, parentWindow fyne.Window) {
	var title, message string

	switch userRole {
	case 1: // 群主
		title = "群主权限"
		message = fmt.Sprintf("您是群组 '%s' 的群主 👑\n\n可用功能：\n• 群组设置管理\n• 成员角色管理\n• 群组类型变更\n• 所有管理功能", groupName)
	case 2: // 管理员
		title = "管理员权限"
		message = fmt.Sprintf("您是群组 '%s' 的管理员 🛡️\n\n可用功能：\n• 成员管理\n• 基础设置\n• 部分管理功能", groupName)
	case 3: // 普通成员
		title = "成员权限"
		message = fmt.Sprintf("您是群组 '%s' 的成员\n\n可用功能：\n• 查看群组信息\n• 查看成员列表\n• 参与群组活动", groupName)
	case 4: // 访客
		title = "访客权限"
		message = fmt.Sprintf("您当前是群组 '%s' 的访客\n\n可用功能：\n• 查看基本信息\n• 申请加入群组\n\n如需更多权限，请联系群主或管理员", groupName)
	default:
		title = "权限说明"
		message = fmt.Sprintf("无法确定您在群组 '%s' 中的权限\n\n请联系群主或管理员获取帮助", groupName)
	}

	dialog.ShowInformation(title, message, parentWindow)
}

// ShowServiceSpecificGuidance 显示服务特定的权限引导
func ShowServiceSpecificGuidance(serviceID int, userRole int, groupName string, parentWindow fyne.Window) {
	var servicePrefix string

	switch serviceID {
	case 1: // QQ
		servicePrefix = "QQ群"
	case 2: // 微信
		servicePrefix = "微信群"
	case 3: // 微博
		servicePrefix = "微博超话"
	default:
		servicePrefix = "群组"
	}

	title := fmt.Sprintf("%s权限说明", servicePrefix)

	var message string
	switch userRole {
	case 1: // 群主
		if serviceID == 1 { // QQ
			message = fmt.Sprintf("您是%s '%s' 的群主 👑\n\nQQ群主特权：\n• 完整的群管理权限\n• 群设置和公告管理\n• 成员踢出和禁言\n• 群等级和头衔管理", servicePrefix, groupName)
		} else if serviceID == 2 { // 微信
			message = fmt.Sprintf("您是%s '%s' 的群主 👑\n\n微信群主特权：\n• 群聊设置管理\n• 成员邀请和移除\n• 群公告发布\n• 群名称和头像修改", servicePrefix, groupName)
		} else if serviceID == 3 { // 微博
			message = fmt.Sprintf("您是%s '%s' 的主持人 👑\n\n超话主持人特权：\n• 超话设置管理\n• 内容审核和置顶\n• 成员管理\n• 活动组织", servicePrefix, groupName)
		}
	case 2: // 管理员
		if serviceID == 1 { // QQ
			message = fmt.Sprintf("您是%s '%s' 的管理员 🛡️\n\nQQ群管理员权限：\n• 成员管理\n• 消息管理\n• 部分群设置", servicePrefix, groupName)
		} else if serviceID == 2 { // 微信
			message = fmt.Sprintf("您是%s '%s' 的管理员 🛡️\n\n微信群管理员权限：\n• 成员邀请\n• 消息管理\n• 群公告协助", servicePrefix, groupName)
		} else if serviceID == 3 { // 微博
			message = fmt.Sprintf("您是%s '%s' 的版主 🛡️\n\n超话版主权限：\n• 内容审核\n• 成员管理\n• 活动协助", servicePrefix, groupName)
		}
	default:
		message = fmt.Sprintf("您在%s '%s' 中的权限有限\n\n如需申请管理权限，请联系群主", servicePrefix, groupName)
	}

	dialog.ShowInformation(title, message, parentWindow)
}
