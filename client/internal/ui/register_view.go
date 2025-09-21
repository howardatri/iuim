package ui

import (
	"fmt"

	"fyne-im/network"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/dialog"
	"fyne.io/fyne/v2/widget"
)

// RegisterView 创建注册界面
func RegisterView(window fyne.Window, netManager *network.NetworkManager, onBackToLogin func()) fyne.CanvasObject {
	// 创建用户名输入框
	usernameEntry := widget.NewEntry()
	usernameEntry.SetPlaceHolder("请输入用户名")

	// 创建密码输入框
	passwordEntry := widget.NewPasswordEntry()
	passwordEntry.SetPlaceHolder("请输入密码")

	// 创建确认密码输入框
	confirmPasswordEntry := widget.NewPasswordEntry()
	confirmPasswordEntry.SetPlaceHolder("请确认密码")

	// 创建注册按钮
	registerButton := widget.NewButton("注册", func() {
		username := usernameEntry.Text
		password := passwordEntry.Text
		confirmPassword := confirmPasswordEntry.Text

		// 验证输入
		if username == "" || password == "" {
			dialog.ShowError(fmt.Errorf("用户名和密码不能为空"), window)
			return
		}

		if password != confirmPassword {
			dialog.ShowError(fmt.Errorf("两次输入的密码不一致"), window)
			return
		}

		// 调用网络管理器的Register方法
		userInfo := map[string]string{
			"username": username,
			"password": password,
		}
		result, err := netManager.Register(userInfo)
		if err != nil {
			dialog.ShowError(fmt.Errorf("注册失败: %v", err), window)
			return
		}

		// 检查注册结果
		code, ok := result["code"].(float64)
		if !ok || code != 0 {
			message := "注册失败"
			if msg, ok := result["message"].(string); ok {
				message = msg
			}
			dialog.ShowError(fmt.Errorf(message), window)
			return
		}

		// 注册成功
		dialog.ShowInformation("注册成功", "请返回登录页面进行登录", window)

		// 调用返回登录页面的回调
		if onBackToLogin != nil {
			onBackToLogin()
		}
	})

	// 创建"返回登录"按钮
	backButton := widget.NewButton("返回登录", func() {
		if onBackToLogin != nil {
			onBackToLogin()
		}
	})

	// 创建布局
	content := container.NewVBox(
		widget.NewLabel("IUIM 注册"),
		widget.NewLabel("用户名"),
		usernameEntry,
		widget.NewLabel("密码"),
		passwordEntry,
		widget.NewLabel("确认密码"),
		confirmPasswordEntry,
		registerButton,
		backButton,
	)

	return content
}
