package main

import (
	"fmt"
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
		loginView := ui.LoginView(w, netManager, func(result map[string]interface{}) {
			// 登录成功后的回调
			// 从嵌套结构中解析userID
			fmt.Println("test")
			//打印接收到的json
			println(result)
			if data, ok := result["data"].(map[string]interface{}); ok {
				if userInfo, ok := data["user_info"].(map[string]interface{}); ok {
					if userIDFloat, ok := userInfo["user_id"].(float64); ok {
						// 打印原始的float64值和它的类型，以供调试
						println("成功解析到user_id (类型: %T, 值: %f)\n", userIDFloat, userIDFloat)
						// 将float64安全地转换为int，以供程序后续使用
						userID := int(userIDFloat)
						println("转换为整数后的userID (类型: %T, 值: %d)\n", userID, userID)
						//打印userID
						println("登录成功，用户ID:", userID)
						// 创建并显示服务大厅视图
						serviceHallView := ui.NewServiceHallView(w, netManager, userID)
						w.SetContent(serviceHallView.GetContainer())
					}
				}
			}
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
