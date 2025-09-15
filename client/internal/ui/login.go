package ui

import (
	"fyne-im/internal/services"
	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/widget"
)

// LoginScreen 登录界面
type LoginScreen struct {
	app           fyne.App
	window        fyne.Window
	networkService *services.NetworkService
	onLogin       func()
}

// NewLoginScreen 创建一个新的登录界面
func NewLoginScreen(app fyne.App, window fyne.Window, networkService *services.NetworkService, onLogin func()) *LoginScreen {
	return &LoginScreen{
		app:           app,
		window:        window,
		networkService: networkService,
		onLogin:       onLogin,
	}
}

// Show 显示登录界面
func (l *LoginScreen) Show() {
	// 创建输入框
	userIDEntry := widget.NewEntry()
	userIDEntry.SetPlaceHolder("用户ID")
	
	passwordEntry := widget.NewPasswordEntry()
	passwordEntry.SetPlaceHolder("密码")
	
	// 创建登录按钮
	loginButton := widget.NewButton("登录", func() {
		userID := userIDEntry.Text
		password := passwordEntry.Text
		
		// 连接服务器
		if !l.networkService.IsConnected() {
			err := l.networkService.Connect()
			if err != nil {
				l.showError("连接服务器失败: " + err.Error())
				return
			}
		}
		
		// 登录
		success, err := l.networkService.Login(userID, password)
		if err != nil {
			l.showError("登录失败: " + err.Error())
			return
		}
		
		if success {
			// 登录成功，切换到主界面
			if l.onLogin != nil {
				l.onLogin()
			}
		} else {
			l.showError("用户名或密码错误")
		}
	})
	
	// 创建注册按钮
	registerButton := widget.NewButton("注册", func() {
		// 暂时不实现注册功能
		l.showInfo("注册功能暂未实现")
	})
	
	// 创建布局
	content := container.NewVBox(
		widget.NewLabel("IUIM统一即时通信系统"),
		userIDEntry,
		passwordEntry,
		loginButton,
		registerButton,
	)
	
	l.window.SetContent(content)
}

// 显示错误信息
func (l *LoginScreen) showError(message string) {
	dialog := widget.NewLabel(message)
	popup := widget.NewModalPopUp(dialog, l.window.Canvas())
	popup.Show()
}

// 显示信息
func (l *LoginScreen) showInfo(message string) {
	dialog := widget.NewLabel(message)
	popup := widget.NewModalPopUp(dialog, l.window.Canvas())
	popup.Show()
}