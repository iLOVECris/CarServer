local Errcode = 
{
	PlayerAccountNoExist = 194,--玩家账户不存在
	PlayerAccountParamErr = 195,--玩家账户参数有错误
	ModifyFaceIconErr = 196,--修改头像失败
	ModifyFaceIconOK = 197,--修改头像成功
	ModifyNameOk = 198,--修改名称成功
	ModifyNameErr = 199,--修改名字失败
	QueryErr = 200,--数据查询错误
	Freeze = 201,--玩家被封禁
	NotGuest = 202,--玩家不是游客
	UpgradeErr = 203,--升级账号失败
	GetBonusErr = 204,--获取奖励失败
}
return Errcode