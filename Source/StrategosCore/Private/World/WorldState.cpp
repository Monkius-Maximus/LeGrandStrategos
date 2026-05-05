#include "World/WorldState.h"
#include "World/Nation.h"
#include "World/Province.h"
#include "World/Army.h"

UNation* UWorldState::GetNation(FName NationId) const
{
	const TObjectPtr<UNation>* Found = Nations.Find(NationId);
	return Found ? Found->Get() : nullptr;
}

UProvince* UWorldState::GetProvince(FName ProvinceId) const
{
	const TObjectPtr<UProvince>* Found = Provinces.Find(ProvinceId);
	return Found ? Found->Get() : nullptr;
}

UArmy* UWorldState::GetArmy(FName ArmyId) const
{
	const TObjectPtr<UArmy>* Found = Armies.Find(ArmyId);
	return Found ? Found->Get() : nullptr;
}

UNation* UWorldState::AddNation(FName Id)
{
	if (Nations.Contains(Id))
	{
		return Nations[Id].Get();
	}

	UNation* NewNation = NewObject<UNation>(this);
	NewNation->Id = Id;
	Nations.Add(Id, NewNation);
	return NewNation;
}

UProvince* UWorldState::AddProvince(FName Id)
{
	if (Provinces.Contains(Id))
	{
		return Provinces[Id].Get();
	}

	UProvince* NewProvince = NewObject<UProvince>(this);
	NewProvince->Id = Id;
	Provinces.Add(Id, NewProvince);
	return NewProvince;
}

UArmy* UWorldState::AddArmy(FName Id)
{
	if (Armies.Contains(Id))
	{
		return Armies[Id].Get();
	}

	UArmy* NewArmy = NewObject<UArmy>(this);
	NewArmy->Id = Id;
	Armies.Add(Id, NewArmy);
	return NewArmy;
}
