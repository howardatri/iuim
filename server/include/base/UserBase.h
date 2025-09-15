#ifndef USER_BASE_H
#define USER_BASE_H

#include <string>
#include <vector>
using namespace std;

// 基础用户类
class UserBase {
protected:
    string id;          // 用户ID
    string nickname;    // 昵称
    string password;    // 密码
    string birthdate;   // 出生日期
    string regTime;     // 注册时间(T龄)
    string location;    // 所在地
    vector<string> friendList;  // 好友列表
    vector<string> groupList;   // 群组列表

public:
    // 构造与析构函数
    UserBase();
    UserBase(const string& id, const string& nickname, const string& password);
    virtual ~UserBase();
    
    // 基本信息操作接口
    virtual string GetID() const;
    virtual void SetNickname(const string& name);
    virtual string GetNickname() const;
    virtual bool CheckPassword(const string& pwd) const;
    
    // 好友管理接口
    virtual bool AddFriend(const string& friendID);
    virtual bool RemoveFriend(const string& friendID);
    virtual bool ModifyFriend(const string& friendID, const string& newNickname);
    virtual vector<string> QueryFriends() const;
    
    // 群组管理接口
    virtual bool JoinGroup(const string& groupID);
    virtual bool QuitGroup(const string& groupID);
    virtual vector<string> QueryGroups() const;
};

#endif // USER_BASE_H