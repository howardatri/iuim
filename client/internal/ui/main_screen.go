package ui

import (
	"fyne-im/internal/services"
	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/widget"
)

// MainScreen 主界面
type MainScreen struct {
	app            fyne.App
	window         fyne.Window
	networkService *services.NetworkService
	userID         string
	onLogout       func()
}

// NewMainScreen 创建一个新的主界面
func NewMainScreen(app fyne.App, window fyne.Window, networkService *services.NetworkService, onLogout func()) *MainScreen {
	return &MainScreen{
		app:            app,
		window:         window,
		networkService: networkService,
		onLogout:       onLogout,
	}
}

// Show 显示主界面
func (m *MainScreen) Show(userID string) {
	m.userID = userID
	
	// 创建标签页
	tabs := container.NewAppTabs(
		container.NewTabItem("好友", m.createFriendsTab()),
		container.NewTabItem("群组", m.createGroupsTab()),
		container.NewTabItem("设置", m.createSettingsTab()),
	)
	
	m.window.SetContent(tabs)
}

// 创建好友标签页
func (m *MainScreen) createFriendsTab() fyne.CanvasObject {
	// 简单实现，显示一个空列表
	list := widget.NewList(
		func() int {
			return 0
		},
		func() fyne.CanvasObject {
			return widget.NewLabel("好友项")
		},
		func(id widget.ListItemID, obj fyne.CanvasObject) {
			// 暂无好友数据
		},
	)
	
	return container.NewBorder(
		widget.NewLabel("好友列表"),
		widget.NewButton("添加好友", func() {
			// 暂不实现添加好友功能
		}),
		nil, nil,
		list,
	)
}

// 创建群组标签页
func (m *MainScreen) createGroupsTab() fyne.CanvasObject {
	// 简单实现，显示一个空列表
	list := widget.NewList(
		func() int {
			return 0
		},
		func() fyne.CanvasObject {
			return widget.NewLabel("群组项")
		},
		func(id widget.ListItemID, obj fyne.CanvasObject) {
			// 暂无群组数据
		},
	)
	
	return container.NewBorder(
		widget.NewLabel("群组列表"),
		widget.NewButton("创建群组", func() {
			// 暂不实现创建群组功能
		}),
		nil, nil,
		list,
	)
}

// 创建设置标签页
func (m *MainScreen) createSettingsTab() fyne.CanvasObject {
	logoutButton := widget.NewButton("退出登录", func() {
		// 登出
		success, err := m.networkService.Logout(m.userID)
		if err != nil || !success {
			// 登出失败，但仍然切换到登录界面
		}
		
		// 断开连接
		m.networkService.Disconnect()
		
		// 切换到登录界面
		if m.onLogout != nil {
			m.onLogout()
		}
	})
	
	return container.NewVBox(
		widget.NewLabel("设置"),
		logoutButton,
	)
}