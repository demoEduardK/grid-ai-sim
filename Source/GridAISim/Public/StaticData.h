#pragma once

/***/
UENUM(Blueprintable)
enum class ETeam : uint8
{
	NoTeam UMETA(DisplayName="NoTeam"),
	BlueTeam UMETA(DisplayName="BlueTeam"),
	RedTeam UMETA(DisplayName="RedTeam"),
	MAX UMETA(DisplayName="MaxTeams")
};

/***/
UENUM()
enum class EActorState : uint8
{
	Idle,
	Moving,
	Attacking,
	Dead,
	MAX
};