package main

import (
	"encoding/json"
	"fmt"
	"net"
	"strings"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/widget"
)

const (
	gatewayHost = "localhost"
	gatewayPort = "8233"
)

// 响应结构
type Response struct {
	Code    int    `json:"code"`
	Message string `json:"message"`
	Service string `json:"service"`
}

func main() {
	a := app.New()
	w := a.NewWindow("IUIM 客户端")
	w.Resize(fyne.NewSize(400, 300))

	// 创建状态标签
	statusLabel := widget.NewLabel("未连接")

	// 创建连接测试按钮
	testButton := widget.NewButton("连接测试", func() {
		statusLabel.SetText("正在连接...")
		
		// 发送健康检查命令
		response, err := sendCommand("health_check")
		if err != nil {
			statusLabel.SetText(fmt.Sprintf("连接失败: %s", err.Error()))
			return
		}
		
		// 解析响应
		var resp Response
		if err := json.Unmarshal([]byte(response), &resp); err != nil {
			statusLabel.SetText(fmt.Sprintf("解析响应失败: %s", err.Error()))
			return
		}
		
		// 显示响应结果
		if resp.Code == 0 {
			statusLabel.SetText(fmt.Sprintf("连接成功: %s", resp.Message))
		} else {
			statusLabel.SetText(fmt.Sprintf("连接失败: %s", resp.Message))
		}
	})

	// 创建布局
	content := container.NewVBox(
		widget.NewLabel("IUIM 客户端"),
		testButton,
		statusLabel,
	)

	w.SetContent(content)
	w.ShowAndRun()
}

// 发送命令到网关
func sendCommand(cmd string) (string, error) {
	// 连接到网关
	conn, err := net.Dial("tcp", gatewayHost+":"+gatewayPort)
	if err != nil {
		return "", fmt.Errorf("连接网关失败: %w", err)
	}
	defer conn.Close()

	// 发送命令
	_, err = fmt.Fprintf(conn, "%s\n", cmd)
	if err != nil {
		return "", fmt.Errorf("发送命令失败: %w", err)
	}

	// 读取响应
	buf := make([]byte, 1024)
	n, err := conn.Read(buf)
	if err != nil {
		return "", fmt.Errorf("读取响应失败: %w", err)
	}

	// 返回响应
	return strings.TrimSpace(string(buf[:n])), nil
}