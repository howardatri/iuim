package models

// User 表示系统中的用户
type User struct {
	ID       string
	Nickname string
	Password string
	Friends  []string
	Groups   []string
}

// NewUser 创建一个新用户
func NewUser(id, nickname, password string) *User {
	return &User{
		ID:       id,
		Nickname: nickname,
		Password: password,
		Friends:  make([]string, 0),
		Groups:   make([]string, 0),
	}
}

// AddFriend 添加好友
func (u *User) AddFriend(friendID string) bool {
	// 检查是否已经是好友
	for _, id := range u.Friends {
		if id == friendID {
			return false
		}
	}
	u.Friends = append(u.Friends, friendID)
	return true
}

// RemoveFriend 删除好友
func (u *User) RemoveFriend(friendID string) bool {
	for i, id := range u.Friends {
		if id == friendID {
			u.Friends = append(u.Friends[:i], u.Friends[i+1:]...)
			return true
		}
	}
	return false
}

// JoinGroup 加入群组
func (u *User) JoinGroup(groupID string) bool {
	// 检查是否已经在群组中
	for _, id := range u.Groups {
		if id == groupID {
			return false
		}
	}
	u.Groups = append(u.Groups, groupID)
	return true
}

// QuitGroup 退出群组
func (u *User) QuitGroup(groupID string) bool {
	for i, id := range u.Groups {
		if id == groupID {
			u.Groups = append(u.Groups[:i], u.Groups[i+1:]...)
			return true
		}
	}
	return false
}