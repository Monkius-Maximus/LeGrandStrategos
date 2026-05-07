#pragma once

#include "CoreMinimal.h"
#include "Economy/PopStratum.h"
#include "Economy/TaxLevel.h"
#include "Treasury.generated.h"

/**
 * FTreasury — Saldo nacional + breakdown de income/expense + dívida.
 *
 * Atualizado mensalmente pelo UEconomySubsystem. Quando Balance fica
 * negativo após PayExpenses, um empréstimo automático é registrado em
 * DebtBalance (com InterestRate anual).
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FTreasury
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Treasury")
	float Balance = 0.f;

	// Breakdown da última atualização mensal — útil para HUD.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Treasury|Last Month")
	float Income_Taxes = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Treasury|Last Month")
	float Income_Tariffs = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Treasury|Last Month")
	float Income_StateProfits = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Treasury|Last Month")
	float Expense_Maintenance = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Treasury|Last Month")
	float Expense_ArmyUpkeep = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Treasury|Last Month")
	float Expense_AdminCost = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Treasury|Last Month")
	float Expense_DebtInterest = 0.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Treasury|Debt")
	float DebtBalance = 0.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Treasury|Debt", meta = (ClampMin = "0.0"))
	float AnnualInterestRate = 0.05f;

	/** Tax level por estrato (default Medium). */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Treasury|Policy")
	TMap<EPopStratum, ETaxLevel> TaxLevelByStratum;

	float GetMonthlyIncome() const
	{
		return Income_Taxes + Income_Tariffs + Income_StateProfits;
	}

	float GetMonthlyExpenses() const
	{
		return Expense_Maintenance + Expense_ArmyUpkeep + Expense_AdminCost + Expense_DebtInterest;
	}
};
