local WeekBonus = require("WeekBonus")
local bonustable = {}

bonustable[WeekBonus.monday] = 
{
	id = 111,
	num = 100,
}
bonustable[WeekBonus.tuesday] = 
{
	id = 112,
	num = 200,
}
bonustable[WeekBonus.wednesday] = 
{
	id = 113,
	num = 10,
}
bonustable[WeekBonus.thursday] = 
{
	id = 114,
	num = 10,
}
bonustable[WeekBonus.friday] = 
{
	id = 115,
	num = 20,
}
bonustable[WeekBonus.saturday] = 
{
	id = 116,
	num = 3,
}
bonustable[WeekBonus.weekday] = 
{
	id = 117,
	num = 1,
}
return bonustable