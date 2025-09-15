package main

import (
	"fyne-im/internal/services"
	"fyne-im/internal/ui"
	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
)

func main() {
	// 创建应用
	a := app.New()
	window := a.NewWindow("IUIM统一即时通信系统")
	window.Resize(fyne.NewSize(800, 600))
	
	// 创建网络服务
	networkService := services.NewNetworkService("127.0.0.1", 8888)
	
	// 创建登录界面和主界面
	var loginScreen *ui.LoginScreen
	var mainScreen *ui.MainScreen
	
	// 定义界面切换函数
	showLogin := func() {
		loginScreen = ui.NewLoginScreen(a, window, networkService, func() {
			mainScreen.Show("test") // 暂时使用固定用户ID
		})
		loginScreen.Show()
	}
	
	mainScreen = ui.NewMainScreen(a, window, networkService, showLogin)
	
	// 显示登录界面
	showLogin()
	
	// 运行应用
	window.ShowAndRun()
}