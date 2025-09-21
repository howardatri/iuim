package main

import (
	"fyne-im/internal/ui"
	"fyne-im/network"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
)

func main() {
	a := app.New()
	w := a.NewWindow("IUIM 客户端")
	w.Resize(fyne.NewSize(400, 500))

	// 创建网络管理器
	netManager := network.NewNetworkManager()

	// 定义界面切换回调函数
	var showLoginView, showRegisterView func()

	// 显示登录界面
	showLoginView = func() {
		loginView := ui.LoginView(w, netManager, func() {
			// 登录成功后的回调
			// 这里可以添加登录成功后的逻辑，如显示主界面等
		}, func() {
			// 点击"去注册"按钮的回调
			showRegisterView()
		})
		w.SetContent(loginView)
	}

	// 显示注册界面
	showRegisterView = func() {
		registerView := ui.RegisterView(w, netManager, func() {
			// 返回登录界面的回调
			showLoginView()
		})
		w.SetContent(registerView)
	}

	// 启动时显示登录界面
	showLoginView()

	w.ShowAndRun()
}
