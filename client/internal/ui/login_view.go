package ui

import (
	"fmt"

	"fyne-im/network"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/dialog"
	"fyne.io/fyne/v2/widget"
)

// LoginView 创建登录界面
func LoginView(window fyne.Window, netManager *network.NetworkManager, onLoginSuccess func(result map[string]interface{}), onRegister func()) fyne.CanvasObject {
	// 创建用户名输入框
	usernameEntry := widget.NewEntry()
	usernameEntry.SetPlaceHolder("请输入用户名")

	// 创建密码输入框
	passwordEntry := widget.NewPasswordEntry()
	passwordEntry.SetPlaceHolder("请输入密码")

	// 创建登录按钮
	loginButton := widget.NewButton("登录", func() {
		username := usernameEntry.Text
		password := passwordEntry.Text

		// 验证输入
		if username == "" || password == "" {
			dialog.ShowError(fmt.Errorf("用户名和密码不能为空"), window)
			return
		}

		// 调用网络管理器的Login方法
		result, err := netManager.Login(username, password)
		if err != nil {
			dialog.ShowError(fmt.Errorf("登录失败: %v", err), window)
			return
		}

		// 检查登录结果
		code, ok := result["code"].(float64)
		if !ok || code != 0 {
			message := "登录失败"
			if msg, ok := result["message"].(string); ok {
				message = msg
			}
			dialog.ShowError(fmt.Errorf(message), window)
			return
		}

		// 从 data -> user_info 的嵌套结构中解析用户ID
		userID := 0
		if data, ok := result["data"].(map[string]interface{}); ok {
			if userInfo, ok := data["user_info"].(map[string]interface{}); ok {
				if idFloat, ok := userInfo["user_id"].(float64); ok {
					userID = int(idFloat)
				}
			}
		}

		if userID == 0 {
			dialog.ShowError(fmt.Errorf("无法从登录响应中获取有效的用户ID，请检查服务端响应"), window)
			return
		}

		// 登录成功
		dialog.ShowInformation("登录成功", "欢迎回来！", window)

		// 调用登录成功回调
		if onLoginSuccess != nil {
			onLoginSuccess(result)
		}
	})

	// 创建"去注册"按钮
	registerButton := widget.NewButton("没有账号？去注册", func() {
		if onRegister != nil {
			onRegister()
		}
	})

	// 创建布局
	content := container.NewVBox(
		widget.NewLabel("IUIM 登录"),
		widget.NewLabel("用户名"),
		usernameEntry,
		widget.NewLabel("密码"),
		passwordEntry,
		loginButton,
		registerButton,
	)

	return content
}

// SetRegisterButtonAction 设置注册按钮的点击事件
func SetRegisterButtonAction(content fyne.CanvasObject, action func()) {
	// 查找注册按钮并设置点击事件
	for _, obj := range content.(*fyne.Container).Objects {
		if button, ok := obj.(*widget.Button); ok && button.Text == "没有账号？去注册" {
			button.OnTapped = action
			break
		}
	}
}
